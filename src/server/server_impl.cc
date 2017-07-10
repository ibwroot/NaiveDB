#include "server_impl.h"

#include <leveldb/db.h>
#include <leveldb/cache.h>
#include <leveldb/write_batch.h>

#include <cstdio>

//#include "server/sync.h"

namespace raft_demo {

//ServerImpl::ServerImpl(Sync* sync) :
//    sync_ = sync,
ServerImpl::ServerImpl() :
    is_leader(true) {
    printf("ServerImpl construct!\n");
    printf("start db!\n");
    leveldb::Options options;
    options.create_if_missing = true;
    db_cache_ = leveldb::NewLRUCache(DB_CACHE_SIZE * 1024L * 1024L);
    options.block_cache = db_cache_;
    leveldb::Status s = leveldb::DB::Open(options, DB_PATH, &db_);
    if (!s.ok()) {
        db_ = NULL;
        printf("leveldb open fail!\n");
        exit(EXIT_FAILURE);
    }
}

ServerImpl::~ServerImpl() {
    delete db_;
    db_ = NULL;
}

void ServerImpl::Echo(::google::protobuf::RpcController* controller,
                        const EchoRequest* request,
                        EchoResponse* response,
                        ::google::protobuf::Closure* done) {
    if (!is_leader) {
        response->set_status(kIsFollower);
        done->Run();
        return;
    }
    printf("-->%s %s\n", __func__, request->echo().c_str());
    response->set_echo(request->echo());
    response->set_status(kOK);
    done->Run();
}

void ServerImpl::Put(::google::protobuf::RpcController* controller,
                        const PutRequest* request,
                        PutResponse* response,
                        ::google::protobuf::Closure* done) {
    if (!is_leader) {
        response->set_status(kIsFollower);
        done->Run();
        return;
    }
    printf("-->%s key:%s value:%s\n", __func__, request->key().c_str(),
                                                request->value().c_str());
    //write to db
    leveldb::Status s = db_->Put(leveldb::WriteOptions(), request->key(), request->value());
    if (!s.ok()) {
        printf("Put leveldb fail!\n");
    }
    response->set_status(kOK);
    done->Run();
}

void ServerImpl::Get(::google::protobuf::RpcController* controller,
                        const GetRequest* request,
                        GetResponse* response,
                        ::google::protobuf::Closure* done) {
    if (!is_leader) {
        response->set_status(kIsFollower);
        done->Run();
        return;
    }
    printf("-->%s key:%s \n", __func__, request->key().c_str());

    //read db
    std::string value;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), request->key(), &value);
    if (!s.ok()) {
        printf("Get leveldb fail!\n");
        response->set_status(kKeyNotFound);
    } else {
        response->set_value(value);
        response->set_status(kOK);
    }
    done->Run();
}

void ServerImpl::PutBatch(::google::protobuf::RpcController* controller,
                            const PutBatchRequest* request,
                            PutBatchResponse* response,
                            ::google::protobuf::Closure* done) {
    if (!is_leader) {
        response->set_status(kIsFollower);
        done->Run();
        return;
    }

    const std::string& databuf = request->databuf();
    int size = databuf.size();
    int key_last = -1;
    int value_last = -1;
    leveldb::WriteBatch batch;
    for (int i = 0; i < size; ++i) {
        if (databuf[i] != ',' && databuf[i] != '\n') {
            continue;
        } else if (databuf[i] == ',') {
            key_last = i;
        } else if (databuf[i] == '\n') {
            const std::string& k = databuf.substr(value_last + 1, key_last - value_last - 1);
            const std::string& v = databuf.substr(key_last + 1, i - key_last - 1);
            printf("k:%s v:%s\n", k.c_str(), v.c_str());
            batch.Put(k, v);
            value_last = i;
        }
    }
    leveldb::Status s = db_->Write(leveldb::WriteOptions(), &batch);
    if (!s.ok()) {
        printf("PutBatch leveldb fail!\n");
    }
    response->set_status(kOK);
    done->Run();
}



}
