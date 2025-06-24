#ifndef Http_h
#define Http_h

#include <Arduino.h>
#include "AtCommand.h"

namespace rlc
{
    class Http
    {
    public:
        Http(rlc::AtCommand &command_helper, rlc::Console &console);
                bool begin(); 

        bool post(String url, String content, String content_type);
        bool post_file_buffer(String url, const uint8_t *buf, size_t size, String filename);
        bool post_file_from_sd(String url, String path_on_sd, String filename);
        

 
    private:
        rlc::AtCommand _command_helper;
        rlc::Console _console;
    };
}

#endif