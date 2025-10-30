#include "wal.h"
#include <fstream>
#include <string>
#include <vector>

static bool read_exact(std::ifstream& f, void* dst, std::size_t n) {
    return static_cast<bool>(f.read(reinterpret_cast<char*>(dst), static_cast<std::streamsize>(n)));
}

static uint32_t u32be(const unsigned char* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}

bool read_wal(const std::string& path, WalFile& out, std::string& err) {
    std::ifstream f(path, std::ios::binary);
    if (!f) { err = "open failed"; return false; }

    unsigned char hdr[32];
    if (!read_exact(f, hdr, sizeof(hdr))) { err = "short header"; return false; }

    WalHeader H;
    H.magic          = u32be(hdr + 0);
    H.version        = u32be(hdr + 4);
    H.page_size      = u32be(hdr + 8);
    H.checkpoint_seq = u32be(hdr +12);
    H.salt1          = u32be(hdr +16);
    H.salt2          = u32be(hdr +20);
    H.checksum1      = u32be(hdr +24);
    H.checksum2      = u32be(hdr +28);

    uint32_t ps = H.page_size == 1 ? 65536u : H.page_size;
    if (ps == 0) { err = "invalid page size"; return false; }

    out.header = H;
    out.frames.clear();
    out.path = path;

    for (;;) {
        unsigned char fh[24];
        if (!f.read(reinterpret_cast<char*>(fh), 24)) {
            if (f.eof()) break;
            err = "short frame header";
            return false;
        }

        WalFrameHeader FH;
        FH.page_no       = u32be(fh + 0);
        FH.db_size_after = u32be(fh + 4);
        FH.salt1         = u32be(fh + 8);
        FH.salt2         = u32be(fh +12);
        FH.checksum1     = u32be(fh +16);
        FH.checksum2     = u32be(fh +20);

        std::vector<std::byte> page(ps);
        if (!read_exact(f, page.data(), ps)) { err = "short frame page"; return false; }

        WalFrame frame;
        frame.header = FH;
        frame.page_bytes = std::move(page);
        out.frames.push_back(std::move(frame));
    }

    return true;
}