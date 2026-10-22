#include <httplib.h>

int main() {
    httplib::Server svr;

    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.status = 200;
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    const char* host = "0.0.0.0";
    const int port = 8080;
    svr.listen(host, port);
    return 0;
}