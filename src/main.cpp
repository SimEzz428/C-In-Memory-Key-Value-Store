#include "http_server.h"

int main() {
    HttpServer srv(8080);
    srv.run();
    return 0;
}