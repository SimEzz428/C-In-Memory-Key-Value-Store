#pragma once
#include <netinet/in.h>
#include <string>
#include <thread>
#include <vector>

class HttpServer {
public:
    explicit HttpServer(int port);
    ~HttpServer();
    void run();
    static std::string url_decode(const std::string& s);

private:
    int port_;
    int server_fd_;
    sockaddr_in addr_;
    std::vector<std::thread> workers_;
    void handle_client(int client_fd);
    static std::string handle_request(const std::string& request);
};