#ifndef CLIENT_IMPL_H
#define CLIENT_IMPL_H

#include <string>
#include <sstream>

#include <sofa/pbrpc/pbrpc.h>
#include <gflags/gflags.h>

#include "proto/server_service.pb.h"

#include "client/server_client.h"


namespace raft_demo {

class client_impl {
public:
    client_impl(std::string server_nodes);
    ~client_impl();
    int Echo(std::string echo);
    int Put(std::string key, std::string value);
    int Get(std::string key, std::string& value);
    int PutBatch(const std::string& data);
    int GetBatch(const std::string& key_start, const std::string& key_end, std::string& values);
    int GetNodeData(const std::string& key_hash_start, const std::string& key_hash_end,
                                std::string& databuf);
private:
    unsigned int toUINT32(const std::string s);
private:
    RpcClient* rpc_client_;
    ServerClient* server_client_;
};
}
#endif
