#ifndef RAFT_SERVER_IMPL_H_
#define RAFT_SERVER_IMPL_H_

#include <leveldb/db.h>

#include "proto/server_service.pb.h"
//#include "astatus_code.pb.h"

#define DB_CACHE_SIZE 10
#define DB_PATH "/root/raft_demo/db"

namespace raft_demo {

class ServerImpl : public ServerService {
public:
    ServerImpl();//Sync* sync);
    virtual ~ServerImpl();
    void Echo(::google::protobuf::RpcController* controller,
            const EchoRequest* request,
            EchoResponse* response,
            ::google::protobuf::Closure* done);
    void Put(::google::protobuf::RpcController* controller,
            const PutRequest* request,
            PutResponse* response,
            ::google::protobuf::Closure* done);
    void Get(::google::protobuf::RpcController* controller,
            const GetRequest* request,
            GetResponse* response,
            ::google::protobuf::Closure* done);
private:
//    Sync* sync_;
    bool is_leader;
    leveldb::DB* db_;
    leveldb::Cache* db_cache_;

};
}
#endif
