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


void ServerImpl::GetBatch(::google::protobuf::RpcController* controller,
                            const GetBatchRequest* request,
                            GetBatchResponse* response,
                            ::google::protobuf::Closure* done) {
    printf("%s!!!!\n", __func__);
    if (!is_leader) {
        response->set_status(kIsFollower);
        done->Run();
        return;
    }
    auto key_start = request->key_start();
    auto key_end = request->key_end();
    std::string databuf;
    leveldb::Iterator* it = db_->NewIterator(leveldb::ReadOptions());
    for (it->Seek(key_start); it->Valid(); it->Next()) {
        leveldb::Slice key = it->key();
        if (key.compare(key_end) >= 0) {
            break;
        }
        printf("it->key().ToString():%s\n", it->key().ToString().c_str());
        databuf += it->key().ToString() + "," + it->value().ToString() + "\n";
        printf("key.data():%s value.data():%s databuf:%s\n", it->key().ToString().c_str(), 
                                                            it->value().ToString().c_str(), databuf.c_str());
    }
    response->set_databuf(databuf);
    response->set_status(kOK);
    delete it;

    done->Run();

}

unsigned int ServerImpl::MurMurHash(const void *key, unsigned int len) {
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    const int seed = 97;
    unsigned int h = seed ^ len;
    // Mix 4 bytes at a time into the hash
    const unsigned char *data = (const unsigned char *)key;
    while(len >= 4) {
        unsigned int k = *(unsigned int *)data;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
        data += 4;
        len -= 4;
    }
    // Handle the last few bytes of the input array
    switch(len)
    {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
        h *= m;
    };
    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
}

void ServerImpl::GetNodeData(::google::protobuf::RpcController* controller,
                            const GetNodeDataRequest* request,
                            GetNodeDataResponse* response,
                            ::google::protobuf::Closure* done) {
    printf("%s!!!!\n", __func__);
    if (!is_leader) {
        response->set_status(kIsFollower);
        done->Run();
        return;
    }
    std::string databuf;
    unsigned int key_hash_start = request->key_hash_start();
    unsigned int key_hash_end = request->key_hash_end();
    leveldb::Iterator* it = db_->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        std::string key_(it->key().ToString());
        unsigned int key_hash = MurMurHash(key_.c_str(), key_.size());
        if (key_hash <= key_hash_end && key_hash > key_hash_start) {
            databuf += key_ + "," + it->value().ToString() + "\n";
        }
    }
    response->set_databuf(databuf);
    response->set_status(kOK);
    delete it;

    done->Run();
}


}//namespace raft_demo
