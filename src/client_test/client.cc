//sync client
#include <sofa/pbrpc/pbrpc.h>

#include "proto/server_service.pb.h"

sofa::pbrpc::RpcClient g_rpc_client;

int echo(raft_demo::ServerService_Stub* stub, const std::string& message) {
    sofa::pbrpc::RpcController cntl;
    cntl.SetTimeout(3000);
    raft_demo::EchoRequest request;
    request.set_echo(message);
    raft_demo::EchoResponse response;
    stub->Echo(&cntl, &request, &response, NULL);
    SLOG(NOTICE, "RemoteAddress=%s", cntl.RemoteAddress().c_str());
    SLOG(NOTICE, "IsRequestSent=%s", cntl.IsRequestSent() ? "true" : "false");
    if (cntl.IsRequestSent()) {
        SLOG(NOTICE, "LocalAddress=%s", cntl.LocalAddress().c_str());
        SLOG(NOTICE, "SentBytes=%ld", cntl.SentBytes());
    }

    if (cntl.Failed()) {
        SLOG(ERROR, "request failed: %s", cntl.ErrorText().c_str());
        return -1;
    } else {
        SLOG(NOTICE, "request succeed: %s", response.echo().c_str());
        return 0;
    }
}

int Put(raft_demo::ServerService_Stub* stub, const std::string& key, const std::string& value) {
    return 0;
}

std::string Get(raft_demo::ServerService_Stub* stub, const std::string& key) {
    return NULL;
}

int Delete(raft_demo::ServerService_Stub* stub, const std::string& key) {
    return 0;
}

/*
int echo(raft_demo::ServerService_Stub* stub, const std::string& message) {
    // Prepare parameters.
    sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
    cntl->SetTimeout(3000);

    raft_demo::EchoRequest* request =
        new raft_demo::EchoRequest();
    request->set_echo(message);
    raft_demo::EchoResponse* response =
        new raft_demo::EchoResponse();
    // Sync call.
    stub->Echo(cntl, request, response, NULL);
    
    SLOG(NOTICE, "RemoteAddress=%s", cntl->RemoteAddress().c_str());
    SLOG(NOTICE, "IsRequestSent=%s", cntl->IsRequestSent() ? "true" : "false");
    if (cntl->IsRequestSent()) {
        SLOG(NOTICE, "LocalAddress=%s", cntl->LocalAddress().c_str());
        SLOG(NOTICE, "SentBytes=%ld", cntl->SentBytes());
    }

    if (cntl->Failed()) {
        SLOG(ERROR, "request failed: %s", cntl->ErrorText().c_str());
    } else {
        SLOG(NOTICE, "request succeed: %s", response->echo().c_str());
    }
    delete cntl;
    delete request;
    delete response;
    return 0;
}
*/

int main() {
    SOFA_PBRPC_SET_LOG_LEVEL(NOTICE);
    // Define an rpc channel.
    sofa::pbrpc::RpcChannelOptions channel_options;
    //sofa::pbrpc::RpcChannel rpc_channel(&g_rpc_client, "127.0.0.1:12321", channel_options);
    sofa::pbrpc::RpcChannel rpc_channel(&g_rpc_client, "10.10.2.203:12321", channel_options);
    raft_demo::ServerService_Stub* stub =
        new raft_demo::ServerService_Stub(&rpc_channel);

    echo(stub, "wocao!");
    delete stub;

    return EXIT_SUCCESS;
}
