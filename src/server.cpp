#include <iostream>
#include <thread>
#include <vector>
#include "library/configlib.h"
#include <filesystem>

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace ConfigServer;

ServerSystemConfig config = parseConfig("server_config.json");

void configure_server(httplib::Server& svr, const ServerSystemConfig& config) {
    for (const auto& block : config.settings.blocks) {
        if (!block.enabled) continue;
        if (config.settings.create_directory_if_missing) {
            if (!fs::exists("image"))
                fs::create_directory("image");
            if (!fs::exists("logger"))
                fs::create_directory("logger");
            if (!fs::exists("logger/" + block.name))
                fs::create_directory("logger/" + block.name);
            if (!fs::exists("image/" + block.name))
                fs::create_directory("image/" + block.name);
        }

        for (const auto& type : block.permissions.allowed_types) {
            if (type == "sensor") {
                svr.Post("/sensor/" + block.name, [&, block](const httplib::Request& req, httplib::Response& res) {
                    auto client_ip = req.remote_addr;

                    auto authHeader = req.get_header_value("Authorization");
                    if (authHeader.empty() || authHeader != block.security.token) {
                        res.status = 401;
                        res.set_content("Your data was not delivered due to an unknown token on the server.", "text/plain");
                        return;
                    }

                    if (block.security.enabled_ip_check)
                        for (const auto& allowed_ips : block.security.allowed_ips)
                            if (!is_ip_allowed(client_ip, allowed_ips)) {
                                res.status = 403;
                                res.set_content("Your client cannot connect to the server due to an unresolved client address.", "text/plain");
                                return;
                            }

                    auto data = json::parse(req.body);

                    save_logger(data, client_ip, config.settings.formatting.date_format, config.settings.formatting.time_format, block.name, block.directory.logger, config.settings.timestamp_filename);
                    res.set_content(data.dump(), "application/json");
                    });
            }

            if (type == "image") {
                svr.Post("/image/" + block.name, [&, block](const httplib::Request& req, httplib::Response& res) {
                    auto client_ip = req.remote_addr;

                    auto authHeader = req.get_header_value("Authorization");
                    if (authHeader.empty() || authHeader != block.security.token) {
                        res.status = 401;
                        res.set_content("Your data was not delivered due to an unknown token on the server.", "text/plain");
                        return;
                    }

                    if (block.security.enabled_ip_check)
                        for (const auto& allowed_ips : block.security.allowed_ips)
                            if (!is_ip_allowed(client_ip, allowed_ips)) {
                                res.status = 403;
                                res.set_content("Your client cannot connect to the server due to an unresolved client address.", "text/plain");
                                return;
                            }

                    save_image(req.body, client_ip, config.settings.formatting.date_format, config.settings.formatting.time_format, block.name, block.directory.image, config.settings.timestamp_filename);
                    });
            }
        }
    }
}

int main() {
    setlocale(LC_ALL, "ru");

    httplib::Server svr;
    configure_server(svr, config);

    cout << "HTTP Server listening on http://" << config.server.address << ":" << config.server.port << endl;
    svr.listen(config.server.address.c_str(), config.server.port);

    return 0;
}
