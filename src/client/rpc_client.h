#ifndef RAFT_DEMO_RPC_CLIENT_H_
#define RAFT_DEMO_RPC_CLIENT_H

#include <unistd.h>
#include <functional>
#include <string>
#include <cstdio>
#include <sofa/pbrpc/pbrpc.h>

namespace raft_demo {

class RpcClient {
public:
    RpcClient() {
        sofa::pbrpc::RpcClientOptions options;
        rpc_client_ = new sofa::pbrpc::RpcClient(options);
    }
    ~RpcClient() {
        delete rpc_client_;
    }
    template <class T>
    bool GetStub(const std::string server, T** stub) {
        //TODO lock host_map
        printf("-->%s server:%s\n", __func__, server.c_str());
        sofa::pbrpc::RpcChannel* channel = NULL;
        std::map<std::string, sofa::pbrpc::RpcChannel*>::iterator it = host_map_.find(server);
        if (it != host_map_.end()) {
            channel = it->second;
        } else {
            //定义channel， 通讯通道，每个服务器地址对应一个channel
            sofa::pbrpc::RpcChannelOptions channel_options;
            channel = new sofa::pbrpc::RpcChannel(rpc_client_, server, channel_options);
            host_map_[server] = channel;
        }
        *stub = new T(channel);
        return true;
    }
    template <class Stub, class Request, class Response, class Callback>
    bool SendRequest(Stub* stub, void(Stub::*func)(google::protobuf::RpcController*,
                                                    const Request*, Response*, Callback*),
                    const Request* request, Response* response,
                    int32_t rpc_timeout, int retry_times) {
        //controller用于本次调用
        sofa::pbrpc::RpcController controller;
        controller.SetTimeout(rpc_timeout * 1000L);
        for (int32_t retry = 0; retry < retry_times; ++retry) {
            (stub->*func)(&controller, request, response, NULL);
            if (controller.Failed()) {
                if (retry < retry_times - 1) {
                    printf("SendRequest failed, retry...\n");
                    usleep(1000000);
                } else {
                    printf("SendRequest fail!\n");
                }
            } else {
                return true;
            }
            controller.Reset();
        }
        return false;
    }

private:
    sofa::pbrpc::RpcClient* rpc_client_;
    std::map<std::string, sofa::pbrpc::RpcChannel*> host_map_;
};
}
#endif
