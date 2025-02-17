#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include "library/configlib.h"
#include "library/httplib.h"
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <string>

#define NUM_READINGS 10

using namespace std;
using namespace cv;
using namespace ConfigClient;
using json = nlohmann::json;
namespace fs = std::filesystem;
using ordered_json = nlohmann::ordered_json;

MonitoringSystemConfig config = parseConfig("client_config.json");

void send_http_request(const MonitoringSystemConfig& config, const ordered_json& sensor_data, string path) {
    httplib::Client cli(config.client.address, config.client.port);
    httplib::Headers headers = {
        { "Authorization", config.security.token }
    };

    auto res = cli.Post((path + config.security.name).c_str(), headers, sensor_data.dump(), "application/json");
    if (res) {
        cout << "Server response: " << res->body << endl;
    }
    else {
        cerr << "Failed to connect to server" << endl;
    }
}

void send_image_http_request(const MonitoringSystemConfig& config, const string& imagePath, string path) {
    ifstream file(imagePath, ios::binary);
    if (!file) {
        cerr << "Failed to open image file: " << imagePath << endl;
        return;
    }

    vector<char> buf((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    if (buf.empty()) {
        cerr << "Failed to read image data from file." << endl;
        return;
    }

    httplib::Client cli(config.client.address, config.client.port);
    httplib::Headers headers = {
        { "Authorization", config.security.token }
    };

    auto res = cli.Post((path + config.security.name).c_str(), headers, buf.data(), buf.size(), "application/octet-stream");
    if (res && res->status == 200)
        cout << "Image sent successfully: " << res->body << endl;
    else
        cerr << "Failed to send image. Status code: " << res->status << " Body: " << res->body << endl;
}

void send_image_http_request_opencv(const MonitoringSystemConfig& config, const Mat& frame, const string& path, int camera_number) {
    vector<uchar> buf;
    vector<int> compression_params;

    if (config.opencv.image_format == "png")
        compression_params = { IMWRITE_PNG_COMPRESSION, 3 };
    else if (config.opencv.image_format == "jpeg" || config.opencv.image_format == "jpg")
        compression_params = { IMWRITE_JPEG_QUALITY, 90 };
    else
        cerr << "Error: Unsupported image format." << endl;

    imencode(".png", frame, buf, compression_params);

    httplib::Client cli(config.client.address, config.client.port);
    httplib::Headers headers = {
        { "Authorization", config.security.token },
        { "Camera-Number", to_string(camera_number) }
    };

    auto res = cli.Post((path + config.security.name).c_str(), headers, reinterpret_cast<const char*>(buf.data()), buf.size(), "application/octet-stream");
    if (res && res->status == 200)
        cout << "Image sent successfully: " << res->body << endl;
    else
        cerr << "Failed to send image. Status code: " << res->status << " Body: " << res->body << endl;
}

void capture_and_send_image(const MonitoringSystemConfig& config, VideoCapture& cap, string path, int camera_number) {
    Mat frame;
    cap >> frame;

    if (frame.empty()) {
        cerr << "Error: Could not capture frame." << endl;
        return;
    }

    send_image_http_request_opencv(config, frame, path, camera_number);
}

unsigned int measureDistance(int trigPin, int echoPin, bool inMillimeters) {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    float totalDistance = 0;

    for (int i = 0; i < NUM_READINGS; ++i) {
        digitalWrite(trigPin, HIGH);
        this_thread::sleep_for(chrono::microseconds(10));
        digitalWrite(trigPin, LOW);

        while (digitalRead(echoPin) == LOW);
        auto start = chrono::high_resolution_clock::now();
        while (digitalRead(echoPin) == HIGH);
        auto stop = chrono::high_resolution_clock::now();

        auto duration = chrono::duration_cast<chrono::microseconds>(stop - start).count();
        float distance = (duration * 0.0343) / 2;
        totalDistance += distance;
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    float avgDistance = totalDistance / NUM_READINGS;
    return inMillimeters ? avgDistance * 10 : avgDistance;
}

ordered_json load_sensor_data() {
    ordered_json sensor_data;
    for (const auto& sensor : config.sensor) {
        if (sensor.enabled) {
            ordered_json sensor_info;
            sensor_info["name"] = sensor.name;
            sensor_info["location"] = sensor.location;
            sensor_info["id"] = sensor.id;
            sensor_info["type"] = sensor.type;

            for (const auto& pinConfig : sensor.pins) {
                ordered_json pin_info;
                pin_info["pin"] = pinConfig.pin;
                pin_info["purpose"] = pinConfig.purpose;
                sensor_info["pins"].push_back(pin_info);
            }

            if (sensor.id == "ultrasonic") {
                int trigPin = -1;
                int echoPin = -1;

                for (const auto& pinConfig : sensor.pins) {
                    if (pinConfig.purpose == "trigger")
                        trigPin = pinConfig.pin;
                    else if (pinConfig.purpose == "echo")
                        echoPin = pinConfig.pin;
                }

                if (trigPin != -1 && echoPin != -1)
                    sensor_info["value"] = measureDistance(trigPin, echoPin, false);
            }
            else if (sensor.id == "button") {
                int buttonPin = -1;

                for (const auto& pinConfig : sensor.pins) {
                    if (pinConfig.purpose == "out")
                        buttonPin = pinConfig.pin;
                }

                if (buttonPin != -1) {
                    pinMode(buttonPin, INPUT);
                    if (!digitalRead(buttonPin) == LOW)
                        sensor_info["value"] = 1;
                    else
                        sensor_info["value"] = 0;
                }

            }
            else
                sensor_info["value"] = 0;

            if (sensor_info["value"] >= sensor.thresholds.critical) {
                cerr << "ERROR: Sensor '" << sensor.name << "' exceeded critical value: "
                    << sensor_info["value"] << " > " << sensor.thresholds.critical << endl;
                sensor_info["threshold_type"] = "critical";
                sensor_info["threshold_value"] = sensor.thresholds.critical;
            }
            else if (sensor_info["value"] >= sensor.thresholds.warning) {
                cout << "WARNING: Sensor '" << sensor.name << "' exceeded warning value: "
                    << sensor_info["value"] << " > " << sensor.thresholds.warning << endl;
                sensor_info["threshold_type"] = "warning";
                sensor_info["threshold_value"] = sensor.thresholds.warning;
            }
            else {
                sensor_info["threshold_type"] = "normal";
                sensor_info["threshold_value"] = sensor.thresholds.normal;
            }

            sensor_data["sensors"].push_back(sensor_info);
            //sensor_data.push_back(sensor_info);
        }
    }
    return sensor_data;
}

/*
void load_component_data(const ordered_json& sensor_data) {
    for (const auto& component : config.components) {
        if (component.enabled) {
            int controlPin = -1;
            int dirPin = -1;
            int stepPin = -1;

            for (const auto& pinConfig : component.pins) {
                if (pinConfig.purpose == "control") {
                    controlPin = pinConfig.pin;
                    break;
                }
                else if (pinConfig.purpose == "dir") {
                    dirPin = pinConfig.pin;
                    break;
                }
                else if (pinConfig.purpose == "step") {
                    stepPin = pinConfig.pin;
                    break;
                }
            }

            if (component.id == "servo")
                pinMode(controlPin, OUTPUT);
            else if (component.id == "motor") {
                pinMode(dirPin, OUTPUT);
                pinMode(stepPin, OUTPUT);
            }
            else
                cerr << "ERROR: NONE ID MODE!!!" << endl;

            for (const auto& condition : component.conditions) {
                string sensor_name = condition.when.substr(7, condition.when.find(':', 7) - 7);
                string threshold_type = condition.when.substr(condition.when.find(':', 7) + 1);

                bool condition_met = false;
                for (const auto& sensor : sensor_data["sensors"]) {
                    if (sensor["name"] == sensor_name) {
                        if (threshold_type == "normal" && sensor["value"] >= sensor["threshold_value"])
                            condition_met = true;
                        else if (threshold_type == "warning" && sensor["value"] >= sensor["threshold_value"])
                            condition_met = true;
                        else if (threshold_type == "critical" && sensor["value"] >= sensor["threshold_value"])
                            condition_met = true;
                    }

                    if (condition_met) {
                        if (sensor["threshold_type"] == threshold_type) {
                            if (condition.action == "turn_on") {
                                digitalWrite(condition.pin, 1);
                                cout << component.name << " turned ON." << endl;
                                break;
                            }
                            else if (condition.action == "turn_off") {
                                digitalWrite(condition.pin, 0);
                                cout << component.name << " turned OFF." << endl;
                                break;
                            }
                            else if (condition.action == "turn_motor") {
                                for (int i = 0; i < condition.value; i++) {
                                    digitalWrite(stepPin, HIGH);
                                    delayMicroseconds(1000);
                                    digitalWrite(stepPin, LOW);
                                    delayMicroseconds(1000);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}
*/

//void receive_http_data(const MonitoringSystemConfig& config, string path) {
//    httplib::Client cli(config.receive_http_data.address, config.receive_http_data.port);
//    httplib::Headers headers = {
//        { "Authorization", config.security.token }
//    };
//
//    while (true) {
//        auto res = cli.Get((path + config.security.name).c_str(), headers);
//        if (res) {
//            cout << "Received data: " << res->body << endl;
//        }
//        else {
//            cerr << "Failed to connect to server" << endl;
//        }
//
//        if (config.client.delay_resend > 0)
//            this_thread::sleep_for(chrono::milliseconds(config.receive_http_data.delay_receipt));
//    }
//}


void request_send_data(const MonitoringSystemConfig& config) {
    httplib::Server server;

    server.Post("/send_data/", [config](const httplib::Request& req, httplib::Response& res) {
        try {
            ordered_json received_data = json::parse(req.body);
            string json_string = received_data.dump();
            cout << received_data.dump(4) << endl;
            int serial_port = serialOpen((config.receive_http_data.serial).c_str(), config.receive_http_data.baudrate);

            if (serial_port < 0) {
                cerr << "Error Open Serial Arduino!!" << endl;
                return;
            }

            serialPuts(serial_port, json_string.c_str());

            serialClose(serial_port);

            res.set_content("Sus Data!!", "text/plain");

            if (config.client.delay_resend > 0) {
                this_thread::sleep_for(chrono::milliseconds(config.receive_http_data.delay_receipt));
            }
        }
        catch (const exception& e) {
            res.status = 400;
            // res.set_content("Ошибка обработки данных", "text/plain");
             //cout << "Error data!!" << endl;
        }
        });

    cout << "Сервер запущен на порту " << config.receive_http_data.address << ":" << config.receive_http_data.port << endl;
    server.listen(config.receive_http_data.address, config.receive_http_data.port);
}

int main() {
    setlocale(LC_ALL, "ru");

    if (wiringPiSetup() == -1) {
        cerr << "Failed to initialize wiringPi" << endl;
        return 1;
    }

    vector<VideoCapture> cameras;
    {
        if (config.opencv.enabled) {
            for (int i = 0; i < config.opencv.limit_cameras; ++i) {
                VideoCapture cap(i);
                if (cap.isOpened()) {
                    cameras.push_back(move(cap));
                    cout << "Camera " << i << " connected successfully." << endl;
                }
                else
                    cout << "Camera " << i << " could not be opened." << endl;
            }
        }
        if (cameras.empty() && config.opencv.enabled) {
            cerr << "Error: No cameras found." << endl;
            return 0;
        }
    }

    thread requestSendDataThread([&]() {
        request_send_data(config);
        });

    while (true) {
        ordered_json sensor_data = load_sensor_data();
        cout << sensor_data.dump(4) << endl;

        send_http_request(config, sensor_data, "/sensor/");

        if (config.opencv.enabled) {
            for (size_t i = 0; i < cameras.size(); ++i) {
                capture_and_send_image(config, cameras[i], "/image/", i);
            }
        }

        if (config.client.delay_resend > 0) {
            this_thread::sleep_for(chrono::milliseconds(config.client.delay_resend));
        }
    }

    requestSendDataThread.join();

    return 0;
}
