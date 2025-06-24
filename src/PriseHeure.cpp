#include "PriseHeure.h"

namespace rlc {

    long PriseHeure::calcul_sleep_ms(String datetime, int target_hour, int target_minute) {
        if (datetime.length() < 19) return -1;

        int current_hour = datetime.substring(11, 13).toInt();
        int current_minute = datetime.substring(14, 16).toInt();
        int current_second = datetime.substring(17, 19).toInt();

        int now_sec = (current_hour * 3600 + current_minute * 60 + current_second) + 27;
        int target_sec = target_hour * 3600 + target_minute * 60;

        long seconds_to_sleep = 0;

        if (now_sec >= target_sec) {
            seconds_to_sleep = 24 * 3600 - now_sec + target_sec;
        } else {
            seconds_to_sleep = target_sec - now_sec;
        }

        return seconds_to_sleep * 1000L;
    }

}
