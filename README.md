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
    - NOTE: The timer will be registering 0 at half of this frequency (ie: travelling below 0.5km/h registers 0 rpm with the sensors)



* Pump control frequency and duty cycle calculations:
    - Based on the data sheet for the [cooling pump](https://www.tecomotive.com/download/PWMinfo_EN.pdf), the pump:
        - Requires an input frequency between 50-1000Hz
        - Needs an uninterupted high pulse for 3ms to ensure it is woken up properly
        - Has certain ranges of functionality depending on the duty cycle of the PWM generation, of those features the following are being used:
            - Pump stop -> 0-12% duty cycle
            - Controlled Operation -> 13-85% duty cycle
            - Max speed -> 86-97% duty cycle
    - The PWM is set to generate a frequency of 100Hz, the Auto Reload Register (ARR) is set to 4000, since this is also the period of each pulse this ensures that the motor can be woken up properly at 75% duty cycle. The overall prescaler is divided by 4. In order to find the prescale value the formula that was used is:
        $`Frequency = chip clock /(ARR*Prescaler)`$
        $`100 = 96MHz/(4000*Prescaler)`$
        $`Prescaler = 240`$

    - Since the pump cycle function takes an input of the desired pump speed (percentage value), a linear operation is done to ensure that the correct duty cycle value is being selected for the desired speed. 
        - Knowing that the original bounds (pump speed) are from 1 - 99% and that they need to be converted to 13 - 85%, the following linear mapping is chosen [(more information can be found here)](https://stackoverflow.com/questions/345187/math-mapping-numbers):
            $`duty cycle = (((X - A)/(B - A)) * (D - C)) + C`$
            Where A = lower bound of old value (1)
                  B = upper bound of old value (99)
                  C = lower bound of new value (13)
                  D = upper bound of new value (99)
                  X = input value (pump speed)
            Yielding:
            $`duty cycle = (((pump speed - 1)/(99 - 1)) * (85 - 13)) + 13`$
        
        - With the duty cycle value updated, the pulse value for PWM (CCR1) needs to be updated using the following formula:
            $`duty cycle = (CCR1 / ARR)`$
            $`CCR1 = (duty cycle * ARR)`$
            $`CCR1 = (duty cycle * 4000)`$
