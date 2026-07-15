#include "Web_Transport_Datagram_Node.hpp"
#include <Mlib/AGameHelper/Emscripten/AAnimation_Frame_Worker.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary.hpp>
#include <Mlib/Remote/Network_Transmission_Status.hpp>
#include <Mlib/Remote/Remote_Socket.hpp>
#include <cstddef>
#include <emscripten/bind.h>
#include <emscripten/em_js.h>
#include <emscripten/val.h>
#include <future>

using namespace Mlib;
using emscripten::EM_VAL;

enum class JsStatusCode: int {
    SUCCESS = 0,
    FAILURE = 1
};

// Expose the Enum to the global Emscripten Module
EMSCRIPTEN_BINDINGS(js_status_code_bindings) {
    emscripten::enum_<JsStatusCode>("JsStatusCode")
        .value("SUCCESS", JsStatusCode::SUCCESS)
        .value("FAILURE", JsStatusCode::FAILURE);
}

extern "C" EMSCRIPTEN_KEEPALIVE void resolve_promise(void* promise_ptr, int js_status_code) {
    auto* prop = static_cast<std::promise<int>*>(promise_ptr);
    prop->set_value(js_status_code);
}

// EM_JS functions need to be wrapped in a namespace or be global to ensure
// they are correctly linked.
EM_JS(int, createWebTransportSocket,
    (const char* serverUrlPtr, std::ptrdiff_t serverUrlLen,
     int maxStoredReceivedMessages,
     const uint8_t* certHashPtr, std::ptrdiff_t certHashLen,
     const char* remoteSecretPtr, std::ptrdiff_t remoteSecretLen),
{
    const utf8Decoder = new TextDecoder('utf-8');
    const serverUrl = utf8Decoder.decode(HEAPU8.slice(Number(serverUrlPtr), Number(serverUrlPtr) + Number(serverUrlLen)));
    console.log(`Connecting to ${serverUrl}...`);

    // NOTE: In production, configure valid hashes or allow self-signed for testing
    const options = {};
    if (Number(certHashPtr) !== 0) {
        console.log("Using cert hash");
        if (Number(certHashLen) !== 32) {
            console.error("Certificate hash does not have 32 bytes");
            return -1;
        }
        options["serverCertificateHashes"] = [
            {
                "algorithm": "sha-256",
                "value": HEAPU8.slice(Number(certHashPtr), Number(certHashPtr) + Number(certHashLen))
            }
        ];
        // console.log(`Sending options: ${JSON.stringify(options)}`);
        console.log("Sending cert hash");
    } else {
        console.log("Not using cert hash");
    }
    const queryList = [];
    const remoteSecret = utf8Decoder.decode(HEAPU8.slice(Number(remoteSecretPtr), Number(remoteSecretPtr) + Number(remoteSecretLen)));
    if (remoteSecret.length > 0) {
        queryList.push("remote_secret=" + remoteSecret);
    }

    if (globalThis.webTransportSockets === undefined) {
        globalThis.webTransportSockets = {};
        globalThis.webTransportHandles = {
            handle: 0,
            generate() { return this.handle++; }
        };
        globalThis.webTransportPermanent = {};
    }
    const handle = globalThis.webTransportHandles.generate();
    globalThis.webTransportPermanent[handle] = {
        packetQueue: []
    };
    function createWebTransport() {
        const queryString = (queryList.length === 0)
            ? ""
            : ("?" + queryList.join("&"));
        const socket = new globalThis["WebTransport"](serverUrl + queryString, options);
        socket["_isClosed"] = false;
        globalThis.webTransportSockets[handle] = socket;
    }
    async function connectTransport() {
        try {
            const socket = globalThis.webTransportSockets[handle];
            const permanent = globalThis.webTransportPermanent[handle];
            let closedDueToTimeout = false;
            // Background reader
            (async () => {
                let reader = null;
                try {
                    console.log("Wait for WebTransport to become ready");
                    await socket.ready;
                    console.log("WebTransport ready");
                    reader = socket["datagrams"]["readable"].getReader();
                    while (true) {
                        const timeoutId = setTimeout(() => {
                            closedDueToTimeout = true;
                            if (reader) {
                                reader.cancel().catch(() => {});
                            }
                            socket.close();
                        }, 7000);
                        try {
                            const { value, done } = await reader.read();
                            if (done) {
                                break;
                            }
                            permanent.packetQueue.push(value);
                            if (permanent.packetQueue.length > maxStoredReceivedMessages) {
                                console.error(`Packet queue longer than ${maxStoredReceivedMessages}, removing oldest entry`);
                                permanent.packetQueue.shift();
                            }
                        } finally {
                            clearTimeout(timeoutId);
                        }
                    }
                } catch (e) {
                    console.error("Reader error:", e);
                } finally {
                    if (reader) {
                        reader.releaseLock();
                    }
                    socket["_isClosed"] = true;
                }
            })();
            try {
                await socket.closed;
                if (closedDueToTimeout) {
                    closedDueToTimeout = false;
                    throw new Error("Connection closed due to timeout.");
                } else {
                    console.log("Connection closed gracefully.");
                }
            } catch (error) {
                console.error(`Connection lost due to error/timeout: ${error}`);
                globalThis.webTransportSockets[handle] = null; 
                console.error("Attempting to reconnect in 5 seconds...");
                setTimeout(() => {
                    createWebTransport(); 
                    connectTransport();
                }, 5000);
            }
        } catch (err) {
            console.error("WebTransport handshake failed:", err);
        }
    }
    createWebTransport();
    connectTransport();
    return handle;
});

EM_JS(void, closeWebTransportSocket, (int transportHandle), {
    try {
        const socket = globalThis.webTransportSockets[transportHandle];
        if (socket !== null) {
            socket.close();
            console.log("WebTransport socket closed.");
        } else {
            console.log("WebTransport socket is null.");
        }
    } catch (error) {
        console.error("Could not close WebTransport socket:", error);
    }
});

// Marked as ASYNC because we await writer.write
EM_JS(bool, sendUsingWebTransportSocket, (int transportHandle, const uint8_t* dataPtr, std::ptrdiff_t dataLength, void* promise_ptr), {
    // Grab raw bytes directly out of the Wasm memory heap
    const dataArray = HEAPU8.slice(Number(dataPtr), Number(dataPtr) + Number(dataLength));

    try {
        const socket = globalThis.webTransportSockets[transportHandle];
        if (socket === null) {
            return false;
        }
        const writer = socket["datagrams"]["writable"].getWriter();
        (async () => {
            try {
                await writer.write(dataArray);
            } catch (error) {
                console.error("Failed to write data:", error);
                _resolve_promise(promise_ptr, Module["JsStatusCode"]["FAILURE"]["value"]);
                return;
            } finally {
                writer.releaseLock();
            }
            _resolve_promise(promise_ptr, Module["JsStatusCode"]["SUCCESS"]["value"]);
        })();
        return true;
    } catch (error) {
        console.error("Failed to write data:", error);
        return false;
    }
});

EM_JS(int, tryReadFromWebTransportSocket, (int transportHandle, uint8_t* outBufferPtr, int maxCapacity), {
    const socket = globalThis.webTransportSockets[transportHandle];
    if (socket === null) {
        return -3;
    }
    const permanent = globalThis.webTransportPermanent[transportHandle];
    if (permanent.packetQueue.length === 0) {
        if (socket["_isClosed"]) {
            return -1;
        } else {
            return 0;
        }
    }
    const value = permanent.packetQueue.shift();
    const bytesToCopy = Math.min(value.length, maxCapacity);
    if (value.length > maxCapacity) {
        console.warn(`Packet truncated: ${value.length} > ${maxCapacity}`);
    }
    HEAPU8.set(value.subarray(0, bytesToCopy), Number(outBufferPtr));
    return value.length; // Return original length to indicate size, even if truncated
});

WebTransportDatagramNode::WebTransportDatagramNode(
    ConstructorKey,
    const RemoteSocket& socket,
    std::vector<std::byte> cert_hash,
    std::string remote_secret)
    : remote_socket_{socket}
    , socket_handle_{-1}
    , cert_hash_(std::move(cert_hash))
    , remote_secret_(std::move(remote_secret))
{}

std::shared_ptr<WebTransportDatagramNode> WebTransportDatagramNode::create(
    const RemoteSocket& socket,
    std::vector<std::byte> cert_hash,
    std::string remote_secret)
{
    return std::make_shared<WebTransportDatagramNode>(ConstructorKey{}, socket, std::move(cert_hash), std::move(remote_secret));
}

WebTransportDatagramNode::~WebTransportDatagramNode() {
    on_destroy.clear();
    if (socket_handle_ != -1) {
        execute_in_main_thread([&](){
            closeWebTransportSocket(socket_handle_);
        });
    }
}

void WebTransportDatagramNode::start_receive_thread(uint32_t max_stored_received_messages) {
    if (socket_handle_ != -1) {
        throw std::runtime_error("Receive thread already started");
    }
    std::string url = "https://" + remote_socket_.hostname + ':' + std::to_string(remote_socket_.port);
    execute_in_main_thread([&](){
        socket_handle_ = createWebTransportSocket(
            url.data(), integral_cast<std::ptrdiff_t>(url.length()),
            integral_cast<int>(max_stored_received_messages),
            cert_hash_.empty() ? nullptr : (const uint8_t*)cert_hash_.data(),
            cert_hash_.empty() ? 0 : integral_cast<std::ptrdiff_t>(cert_hash_.size()),
            remote_secret_.data(), integral_cast<std::ptrdiff_t>(remote_secret_.length()));
    });
    if (socket_handle_ == -1) {
        throw std::runtime_error("Could not create WebTransport socket");
    }
}

void WebTransportDatagramNode::bind() {
    throw std::runtime_error("WebTransportDatagramNode::bind not available");
}

void WebTransportDatagramNode::send(std::istream& istr) {
    if (socket_handle_ == -1) {
        throw std::runtime_error("WebTransportDatagramNode::send on a null socket");
    }
    auto data = read_all_vector(istr, "WebTransport message", IoVerbosity::SILENT);
    // if (!send_handle_.isNull()) {
    //     // Note that WebTransport sends are often fire-and-forget.
    //     send_handle_.await();
    // }
    std::promise<JsStatusCode> done;
    bool success;
    execute_in_main_thread([&](){
        success = sendUsingWebTransportSocket(
            socket_handle_,
            (const uint8_t*)data.data(),
            integral_cast<std::ptrdiff_t>(data.size()),
            &done);
    });
    if (!success) {
        lwarn() << "Could not send WebTransport message";
    } else {
        // linfo() << "Send: Waiting for WebTransport status code";
        auto status_code = done.get_future().get();
        // linfo() << "Send: Received WebTransport status code " + std::to_string((int)status_code);
        if (status_code != JsStatusCode::SUCCESS) {
            lwarn() << "Could not send using WebTransport";
        }
    }
}

std::shared_ptr<ISendSocket> WebTransportDatagramNode::try_receive(
    std::ostream& ostr,
    NetworkTransmissionStatus& transmission_status)
{
    if (socket_handle_ == -1) {
        throw std::runtime_error("WebTransportDatagramNode::try_receive on a null socket");
    }
    std::vector<std::byte> receive_buffer(65535);
    int bytesReceived;
    execute_in_main_thread([&](){
        bytesReceived = tryReadFromWebTransportSocket(
            socket_handle_,
            (uint8_t*)receive_buffer.data(),
            integral_cast<int>(receive_buffer.size()));
    });
    if ((bytesReceived == -1) || (bytesReceived == -3)){
        transmission_status = NetworkTransmissionStatus::DISCONNECTED;
        linfo() << "WebTransport disconnect";
        return nullptr;
    }
    if (bytesReceived < 0) {
        transmission_status = NetworkTransmissionStatus::ERROR;
        throw std::runtime_error("Error reading WebTransport data: " + std::to_string(bytesReceived));
    }
    if (bytesReceived == 0) {
        transmission_status = NetworkTransmissionStatus::SUCCESS;
        return nullptr;
    }
    if (bytesReceived > receive_buffer.size()) {
        transmission_status = NetworkTransmissionStatus::ERROR;
        throw std::runtime_error("Received more bytes than the buffer size");
    }
    receive_buffer.resize(integral_cast<size_t>(bytesReceived));
    write_iterable(ostr, receive_buffer, "WebTransport message");
    transmission_status = NetworkTransmissionStatus::SUCCESS;
    return shared_from_this();
}
