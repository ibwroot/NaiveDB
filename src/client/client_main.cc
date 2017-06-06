#include <string>

#include <sofa/pbrpc/pbrpc.h>

#include "proto/server_service.pb.h"

#include "client/server_client.h"

using namespace raft_demo;

/*
int ConnectServer(RpcClient* rpc_client_, ServerClient* server_client_, std::string server_node) {
    //TODO: single node or cluster
    rpc_client_ = new RpcClient();
    server_client_ = new ServerClient(rpc_client_, server_node);
    return 0;
}
*/

void PrintUsage() {
    printf("Usage:\n./client <command>\n");
    printf("command:");
    printf("\t  put <key> <value>\n");
    printf("\t\t  get <key>\n");
}

int Put(ServerClient* server_client_, int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return -1;
    }
    PutRequest request;
    PutResponse response;
    request.set_key(argv[0]);
    request.set_value(argv[1]);
    bool ret = server_client_->SendRequest(&ServerService_Stub::Put, &request, &response, 15, 3);
    if (!ret) {
        printf("server_client_->SendRequest Put fail!\n");
        return -1;
    }
    return 0;
}

int Get(ServerClient* server_client_, int argc, char* argv[]) {
    if (argc != 1) {
        PrintUsage();
        return -1;
    }
    GetRequest request;
    GetResponse response;
    request.set_key(argv[0]);
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage();
        return 0;
    }
    //init server
    std::string server_node("10.10.2.203:12321,10.10.2.204:12321");
    RpcClient* rpc_client_ = new RpcClient();
    ServerClient* server_client_ = new ServerClient(rpc_client_, server_node);
    //echo test
    EchoRequest request;
    EchoResponse response;
    request.set_echo("lusen hello");
    server_client_->SendRequest(&ServerService_Stub::Echo, &request, &response, 15, 3);

    int ret = -1;
    if (strcmp(argv[1], "put") == 0) {
        ret = Put(server_client_, argc - 2, argv + 2);
    } else if (strcmp(argv[1], "get") == 0) {
        ret = Get(server_client_, argc - 2, argv + 2);
    } else {
        PrintUsage();
    }

    delete rpc_client_;
    delete server_client_;
    return ret;
}
