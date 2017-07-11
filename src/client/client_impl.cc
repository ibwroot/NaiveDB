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
    return 0;
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

int client_impl::Get(std::string key, std::string& value) {
    GetRequest request;
    GetResponse response;
    request.set_key(key);
    bool ret = server_client_->SendRequest(&ServerService_Stub::Get, &request, &response, 15, 3);
    if (!ret) {
        printf("server_client_->SendRequest Get fail!\n");
        return -1;
    }
    if (response.status() == kOK) {
        value = response.value();
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

int client_impl::GetBatch(const std::string& key_start, const std::string& key_end, std::string& databuf) {
    GetBatchRequest request;
    GetBatchResponse response;
    request.set_key_start(key_start);
    request.set_key_end(key_end);
    bool ret = server_client_->SendRequest(&ServerService_Stub::GetBatch, &request, &response, 15, 3);
    if (!ret) {
        printf("server_client_->SendRequest GetBatch fail!\n");
        return -1;
    }
    printf("%s %s\n", __func__, response.databuf().c_str());
    databuf = response.databuf();
    return 0;
}

unsigned int client_impl::toUINT32(const std::string s) {
    std::stringstream a;
    a<<s;
    unsigned int ret = 0;
    a>>ret;
    return ret;
}

int client_impl::GetNodeData(const std::string& key_hash_start, const std::string& key_hash_end,
                            std::string& databuf) {
    GetNodeDataRequest request;
    GetNodeDataResponse response;
    request.set_key_hash_start(toUINT32(key_hash_start));
    request.set_key_hash_end(toUINT32(key_hash_end));
    bool ret = server_client_->SendRequest(&ServerService_Stub::GetNodeData, &request, &response, 15, 3);
    if (!ret) {
        printf("server_client_->SendRequest GetBatch fail!\n");
        return -1;
    }
    printf("%s %s\n", __func__, response.databuf().c_str());
    databuf += response.databuf();
    return 0;
}

}
