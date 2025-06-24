#include "Gps.h"

namespace rlc
{
    Gps::Gps(rlc::AtCommand &command_helper) : _command_helper(command_helper)
    {
    }

    bool Gps::current_location()
    {
        return current_location(60000, false);
    }

    bool Gps::current_location(const int timeout, bool is_close_session)
    {
        bool is_refreshed = false;
         _command_helper.send_command_and_wait("AT+CGPS=1,1");
            delay(1000);

  // En cas d’échec, tentative fallback
         _command_helper.send_command_and_wait("AT+CGPS?");   // Voir si actif
            delay(500);
         _command_helper.send_command_and_wait("AT+CGPS=1");  // Fallback classique
            delay(1000);

  // Activation config avancée (juste au cas où)
  _command_helper.send_command_and_wait("AT+CGPSINFOCFG=10,31");
        

        int attempt = 0;
        unsigned long time = millis();
        while ((time + timeout) > millis())
        {
            attempt++;

            if (_command_helper.send_command_and_wait("AT+CGPSINFO"))
            {
                _command_helper.last_command_response.trim();
                if (_command_helper.last_command_response == "" || _command_helper.last_command_response.indexOf(",,,,,,,,") > 0)
                {
                    // No GPS lock....try again
                    delay(2000);
                    continue;
                }

                // Parse the location data from the response
                int eol1 = _command_helper.last_command_response.indexOf('\n');
                int eol2 = _command_helper.last_command_response.indexOf('\n', eol1 + 1);
                location_data = _command_helper.last_command_response.substring(eol1 + 1, eol2);
                location_data.trim();

                rlc::GpsPoint np = rlc::GpsPoint::from_nmea_str(location_data);
                last_gps_point.copy(np);

                is_refreshed = true;
                break;
            }
        }

        if (is_close_session)
        {
            _command_helper.send_command_and_wait("AT+CGPS=0");
        }

        return is_refreshed;
    }

}
