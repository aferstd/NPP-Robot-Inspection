# 🤖 A robot for checking NPP pipes for cracks and corrosion using artificial intelligence
## Description
This project is designed on the theme "Robot for detecting defects using artificial intelligence in nuclear power plants", the project itself consists of a client (on Orange Pi) and a server for transmitting data from the robot to the server (images from cameras and sensors) and artificial intelligence (corrosion and cracks), including drawings about the robot.
The program itself allows you to change the available protocols for transmitting information: HTTPS, and websites have also been developed to manage and display data from the robot. The following languages were used: C++ and Python
## FAQ

#### What does this project do?

This project allows robots to be controlled at nuclear power plants to detect corrosion and cracks on the outer parts of pipes.

#### What is the benefit of this project?

The usefulness of this project is to control the robot remotely for nuclear power plant employees, which allows them to safely find out where there are cracks and corrosion on pipes where ordinary people cannot climb.

#### How much does the AI ideally show the defects?

Artificial intelligence can detect defects on pipes with up to 90% accuracy, it has been trained on 350,000 images up to the 15th generation.
## Building from source

Install Library Client-Server on linux

```bash
  sudo apt update 
  sudo apt install git g++ make build-essential libopencv-dev
```

Additionally for client (OrangePi)
```bash
  git clone https://github.com/orangepi-xunlong/WiringOP.git
  cd WiringOP
  sudo ./build
```

Clone the project repository:
```bash
  git clone https://github.com/aferstd/robot_client_server.git
  cd robot_client_server
```

Assemble the project using the command

```bash
  make
```
## Authors

- [@aferstd](https://www.github.com/aferstd) - ``` Client-Server ```
- [@Darkwell](https://www.github.com/Darkwell) - ```AI (corrosion and cracks)```
