# VCU-Firmware

Firmware for the vehicle control unit in GRC22-Present. We currently utilize an [Espressif ESP32](https://www.espressif.com/en/products/socs/esp32) board as the compute for the VCU. On our roadmap is to move to a in-house designed board.

### Setup 

- Install [VsCode](https://code.visualstudio.com/download)
- Install the Platform IO extension through the vs code extensions tab (Ctrl + Shift + x on windows)

## Running

Firmware can be run or debugged directly on the ESP32 using the platform IO extension to seamlessly detect and upload software

## Resources
    - [Insulation Monitoring Device](https://www.bender.de/fileadmin/content/Products/d/e/IR155-32xx-V004_D00115_D_XXEN.pdf)
    - [Cooling Pump](https://www.tecomotive.com/download/PWMinfo_EN.pdf)
    - [APPS Sensors](https://isc.kyocera-avx.de/wp-content/uploads/Position-sensor_91681.pdf)

# Algorithms/Methodoligies used
* Insulation Monitoring Device (IMD) Readings:
    - The IMD outputs a pwm frequency based on its state
    - IMD outputs pwm duty cycle for resistance
    - This information is decoded and sent out through CAN

* Pump control frequency and duty cycle calculations:
    - Based on the data sheet, the pump:
        - Requires an input frequency between 50-1000Hz
        - Needs an uninterupted high pulse for 3ms to ensure it is woken up properly
        - Has certain ranges of functionality depending on the duty cycle of the PWM generation, of those features the following are being used:
            - Pump stop -> 0-12% duty cycle
            - Controlled Operation -> 13-85% duty cycle
            - Max speed -> 86-97% duty cycle
    - The ledcSetup() function is used to setup the pwm generator, with the frequency 50Hz with a resolution at 10 bits
    - The Pump GPIO pin is attached to the pwm generator channel using ledcAttachPin()
    - PWM outputs are generated using ledcWrite()
    - To calculate the duty cycle the following formula can be used:
        $`Duty cycle = (2^10 - 1) * desired duty cycle / 100`$
        - The bit value is subtracted by 1 since the program starts counting at 0
        - This formula is further changed since the pump control on the Gryphon Racing Shutdown Board contains N channel MOSFET
        - Because of this the Duty cycle value needs to be inverted for the proper output to the pump, the updated formula is as follows:
        $`Duty cycle = (2^10 - 1) - (2^10 - 1) * desired duty cycle / 100`$
        This formula is used for our max speed and stop cases, this allows us to maintain a consistent speed for the pump

    - For the remaining pump speeds, the following is done:
    - Since the pump cycle function takes an input of the desired pump speed (percentage value), a linear operation is done to ensure that the correct duty cycle value is being selected for the desired speed. 
        - Knowing that the original bounds (pump speed) are from 1 - 99% and that they need to be converted to 15 - 85% (15% rather than 13% since the pump will always be on at 15% with the frequency we are using), the following linear mapping is chosen [(more information can be found here)](https://stackoverflow.com/questions/345187/math-mapping-numbers):
            $`duty cycle = (((X - A)/(B - A)) * (D - C)) + C`$
            Where A = lower bound of old value (1)
                  B = upper bound of old value (99)
                  C = lower bound of new value (15)
                  D = upper bound of new value (85)
                  X = input value (pump speed in percent)
            Yielding:
            $`duty cycle = (((pump speed - 1)/(99 - 1)) * (85 - 15)) + 15`$
        
