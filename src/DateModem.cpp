#include "DateModem.h"

namespace rlc
{
    DateModem::DateModem(AtCommand &command) : cmd(command)
    {
        last_error = "";
    }

    bool DateModem::wait_for_network(unsigned long timeout)
    {
        unsigned long start = millis();
        while (millis() - start < timeout)
        {
            bool ok = cmd.send_command_and_wait("AT+CREG?");
            String response = cmd.last_command_response;

            if (ok && (response.indexOf("+CREG: 0,1") != -1 || response.indexOf("+CREG: 0,5") != -1))
            {
                return true;
            }

            delay(1000);
        }

        last_error = "Réseau GSM non disponible.";
        return false;
    }

    String DateModem::get_time_string()
    {
        bool ok = cmd.send_command_and_wait("AT+CCLK?");
        String response = cmd.last_command_response;

        int index = response.indexOf("+CCLK:");
        if (ok && index != -1)
        {
            int startQuote = response.indexOf("\"", index);
            int endQuote = response.indexOf("\"", startQuote + 1);
            if (startQuote != -1 && endQuote != -1)
            {
                return response.substring(startQuote + 1, endQuote);
            }
        }

        last_error = "Heure non disponible.";
        return "Non disponible";
    }


String DateModem::get_datetime_string()
{
    if (!wait_for_network())
    {
        last_error = "Pas de réseau";
        return "Non disponible";
    }

    if (!cmd.send_command_and_wait("AT+CCLK?"))
    {
        last_error = "Échec AT+CCLK?";
        return "Non disponible";
    }

    String response = cmd.last_command_response;
    int start = response.indexOf('"');
    int end = response.lastIndexOf('"');
    if (start == -1 || end == -1 || end <= start)
    {
        last_error = "Format invalide";
        return "Non disponible";
    }

    String raw = response.substring(start + 1, end); // ex : "25/05/20,11:36:50+08"
    
    String year = "20" + raw.substring(0, 2);  // "25" → "2025"
    String month = raw.substring(3, 5);        // "05"
    String day = raw.substring(6, 8);          // "21"

    String time = raw.substring(9, 17);

    return year + "-" + month + "-" + day + " " + time;  // Format final : yyyy-mm-dd hh:mm:ss
}




}
