#ifndef WINC1500_MAIN_H
#define WINC1500_MAIN_H

#include "atmel_start.h"

#include "driver/include/m2m_wifi.h"

#define CONF_WINC_PIN_CHIP_SELECT WINC_PIN_CHIP_SELECT
#define CONF_WINC_PIN_CHIP_ENABLE WINC_PIN_CHIP_ENABLE
#define CONF_WINC_PIN_RESET WINC_PIN_RESET

void wifi_init(tstrWifiInitParam *params);

#endif
