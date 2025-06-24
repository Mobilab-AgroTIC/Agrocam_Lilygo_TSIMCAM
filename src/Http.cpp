#include "Http.h"
#include <SD.h>




namespace rlc
{
    Http::Http(rlc::AtCommand &command_helper, rlc::Console &console) : _command_helper(command_helper),
                                                                        _console(console)
    {
    }

    bool Http::post(String url, String content, String content_type)
    {
        bool is_sent = false;
        // this line ensures that any previous sessions area terminated
        //
        _command_helper.send_command_and_wait("AT+HTTPTERM");

        if (_command_helper.send_command_and_wait("AT+HTTPINIT"))
        {
            if (_command_helper.send_command_and_wait("AT+HTTPPARA=\"URL\",\"" + url + "\""))
            {
                if (_command_helper.send_command_and_wait("AT+HTTPPARA=\"CONTENT\",\"" + content_type + "\""))
                {
                    if (_command_helper.send_command_and_wait("AT+HTTPDATA=" + String(content.length()) + ",5000", "DOWNLOAD", 1000))
                    {
                        _command_helper.send_data(content + "\r\n\r\n");

                        _command_helper.send_command_and_wait("AT+HTTPACTION=1", "HTTP_PEER_CLOSED", 10000);

                        is_sent = true;
                    }
                }
            }
            _command_helper.send_command_and_wait("AT+HTTPTERM");
        }

        return is_sent;
    }




   /*bool Http::post_file_buffer(String url, const uint8_t *buf, size_t size, String filename)
    {
        String boundary = "BOUNDARY12345";
        String multipart_boundary = "multipart/form-data; boundary=" + boundary;

        String content_disposition = "Content-Disposition: form-data; name=\"photo\"; filename=\"" + filename + "\"";
        String content_type = "Content-Type: image/jpeg";
        //String content_tranfer_encoding = "Content-Transfer-Encoding: binary";

        String head = "--" + boundary + "\r\n" + content_disposition + "\r\n" + content_type + "\r\n\r\n"; // + content_tranfer_encoding + "\r\n\r\n";
        //String tail = "\r\n--" + boundary + "--\r\n";
        String tail = 
    "\r\n--" + boundary + "\r\n"
    "Content-Disposition: form-data; name=\"key\"\r\n\r\n"
    "NZ0AZ8YF\r\n"
    "--" + boundary + "--\r\n";


        int total_size = head.length() + size + tail.length();
        
        Serial.println("== DEBUG ==");
Serial.println("Photo size: " + String(size));
Serial.println("Total size: " + String(total_size));
Serial.println("HEAD:\n" + head);
Serial.println("TAIL:\n" + tail);


        bool is_sent = false;

        // this line ensures that any previous sessions area terminated
        //
        _command_helper.send_command_and_wait("AT+HTTPTERM");

        if (_command_helper.send_command_and_wait("AT+HTTPINIT"))
        {
            if (_command_helper.send_command_and_wait("AT+HTTPPARA=\"URL\",\"" + url + "\""))
            {
                if (_command_helper.send_command_and_wait("AT+HTTPPARA=\"CONTENT\",\"" + multipart_boundary + "\""))
                {
                    if (_command_helper.send_command_and_wait("AT+HTTPDATA=" + String(total_size) + ",180000", "DOWNLOAD", 1000))
                    {
                        _command_helper.write(head, 1000);

                        size_t max_chunk_size = 4096;
                        uint8_t chunk_buf[max_chunk_size];
                        size_t current_position = 0;
                        while (current_position < size)
                        {
                            size_t chunk_size = 0;
                            for(size_t i=0;i<max_chunk_size;i++)
                            {
                                if(current_position + i < size)
                                {
                                    chunk_size += 1;
                                    chunk_buf[i] = buf[current_position + i];
                                    //delay(5);
                                }
                            }
                            size_t num_bytes_sent = _command_helper.write(chunk_buf, chunk_size, 2000);

                            current_position += max_chunk_size;
                        }

                        _command_helper.write(tail, 1000);
                    }

                    _command_helper.send_command_and_wait("AT+HTTPACTION=1", "HTTP_PEER_CLOSED", 1000);

               


                    is_sent = true;
                }
            }
            _command_helper.send_command_and_wait("AT+HTTPTERM");
        }

        return is_sent;
    }*/








bool Http::post_file_from_sd(String url, String path_on_sd, String filename)
{
    String boundary = "BOUNDARY12345";
    String multipart_boundary = "multipart/form-data; boundary=" + boundary;
    String content_disposition = "Content-Disposition: form-data; name=\"photo\"; filename=\"" + filename + "\"";
    String content_type = "Content-Type: image/jpeg";
    String head = "--" + boundary + "\r\n" + content_disposition + "\r\n" + content_type + "\r\n\r\n";
    String tail =
        "\r\n--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"key\"\r\n\r\n"
        "NZ0AZ8YF\r\n"
        "--" + boundary + "--\r\n";

    File photoFile = SD.open(path_on_sd.c_str());
    if (!photoFile || photoFile.isDirectory()) {
        _console.println(" Erreur : impossible d’ouvrir le fichier photo sur la SD : " + path_on_sd);
        return false;
    }

    size_t file_size = photoFile.size();
    int total_size = head.length() + file_size + tail.length();
    int fake_total_size = 160000; // pour le debug

    _console.println("== DEBUG ==");
    _console.println("Photo file size (SD): " + String(file_size));
    _console.println("Total HTTP payload size: " + String(total_size));
    _console.println("HEAD:\n" + head);
    _console.println("TAIL:\n" + tail);

    bool is_sent = false;

    _command_helper.send_command_and_wait("AT+HTTPTERM");
    if (_command_helper.send_command_and_wait("AT+HTTPINIT")) {
        if (_command_helper.send_command_and_wait("AT+HTTPPARA=\"URL\",\"" + url + "\"")) {
            if (_command_helper.send_command_and_wait("AT+HTTPPARA=\"CONTENT\",\"" + multipart_boundary + "\"")) {
                if (_command_helper.send_command_and_wait("AT+HTTPDATA=" + String(total_size) + ",60000", "DOWNLOAD", 3000)) 
                {   _console.println(" en cours d envoi des données HTTP...");

                    _command_helper.write(head, 1000);
                    size_t max_chunk_size = 4096;
                    uint8_t chunk_buf[max_chunk_size];
                    while (photoFile.available()) {
                        size_t chunk_size = photoFile.read(chunk_buf, max_chunk_size);
                        _command_helper.write(chunk_buf, chunk_size, 2000);
                    }
                    _command_helper.write(tail, 1000);
                }
                else {
                    _console.println(" Erreur : echec de lenvoi des données HTTP.");
                }
                _command_helper.send_command_and_wait("AT+HTTPACTION=1", "HTTP_PEER_CLOSED", 2000);
                _command_helper.send_command_and_wait("AT+HTTPACTION?");
                _console.println(" Code HTTP brut : " + _command_helper.last_command_response);

                is_sent = true;
            }
        }
        _command_helper.send_command_and_wait("AT+HTTPTERM");
    }

    photoFile.close();
    return is_sent;
}










} // namespace rlc
