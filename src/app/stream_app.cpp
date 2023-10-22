#include "app/stream_app.h"
#include "proto_res/proto_cmd.pb.h"
#include "proto_res/proto_example.pb.h"
#include "proto_res/proto_message_head.pb.h"
#include <tubekit-log/logger.h>
#include <string>
#include <set>
#include "thread/mutex.h"
#include "utility/singleton.h"
#include "connection/connection_mgr.h"
#include "socket/socket.h"
#include "socket/socket_handler.h"
#include <vector>
#include <endian.h>

using tubekit::app::stream_app;
using tubekit::connection::connection_mgr;
using tubekit::connection::stream_connection;
using tubekit::socket::socket;
using tubekit::socket::socket_handler;
using tubekit::utility::singleton;

namespace tubekit::app
{
    std::set<void *> global_player;
    tubekit::thread::mutex global_player_mutex;
}

int process_protocol(tubekit::connection::stream_connection &m_stream_connection, int cmd, std::string &&packet)
{
    // EXAMPLE_REQ
    if (cmd == ProtoCmd::EXAMPLE_REQ)
    {
        ProtoExampleReq exampleReq;
        if (exampleReq.ParsePartialFromArray(packet.c_str(), packet.size()))
        {
            std::cout << exampleReq.testcontext() << std::endl;
        }
        else
        {
            return -1;
        }
        return 0;
    }
    return -1;
}

void stream_app::process_connection(tubekit::connection::stream_connection &m_stream_connection)
{
    using tubekit::app::global_player;
    using tubekit::app::global_player_mutex;
    uint64_t all_data_len = m_stream_connection.m_recv_buffer.can_readable_size();
    char *all_data_buffer = new char[all_data_len];
    m_stream_connection.m_recv_buffer.copy_all(all_data_buffer, all_data_len);
    uint64_t offset = 0;

    while (1)
    {
        char *tmp_buffer = all_data_buffer + offset;
        uint64_t data_len = all_data_len - offset;
        if (data_len <= sizeof(uint64_t))
        {
            break;
        }

        uint64_t headLen = *(uint64_t *)tmp_buffer;
        headLen = be64toh(headLen);
        if (headLen + sizeof(uint64_t) > data_len)
        {
            break;
        }

        ProtoMessageHead protoHead;
        if (!protoHead.ParseFromArray(tmp_buffer + sizeof(uint64_t), headLen))
        {
            m_stream_connection.mark_close();
            break;
        }
        uint32_t bodyLen = protoHead.bodylen();
        uint32_t cmd = protoHead.cmd();

        if (data_len < sizeof(uint64_t) + headLen + bodyLen)
        {
            break;
        }

        if (0 != process_protocol(m_stream_connection, cmd, std::move(std::string(tmp_buffer + sizeof(uint64_t) + headLen, bodyLen))))
        {
            m_stream_connection.mark_close();
            m_stream_connection.m_recv_buffer.clear();
            break;
        }
        offset += sizeof(uint64_t) + headLen + bodyLen;
    }

    if (!m_stream_connection.m_recv_buffer.read_ptr_move_n(offset))
    {
        m_stream_connection.mark_close();
    }
    // std::vector<void *> vec;
    // global_player_mutex.lock();
    // for (auto socket_ptr : global_player)
    // {
    //     if (socket_ptr != m_stream_connection.get_socket_ptr())
    //     {
    //         vec.push_back(socket_ptr);
    //     }
    // }
    // global_player_mutex.unlock();
    // for (auto socket_ptr : vec)
    // {
    //     bool b_ret = singleton<connection_mgr>::instance()->safe_send(socket_ptr, tmp_buffer, data_len);
    // }
    delete all_data_buffer;
}

void stream_app::on_close_connection(tubekit::connection::stream_connection &m_stream_connection)
{
    using tubekit::app::global_player;
    using tubekit::app::global_player_mutex;
    global_player_mutex.lock();
    global_player.erase(m_stream_connection.get_socket_ptr());
    LOG_ERROR("player online %d", global_player.size());
    global_player_mutex.unlock();
}

void stream_app::on_new_connection(tubekit::connection::stream_connection &m_stream_connection)
{
    using tubekit::app::global_player;
    using tubekit::app::global_player_mutex;
    global_player_mutex.lock();
    global_player.insert(m_stream_connection.get_socket_ptr());
    LOG_ERROR("player online %d", global_player.size());
    global_player_mutex.unlock();
}

bool stream_app::new_client_connection(const std::string &ip, int port)
{
    socket::socket *socket_object = singleton<socket_handler>::instance()->alloc_socket();
    if (!socket_object)
    {
        LOG_ERROR("alloc_socket return nullptr");
        return false;
    }
    bool b_ret = socket_object->connect(ip, port);
    if (!b_ret)
    {
        LOG_ERROR("connection remote %s:%d failed", ip.c_str(), port);
        singleton<socket_handler>::instance()->remove(socket_object);
        return false;
    }
    int i_ret = singleton<socket_handler>::instance()->attach(socket_object);
    if (0 != i_ret)
    {
        LOG_ERROR("attach to socket_handler error ret %d", i_ret);
        singleton<socket_handler>::instance()->remove(socket_object);
        return false;
    }
    // maybe to do some management for client socket...
    return true;
}