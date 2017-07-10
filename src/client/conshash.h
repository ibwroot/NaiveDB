#include <string>
#include <vector>
#include <map>

namespace raft_demo {

class conshash{
public:
    conshash(std::string& server_nodes, int vnode_num, int duplicates);
    ~conshash();
    std::string GetTargetAddrs(const std::string& key);
    void DeleteNode(std::string addr);
    std::vector<std::string> AddNode(const std::string& addr);
    std::map<unsigned int, std::string> server_keytoaddr;

private:
    unsigned int MurMurHash(const void *key, unsigned int len);
private:
    int duplicates;
    int vnode_num;

};




}
