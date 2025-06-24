#pragma once
#include <Arduino.h>

namespace rlc {
    class PriseHeure {
    public:
        static long calcul_sleep_ms(String datetime, int target_hour, int target_minute);
    };
}
