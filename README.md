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

# Algorithms/Methodoligies used

* Wheel speed sensor calculations:
    - In order to account for the car being at a "stop" the timer configurations needed to be modified for an updated max period (the amount of 'ticks' that can be held until the timer overflows)
        - Assuming that 2 overflows would mean that the car is not moving (subject to change), it was necessary to calculate the period necessary to account for this
    - Since $(1 km/h is = 0.27m/s)$, the amount of rotations on a 32-tooth wheelbase within one second is equal to 0.2115r/s
        - This means that there are 4.728 seconds/rotation when the wheel is moving at 1 km/h 
        - This also equates to 12.69 rotations per minute
    - This means that there are 0.14775 seconds per tooth when traveling at this speed
        - This value allows us to find that the frequency at 1km/h is equal to 6.768Hz by the equation: $(Frequency= 1/Time Interval)$
    - Since the onboard timer has a clock configuration of 96MHz, dividing that value with the frequency at 1km/h yields a max period of 14,184,397
    - Time Interval Calculation: To determine the time interval between two rising edges at the same sensor, calculate the difference in timer values captured at consecutive rising edges. The equation used is:
    - $(Time Interval =(Current Timer Value + ( Overflow Counter × Timer Period ) ) − Previous Timer Value)$
    - NOTE: The timer will be registering 0 at half of this frequency (ie: traveling below 0.5km/h registers 0 rpm with the sensors)
    - The formula used to calculate RPM (Revolutions Per Minute) based on wheel speed frequency is as follows: $(RPM= Wheel Frequency(60)/Number of Teeth on the Wheel)$
    - 
