#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <curl/curl.h>
#include <zip.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace std;

#define CONFIG_PATH "version_info.json"
#define VERSION_SUFFIX "^ver"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ofstream* out = static_cast<ofstream*>(userp);
    size_t totalSize = size * nmemb;
    out->write(static_cast<const char*>(contents), totalSize);
    return totalSize;
}

void download_file(const string& url, const string& output_path) {
    try {
        CURL* curl;
        CURLcode res;
        ofstream file(output_path, ios::binary);
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                throw runtime_error(curl_easy_strerror(res));
            }
            file.close();
            cout << "Файл обновления скачан." << endl;
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }
    catch (const exception& e) {
        cerr << "Ошибка при скачивании файла: " << e.what() << endl;
    }
}

void unzip(const string& zip_file_path, const string& dest_dir) {
    try {
        int err = 0;
        zip_t* archive = zip_open(zip_file_path.c_str(), 0, &err);
        if (!archive) {
            throw runtime_error("Ошибка при открытии ZIP файла.");
        }

        zip_int64_t num_entries = zip_get_num_entries(archive, 0);
        for (zip_int64_t i = 0; i < num_entries; ++i) {
            const char* name = zip_get_name(archive, i, 0);
            string full_path = dest_dir + "/" + name;

            zip_file_t* zf = zip_fopen(archive, name, 0);
            if (!zf) {
                zip_close(archive);
                throw runtime_error("Ошибка при открытии файла в ZIP.");
            }

            ofstream out(full_path, ios::binary);
            char buffer[8192];
            zip_int64_t n;
            while ((n = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
                out.write(buffer, n);
            }
            zip_fclose(zf);
            out.close();
            this_thread::sleep_for(chrono::milliseconds(50)); // Имитация задержки
        }
        zip_close(archive);
        cout << "Обновление распаковано." << endl;
    }
    catch (const exception& e) {
        cerr << "Ошибка при распаковке ZIP файла: " << e.what() << endl;
    }
}

void move_old_files(const vector<string>& files_to_move, const string& tmp_dir) {
    fs::create_directory(tmp_dir);
    for (const auto& file_path : files_to_move) {
        if (fs::exists(file_path))
            fs::rename(file_path, tmp_dir + "/" + fs::path(file_path).filename().string());
        else
            cout << "Файл " << file_path << " не найден." << endl;
    }
}

vector<string> get_version_list(const string& directory) {
    vector<string> versions;
    for (const auto& entry : fs::directory_iterator(directory))
        if (entry.is_directory() && entry.path().filename().string().find(VERSION_SUFFIX) != string::npos)
            versions.push_back(entry.path().filename().string());
    return versions;
}

vector<string> choose_versions(const vector<string>& available_versions) {
    cout << "Доступные версии:" << endl;
    for (size_t i = 0; i < available_versions.size(); ++i)
        cout << i + 1 << ". " << available_versions[i] << endl;

    string input;
    cout << "Введите номера версий, которые нужно оставить (через запятую): ";
    getline(cin, input);

    vector<string> chosen_versions;
    stringstream ss(input);
    string token;

    while (getline(ss, token, ',')) {
        int index = stoi(token) - 1;
        if (index >= 0 && index < available_versions.size())
            chosen_versions.push_back(available_versions[index]);
    }
    return chosen_versions;
}

void move_files_from_single_version(const string& version_path) {
    for (const auto& entry : fs::directory_iterator(version_path))
        if (entry.is_regular_file())
            fs::rename(entry.path(), fs::current_path() / entry.path().filename());
    fs::remove_all(version_path);
}

void remove_unwanted_versions(const string& directory, const vector<string>& chosen_versions) {
    auto all_versions = get_version_list(directory);
    if (chosen_versions.size() == 1)
        move_files_from_single_version(directory + "/" + chosen_versions[0]);
    for (const auto& version : all_versions)
        if (find(chosen_versions.begin(), chosen_versions.end(), version) == chosen_versions.end()) {
            fs::remove_all(directory + "/" + version);
            cout << "Версия " << version << " удалена." << endl;
        }
}

void update_application(const string& zip_url, const vector<string>& files_to_move) {
    const string zip_file_path = "update.zip";
    const string tmp_dir = "old";

    download_file(zip_url, zip_file_path);

    if (!files_to_move.empty())
        move_old_files(files_to_move, tmp_dir);

    unzip(zip_file_path, ".");

    try {
        fs::remove(zip_file_path);
        cout << "Файл обновления удален." << endl;
    }
    catch (const exception& e) {
        cerr << "Ошибка при удалении файла обновления: " << e.what() << endl;
    }

    auto available_versions = get_version_list(".");
    if (!available_versions.empty()) {
        auto chosen_versions = choose_versions(available_versions);
        remove_unwanted_versions(".", chosen_versions);
        cout << "Загруженные версии: ";
        for (const auto& version : chosen_versions)
            cout << version << " ";
        cout << endl;
    }
    else
        cout << "Нет доступных версий для выбора." << endl;
}

int main() {
    try {
        ifstream input_file(CONFIG_PATH);
        json json_data;
        input_file >> json_data;

        string zip_url = json_data["repository_url"];
        vector<string> files_to_move = json_data["files_to_move"].get<vector<string>>();

        update_application(zip_url, files_to_move);
    }
    catch (const exception& e) {
        cerr << "Ошибка при открытии файла: " << e.what() << endl;
    }
    return 0;
}