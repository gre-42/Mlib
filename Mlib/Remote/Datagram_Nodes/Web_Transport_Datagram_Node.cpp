#include "Web_Transport_Datagram_Node.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary.hpp>
#include <Mlib/Remote/Remote_Socket.hpp>
#include <emscripten/em_js.h>
#include <emscripten/val.h>

using namespace Mlib;
using emscripten::EM_VAL;

// EM_JS functions need to be wrapped in a namespace or be global to ensure 
// they are correctly linked.
EM_ASYNC_JS(EM_VAL, createWebTransportSocket, (const char* serverUrlPtr, int maxStoredReceivedMessages), {
    const serverUrl = UTF8ToString(serverUrlPtr);
    console.log(`Connecting to ${serverUrl}...`);
    
    // NOTE: In production, configure valid hashes or allow self-signed for testing
    const devOptions = {
        serverCertificateHashes: [
            {
                algorithm: "sha-256",
                value: new Uint8Array([
                    0x1a, 0x2b, 0x3c, 0x4d, 0x5e, 0x6f, 0x7a, 0x8b,
                    0x9c, 0x0d, 0x1e, 0x2f, 0x3a, 0x4b, 0x5c, 0x6d,
                    0x7e, 0x8f, 0x90, 0xa1, 0xb2, 0xc3, 0xd4, 0xe5,
                    0xf6, 0xa7, 0xb8, 0xc9, 0xd0, 0xe1, 0xf2, 0xa3
                ])
            }
        ]
    };

    try {
        const transport = new globalThis["WebTransport"](serverUrl, devOptions);
        await transport.ready;
        console.log("WebTransport ready");

        transport["_packetQueue"] = [];
        transport["_isClosed"] = false;

        // Background reader
        (async () => {
            try {
                const reader = transport["datagrams"]["readable"].getReader();
                while (true) {
                    const { value, done } = await reader.read();
                    if (done) {
                        break;
                    }
                    transport["_packetQueue"].push(value);
                    if (transport["_packetQueue"].length > maxStoredReceivedMessages) {
                        console.error(`Packet queue longer than ${maxStoredReceivedMessages}, removing oldest entry`);
                        transport["_packetQueue"].shift();
                    }
                }
            } catch (e) {
                console.error("Reader error:", e);
            } finally {
                transport["_isClosed"] = true;
            }
        })();

        return Emval.toHandle(transport);
    } catch (err) {
        console.error("WebTransport handshake failed:", err);
        return 0;
    }
});

EM_JS(void, closeWebTransportSocket, (EM_VAL transportHandle), {
    const transport = Emval.toValue(transportHandle);
    if (transport) {
        transport.close();
        console.log("WebTransport socket closed.");
    }
});

// Marked as ASYNC because we await writer.write
EM_ASYNC_JS(bool, sendUsingWebTransportSocket, (EM_VAL transportHandle, const uint8_t* dataPtr, int dataLength), {
    const transport = Emval.toValue(transportHandle);

    // Grab raw bytes directly out of the Wasm memory heap
    const dataArray = HEAPU8["subarray"](dataPtr, dataPtr + dataLength);
    
    try {
        const writer = transport["datagrams"]["writable"].getWriter();
        // writer.write() returns a Promise, we await it
        await writer.write(dataArray);
        writer.releaseLock();
        return true;
    } catch (error) {
        console.error("Failed to write data:", error);
        return false;
    } finally {
        transport.delete();
    }
});

EM_JS(int, tryReadFromWebTransportSocket, (EM_VAL transportHandle, uint8_t* outBufferPtr, int maxCapacity), {
    const transport = Emval.toValue(transportHandle);
    
    if (!transport) return -3;
    if (transport["_isClosed"] && transport["_packetQueue"].length === 0) return -1;
    if (transport["_packetQueue"].length === 0) return 0;

    const value = transport["_packetQueue"].shift();

    const bytesToCopy = Math.min(value.length, maxCapacity);

    if (value.length > maxCapacity) {
        console.warn(`Packet truncated: ${value.length} > ${maxCapacity}`);
    }

    // Direct, ultra-fast memory copy into C++ preallocated memory
    HEAPU8.set(value.subarray(0, bytesToCopy), outBufferPtr);

    return value.length; // Return original length to indicate size, even if truncated
});

WebTransportDatagramNode::WebTransportDatagramNode(
    const RemoteSocket& socket)
    : remote_socket_{ socket }
    , socket_{ emscripten::val::null() }
{}

WebTransportDatagramNode::~WebTransportDatagramNode() {
    on_destroy.clear();
    if (!socket_.isNull()) {
        closeWebTransportSocket(socket_.as_handle());
    }
}

void WebTransportDatagramNode::start_receive_thread(size_t max_stored_received_messages) {
    if (!socket_.isNull()) {
        throw std::runtime_error("Receive thread already started");
    }
    
    std::string url = "https://" + remote_socket_.hostname + ':' + std::to_string(remote_socket_.port);
    socket_ = emscripten::val::take_ownership(createWebTransportSocket(
        url.c_str(), integral_cast<int>(max_stored_received_messages)));
    
    // Note: Since createWebTransportSocket returns a Promise (via EM_VAL), 
    // you might need to handle the promise resolution before using it 
    // in subsequent calls, depending on how `take_ownership` handles it.
}

void WebTransportDatagramNode::bind() {
    throw std::runtime_error("WebTransportDatagramNode::bind not available");
}

void WebTransportDatagramNode::shutdown() {
    if (!socket_.isNull()) {
        closeWebTransportSocket(socket_.as_handle());
        socket_ = emscripten::val::null();
    }
}

void WebTransportDatagramNode::send(std::istream& istr) {
    if (socket_.isNull()) {
        throw std::runtime_error("WebTransportDatagramNode::send on a null socket");
    }
    auto data = read_all_vector(istr, "WebTransport message", IoVerbosity::SILENT);
    // if (!send_handle_.isNull()) {
    //     // Note that WebTransport sends are often fire-and-forget.
    //     send_handle_.await();
    // }
    auto success = sendUsingWebTransportSocket(
        socket_.as_handle(),
        (const uint8_t*)data.data(),
        integral_cast<int>(data.size()));
    if (!success) {
        lwarn() << "Could not send WebTransport message";
    }
}

std::shared_ptr<ISendSocket> WebTransportDatagramNode::try_receive(std::ostream& ostr)
{
    if (socket_.isNull()) {
        throw std::runtime_error("WebTransportDatagramNode::try_receive on a null socket");
    }
    std::vector<std::byte> receive_buffer(65535);
    int bytesReceived = tryReadFromWebTransportSocket(
        socket_.as_handle(),
        (uint8_t*)receive_buffer.data(),
        integral_cast<int>(receive_buffer.size()));
    if (bytesReceived < 0) {
        throw std::runtime_error("Error reading WebTransport data: " + std::to_string(bytesReceived));
    }
    if (bytesReceived == 0) {
        return nullptr;
    }
    if (bytesReceived > receive_buffer.size()) {
        throw std::runtime_error("Received more bytes than the buffer size");
    }
    receive_buffer.resize(integral_cast<size_t>(bytesReceived));
    write_iterable(ostr, receive_buffer, "WebTransport message");
    return shared_from_this();
}
