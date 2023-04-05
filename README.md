# SMB-Firmware

Firmware for segment monitor boards. Utilizes an STM32F103x8 µC to monitor temperatures across a segment. Conforms to Orion BMS TEM [CAN specification](https://www.orionbms.com/products/thermistor-expansion-module).

## CAN Specification

| CAN ID\* | Description                   | Broadcast Period [ms] | Length |
| -------- | ----------------------------- | --------------------- | ------ |
| 18EEFF80 | J1939 Address Claim Broadcast | 200                   | 8      |
| 1839F380 | BMS Thermistor Broadcast      | 100                   | 8      |
| 1838F380 | General Thermistor Broadcast  | 100                   | 8      |
| 18400080 | Unknown                       | 100                   | 8      |
| 18408000 | Unknown                       | 100                   | 8      |

| 80 | Legacy - RESERVED | 100 | 4 |

- \*CAN Ids are relative to the module #
- View the [CAN specification](https://www.orionbms.com/products/thermistor-expansion-module) for the Orion BMS TEMs for more info

Note

- F3 in the table is the BMS j1939 Address
- Last two digits of CAN ID is the CAN ID of the thermistor expansion model
