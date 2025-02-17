#include <iostream>
#include <thread>
#include <vector>
#include "library/configlib.h"
#include <filesystem>
#include <cstdlib>

const string __mysql_logger__ = "mysql/logger/";
const string __image__ = "image/";

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace ConfigServer;
using ordered_json = nlohmann::ordered_json;

struct BlockData {
    string name_block;
    string ip_device;
    string images;
    string sensors;

    void clear_all() {
        name_block.clear();
        ip_device.clear();
        images.clear();
        sensors.clear();
    }

} block_data;

ServerSystemConfig config = parseConfig("server_config.json");

void save_to_database(const ServerSystemConfig& config) {
    for (const auto& block : config.settings.blocks)
        if (block.enabled)
            for (const auto& type : block.permissions.allowed_types)
                if ((type == "image" && block_data.images.empty()) ||
                    (type == "sensor" && block_data.sensors.empty()))
                    //cout << "Нет данных для сохранения в базу данных." << endl;
                    return;

    ordered_json block_data_json;
    block_data_json["name_block"] = block_data.name_block;
    block_data_json["ip_device"] = block_data.ip_device;
    block_data_json["data_format"] = getCurrentDate(config.settings.formatting.date_format);
    block_data_json["time_format"] = getCurrentTime(config.settings.formatting.time_format);
    block_data_json["images"] = block_data.images;
    block_data_json["sensors"] = block_data.sensors;
    save_mysql_logger(block_data_json, block_data.ip_device, config.settings.formatting.date_format, config.settings.formatting.time_format, block_data.name_block, __mysql_logger__, config.settings.timestamp_filename);

    block_data_json.clear();
    block_data.clear_all();
}

void configure_server(httplib::Server& svr, const ServerSystemConfig& config) {
    for (const auto& block : config.settings.blocks) {
        if (!block.enabled) continue;
        if (config.settings.create_directory_if_missing) {
            if (!fs::exists(__mysql_logger__))
                fs::create_directories(__mysql_logger__);
            if (!fs::exists(__image__))
                fs::create_directory(__image__);
            if (!fs::exists(__image__ + block.name))
                fs::create_directory(__image__ + block.name);
        }

        for (const auto& type : block.permissions.allowed_types) {
            svr.Post("/" + type + "/" + block.name, [&, block, type](const httplib::Request& req, httplib::Response& res) {
                auto client_ip = req.remote_addr;
                auto authHeader = req.get_header_value("Authorization");

                if (authHeader.empty() || authHeader != block.security.token) {
                    res.status = 401;
                    res.set_content("Ваши данные не были доставлены из - за неизвестного токена на сервере.", "text/plain");
                    return;
                }

                if (block.security.enabled_ip_check)
                    for (const auto& allowed_ips : block.security.allowed_ips)
                        if (!is_ip_allowed(client_ip, allowed_ips)) {
                            res.status = 403;
                            res.set_content("Ваш клиент не может подключиться к серверу из-за неразрешенного адреса клиента.", "text/plain");
                            return;
                        }

                // SENSOR БЫЛ ПЕРЕНЕСЁН В MYSQL/LOGGER!!!!!!!!!!

                if (type == "sensor") {
                    auto data = ordered_json::parse(req.body);
                    block_data.name_block = block.name;
                    block_data.ip_device = client_ip;
                    block_data.sensors = data.dump();
                }
                else if (type == "image") {
                    auto camera_number_header = req.get_header_value("Camera-Number");
                    int camera_number = 0;

                    if (!camera_number_header.empty()) {
                        try {
                            camera_number = stoi(camera_number_header);
                        }
                        catch (const std::invalid_argument& e) {
                            res.status = 400;
                            res.set_content("Неверный номер камеры", "text/plain");
                            return;
                        }
                    }
                    else {
                        res.status = 400;
                        res.set_content("Заголовок Camera-Number отсутствует", "text/plain");
                        return;
                    }

                    string filename = save_image_path(client_ip, config.settings.formatting.date_format, config.settings.formatting.time_format, block.name, __image__, config.settings.timestamp_filename, camera_number);
                    block_data.name_block = block.name;
                    block_data.ip_device = client_ip;
                    block_data.images += "http://rosatom/server/" + filename + ",";
                    save_image(req.body, filename);
                }
                });
        }
    }
}

int main() {
    setlocale(LC_ALL, "ru");

    httplib::Server svr;
    thread serverThread([&]() { configure_server(svr, config); });

    thread databaseThread([&]() {
        while (true) {
            this_thread::sleep_for(chrono::milliseconds(20));
            save_to_database(config);
        }
        });

    cout << "HTTP Server listening on http://" << config.server.address << ":" << config.server.port << endl;
    svr.listen(config.server.address.c_str(), config.server.port);

    serverThread.join();
    databaseThread.join();

    system("pause");

    return 0;
}