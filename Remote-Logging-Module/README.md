# Remote-Logging-Module

An onboard data logging system for our cars, using radio transmission for remote data monitoring.

## Prerequisites

- [Visual Studio Code](https://code.visualstudio.com/download)
- [C/C++ Extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
- [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)
- [Foxglove Studio](https://foxglove.dev/download)

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

## DBC to CPP

```bash
python3 -m pip install cantools
pip install python-can==4.2.2 # downgrade required
python3 -m cantools generate_c_source DBCs/RLM.dbc -o src
```

## Foxglove WebSocket Server

1. In a terminal, navigate to the `/software/foxglove` directory
2. Install the necessary libraries:

   ```bash
   pip install -r requirements.txt
   ```

3. Plug in the receiver module and note the serial port (eg. `/dev/ttyUSB0`)

   _NOTE: port names may differ between operating systems_

4. Start the Foxglove WebSocket server:

   ```bash
   python3 foxglove_server.py [SERIAL_PORT]
   ```

5. Run the Foxglove Studio application and open a connection to `ws://localhost:8765`

## Resources

**Datasheets**

- [nRF24L01+](https://www.sparkfun.com/datasheets/Components/SMD/nRF24L01Pluss_Preliminary_Product_Specification_v1_0.pdf)
- [MPU-6050](https://cdn.sparkfun.com/datasheets/Sensors/Accelerometers/RM-MPU-6000A.pdf)
- [BN-220](https://files.banggood.com/2016/11/BN-220%20GPS+Antenna%20datasheet.pdf)

**Other**

- [Design Requirements (GRC OneDrive)](https://uoguelphca-my.sharepoint.com/:w:/r/personal/ugracing_uoguelph_ca/_layouts/15/Doc.aspx?sourcedoc=%7B7C1A61FF-9C05-44D4-A12C-FCA501E7928A%7D&file=Remote%20Data%20Logger%20Design%20Requirements.docx&action=default&mobileredirect=true&DefaultItemOpen=1&web=1)
- [Foxglove WebSocket Server (JSON)](https://github.com/foxglove/ws-protocol/blob/main/python/src/foxglove_websocket/examples/json_server.py)
