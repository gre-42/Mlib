#include <Mlib/Io/Binary.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Remote/Distributed_System.hpp>
#include <Mlib/Remote/Incremental_Objects/Communicator_Proxy.hpp>
#include <Mlib/Remote/Incremental_Objects/Communicator_Proxy_Factory.hpp>
#include <Mlib/Remote/Sockets/Udp_Node.hpp>
#include <cstdint>

using namespace Mlib;

enum class ObjectType: uint32_t {
    INT32,
    STRING
};

class SharedInteger: public ISharedObject {
public:
    explicit SharedInteger(int32_t i)
    : value_{ i }
    {}
    explicit SharedInteger(std::istream& istr) {
        read(istr);
    }
    virtual void read(std::istream& istr) override {
        value_ = read_binary<int32_t>(istr, "int32", IoVerbosity::SILENT);
    }
    virtual void write(std::ostream& ostr, ObjectCompression compression) override {
        write_binary(ostr, ObjectType::INT32, "ObjectType::INT32");
        write_binary(ostr, value_, "int32");
    }
private:
    int32_t value_;
};

class SharedString: public ISharedObject {
public:
    explicit SharedString(std::string s)
    : value_{ std::move(s) }
    {}
    explicit SharedString(std::istream& istr) {
        read(istr);
    }
    virtual void read(std::istream& istr) override {
        auto len = read_binary<uint32_t>(istr, "len", IoVerbosity::SILENT);
        value_ = read_string(istr, len, "string", IoVerbosity::SILENT);
    }
    virtual void write(std::ostream& ostr, ObjectCompression compression) override {
        write_binary(ostr, ObjectType::STRING, "ObjectType::STRING");
        write_binary(ostr, integral_cast<uint32_t>(value_.length()), "len");
        write_iterable(ostr, value_, "string");
    }
private:
    std::string value_;
};

class SharedObjectFactory: public ISharedObjectFactory {
public:
    SharedObjectFactory()
        : object_pool_{ InObjectPoolDestructor::CLEAR }
    {}
    virtual DanglingBaseClassRef<ISharedObject> create_shared_object(std::istream& istr) override {
        auto t = read_binary<ObjectType>(istr, "object type", IoVerbosity::SILENT);
        switch (t) {
        case ObjectType::INT32:
            return { object_pool_.create<SharedInteger>(CURRENT_SOURCE_LOCATION, istr), CURRENT_SOURCE_LOCATION };
        case ObjectType::STRING:
            return { object_pool_.create<SharedString>(CURRENT_SOURCE_LOCATION, istr), CURRENT_SOURCE_LOCATION };
        }
        THROW_OR_ABORT("Unknown object type: " + std::to_string((uint32_t)t));
    }
private:
    ObjectPool object_pool_;
};

void test_remote() {
    UdpNode server{"127.0.0.1", 1542};
    server.bind();
    server.start_receive_thread(100);
    UdpNode client{"127.0.0.1", 1542};
    client.start_receive_thread(100);

    linfo() << "server: " << &server << ", client: " << &client;

    SharedString s{ "Hello world" };

    SharedObjectFactory shared_object_factory;
    CommunicatorProxyFactory communicator_proxy_factory{
        {shared_object_factory, CURRENT_SOURCE_LOCATION} };

    DistributedSystem server_sys{
        {communicator_proxy_factory, CURRENT_SOURCE_LOCATION},
        42};
    DistributedSystem client_sys{
        {communicator_proxy_factory, CURRENT_SOURCE_LOCATION},
        43};

    client_sys.add_handshake_socket({client, CURRENT_SOURCE_LOCATION});
    
    // server_sys.add_communicator_proxy(
    //     communicator_proxy_factory.create_communicator_proxy({server, CURRENT_SOURCE_LOCATION}),
    //     TransmissionType::UNICAST);

    size_t cycle = 0;
    auto send_and_receive = [&](){
        if (cycle == 0) {
            linfo() << std::setw(2) << cycle++ << ": server: " << server_sys << " - client: " << client_sys;
        }
        server_sys.send_and_receive(TransmissionType::UNICAST); std::this_thread::sleep_for(std::chrono::milliseconds(200));
        linfo() << std::setw(2) << cycle++ << ": server: " << server_sys << " - client: " << client_sys;
        client_sys.send_and_receive(TransmissionType::UNICAST); std::this_thread::sleep_for(std::chrono::milliseconds(200));
        linfo() << std::setw(2) << cycle++ << ": server: " << server_sys << " - client: " << client_sys;
    };
    for (size_t i = 0; i < 3; ++i) {
        send_and_receive();
    }
    server_sys.add_receive_socket({server, CURRENT_SOURCE_LOCATION});
    client_sys.add_receive_socket({client, CURRENT_SOURCE_LOCATION});
    client_sys.send_and_receive(TransmissionType::HANDSHAKE);
    for (size_t i = 0; i < 3; ++i) {
        send_and_receive();
    }
    server_sys.add_object({s, CURRENT_SOURCE_LOCATION});
    for (size_t i = 0; i < 3; ++i) {
        send_and_receive();
    }
}

int main(int argc, char** argv) {
    try {
        test_remote();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
