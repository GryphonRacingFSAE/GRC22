# Remote-Logging-Module

An onboard data logging system for our cars, using radio transmission for remote data monitoring.

## Prerequisites

- [Visual Studio Code](https://code.visualstudio.com/download)
- [C/C++ Extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
- [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)

## Setup

Plug in both modules and check `platformio.ini` to ensure the correct ports are selected:
```
[env:transmitter]
...
upload_port = /dev/ttyUSB0

[env:receiver]
...
upload_port = /dev/ttyUSB1
```
_NOTE: port names may differ between operating systems_

## Build + Run

For the transmitter module:

`PLATFORMIO > PROJECT TASKS > transmitter > Upload and Monitor`

For the receiver module:

`PLATFORMIO > PROJECT TASKS > receiver > Upload and Monitor`

## Resources

**Datasheets**
- [nRF24L01+](https://www.sparkfun.com/datasheets/Components/SMD/nRF24L01Pluss_Preliminary_Product_Specification_v1_0.pdf)
- [MPU-6050](https://cdn.sparkfun.com/datasheets/Sensors/Accelerometers/RM-MPU-6000A.pdf)
- [BN-220](https://files.banggood.com/2016/11/BN-220%20GPS+Antenna%20datasheet.pdf)

**Websites**
- [BonoGPS](https://github.com/renatobo/bonogps) (future idea: GPS app integration)
