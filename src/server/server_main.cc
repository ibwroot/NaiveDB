#include <stdio.h>

#include <sofa/pbrpc/pbrpc.h>

#include "server_impl.h"
//#include "server/raft_impl.h"
//#include "server/sync.h"

int main(int argc, char* argv[]) {
    printf("Server start ...\n");

    // rpc server
    sofa::pbrpc::RpcServerOptions options;
    sofa::pbrpc::RpcServer rpc_server(options);

    // server
//    raft_demo::Sync* sync = NULL;
//
//    raft_demo::RaftImpl* raft_impl = new raft_demo::RaftImpl();
//    google::protobuf::Service* sync_service = raft_impl->GetService();
//    sync = raft_impl;
    raft_demo::ServerImpl* server_service = new raft_demo::ServerImpl();

    if (!rpc_server.RegisterService(server_service)) {
        return -1;            
    }
//    if (sync_service && !rpc_server.RegisterService(sync_service)) {
//        return -1;
//    }
    std::string port(":12321");
    std::string listen_addr = std::string("0.0.0.0") + port;
    if (!rpc_server.Start(listen_addr)) {
        return -1;
    }
    rpc_server.Run();
    printf("Server exit.\n");
    return 0;
}
