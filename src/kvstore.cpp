#include "kvstore.h"
#include "journal.h"
#include <unordered_map>
#include <mutex>

namespace {
    std::mutex KV_MU;
    std::unordered_map<std::string,std::string> KV_MAP;
    KvStore GLOBAL_KV;
    Journal JOURNAL(std::string(getenv("HOME")?getenv("HOME"):"./") + "/.dataforge.journal");
    struct Boot {
        Boot(){ JOURNAL.replay(GLOBAL_KV); }
    } BOOT;
}

KvStore& GlobalKV(){ return GLOBAL_KV; }

bool KvStore::set(const std::string& key,const std::string& value,std::optional<int>){
    std::lock_guard<std::mutex> lock(KV_MU);
    KV_MAP[key]=value;
    JOURNAL.append_set(key,value);
    return true;
}

bool KvStore::get(const std::string& key,std::string& out_value) const{
    std::lock_guard<std::mutex> lock(KV_MU);
    auto it=KV_MAP.find(key);
    if(it==KV_MAP.end()) return false;
    out_value=it->second;
    return true;
}

bool KvStore::erase(const std::string& key){
    std::lock_guard<std::mutex> lock(KV_MU);
    bool ok = KV_MAP.erase(key)>0;
    if(ok) JOURNAL.append_del(key);
    return ok;
}

bool kv_set(const std::string& key,const std::string& value);
bool kv_get(const std::string& key,std::string& out_value);
bool kv_del(const std::string& key);