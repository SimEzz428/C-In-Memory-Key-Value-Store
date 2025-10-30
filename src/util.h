#pragma once
#include <filesystem>
#include <string>

namespace util {
inline void ensure_dir(const std::filesystem::path& p) {
    std::error_code ec;
    std::filesystem::create_directories(p, ec);
}

inline std::string trim(std::string s) {
    while (!s.empty() && (s.back()=='\n' || s.back()=='\r' || s.back()==' ' || s.back()=='\t')) s.pop_back();
    size_t i=0; while (i<s.size() && (s[i]==' ' || s[i]=='\t')) ++i;
    return s.substr(i);
}
}