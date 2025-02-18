# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude

# Путь к библиотекам и OpenCV
INCLUDES = -I./include -I/usr/include/opencv4
LIBS = -lpthread -lwiringPi -L/usr/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lcurl -lzip

# Директории
BUILD_DIR = build
SERVER_DIR = $(BUILD_DIR)/server
CLIENT_DIR = $(BUILD_DIR)/client
CONFIGS_DIR = configs

# Исходные файлы и объекты
SERVER_SRCS = src/server.cpp
CLIENT_SRCS = src/client.cpp
UPDATE_SRCS = src/update.cpp

SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
UPDATE_OBJS = $(UPDATE_SRCS:.cpp=.o)

# Имена исполняемых файлов и конфигурационных файлов
SERVER_TARGET = $(SERVER_DIR)/server
CLIENT_TARGET = $(CLIENT_DIR)/client
UPDATE_TARGET = $(BUILD_DIR)/update

SERVER_CONFIG = $(CONFIGS_DIR)/server_config.json
SERVER_VERSION = $(CONFIGS_DIR)/version_info.json

CLIENT_CONFIG = $(CONFIGS_DIR)/client_config.json
CLIENT_VERSION = $(CONFIGS_DIR)/version_info.json

# Создание директорий build/server и build/client
$(shell mkdir -p $(SERVER_DIR))
$(shell mkdir -p $(CLIENT_DIR))

# Определение компилируемого проекта
TARGET ?= all

# Правила компиляции
all: $(SERVER_TARGET) $(CLIENT_TARGET) $(UPDATE_TARGET)

server: $(SERVER_TARGET)

client: $(CLIENT_TARGET)

update: $(UPDATE_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	cp $(SERVER_CONFIG) $(SERVER_DIR)/
	cp $(SERVER_VERSION) $(SERVER_DIR)/
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	cp $(CLIENT_CONFIG) $(CLIENT_DIR)/
	cp $(CLIENT_VERSION) $(CLIENT_DIR)/
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(UPDATE_TARGET): $(UPDATE_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)
	cp $@ $(SERVER_DIR)/
	cp $@ $(CLIENT_DIR)/

# Правило для компиляции и удаления объектных файлов
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

# Компиляция на основе переданного параметра
build:
	$(MAKE) $(TARGET)

.PHONY: all server client update clean build