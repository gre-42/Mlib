#include "Udp_Node.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Remote/Sockets/Asio.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using boost::asio::ip::udp;
using boost::asio::ip::address;
using namespace Mlib;

UdpNode::UdpNode(
    const std::string& ip_address,
    uint16_t port)
    : io_context_{ std::make_shared<boost::asio::io_context>() }
    , socket_{ std::make_shared<boost::asio::ip::udp::socket>(*io_context_) }
    , endpoint_{ address{boost::asio::ip::make_address_v4(ip_address)}, port }
    , send_socket_pool_{ InObjectPoolDestructor::CLEAR }
{
    socket_->open(udp::v4());
}

UdpNode::UdpNode(
    std::shared_ptr<boost::asio::io_context> io_context,
    std::shared_ptr<boost::asio::ip::udp::socket> socket,
    boost::asio::ip::udp::endpoint endpoint,
    size_t max_stored_received_messages)
    : io_context_{ io_context }
    , socket_{ socket }
    , endpoint_{ endpoint }
    , send_socket_pool_{ InObjectPoolDestructor::CLEAR }
{}

void UdpNode::start_receive_thread(size_t max_stored_received_messages) {
    if (receive_thread_.joinable()) {
        THROW_OR_ABORT("UDP receive-thread already started");
    }
    receive_thread_ = std::jthread{[&, max_stored_received_messages](){
        std::vector<std::byte> receive_buffer(1024 * 1024);
        while (!receive_thread_.get_stop_token().stop_requested()) {
            boost::system::error_code ec;
            boost::asio::ip::udp::endpoint endpoint2;
            auto len = socket_->receive_from(
                boost::asio::buffer(receive_buffer),
                endpoint2,
                0,
                ec);
            // linfo() << this << " receive_from. Error: " << (int)(bool)ec << ", Length: " << len;
            if (ec) {
                linfo() << "receive_from failed: " << ec.message();
                continue;
            }
            // for (size_t i = 0; i < len; ++i) {
            //     print_char((char)receive_buffer[i]);
            // }
            {
                std::scoped_lock lock{ message_mutex_ };
                if (messages_received_.size() >= max_stored_received_messages) {
                    lwarn() << "Message buffer overflow, discarding oldest message";
                    messages_received_.pop_front();
                }
                messages_received_.emplace_back(
                    std::vector<std::byte>(receive_buffer.data(), receive_buffer.data() + len),
                    DanglingBaseClassRef<ISendSocket>(
                        send_socket_pool_.create<UdpNode>(
                            CURRENT_SOURCE_LOCATION,
                            io_context_,
                            socket_,
                            endpoint2,
                            max_stored_received_messages),
                        CURRENT_SOURCE_LOCATION));
            }
        }
    }};
}

UdpNode::~UdpNode() {
    if (receive_thread_.joinable()) {
        // linfo() << "---------------- shutdown --------------";
        shutdown();
        receive_thread_.request_stop();
        receive_thread_.join();
        messages_received_.clear();
        send_socket_pool_.clear();
    }
}

void UdpNode::bind() {
    socket_->bind(endpoint_);
}

void UdpNode::shutdown() {
    boost::system::error_code ec;
    socket_->shutdown(boost::asio::socket_base::shutdown_both, ec);
    socket_->close();
}

void UdpNode::send(std::istream& istr) {
    boost::system::error_code ec;
    istr.seekg(0, std::ios::end);
    auto len = integral_cast<size_t>(istr.tellg() - std::streampos(0));
    istr.seekg(0);
    std::vector<std::byte> data(len);
    read_vector(istr, data, "send buffer", IoVerbosity::SILENT);
    auto sent = socket_->send_to(boost::asio::buffer(data), endpoint_, 0, ec);
    if (ec) {
        THROW_OR_ABORT("UDP send failed: \"" + ec.message() + '"');
    }
    if (sent != len) {
        THROW_OR_ABORT((std::stringstream() << "Bytes sent: " << sent << ". Expected: " << len).str());
    }
}

DanglingBaseClassPtr<ISendSocket> UdpNode::try_receive(std::ostream& ostr) {
    std::scoped_lock lock{ message_mutex_ };
    // linfo() << this << " contains " << messages_received_.size() << " messages";
    if (messages_received_.empty()) {
        return nullptr;
    }
    std::list<ReceivedMessage> lmessage;
    lmessage.splice(lmessage.end(), messages_received_, messages_received_.begin());
    const auto& message = lmessage.front();
    write_iterable(ostr, message.message, "UDP message");
    return message.reply_socket.ptr();
}
