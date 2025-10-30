#include "http_server.h"
#include "kvstore.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <map>

static KvStore g_store;

static std::string http_200(const std::string& body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: text/plain; charset=utf-8\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    return oss.str();
}

static std::string http_404() {
    return http_200("not found");
}

static std::map<std::string,std::string> parse_query(const std::string& q) {
    std::map<std::string,std::string> m;
    std::size_t start = 0;
    while (start < q.size()) {
        auto amp = q.find('&', start);
        auto part = q.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
        auto eq = part.find('=');
        std::string k = eq == std::string::npos ? part : part.substr(0, eq);
        std::string v = eq == std::string::npos ? ""   : part.substr(eq + 1);
        m[HttpServer::url_decode(k)] = HttpServer::url_decode(v);
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    return m;
}

HttpServer::HttpServer(int port) : port_(port), server_fd_(-1) {
    server_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    ::setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr_ = {};
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(static_cast<uint16_t>(port_));

    ::bind(server_fd_, reinterpret_cast<sockaddr*>(&addr_), sizeof(addr_));
    ::listen(server_fd_, 64);
}

HttpServer::~HttpServer() {
    if (server_fd_ >= 0) ::close(server_fd_);
}

void HttpServer::run() {
    for (;;) {
        int cfd = ::accept(server_fd_, nullptr, nullptr);
        if (cfd < 0) continue;
        workers_.emplace_back([this, cfd]() {
            handle_client(cfd);
        });
        workers_.back().detach();
    }
}

void HttpServer::handle_client(int client_fd) {
    char buf[4096];
    std::string req;
    for (;;) {
        ssize_t n = ::recv(client_fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        req.append(buf, buf + n);
        if (req.find("\r\n\r\n") != std::string::npos) break;
    }
    std::string body = handle_request(req);
    auto resp = http_200(body);
    ::send(client_fd, resp.data(), resp.size(), 0);
    ::close(client_fd);
}

std::string HttpServer::url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            auto hex = s.substr(i + 1, 2);
            char c = static_cast<char>(std::strtoul(hex.c_str(), nullptr, 16));
            out.push_back(c);
            i += 2;
        } else if (s[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

std::string HttpServer::handle_request(const std::string& request) {
    std::istringstream iss(request);
    std::string method, target, version;
    iss >> method >> target >> version;

    if (method != "GET") return http_404();

    std::string path = target;
    std::string query;
    if (auto pos = target.find('?'); pos != std::string::npos) {
        path = target.substr(0, pos);
        query = target.substr(pos + 1);
    }

    if (path == "/health") return "ok";

    auto q = parse_query(query);

    if (path == "/set") {
        auto itK = q.find("key");
        auto itV = q.find("value");
        if (itK != q.end() && itV != q.end()) {
            g_store.set(itK->second, itV->second, std::nullopt);
            return "ok";
        }
        return http_404();
    }

    if (path == "/get") {
        auto itK = q.find("key");
        if (itK != q.end()) {
            std::string v;
            if (g_store.get(itK->second, v)) return v;
            return "not found";
        }
        return http_404();
    }

    if (path == "/del") {
        auto itK = q.find("key");
        if (itK != q.end()) {
            if (g_store.erase(itK->second)) return "ok";
            return "not found";
        }
        return http_404();
    }

    return http_404();
}