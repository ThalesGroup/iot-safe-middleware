## IoT Safe Middleware Library Overview
IoT Safe Middleware implements a set of API to support interactions with GSMA IoT Safe Applet on a SIM.
This project consists of IoT Safe Middleware library, unit tests and a sample application.

## System Requirements

### Build Requirements
#### On Linux platform

+ CMake 3.14.5 or higher

+ Build essentials C++ compiler, linker and etc.

+ ``` bash
  sudo apt-get install libcpputest-dev
  ```

#### On Raspberry Pi platform

Additionally, install the [CMake for Raspberry](https://snapcraft.io/install/cmake/raspbian)

```bash
sudo apt update
sudo apt install snapd
sudo reboot
sudo snap install --candidate cmake --classic
```

### Hardware Test Requirements
+ Cinterion Modem
+ IoT Safe SIM
+ Raspberry Pi 

## Build Steps
### Make
If *CppUTest* is already installed and in the system path, the IoT Safe library and simple demo can be built by running ```make``` from the root folder.

The make will build:

- iotsafelib.a

- simpledemo

- CppUTestIoTSafe

  **simpledemo** *or* **CppUTestIotSafe** *can by run from the command line*

### CMake

#### Check CMake version

```cmake --version```

**Version must be above 3.14.5**

Alternatively, *CMake* can be used to automatically pull *CppUTest* dependency and build the project.
```bash
	mkdir build
	cd build
	cmake ..
	make
	make test
```

## References
+ [IOT Safe GSMA Specification](https://www.gsma.com/iot/iot-safe/)
