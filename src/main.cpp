#include <httplib.h>

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>

#include "store.hpp"

using nlohmann::json;

namespace {

void SendJson(httplib::Response& res, int status, const json& body) {
    res.status = status;
    res.set_content(body.dump(), "application/json");
}

}  // namespace

int main() {
    httplib::Server svr;
    ItemStore store;

    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.status = 200;
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    svr.Post("/items", [&store](const httplib::Request& req, httplib::Response& res) {
        json body;
        try {
            body = json::parse(req.body);
        } catch (const json::parse_error&) {
            SendJson(res, 400, {{"error", "invalid json"}});
            return;
        }
        if (!body.contains("name") || !body["name"].is_string()) {
            SendJson(res, 400, {{"error", "name is required"}});
            return;
        }
        Item item = store.Create(body["name"].get<std::string>());
        SendJson(res, 201, {{"id", item.id}, {"name", item.name}});
    });

    svr.Get(R"(/items/(\d+))", [&store](const httplib::Request& req, httplib::Response& res) {
        std::uint64_t id = std::stoull(req.matches[1]);
        auto found = store.Get(id);
        if (!found) {
            SendJson(res, 404, {{"error", "not found"}});
            return;
        }
        SendJson(res, 200, {{"id", found->id}, {"name", found->name}});
    });

    svr.Put(R"(/items/(\d+))", [&store](const httplib::Request& req, httplib::Response& res) {
        std::uint64_t id = std::stoull(req.matches[1]);
        json body;
        try {
            body = json::parse(req.body);
        } catch (const json::parse_error&) {
            SendJson(res, 400, {{"error", "invalid json"}});
            return;
        }
        if (!body.contains("name") || !body["name"].is_string()) {
            SendJson(res, 400, {{"error", "name is required"}});
            return;
        }
        auto updated = store.Update(id, body["name"].get<std::string>());
        if (!updated) {
            SendJson(res, 404, {{"error", "not found"}});
            return;
        }
        SendJson(res, 200, {{"id", updated->id}, {"name", updated->name}});
    });

    svr.listen("0.0.0.0", 8080);
    return 0;
}