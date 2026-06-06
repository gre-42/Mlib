#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Remote/Communicator_Proxies.hpp>
#include <Mlib/Remote/Datagram_Nodes/Datagram_Node_Factory.hpp>
#include <Mlib/Remote/Datagram_Nodes/IDatagram_Node.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Communicator_Proxy.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Communicator_Proxy_Factory.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Remote_Objects.hpp>
#include <Mlib/Remote/Incremental_Objects/Proxy_Tasks.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>
#include <Mlib/Remote/Remote_Socket.hpp>
#include <cstdint>

using namespace Mlib;

enum class ObjectType: uint32_t {
    INT32,
    STRING
};

class SharedInteger final: public IIncrementalObject {
public:
    explicit SharedInteger(int32_t i)
        : value_{ i }
    {}
    explicit SharedInteger(
        BinaryBitwiseWordsReader& reader,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader)
    {
        read(reader, remote_object_id, proxy_tasks, transmitted_fields, transmission_history_reader);
    }
    virtual ~SharedInteger() override {
        on_destroy.clear();
    }
    virtual std::string name() const override {
        return "int";
    }
    virtual void read(
        BinaryBitwiseWordsReader& reader,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader) override
    {
        if (any(transmitted_fields & ~(TransmittedFields::SITE_ID | TransmittedFields::END)))
        {
            throw std::runtime_error("Unexpected transmitted fields");
        }
        value_ = reader.read_binary<int32_t>("int32");
    }
    virtual void write(
        BinaryBitwiseWordsWriter& writer,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        KnownFields known_fields,
        TransmissionHistoryWriter& transmission_history_writer) override
    {
        transmission_history_writer.write_remote_object_id(writer, remote_object_id, TransmittedFields::END);
        writer.write_binary(ObjectType::INT32, "ObjectType::INT32");
        writer.write_binary(value_, "int32");
    }
private:
    int32_t value_;
};

class SharedString final: public IIncrementalObject {
public:
    explicit SharedString(std::string s)
        : value_{ std::move(s) }
    {}
    explicit SharedString(
        BinaryBitwiseWordsReader& reader,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader)
    {
        read(reader, remote_object_id, proxy_tasks, transmitted_fields, transmission_history_reader);
    }
    virtual ~SharedString() override {
        on_destroy.clear();
    }
    virtual std::string name() const override {
        return "string";
    }
    virtual void read(
        BinaryBitwiseWordsReader& reader,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader) override
    {
        value_ = reader.read_string<uint32_t>("string");
    }
    virtual void write(
        BinaryBitwiseWordsWriter& writer,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        KnownFields known_fields,
        TransmissionHistoryWriter& transmission_history_writer) override
    {
        transmission_history_writer.write_remote_object_id(writer, remote_object_id, TransmittedFields::END);
        writer.write_binary(ObjectType::STRING, "ObjectType::STRING");
        writer.write_string<uint32_t>(value_, "string");
    }
private:
    std::string value_;
};

class RemoteObjectFactory final: public IIncrementalObjectFactory {
public:
    RemoteObjectFactory()
        : object_pool_{ InObjectPoolDestructor::CLEAR }
    {}
    virtual ~RemoteObjectFactory() override {
        on_destroy.clear();
    }
    virtual DanglingBaseClassPtr<IIncrementalObject> try_create_shared_object(
        BinaryBitwiseWordsReader& reader,
        const RemoteObjectId& id,
        ProxyTasks proxy_tasks,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader) override
    {
        auto t = reader.read_binary<ObjectType>("object type");
        switch (t) {
        case ObjectType::INT32:
            return { object_pool_.create<SharedInteger>(CURRENT_SOURCE_LOCATION, reader, id, proxy_tasks, transmitted_fields, transmission_history_reader), CURRENT_SOURCE_LOCATION };
        case ObjectType::STRING:
            return { object_pool_.create<SharedString>(CURRENT_SOURCE_LOCATION, reader, id, proxy_tasks, transmitted_fields, transmission_history_reader), CURRENT_SOURCE_LOCATION };
        }
        throw std::runtime_error("Unknown object type: " + std::to_string((uint32_t)t));
    }
private:
    ObjectPool object_pool_;
};

void test_remote() {
    boost::asio::io_context ioc;
    auto server = DatagramNodeFactory::create_udp(ioc, {"127.0.0.1", 1542});
    server->bind();
    server->start_receive_thread(100);
    auto client = DatagramNodeFactory::create_udp(ioc, {"127.0.0.1", 1542});
    client->start_receive_thread(100);

    linfo() << "server: " << &server << ", client: " << &client;

    SharedString s{ "Hello world" };
    LocalSceneLevel local_scene_level;
    std::function<void()> on_schedule_load_scene;
    std::function<void()> on_update_time_of_day;
    SceneLevelSelector server_scene_level{local_scene_level, on_schedule_load_scene, on_update_time_of_day};
    SceneLevelSelector client_scene_level{local_scene_level, on_schedule_load_scene, on_update_time_of_day};

    RemoteObjectFactory shared_object_factory;
    IncrementalRemoteObjects server_objects{ 42, {server_scene_level, CURRENT_SOURCE_LOCATION} };
    IncrementalRemoteObjects client_objects{ 43, {client_scene_level, CURRENT_SOURCE_LOCATION} };
    IncrementalCommunicatorProxyFactory server_communicator_proxy_factory{
        {shared_object_factory, CURRENT_SOURCE_LOCATION},
        {server_objects, CURRENT_SOURCE_LOCATION},
        IoVerbosity::SILENT,
        ProxyTasks::SEND_LOCAL | ProxyTasks::SEND_REMOTE };
    IncrementalCommunicatorProxyFactory client_communicator_proxy_factory{
        {shared_object_factory, CURRENT_SOURCE_LOCATION},
        {client_objects, CURRENT_SOURCE_LOCATION},
        IoVerbosity::SILENT,
        ProxyTasks::SEND_LOCAL | ProxyTasks::SEND_REMOTE };

    CommunicatorProxies server_sys{
        {server_communicator_proxy_factory, CURRENT_SOURCE_LOCATION},
        42};
    CommunicatorProxies client_sys{
        {client_communicator_proxy_factory, CURRENT_SOURCE_LOCATION},
        43};

    client_sys.add_handshake_socket(client);
    
    // server_sys.add_communicator_proxy(
    //     communicator_proxy_factory.create_communicator_proxy({server, CURRENT_SOURCE_LOCATION}),
    //     TransmissionType::UNICAST);

    size_t cycle = 0;
    auto print = [&](){
        linfo() << std::setw(2) << cycle++ <<
            ": server: " << server_sys << ' ' << server_objects <<
            " - client: " << client_sys << ' ' << client_objects;
    };
    auto send_and_receive = [&](){
        print();
        server_sys.send_and_receive(TransmissionType::UNICAST); std::this_thread::sleep_for(std::chrono::milliseconds(200));
        print();
        client_sys.send_and_receive(TransmissionType::UNICAST); std::this_thread::sleep_for(std::chrono::milliseconds(200));
        print();
    };
    for (size_t i = 0; i < 3; ++i) {
        send_and_receive();
    }
    server_sys.add_receive_socket({*server, CURRENT_SOURCE_LOCATION});
    client_sys.add_receive_socket({*client, CURRENT_SOURCE_LOCATION});
    client_sys.send_and_receive(TransmissionType::HANDSHAKE);
    for (size_t i = 0; i < 3; ++i) {
        send_and_receive();
    }
    server_objects.add_local_object({s, CURRENT_SOURCE_LOCATION}, RemoteObjectVisibility::PUBLIC);
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
