#pragma once
#include <string>
#include <unordered_map>
#include <optional>
#include <shared_mutex>
#include <chrono>

class KvStore {
public:
  using Clock = std::chrono::steady_clock;
  KvStore();
  void set(const std::string& key, const std::string& value, std::optional<int> ttl_seconds = std::nullopt);
  std::optional<std::string> get(const std::string& key);
  bool erase(const std::string& key);
  size_t size() const;
private:
  struct Entry {
    std::string value;
    std::optional<Clock::time_point> expires_at;
  };
  bool expired_unlocked(const Entry& e) const;
  mutable std::shared_mutex mtx_;
  std::unordered_map<std::string, Entry> data_;
};