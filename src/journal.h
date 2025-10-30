#pragma once
#include <string>
#include <optional>

class Journal {
public:
    explicit Journal(const std::string& path);
    bool append_set(const std::string& k, const std::string& v, std::optional<int> ttl = std::nullopt);
    bool append_del(const std::string& k);
    template<typename Store>
    bool replay(Store& kv);
private:
    std::string path_;
};