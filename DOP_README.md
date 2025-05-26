
# Специально для Руслана и Сергея!!!!

### Важные данные!!!
1. Логин: root
2. Пароль: orangepi
#### =====================
#### Поставить 3 число на своё, оно и там и там одинаковое!!
#### [Команда на Windows: ipconfig, на Linux: ifconfig]
1. Айпи Orange Pi: 192.168.--.86
2. Айпи Сервера(ноутбка): 192.168.--.195
#### =====================

# Запуск Робота!!!!

1. Сергей включи интернет со своего телефона!!
2. Руслан Включи Робота кнопкой на блоке питания!!!! 
3. На рабочем столе открой файл "22_05_2025Profile.tlp"
4. Теперь открой Командную строку с помощью Win + R и введи "cmd"
5. И напиши команду "ipconfig"
6. Теперь найди "Адаптер беспроводной локальной сети Беспроводная сеть 1" и в нём будет пункт "IPv4-адрес"
7. и теперь ты должен посмотреть на айпи примерно будет такой 192.168.77.86, и третие число (77) ты должен заменить в программе которые ты запустил ранее "22_05_2025Profile.tlp"
9. Нажми кнопку Login
10. В Левой части программы есть кнопка "New Terminal"
11. Вводи команду чтобы перейти в директорию билда проекта ```cd /home/orangepi/NPP-Robot-Inspection-main/build/client```
12. Прежде чем запускать программу посмотри конфиг все ли верно ```nano client_config.json```
14. [Доп. 1] Если ты используешь панель управление через сайт (receive_http_data) то нажми на зелёный флажёк и нажми на "Папка с проектами" и зайди по пути localhost/control/send_data.php и там измени айпи адресс на новый И УКАЗЫВАЕШЬ АЙПИ ORANGE PI!!!!!!!!!
15. [Доп. 2] Если так используешь панель управление то проверь в терминале с помощью команды "sudo dmesg | grep tty" и в самом конце посмотри чтобы драйвер CH340 был актевирован!!! и не отключился!
16. Чтобы запустить программу надо ввести команду ```./client```

## Аварийный способ
### Первый способ если программа не запускается иза конфигурации
1. Чтобы отредактировать конфиг программы нужно ввести комманду в директории [build/client] ```nano client_config.json```

2. Сама конфигурация Клиента (НЕ КОПИРОВАТЬ С КОМЕНТАРИЯМИ!!!!!) 
```json
{
  "client": {
    "address": "127.0.0.1", # Айпи адрес сервера (ноутбук)
    "port": 5555,  # Порт сервера (не изменять!!)
    "delay_resend": 1000  # Задержка отправки данных (не изменять!!) в мс
  },
  "receive_http_data": {
    "enabled": false, # Включение панели управления!
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

3. Чтобы закрыть редактор нужно нажать Ctrl + X а чтобы сохранить нажать Y или Ctrl + Y

4. Запустить программу командой ```./client```

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

### Четвёртый способ если код на Arduino не работает или его нужно изменить!
1. На рабочем столе откройте Arduino IDE
2. Введите код и отредактируейте его ДАЖЕ С КОМЕНТАРИЯМИ он будет работать (коментарии кода начинаются после //)
   ```
	#include <Servo.h>
	#include <ArduinoJson.h>
	
	// Пины для шаговых двигателей 
	#define NUM_MOTORS 4
	const int stepPins[NUM_MOTORS] = {2, 3, 4, 12}; // Шаговые двигатели
	const int dirPins[NUM_MOTORS] = {5, 6, 7, 13};  // Направление двигателей
	
	// Пины для серводвигателей 
	#define NUM_SERVOS 2
	const int servoPins[NUM_SERVOS] = {A4, A5}; // Серводвигатели 
	
	Servo servos[NUM_SERVOS]; // Массив сервоприводов
	
	bool unblock_motors = false; 
	
	void setup() { 
	  Serial.begin(9600);  // Убедитесь, что скорость совпадает с Orange Pi 
	
	  pinMode(8, OUTPUT);
	  digitalWrite(8, LOW);
	
	  // Подключаем сервоприводы
	  for (int i = 0; i < NUM_SERVOS; i++)
	    servos[i].attach(servoPins[i]);
	
	  // Устанавливаем режимы для всех пинов шаговых двигателей
	  for (int i = 0; i < NUM_MOTORS; i++) {
	    pinMode(stepPins[i], OUTPUT);
	    pinMode(dirPins[i], OUTPUT);
	  } 
	} 
	
	void loop() { 
	  if (Serial.available() > 0) { 
	    String receivedData = Serial.readStringUntil('}');
	    receivedData += '}';
	
	    // Создаем объект JSON
	    StaticJsonDocument<200> doc; // Размер документа подберите в зависимости от ваших данных
	    DeserializationError error = deserializeJson(doc, receivedData);
	
	    // Проверяем на наличие ошибок
	    if (error) {
	      Serial.print(F("Ошибка разбора JSON: "));
	      Serial.println(error.f_str());
	      return;
	    }
	
	    // Извлекаем значения из JSON
	    // const char* mode = doc["mode"];
	    int motorDirection = doc["motor_direction"];
	    int servo1Position = doc["servo1_position"];
	    int servo2Position = doc["servo2_position"];
	    int stepSize = doc["step_size"];
	
	    // Устанавливаем позиции сервоприводов
	    servos[0].write(servo1Position); 
	    servos[1].write(servo2Position); 
	
	    // Управляем направлением моторов
	    unblock_motors = (motorDirection == 1 || motorDirection == 2);
	
	    digitalWrite(dirPins[0], (motorDirection == 1) ? HIGH : LOW); // Назад или Вперед
	    digitalWrite(dirPins[1], (motorDirection == 1) ? LOW : HIGH); // Назад или Вперед
	    digitalWrite(dirPins[2], (motorDirection == 1) ? HIGH : LOW); // Назад или Вперед
	    digitalWrite(dirPins[3], (motorDirection == 1) ? LOW : HIGH); // Назад или Вперед
	
	    // Шаговые двигатели 
	    if (unblock_motors) { 
	      for (int i = 0; i < stepSize; i++) { 
	        for (int j = 0; j < NUM_MOTORS; j++)
	          digitalWrite(stepPins[j], HIGH);  // Шаг 
	        delayMicroseconds(1000);             
	        for (int j = 0; j < NUM_MOTORS; j++)
	          digitalWrite(stepPins[j], LOW);   
	        delayMicroseconds(1000);             
	      } 
	    } 
	
	    // Выводим данные в Serial Monitor 
	    Serial.print("Servo 1 Position: "); 
	    Serial.println(servo1Position); 
	    Serial.print("Servo 2 Position: "); 
	    Serial.println(servo2Position); 
	    Serial.print("Motor Direction: "); 
	    Serial.println(motorDirection); 
	    Serial.print("Step Size: "); 
	    Serial.println(stepSize); 
	    // Serial.print("Mode: "); 
	    // Serial.println(mode); 
	  } 
	}
   ```
3. Выберите в верхнем левом углу такую плашку и там выберить порт который у вас там будет
4. Напиши в поле Arduno Uno и у вас будет выбор какую плату выбрать нажимаете с тектом "Arduino Uno"
5. Потом нажмите кнопку загрузить код!! если не работает то значит вы написали не правильно!! все перепроверте!
6. Потом как все загрузили проверьте код и запустите робота, если все работает с примером в Третем способе то все отлично :))
7. Потом подключите синий провод USB стороной и подключите его в синий USB разьем на Orange PI
