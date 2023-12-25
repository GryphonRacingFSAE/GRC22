#include "IdentityCheck.h"
#include "main.h"

void checkIdentityTask() {

    uint bit0;
    uint bit1;
    uint bit2;

    bit0 = HAL_GPIO_ReadPin(GPIOA, DIP0_Pin);
    bit1 = HAL_GPIO_ReadPin(GPIOA, DIP1_Pin) << 1;
    bit2 = HAL_GPIO_ReadPin(GPIOA, DIP2_Pin) << 2;

    MODULE_ID = (bit0 | bit1 | bit2);
}
