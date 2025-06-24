#include "Ftp.h"
#include <SD.h>

namespace rlc
{
    Ftp::Ftp(AtCommand &command, Console &console)
        : _cmd(command), _console(console)
    {
        last_error = "";
    }

    bool Ftp::begin()
    {
        _console.println("🔌 Connexion FTP en cours...");

        bool ok = true;

        ok &= _cmd.send_command_and_wait("AT+FTPCID=1");
        ok &= _cmd.send_command_and_wait("AT+FTPMODE=1"); // mode passif recommandé
        ok &= _cmd.send_command_and_wait("AT+FTPTYPE=I"); // I = binaire, A = ASCII
        ok &= _cmd.send_command_and_wait("AT+FTPSERV=\"" + String(rlc::Config::ftp_server) + "\"");
        ok &= _cmd.send_command_and_wait("AT+FTPPORT=" + String(rlc::Config::ftp_port));
        ok &= _cmd.send_command_and_wait("AT+FTPUN=\"" + String(rlc::Config::ftp_user) + "\"");
        ok &= _cmd.send_command_and_wait("AT+FTPPW=\"" + String(rlc::Config::ftp_pass) + "\"");

        if (ok)
        {
            _console.println(" Connexion FTP configurée.");
        }
        else
        {
            last_error = "Erreur configuration FTP.";
            _console.println(" " + last_error);
        }

        return ok;
    }
}
