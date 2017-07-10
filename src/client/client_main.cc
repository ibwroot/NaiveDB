#include <string>

#include <sofa/pbrpc/pbrpc.h>
#include <gflags/gflags.h>

#include "proto/server_service.pb.h"

#include "client/server_client.h"
#include "client/client_impl.h"
#include "client/conshash.h"

DECLARE_string(flagfile);
DEFINE_string(dd_strategy, "none", "data distribute strategy");
DEFINE_string(server_nodes, "", "server cluster addresses");

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
    printf("\t\t  putbatch <file path>\n");
}

int Put(conshash* ch, int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return -1;
    }
    

    std::string addrs(ch->GetTargetAddrs(argv[0]));
    client_impl* client_ = new client_impl(addrs);
    int ret = client_->Put(argv[0], argv[1]);
    if (ret) {
        printf("client_->SendRequest Put fail!\n");
        return -1;
    }
    return 0;
}

int Get(conshash* ch, int argc, char* argv[]) {
    if (argc != 1) {
        PrintUsage();
        return -1;
    }

    std::string addrs(ch->GetTargetAddrs(argv[0]));
    client_impl* client_ = new client_impl(addrs);
    int ret = client_->Get(argv[0]);
    if (ret) {
        printf("client_->SendRequest Get fail!\n");
        return -1;
    }
}

int PutBatch(conshash* ch, int argc, char* argv[]) {
    //std::string data("1,a\n2,b\n3,c\n4,d\n");
    //int ret = client_->PutBatch(data);
    //if (ret) {
    //    printf("client_->SendRequest PutBatch fail!\n");
    //    return -1;
    //}
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage();
        return 0;
    }
    if (FLAGS_flagfile == "") {
        FLAGS_flagfile = "./config.flag";
    }
    ::google::ParseCommandLineFlags(&argc, &argv, false);
    printf("server_nodes:%s\n", FLAGS_server_nodes.c_str());
    printf("data_distribute_strategy:%s\n", FLAGS_dd_strategy.c_str());

    conshash* ch;
    if (FLAGS_dd_strategy == "none") {
        ch = NULL;
    } else if (FLAGS_dd_strategy == "conshash") {
        ch = new conshash(FLAGS_server_nodes, 0, 3);
    }

    int ret = -1;
    if (strcmp(argv[1], "put") == 0) {
        ret = Put(ch, argc - 2, argv + 2);
    } else if (strcmp(argv[1], "get") == 0) {
        ret = Get(ch, argc - 2, argv + 2);
    } else if (strcmp(argv[1], "putbatch") == 0) {
        ret = PutBatch(ch, argc - 2, argv + 2);
    } else {
        PrintUsage();
    }

    return ret;
}
