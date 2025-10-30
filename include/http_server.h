#pragma once
#include <string>
#include <vector>
#include <thread>
#include <netinet/in.h>

class HttpServer {
public:
    explicit HttpServer(int port);
    ~HttpServer();
    void run();
    static std::string url_decode(const std::string& s);
private:
    void handle_client(int client_fd);
    std::string handle_request(const std::string& request);

    int port_;
    int server_fd_;
    sockaddr_in addr_;
    std::vector<std::thread> workers_;
};