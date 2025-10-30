#pragma once
#include <string>
#include <optional>

class KvStore {
public:
    bool set(const std::string& key,const std::string& value,std::optional<int> ttl=std::nullopt);
    bool get(const std::string& key,std::string& out_value) const;
    bool erase(const std::string& key);
};

KvStore& GlobalKV();