# VCU-Firmware

Firmware for the vehicle control unit in GRC22-Present. We currently utilize a [Nucleo-F767ZI](https://www.st.com/en/evaluation-tools/nucleo-f767zi.html) board as the compute for the VCU. On our roadmap is to move to a in-house designed board.

### Setup 

- Install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)

## Running

Firmware can be run or debugged directly on the nucleo board using it's integrated STLink through STM32CubeIDE

## Code Standards

- Any location where a code implements a rule, it should be marked with a comment with the following template: 
```c
// RULE (YEAR VERSION): REFERENCE EXPLANATION
// RULE (2023 V2): T.4.2.4 Set flag to invalid state due to 10% deviation between APPS signals.
```

- TODO: Naming standards are as follows:
```C++
namespace PascalCase {}
class PascalCase {};
int camelCase() {};
int snake_case = 0;
```

- TODO: Formatting is handled by clang format.
```bash
find ./src -iname *.hpp -o -iname *.cpp | xargs clang-format -i # In the root folder of the repo.
```

## Branch rules

- Name your branch as follows: `firstname/branchtopic`, ex. `dallas/bspc`
- `main` is a protected branch, and thus needs a PR to approve merging with it.

## Resources

* [CMSIS-RTOS2](https://arm-software.github.io/CMSIS_5/RTOS2/html/rtos_api2.html)
* [STM32F767ZI](https://www.st.com/en/microcontrollers-microprocessors/stm32f767zi.html)