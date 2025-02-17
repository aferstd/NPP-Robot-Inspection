import os
import json
import requests
import zipfile
import shutil
import time
from tqdm import tqdm

CONFIG_PATH = "version_info.json"
VERSION_SUFFIX = "^ver"

def download_file(url, output_path):
    try:
        response = requests.get(url, stream=True)
        response.raise_for_status()
        total_size = int(response.headers.get('content-length', 0))

        with open(output_path, 'wb') as file, tqdm(
            desc="Скачивание файла",
            total=total_size,
            unit='B',
            unit_scale=True,
            unit_divisor=1024,
        ) as bar:
            for data in response.iter_content(chunk_size=1024):
                file.write(data)
                bar.update(len(data))
                time.sleep(0.006)

        print("Файл обновления скачан.")
    except Exception as e:
        print(f"Ошибка при скачивании файла: {e}")

def unzip(zip_file_path, dest_dir):
    try:
        with zipfile.ZipFile(zip_file_path, 'r') as zip_ref:
            total_files = len(zip_ref.namelist())
            with tqdm(total=total_files, desc="Распаковка обновления", unit='файл') as bar:
                for file in zip_ref.namelist():
                    zip_ref.extract(file, dest_dir)
                    bar.update(1)
                    time.sleep(0.05)
        print("Обновление распаковано.")
    except Exception as e:
        print(f"Ошибка при распаковке ZIP файла: {e}")

def move_old_files(files_to_move, tmp_dir):
    os.makedirs(tmp_dir, exist_ok=True)
    for file_path in files_to_move:
        if os.path.exists(file_path):
            shutil.move(file_path, os.path.join(tmp_dir, os.path.basename(file_path)))
        else:
            print(f"Файл {file_path} не найден.")

def get_version_list(directory):
    return [name for name in os.listdir(directory) if os.path.isdir(os.path.join(directory, name)) and VERSION_SUFFIX in name]

def choose_versions(available_versions):
    print("Доступные версии:")
    for i, version in enumerate(available_versions, 1):
        print(f"{i}. {version}")

    chosen_indices = input("Введите номера версий, которые нужно оставить (через запятую): ")
    chosen_indices = [int(x.strip()) - 1 for x in chosen_indices.split(",")]

    chosen_versions = [available_versions[i] for i in chosen_indices if i in range(len(available_versions))]
    return chosen_versions

def move_files_from_single_version(version_path):
    for root, _, files in os.walk(version_path):
        for file in files:
            shutil.move(os.path.join(root, file), os.getcwd())
    shutil.rmtree(version_path)

def remove_unwanted_versions(directory, chosen_versions):
    all_versions = get_version_list(directory)
    if len(chosen_versions) == 1:
        move_files_from_single_version(os.path.join(directory, chosen_versions[0]))
    for version in all_versions:
        if version not in chosen_versions:
            shutil.rmtree(os.path.join(directory, version))
            print(f"Версия {version} удалена.")

def update_application(zip_url, files_to_move):
    zip_file_path = "update.zip"
    tmp_dir = "old"

    download_file(zip_url, zip_file_path)

    if files_to_move:
        move_old_files(files_to_move, tmp_dir)

    unzip(zip_file_path, ".")

    try:
        os.remove(zip_file_path)
        print("Файл обновления удален.")
    except Exception as e:
        print(f"Ошибка при удалении файла обновления: {e}")

    available_versions = get_version_list(".")

    if available_versions:
        chosen_versions = choose_versions(available_versions)
        remove_unwanted_versions(".", chosen_versions)
        print(f"Загруженные версии: {', '.join(chosen_versions)}")
    else:
        print("Нет доступных версий для выбора.")

def main():
    try:
        with open(CONFIG_PATH, 'r') as input_file:
            json_data = json.load(input_file)
            zip_url = json_data["repository_url"]
            files_to_move = json_data["files_to_move"]
    except Exception as e:
        print(f"Ошибка при открытии файла: {e}")
        return

    update_application(zip_url, files_to_move)
    #os.system("pause")

if __name__ == "__main__":
    main()
