#include "Hardware.h"
#include "Camera.h"

namespace rlc
{
    Hardware::Hardware(rlc::AtCommand &command_helper, rlc::Console &console, bool is_debug) : _command_helper(command_helper),
                                                                                               _console(console),
                                                                                               _is_debug(is_debug)
    {
        manufacturer = "";
        model = "";
        imei = "";
    }

    void Hardware::init()
    {
        begin_console(10000);

        begin_serial_module();

        pinMode(LTE_PWRKEY_PIN, OUTPUT);

    }

    bool Hardware::init_sd()
    {
#ifdef tsimcam
        SPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
#endif
        if (!SD.begin(SD_CS_PIN))
        {
            return false;
        }
        return true;
    }

    bool Hardware::init_module()
{
    if (is_module_on())
    {
        _command_helper.send_command_and_wait("AT+CMEE=2");


        if (manufacturer == "")
        {
            if (!_command_helper.send_command_and_wait("AT+CGMI"))
                return false;
            manufacturer = _command_helper.last_command_response;
        }

        if (model == "")
        {
            if (!_command_helper.send_command_and_wait("AT+CGMM"))
                return false;
            model = _command_helper.last_command_response;
        }

        if (imei == "")
        {
            if (!_command_helper.send_command_and_wait("AT+CGSN"))
                return false;
            imei = _command_helper.last_command_response;
        }

        // Configuration réseau pour carte Bouygues
        _command_helper.send_command_and_wait("AT+CGREG=1");
        _command_helper.send_command_and_wait("AT+CGDCONT=1,\"IP\",\"ebouygtel.com\"");// APn ICI======
        _command_helper.send_command_and_wait("AT+CGATT=1");
        _command_helper.send_command_and_wait("AT+CGACT=1,1");

        // Debug réseau
        _command_helper.start_verbose();  // Active affichage console AT
        _command_helper.send_command_and_wait("AT+CREG?");
        _command_helper.send_command_and_wait("AT+CGATT?");
        _command_helper.send_command_and_wait("AT+CSQ");
        _command_helper.end_verbose();

        return true;
    }

    return false;
}


    void Hardware::begin_console(unsigned long timeout)
    {
        _console.begin(timeout);
    }

    void Hardware::end_console()
    {
        _console.end();
    }

    void Hardware::begin_serial_module()
    {
        _command_helper.begin(1000);
    }

    void Hardware::end_serial_module()
    {
        _command_helper.end();
    }

    bool Hardware::is_module_on()
    {
        for (int i = 0; i < 5; i++)
        {
            if (_command_helper.send_command_and_wait("AT"))
            {
                return true;
            }
            delay(1000);
        }
        return false;
    }

    bool Hardware::is_cellular_connected()
    {
        return is_cellular_connected(false);
    }

    bool Hardware::is_cellular_connected(bool is_config_if_not)
{
    if (_command_helper.send_command_and_wait("AT+CGREG?"))
    {
        // Accepte aussi bien le mode normal que roaming
        if (_command_helper.last_command_response.indexOf("0,1") > 0 ||
            _command_helper.last_command_response.indexOf("1,1") > 0 ||
            _command_helper.last_command_response.indexOf("0,5") > 0 ||
            _command_helper.last_command_response.indexOf("1,5") > 0)
        {
            return true;
        }
    }

    if (is_config_if_not)
    {
        if (!is_module_on())
        {
            turn_on_module();
        }
        init_module();

        return is_cellular_connected(false);
    }

    return false;
}

    bool Hardware::turn_off_module()
    {
        if (is_module_on()) // if it's on, turn it off.
        {
    // Étape 1 : Éteindre proprement le modem SIM7600
    if (is_module_on()) {
        // Envoi AT+CPOF pour demander arrêt logiciel
        _command_helper.send_command_and_wait("AT+CPOF");// Envoi de la commande pour éteindre le module
        delay(200); // petit délai après commande
    }

    // Étape 2 : Pulse GPIO PWRKEY pour forcer l'arrêt matériel
    pinMode(LTE_PWRKEY_PIN, OUTPUT);
    digitalWrite(LTE_PWRKEY_PIN, LOW);
    delay(1500); // pulse de 1.5 secondes
    digitalWrite(LTE_PWRKEY_PIN, HIGH);
    delay(200); // stabilisation

    
/*
    // Étape 3 : couper liaison UART si possible
    end_serial_module();  // équivalent à SerialAT.end()*/

 

    return true;}

        return true;
    }

    bool Hardware::turn_on_module()
    {
        if (!is_module_on()) // if it's off, turn it on.
        {

#ifdef tsimcam

            digitalWrite(LTE_PWRKEY_PIN, 1);
            delay(500);
            digitalWrite(LTE_PWRKEY_PIN, 0);
#endif
            for (int i = 0; i < 40; i++)
            {
                if (_command_helper.send_command_and_wait("AT"))
                {
                    return true;
                }
                delay(250);
            }

            return false;
        }

        return true;
    }

    bool Hardware::set_module_in_minimum_functionality_mode()
    {
        return _command_helper.send_command_and_wait("AT+CFUN=0", "OK", 5000);
    }

    bool Hardware::set_module_in_full_functionality_mode()
    {
        return _command_helper.send_command_and_wait("AT+CFUN=1", "OK", 5000);
    }

    void Hardware::send_module_output_to_console_out()
    {
        _command_helper.send_module_output_to_console_out();
    }

    void Hardware::send_console_input_to_module()
    {
        _command_helper.send_console_input_to_module();
    }

    String Hardware::to_http_post()
    {
        return "&manufacturer=" + manufacturer + "&model=" + model + "&imei=" + imei;
    }
}