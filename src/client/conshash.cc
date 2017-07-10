#include <cstdio>
#include <map>
#include <algorithm>
#include <string.h>
#include "conshash.h"

namespace raft_demo {

conshash::conshash(std::string& server_nodes, int vnode_num, int duplicates):
       duplicates(duplicates),
       vnode_num(vnode_num) {

    int size = server_nodes.size();
    int last = -1;
    for (int i = 0; i < size; ++i) {
        if (server_nodes[i] != ',') {
            continue;
        } else {
            std::string addr(server_nodes.substr(last + 1, i - last - 1));
            unsigned int key = MurMurHash(addr.c_str(), addr.size());
            server_keytoaddr[key] = addr;
            last = i;
        }
    }
    std::string addr(server_nodes.substr(last + 1));
    unsigned int key = MurMurHash(addr.c_str(), addr.size());
    server_keytoaddr[key] = addr;
    for (auto e : server_keytoaddr) {
        printf("key:%u addr:%s\n", e.first, e.second.c_str());
    }
}

conshash::~conshash() {
}

std::string conshash::GetTargetAddrs(const std::string& key) {
    std::string addrs;
    unsigned int key_ = MurMurHash(key.c_str(), key.size());
    printf("%s key_:%u\n", __func__, key_);
    auto it = server_keytoaddr.lower_bound(key_);
    for (int i = 0; i < duplicates; ++i) {
        if (it == server_keytoaddr.end()) {
            it = server_keytoaddr.begin();
        }
        addrs += it->second;
        if (i != duplicates - 1) {
            addrs += ",";
        }
        ++it;
    }
    printf("%s:%s\n", __func__, addrs.c_str());
    return addrs;
}

std::vector<std::string> conshash::AddNode(const std::string& addr) {
    printf("%s\n", __func__);
    unsigned int key_ = MurMurHash(addr.c_str(), addr.size());
    server_keytoaddr[key_] = addr;
    printf("new node key:%u\n", key_);
    // find previous dumplicates node key
    std::vector<unsigned int> pre;
    auto it = server_keytoaddr.find(key_);
    for (int i = 0; i < duplicates; ++i) {
        if (--it == server_keytoaddr.begin()) {
            pre.push_back(it->first);
            it = server_keytoaddr.end();
        } else {
            pre.push_back(it->first);
        }
    }
    std::reverse(pre.begin(), pre.end());
    for (auto pre_keys : pre) {
        printf("pre_keys:%u\n", pre_keys);
    }
    // find post dumplicates node key
    std::vector<unsigned int> post;
    it = server_keytoaddr.find(key_);
    for (int i = 0; i < duplicates; ++i) {
        if (++it == server_keytoaddr.end()) {
            it = server_keytoaddr.begin();
            post.push_back(it->first);
        } else {
            post.push_back(it->first);
        }
    }
    for (auto post_keys : post) {
        printf("post_keys:%u\n", post_keys);
    }
    std::vector<std::string> ops;
    unsigned key_start = -1;
    unsigned key_end = -1;
    for (int i = 0; i < duplicates; ++i) {
        key_start = pre[i];
        if (i == duplicates - 1) {
            key_end = key_;
            std::string addr_range(server_keytoaddr[post[0]] + "," + std::to_string(key_start) +
                                    "," + std::to_string(key_end));
            ops.push_back(addr_range);
        } else {
            key_end = pre[i + 1];
            std::string addr_range(server_keytoaddr[key_end] + "," + std::to_string(key_start) +
                                    "," + std::to_string(key_end));
            ops.push_back(addr_range);
        }
    }

    for (int i = 0; i < duplicates; ++i) {
        key_start = pre[i];
        if (i == duplicates - 1) {
            key_end = key_;
        } else {
            key_end = pre[i + 1];
        }
        std::string remove_addr_range(server_keytoaddr[post[i]] + "," + std::to_string(key_start) +
                                        "," + std::to_string(key_end));
        ops.push_back(remove_addr_range);
    }
    for (std::string op : ops) {
        printf("op:%s\n", op.c_str());
    }
    return ops;

}

unsigned int conshash::MurMurHash(const void *key, unsigned int len) {
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


}
