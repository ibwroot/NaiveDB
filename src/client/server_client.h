#ifndef RAFT_DEMO_SERVER_CLIENT_H_
#define RAFT_DEMO_SERVER_CLIENT_H_

#include <vector>
#include <string>

#include <sofa/pbrpc/pbrpc.h>

#include "rpc_client.h"
#include "proto/astatus_code.pb.h"
#include "proto/server_service.pb.h"

namespace raft_demo {

class RpcClient;
class ServerService_Stub;
class ServerClient {
public:
    ServerClient(RpcClient* rpc_client, const std::string &servernodes)
    : rpc_client_(rpc_client), leader_id_(0) {
        SplitString(servernodes, &server_nodes_);
        server_stubs_.resize(server_nodes_.size());        
        for (uint32_t i = 0; i < server_nodes_.size(); ++i) {
            printf("server_nodes_[%d]:%s\n", i, server_nodes_[i].c_str());
            rpc_client_->GetStub(server_nodes_[i], &server_stubs_[i]);
        } 
    }
    template <class Request, class Response, class CallBack>
    bool SendRequest(void(ServerService_Stub::*func)(google::protobuf::RpcController*, 
                                                    const Request*, Response*, CallBack*),
                    const Request* request, Response* response,
                    int32_t rpc_timeout, int retry_times = 1) {
        bool ret = false;
        int success = 0;
        for (uint32_t i = 0; i < server_stubs_.size(); ++i) {
            printf("-->%s i:%d \n", __func__, i);
            int server_id = i;
            ret = rpc_client_->SendRequest(server_stubs_[server_id], func, request, response,
                                            rpc_timeout, retry_times);
            if (ret && response->status() != kIsFollower) {
                ++success;
            }
            //MutexLock lock(&mu_);
            //if (server_id == leader_id_) {
            //    leader_id_ = (leader_id_ + 1) % server_stubs_.size();
            //} 
        }
        if (success == server_stubs_.size()) {
            printf("###send request success!\n");
            return true;
        }
        return ret;
    }
private:
    RpcClient* rpc_client_;
    std::vector<std::string> server_nodes_;
    std::vector<ServerService_Stub*> server_stubs_;
    //Mutex mu_;
    int leader_id_;
    void SplitString(const std::string& source, std::vector<std::string>* stringv) {
        int size = source.size();
        int last = -1;
        for (int i = 0; i < size; ++i) {
            if (source[i] != ',') {
                continue;
            } else {
                stringv->push_back(source.substr(last + 1, i - last - 1));
                last = i;
            }
        }
        stringv->push_back(source.substr(last + 1));
    }

};
}
#endif
