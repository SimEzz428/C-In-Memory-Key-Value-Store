#include "checkpoint.h"
#include "wal.h"
#include <unordered_map>
#include <fstream>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <string>

bool rebuild_pages(const WalFile& wal, const std::string& out_path, std::string& err) {
    if (wal.frames.empty()) { err = "no frames"; return false; }

    std::unordered_map<uint32_t, std::vector<std::byte>> latest;
    latest.reserve(wal.frames.size());

    for (const auto& fr : wal.frames) {
        latest[fr.header.page_no] = fr.page_bytes;
    }

    std::ofstream out(out_path, std::ios::binary | std::ios::trunc);
    if (!out) { err = "open out failed"; return false; }

    uint32_t count = static_cast<uint32_t>(latest.size());
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& kv : latest) {
        uint32_t page_no = kv.first;
        const auto& bytes = kv.second;
        uint32_t sz = static_cast<uint32_t>(bytes.size());
        out.write(reinterpret_cast<const char*>(&page_no), sizeof(page_no));
        out.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
        out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        if (!out) { err = "write failed"; return false; }
    }

    return true;
}