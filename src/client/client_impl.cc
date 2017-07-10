#include "client_impl.h"

namespace raft_demo {

client_impl::client_impl(std::string server_nodes) {
    rpc_client_ = new RpcClient();
    server_client_ = new ServerClient(rpc_client_, server_nodes);
}

client_impl::~client_impl() {
    delete rpc_client_;
    delete server_client_;
}

int client_impl::Echo(std::string echo) {
    EchoRequest request;
    EchoResponse response;
    request.set_echo(echo);
    server_client_->SendRequest(&ServerService_Stub::Echo, &request, &response, 15, 3);
}

int client_impl::Put(std::string key, std::string value) {
    PutRequest request;
    PutResponse response;
    request.set_key(key);
    request.set_value(value);
    bool ret = server_client_->SendRequest(&ServerService_Stub::Put, &request, &response, 15, 3);
    if (!ret) {
        printf("server_client_->SendRequest Put fail!\n");
        return -1;
    }
    return 0;
}

int client_impl::Get(std::string key) {
    GetRequest request;
    GetResponse response;
    request.set_key(key);
    bool ret = server_client_->SendRequest(&ServerService_Stub::Get, &request, &response, 15, 3);
    if (!ret) {
        printf("server_client_->SendRequest Get fail!\n");
        return -1;
    }
    if (response.status() == kOK) {
        printf("-->%s value:%s\n", __func__, response.value().c_str());
        return 0;
    } else {
        printf("-->%s key not found!\n", __func__);
        return -1;
    }
}

int client_impl::PutBatch(const std::string& data) {
    // data pre process
    //
    //
    //
    PutBatchRequest request;
    PutBatchResponse response;
    request.set_databuf(data);
    bool ret = server_client_->SendRequest(&ServerService_Stub::PutBatch, &request, &response, 15, 3);
    if (!ret) {
        printf("server_client_->SendRequest PutBatch fail!\n");
        return -1;
    }
    return 0;

}




}
