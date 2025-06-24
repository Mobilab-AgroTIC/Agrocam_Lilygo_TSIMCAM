#ifndef DateModem_h
#define DateModem_h

#include <Arduino.h>
#include "AtCommand.h"

namespace rlc
{
    class DateModem
    {
    public:
        DateModem(AtCommand &command);
        bool wait_for_network(unsigned long timeout = 15000);
        String get_time_string(); // format : yy/MM/dd,hh:mm:ss+TZ
        String get_datetime_string();

        
        String last_error;

    private:
        AtCommand &cmd;
    };
}

#endif
