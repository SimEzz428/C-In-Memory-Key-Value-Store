#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct WalHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t page_size;
    uint32_t checkpoint_seq;
    uint32_t salt1;
    uint32_t salt2;
    uint32_t checksum1;
    uint32_t checksum2;
};

struct WalFrameHeader {
    uint32_t page_no;
    uint32_t db_size_after;
    uint32_t salt1;
    uint32_t salt2;
    uint32_t checksum1;
    uint32_t checksum2;
};

struct WalFrame {
    WalFrameHeader header;
    std::vector<std::byte> page_bytes;
};

struct WalFile {
    WalHeader header;
    std::vector<WalFrame> frames;
    std::string path;
};

bool read_wal(const std::string& path, WalFile& out, std::string& err);