#include "journal.h"
#include "kvstore.h"
#include <fstream>
#include <cstdint>
#include <string>

static void w32(std::ofstream& o, uint32_t x) {
    unsigned char b[4];
    b[0] = static_cast<unsigned char>((x >> 24) & 0xFF);
    b[1] = static_cast<unsigned char>((x >> 16) & 0xFF);
    b[2] = static_cast<unsigned char>((x >>  8) & 0xFF);
    b[3] = static_cast<unsigned char>( x        & 0xFF);
    o.write(reinterpret_cast<const char*>(b), 4);
}

static uint32_t r32(std::ifstream& i) {
    unsigned char b[4];
    if (!i.read(reinterpret_cast<char*>(b), 4)) return 0;
    return (uint32_t(b[0]) << 24) |
           (uint32_t(b[1]) << 16) |
           (uint32_t(b[2]) <<  8) |
            uint32_t(b[3]);
}

Journal::Journal(const std::string& path) : path_(path) {}

bool Journal::append_set(const std::string& k, const std::string& v, std::optional<int>) {
    std::ofstream o(path_, std::ios::binary | std::ios::app);
    if (!o) return false;
    o.put('S');
    w32(o, static_cast<uint32_t>(k.size())); o.write(k.data(), static_cast<std::streamsize>(k.size()));
    w32(o, static_cast<uint32_t>(v.size())); o.write(v.data(), static_cast<std::streamsize>(v.size()));
    return true;
}

bool Journal::append_del(const std::string& k) {
    std::ofstream o(path_, std::ios::binary | std::ios::app);
    if (!o) return false;
    o.put('D');
    w32(o, static_cast<uint32_t>(k.size())); o.write(k.data(), static_cast<std::streamsize>(k.size()));
    return true;
}

template<typename Store>
bool Journal::replay(Store& kv) {
    std::ifstream i(path_, std::ios::binary);
    if (!i) return true; 
    for (;;) {
        int op = i.get();
        if (op == EOF) break;

        if (op == 'S') {
            uint32_t ks = r32(i); if (!i) return false;
            std::string k(ks, '\0'); if (!i.read(k.data(), static_cast<std::streamsize>(ks))) return false;
            uint32_t vs = r32(i); if (!i) return false;
            std::string v(vs, '\0'); if (!i.read(v.data(), static_cast<std::streamsize>(vs))) return false;
            kv.set(k, v);
        } else if (op == 'D') {
            uint32_t ks = r32(i); if (!i) return false;
            std::string k(ks, '\0'); if (!i.read(k.data(), static_cast<std::streamsize>(ks))) return false;
            kv.erase(k);
        } else {
            return false;
        }
    }
    return true;
}


template bool Journal::replay<KvStore>(KvStore& kv);