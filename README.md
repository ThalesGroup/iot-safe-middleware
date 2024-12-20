## IoT Safe Middleware Library Overview
IoT Safe Middleware implements a set of API to support interactions with GSMA IoT Safe Applet on a SIM.
![Overview](doc/middleware-overview.png)
As depicted in this figure, IoT Safe Middleware is a thin layer to enable user application / other stacks to format and send APDUs to GSMA IoT Safe Applet running on a SIM card. 
The IoT Safe middleware would be compiled and bundled with IoT Applications using other protocol stacks such as TLS.
The IoT Applications bundling IoT Safe Middleware would typically run on IoT Devices with cellular connectivity.

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
sudo apt-get install libssl-dev
```

#### Cross compilation On Linux Platform

To cross compile for different target on Linux, please configure `CC`, `AR`, and `CXX` variable in [Makefile](./Makefile). for example to cross compile for ARM (hardware float) target
```
CXX = arm-linux-gnueabihf-g++
CC = arm-linux-gnueabihf-gcc
AR = arm-linux-gnueabihf-ar
```
you also need to install Linux dependency for cross compile target

```bash
sudo apt-get install libcpputest-dev:armhf
```

### Hardware Test Requirements
+ 3G/4G/LTE Modem e.g Cinterion Connect Shield / Concept Board
+ IoT Safe SIM
+ Raspberry Pi 

## Build Steps

### Applet selection

The MVP version of the applet uses the following AID:
```
static uint8_t AID[] = { 0xA0, 0x00, 0x00, 0x00, 0x30, 0x53, 0xF1, 0x24, 0x01, 0x77, 0x01, 0x01, 0x49, 0x53, 0x41 };
```

### Make
If *CppUTest* is already installed and in the system path, the IoT Safe library and simple demo can be built by running ```make``` from the root folder.

The make will build:

- iotsafelib.a

- simpledemo

- jwtdemo 

- CppUTestIoTSafe

  **simpledemo** *or* **jwtdemo** *or* **CppUTestIotSafe** *can be run from the command line*

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


## JWT Signing demo
An example app to demonstrate how the IoT Safe Applet can be used to sign JWT token.
This example uses ECDSA keypair on the SIM/UICC to sign JWT. 
IoT Safe Applet on the  SIM/UICC is provisioned with key containers containing ECDSA private key (container ID = 01) and public key (Container ID = 02)
### JWT Sample header and payload
The following sample header and payload are used in this example
#### JWT Header:
```
{"alg":"ES256","typ":"JWT"}
```

#### JWT Payload:
```
{"id": 42,"claimType": "claimType","claimUrl","claimUrl" }
```

#### A sample JWT computed using the about header and payload would be as follows:
```
eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJpZCI6IDQyLCJjbGFpbVR5cGUiOiAiY2xhaW1UeXBlIiwiY2xhaW1VcmwiOiJjbGFpbVVybCIgfQ.wMG0sp7BfdGDjvW4nJPE6swwEE0b7UvPmMWiYwa71zuAMuDpRbQasgcz3lls0V0jLh7XjmfnFPahSRs0Eh3OUQ
```
For easy verification of this token , the example app is also retrieving Public Key from UICC and print it in on the console
#### e.g Public Key
```
-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAElUtniK6dsGR6i/s/cAIxxISSjR7L
+lcy4O4FOlRWilW+O1R//HLobRJzzK8qEVPhp32r+GgZn8O7OgWoio4c/Q==
-----END PUBLIC KEY-----
```
#### JWT Verification
The generated JWT can be independantly verified by tools such as online portal [https://jwt.io/](https://jwt.io/)  

## References
+ [IOT Safe GSMA Specification](https://www.gsma.com/iot/iot-safe/)
