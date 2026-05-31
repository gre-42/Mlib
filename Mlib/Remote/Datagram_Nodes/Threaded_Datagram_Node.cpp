#include "Threaded_Datagram_Node.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Env.hpp>
#include <Mlib/Os/Io/Binary.hpp>
#include <Mlib/Remote/Remote_Socket.hpp>
#include <Mlib/Remote/Sockets/IDatagram_Socket.hpp>
#include <Mlib/Threads/Termination_Manager.cpp>
#include <mutex>
#include <stdexcept>

using namespace Mlib;

ThreadedDatagramNode::ThreadedDatagramNode(
    std::shared_ptr<IDatagramSocket> socket)
    : socket_{ std::move(socket) }
{}

void ThreadedDatagramNode::start_receive_thread(size_t max_stored_received_messages) {
    if (receive_thread_.joinable()) {
        throw std::runtime_error("UDP receive-thread already started");
    }
    receive_thread_ = std::jthread{[&, max_stored_received_messages](){
        std::vector<std::byte> receive_buffer(1024 * 1024);
        while (!receive_thread_.get_stop_token().stop_requested() && !unhandled_exceptions_occured()) {
            try {
                std::error_code ec;
                std::shared_ptr<IDatagramSocket> reply_socket;
                auto len = socket_->receive(receive_buffer, reply_socket, ec);
                if (getenv_default_bool("NET_DEBUG", false)) {
                    linfo() << this << " receive_from. Error: " << (int)(bool)ec << ", Length: " << len;
                }
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
                        std::make_unique<ThreadedDatagramNode>(reply_socket));
                }
            } catch (...) {
                lerr() << "Unhandled exception in ThreadedDatagramNode";
                add_unhandled_exception(std::current_exception());
            }
        }
    }};
}

ThreadedDatagramNode::~ThreadedDatagramNode() {
    on_destroy.clear();
    if (receive_thread_.joinable()) {
        // linfo() << "---------------- shutdown --------------";
        {
            std::error_code ec;
            socket_->shutdown(ec);
            socket_->close();
        }
        receive_thread_.request_stop();
        receive_thread_.join();
        messages_received_.clear();
    }
}

void ThreadedDatagramNode::bind() {
    socket_->bind();
}

void ThreadedDatagramNode::send(std::istream& istr) {
    std::error_code ec;
    istr.seekg(0, std::ios::end);
    auto len = integral_cast<size_t>(istr.tellg() - std::streampos(0));
    istr.seekg(0);
    std::vector<std::byte> data(len);
    read_vector(istr, data, "send buffer", IoVerbosity::SILENT);
    auto sent = socket_->send(data, ec);
    if (getenv_default_bool("NET_DEBUG", false)) {
        linfo() << this << " send_to. Error: " << (int)(bool)ec << ", Length: " << sent << " / " << data.size();
    }
    if (ec) {
        throw std::runtime_error("UDP send failed: \"" + ec.message() + '"');
    }
    if (sent != len) {
        throw std::runtime_error((std::stringstream() << "Bytes sent: " << sent << ". Expected: " << len).str());
    }
}

std::shared_ptr<ISendSocket> ThreadedDatagramNode::try_receive(std::ostream& ostr) {
    std::scoped_lock lock{ message_mutex_ };
    // linfo() << this << " contains " << messages_received_.size() << " messages";
    if (messages_received_.empty()) {
        return nullptr;
    }
    std::list<ReceivedMessage> lmessage;
    lmessage.splice(lmessage.end(), messages_received_, messages_received_.begin());
    auto& message = lmessage.front();
    write_iterable(ostr, message.message, "UDP message");
    return std::move(message.reply_socket);
}
