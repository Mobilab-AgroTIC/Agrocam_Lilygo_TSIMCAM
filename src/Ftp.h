#pragma once

#include "AtCommand.h"
#include "Console.h"
#include "Config.h"

namespace rlc
{
    class Ftp
    {
    public:
        Ftp(rlc::AtCommand &command, rlc::Console &console);

        bool begin();  // Connexion FTP (AT+FTPCID + AT+FTPSERV + USER + PWD)
        bool upload_file_from_sd(const String &sd_path, const String &ftp_filename);
        void end();    // Déconnexion propre

        String last_error;

    private:
        rlc::AtCommand &_cmd;
        rlc::Console &_console;
    };
}
