
# Специально для Руслана и Сергея!!!!

1. Сергей включи интернет со своего телефона!!
2. Включите Робота
3. На рабочем столе открой файл "22_05_2025Profile.tlp"
4. Нажми кнопку Login
5. В Левой части программы есть кнопка "New Terminal"
6. Вводи команду чтобы перейти в директорию проекта ```cd /home/orangepi/NPP-Robot-Inspection-main/build/client```
7. Чтобы запустить программу надо ввести команду ```./client```

## Аварийный способ
### Первый способ если программа не запускается иза конфигурации
1. Чтобы отредактировать конфиг программы нужно ввести комманду в директории [build/client] ```nano client_config.json```

2. Сама конфигурация Клиента
```json
{
  "client": {
    "address": "127.0.0.1", # Айпи адрес сервера (ноутбук)
    "port": 5555,  # Порт сервера (не изменять!!)
    "delay_resend": 1000  # Задержка отправки данных (не изменять!!) в мс
  },
  "receive_http_data": {
    "enabled": false,
    "address": "127.0.0.1", # Айпи адрес Orange Pi!!!! Не сервера
    "port": 3030, # Порт клиента (не изменять!!)
    "serial": "/dev/ttyUSB0",
    "baudrate": 9600, # если в друг задерка огромная поменять на 115200!! и на плате Arduino тоже в пунте Serial.begin()
    "delay_receipt": 1000 # Время задерка обработки данных в ми
  },
  "opencv": {
    "enabled": true,
    "limit_cameras": 4, # Количество камер! (если не одна камера не будет работать то программа не запустится!!!!!!!!!!!!!!!!)
    "image_format": "png" # Не изменять!!!
  },
  "sensor": [
    {
      "enabled": true,
      "name": "radision_01",
      "location": "reactor-sensor-001",
      "id": "ultrasonic",
      "type": "CM",
      "pins": [
        {
          "pin": 26,
          "purpose": "trigger"
        },
        {
          "pin": 27,
          "purpose": "echo"
        }
      ],
      "thresholds": {
        "normal": 0,
        "warning": 60,
        "critical": 80
      }
    },
    {
      "enabled": false,
      "name": "button_1",
      "location": "button-sensor-001",
      "id": "button",
      "type": "none",
      "pins": [
        {
          "pin": 27,
          "purpose": "out"
        }
      ],
      "thresholds": {
        "normal": 0,
        "warning": 1,
        "critical": 2
      }
    }
  ],
  "security": {
    "name": "block_test_1", # ИМЯ ДОЛЖНО СОВПОДАТЬ И НА КЛИЕНТЕ И НА СЕРВЕРЕ ДЛЯ РАБОТЫ!!!
    "token": "your-token" # ТОЖЕ САМОЕ И С ТОКЕНОМ!!!!
  }
}
```

Конфигурация Сервера
```json
{
	"server": {
		"address": "192.168.1.104", # Айпи адрес сервера (ноутбука)
		"port": 5555  # Порт сервера (не изменять!!)
	},
	"mysql": { # НЕ ТРОГАТЬ!!
		"enabled": true,
		"directory": "mysql/logger"
	},
	"settings": {
		"multi_client": {
			"enabled": true,
			"blocks": [
				{
					"enabled": false,
					"name": "block_center_1", # ИМЯ ДОЛЖНО СОВПОДАТЬ С ORANGEPI И СЕРВЕРОМ!!
					"directory": {
						"logger": "logger/block_center_1",
						"image": "image/block_center_1"
					},
					"security": {
						"token": "hsjsfdhnsdnbsdjdsh", # ТОКЕН ТОЖЕ ДОЛЖЕН СОВПОДАТЬ С ORANGEPI И СЕРВЕРОМ!!
						"enabled_ip_check": false,
						"allowed_ips": [ "localhost" ]
					},
					"permissions": {
						"allowed_types": [ "sensor" ] # есть sensor и image!!
					}
				},
				{
					"enabled": true,
					"name": "block_left_1",
					"directory": {
						"logger": "logger/block_left_1",
						"image": "image/block_left_1"
					},
					"security": {
						"token": "hsjsfdhnsdnbsdjdsh",
						"enabled_ip_check": false,
						"allowed_ips": [ "localhost", "127.0.0.1", "192.168.1.105" ]
					},
					"permissions": {
						"allowed_types": [ "sensor", "image" ]
					}
				}
			],
			"create_directory_if_missing": true,
			"timestamp_filename": true
		},
		"formatting": {
			"date_format": "%d_%m_%Y",
			"time_format": "%H_%M_%S"
		}
	}
}
```

### Второй способ если сдохла программа!!

1. Вводи команду чтобы перейти в главную директорию проекта!!! 
2. Вводи команду чтобы перейти в директорию проекта ```cd /home/orangepi/NPP-Robot-Inspection-main```
3. Очисти файл проекта введя команду ```make clean```
4. Ввести команду ```make client```
5. Вводи команду чтобы перейти в директорию проекта ```cd build/client```
6. Отредактируйте конфигурацию робота!!! ```nano client_config.json```
7. Чтобы запустить программу надо ввести команду ```./client```

### Третий способ если проверить работает ли Arduino!!
1. Подключите синий провод USB к ноутбуку а другой конец к Arduino
2. Запустите на рабочем столе Arduino IDE
3. В правом верхнем углу есть кнопка "Serial Monitor" нажмите на неё
4. Чтобы протестировать работают ли двигатели введите в поле вот этот код!!
```
{
  "motor_direction": 0,
  "servo1_position": 90,
  "servo2_position": 90,
  "step_size": 0
}
```
5. Обьяснительный код (не писать в поле!!)
```
{
  "motor_direction": 0, # 0 - моторы не двигаются, 1 - едут в перед, 2 - едут назад
  "servo1_position": 90, # поворот 1 серво привода от 0 до 180, центральная точка 90!!!
  "servo2_position": 90, # поворот 2 серво привода от 0 до 180, центральная точка 90!!!
  "step_size": 0 # размер шага двигателей лучше ставить 600 можно и выше или ниже смотря сколько поставить!
}
```
