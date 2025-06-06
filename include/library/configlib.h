#ifndef CONFIGLIB_H
#define CONFIGLIB_H

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>

#ifdef _WIN32
#include <nlohmann/json.hpp>
#include "library/httplib.h"
#else
#include "nlohmann/json.hpp"
#include "library/httplib.h"
#include <opencv2/opencv.hpp>
#endif

using namespace std;
using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

namespace ConfigServer {
    struct ServerConfig {
        string address;
        int port;
    };
    struct MySQLConfig {
        bool enabled;
    };
    struct SettingsConfig {
        struct MultiClientBlock {
            bool enabled;
            string name;
            struct SecurityConfig {
                string token;
                bool enabled_ip_check;
                vector<string> allowed_ips;
            } security;
            struct PermissionsConfig {
                vector<string> allowed_types;
            } permissions;
        };
        vector<MultiClientBlock> blocks;
        struct DataTimeFormat {
            string date_format;
            string time_format;
        } formatting;
        bool create_directory_if_missing;
        bool timestamp_filename;
    };
    struct ServerSystemConfig {
        ServerConfig server;
        MySQLConfig mysql;
        SettingsConfig settings;
    };

    struct ServerAllowedScriptConfig {
        bool enabled;
        string path;
    };

    string getCurrentDate(const string& date_format) {
        auto now = chrono::system_clock::now();
        time_t currentTime = chrono::system_clock::to_time_t(now);

        tm localTime;
#ifdef _WIN32
        localtime_s(&localTime, &currentTime);
#else
        localtime_r(&currentTime, &localTime);
#endif

        stringstream ss;
        ss << put_time(&localTime, date_format.c_str());
        return ss.str();
    }

    string getCurrentTime(const string& time_format) {
        auto now = chrono::system_clock::now();
        time_t currentTime = chrono::system_clock::to_time_t(now);

        tm localTime;
#ifdef _WIN32
        localtime_s(&localTime, &currentTime);
#else
        localtime_r(&currentTime, &localTime);
#endif

        stringstream ss;
        ss << put_time(&localTime, time_format.c_str());
        return ss.str();
    }

    ServerSystemConfig parseConfig(const string& configPath) {
        try {
            ifstream configFile(configPath);
            ordered_json configJson;
            configFile >> configJson;

            ServerSystemConfig config;
            config.server.address = configJson["server"]["address"].get<string>();
            config.server.port = configJson["server"]["port"].get<int>();

            config.mysql.enabled = configJson["mysql"]["enabled"].get<bool>();

            for (const auto& block : configJson["settings"]["multi_client"]["blocks"]) {
                SettingsConfig::MultiClientBlock clientBlock;
                clientBlock.enabled = block["enabled"].get<bool>();
                clientBlock.name = block["name"].get<string>();
                clientBlock.security.token = block["security"]["token"].get<string>();
                clientBlock.security.enabled_ip_check = block["security"]["enabled_ip_check"].get<bool>();
                for (const auto& allowed_ips : block["security"]["allowed_ips"]) {
                    clientBlock.security.allowed_ips.push_back(allowed_ips.get<string>());
                }
                for (const auto& allowed_type : block["permissions"]["allowed_types"]) {
                    clientBlock.permissions.allowed_types.push_back(allowed_type.get<string>());
                }
                config.settings.blocks.push_back(clientBlock);
            }

            config.settings.create_directory_if_missing = configJson["settings"]["multi_client"]["create_directory_if_missing"].get<bool>();
            config.settings.timestamp_filename = configJson["settings"]["multi_client"]["timestamp_filename"].get<bool>();

            config.settings.formatting.date_format = configJson["settings"]["formatting"]["date_format"].get<string>();
            config.settings.formatting.time_format = configJson["settings"]["formatting"]["time_format"].get<string>();

            return config;
        }
        catch (const ordered_json::parse_error& e) {
            cerr << "[ " << configPath << "] " << "JSON parse error: " << e.what() << endl;
            throw;
        }
    }

    bool save_mysql_logger(const ordered_json& data, const string& client_ip, const string& date_format, const string& time_format, const string& name, const string& directory_logger, bool timestamp_filename) {
        string filename = directory_logger + "/" + name + "_mysql";
        //if (timestamp_filename)
        //    filename += "_" + getCurrentDate(date_format);
        filename += ".json";

        ofstream file(filename, ios::app);
        if (file.is_open()) {
            file << data.dump(4) << endl << endl;
            file.close();
            cout << "!MYSQL_LOG >> Log saved as " << filename << endl;
            return true;
        }
        else {
            cerr << "!MYSQL_ERR >> Failed to save file to: " << filename << endl;
            return false;
        }
    }

    string save_image_path(const string& client_ip, const string& date_format, const string& time_format, const string& name, const string& directory_image, bool timestamp_filename, int camera_number) {
        string filename = directory_image + "/" + name + "_camera_" + to_string(camera_number) + "_" + client_ip;
        if (timestamp_filename)
            filename += "_" + getCurrentDate(date_format) + "_" + getCurrentTime(time_format);
        filename += ".png";
        return filename;
    }

    bool save_image(const string& image_data, const string& filename) {
        ofstream out(filename, ios::binary);
        if (out) {
            out.write(image_data.c_str(), image_data.size());
            cout << "!LOG >> Image saved as " << filename << endl;
            return true;
        }
        else {
            cerr << "!ERR >> Failed to save image to " << filename << endl;
            return false;
        }
    }

    bool is_ip_allowed(const string& client_ip, const string& allowed_ip) {
        return client_ip == allowed_ip;
    }
}
#ifdef __linux__
namespace ConfigClient {
    struct ClientConnectionConfig {
        string address;
        int port;
        int delay_resend;
    };
    struct ReceiveHttpDataConfig {
        bool enabled;
        string address;
        int port;
        string serial;
        int baudrate;
        int delay_receipt;
    };
    struct OpenCVConfig {
        bool enabled;
        int limit_cameras;
        string image_format;
    };
    struct PinConfig {
        int pin;
        string purpose;
    };
    struct ConditionsConfig {
        string action;
        int pin;
        int value;
        string when;
    };
    struct SensorConfig {
        bool enabled;
        string name;
        string location;
        string id;
        string type;
        vector<PinConfig> pins;
        struct Thresholds {
            double normal;
            double warning;
            double critical;
        } thresholds;
    };
    struct ComponentConfig {
        bool enabled;
        string id;
        string name;
        string location;
        vector<PinConfig> pins;
        vector<ConditionsConfig> conditions;
    };
    struct SecurityConfig {
        string name;
        string token;
    };
    struct MonitoringSystemConfig {
        ClientConnectionConfig client;
        ReceiveHttpDataConfig receive_http_data;
        OpenCVConfig opencv;
        vector<SensorConfig> sensor;
        vector<ComponentConfig> components;
        SecurityConfig security;
    };

    MonitoringSystemConfig parseConfig(const string& configPath) {
        try {
            ifstream configFile(configPath);
            ordered_json configJson;
            configFile >> configJson;

            MonitoringSystemConfig config;

            // �������� ������� ������
            if (configJson.contains("client")) {
                config.client.address = configJson["client"]["address"].get<string>();
                config.client.port = configJson["client"]["port"].get<int>();
                config.client.delay_resend = configJson["client"]["delay_resend"].get<int>();
            }

            if (configJson.contains("receive_http_data")) {
                config.receive_http_data.enabled = configJson["receive_http_data"]["enabled"].get<bool>();
                config.receive_http_data.address = configJson["receive_http_data"]["address"].get<string>();
                config.receive_http_data.port = configJson["receive_http_data"]["port"].get<int>();
                config.receive_http_data.serial = configJson["receive_http_data"]["serial"].get<string>();
                config.receive_http_data.baudrate = configJson["receive_http_data"]["baudrate"].get<int>();
                config.receive_http_data.delay_receipt = configJson["receive_http_data"]["delay_receipt"].get<int>();
            }

            if (configJson.contains("opencv")) {
                config.opencv.enabled = configJson["opencv"]["enabled"].get<bool>();
                config.opencv.limit_cameras = configJson["opencv"]["limit_cameras"].get<int>();
                config.opencv.image_format = configJson["opencv"]["image_format"].get<string>();
            }

            if (configJson.contains("sensor")) {
                for (const auto& sensorJson : configJson["sensor"]) {
                    SensorConfig sensor;
                    sensor.enabled = sensorJson["enabled"].get<bool>();
                    sensor.name = sensorJson["name"].get<string>();
                    sensor.location = sensorJson["location"].get<string>();
                    sensor.id = sensorJson["id"].get<string>();
                    sensor.type = sensorJson["type"].get<string>();

                    // �������� �� ������� ����� "pins"
                    if (sensorJson.contains("pins")) {
                        for (const auto& pinJson : sensorJson["pins"]) {
                            PinConfig pin;
                            pin.pin = pinJson["pin"].get<int>();
                            pin.purpose = pinJson["purpose"].get<string>();
                            sensor.pins.push_back(pin);
                        }
                    }

                    // �������� �� ������� ����� "thresholds"
                    if (sensorJson.contains("thresholds")) {
                        sensor.thresholds.normal = sensorJson["thresholds"]["normal"].get<double>();
                        sensor.thresholds.warning = sensorJson["thresholds"]["warning"].get<double>();
                        sensor.thresholds.critical = sensorJson["thresholds"]["critical"].get<double>();
                    }

                    config.sensor.push_back(sensor);
                }
            }

            if (configJson.contains("components")) {
                for (const auto& componentJson : configJson["components"]) {
                    ComponentConfig component;
                    component.enabled = componentJson["enabled"].get<bool>();
                    component.name = componentJson["name"].get<string>();
                    component.location = componentJson["location"].get<string>();
                    component.id = componentJson["id"].get<string>();

                    // �������� �� ������� ����� "pins"
                    if (componentJson.contains("pins")) {
                        for (const auto& pinJson : componentJson["pins"]) {
                            PinConfig pin;
                            pin.pin = pinJson["pin"].get<int>();
                            pin.purpose = pinJson["purpose"].get<string>();
                            component.pins.push_back(pin);
                        }
                    }

                    // �������� �� ������� ����� "conditions"
                    if (componentJson.contains("conditions")) {
                        for (const auto& conditionsJson : componentJson["conditions"]) {
                            ConditionsConfig conditions_config;
                            conditions_config.action = conditionsJson["action"].get<string>();
                            conditions_config.pin = conditionsJson["pin"].get<int>();
                            conditions_config.value = conditionsJson["value"].get<int>();
                            conditions_config.when = conditionsJson["when"].get<string>();
                            component.conditions.push_back(conditions_config);
                        }
                    }

                    config.components.push_back(component);
                }
            }

            if (configJson.contains("security")) {
                config.security.name = configJson["security"]["name"].get<string>();
                config.security.token = configJson["security"]["token"].get<string>();
            }

            return config;
        }
        catch (const ordered_json::parse_error& e) {
            cerr << "JSON parse error: " << e.what() << endl;
        }
        catch (const ordered_json::type_error& e) {
            cerr << "JSON type error: " << e.what() << endl;
        }

        return MonitoringSystemConfig();
    }
}
#endif

/*
struct VersionConfig {
    string version;
    string repository_url;
    bool auto_update;
    string last_checked;
};

VersionConfig parseVersionConfig(const string configPath) {
    try {
        ifstream configFile(configPath);
        ordered_json configJson;
        configFile >> configJson;

        VersionConfig config;
        config.version = configJson["version"].get<string>();
        config.repository_url = configJson["repository_url"].get<string>();
        config.auto_update = configJson["auto_update"].get<bool>();
        config.last_checked = configJson["last_checked"].get<string>();
    }
    catch (const ordered_json::parse_error& e) {
        cerr << "[ " << configPath << "] " << "JSON parse error: " << e.what() << endl;
        throw;
    }

    return VersionConfig();
}
*/

#endif // !CONFIGLIB_H
