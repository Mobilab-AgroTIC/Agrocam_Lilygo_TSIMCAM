#include <Arduino.h>
#include "Config.h"
#include "Console.h"
#include "AtCommand.h"
#include "Hardware.h"
#include "FileHelper.h"
#include "Gps.h"
#include "Http.h"
#include "Sleep.h"
#include "Battery.h"
#include "DateTime.h"
#include "GpsPoint.h"
#include "MyMath.h"
#include "Camera.h"
// personal includes
#include "DateModem.h"
#include "PriseHeure.h"
#include "moteur.h" // Uncomment if you want to use the motor control
#include "Ftp.h"

#define hour 16
#define minute 35
#define moteur_pin 21 // Pin du moteur servo
#define alim_jst 1


rlc::Console console(Serial); 
HardwareSerial SerialAT(1); // SerialAT est utilisé pour la communication avec le module SIM7600
rlc::AtCommand command_helper(SerialAT, console, false);


const char content_type[] = "application/x-www-form-urlencoded";
String gps_data_file_name = "/gps_data.csv";
String battery_data_file_name = "/bat_data.csv";
String photo_filename_sd = "/photo.jpg";  // initialisation par défaut
String photo_filename_http = "photo.jpg"; // initialisation par défaut

rlc::Hardware hw(command_helper, console, false);
rlc::FileHelper file_helper(console, false);
rlc::Gps gps(command_helper);
rlc::Http http(command_helper, console);
rlc::Sleep sleep_helper(hw, console);
rlc::Battery battery(rlc::Config::battery_zero_point_voltage, rlc::Config::battery_max_voltage, rlc::Config::battery_low_mode_percent);
rlc::Camera camera;
rlc::DateModem date_modem(command_helper);
rlc::Moteur moteur; // Uncomment if you want to use the motor control
//rlc::Ftp ftp;


rlc::DateTime current_date;



bool has_entered_low_power_mode = false;

rlc::GpsPoint new_point;

void setup()
{
    

    
    hw.init();

    pinMode(alim_jst, OUTPUT);      // GPIO1 doit être configuré en sortie
    digitalWrite(alim_jst, HIGH);   // Mettre à HIGH pour activer l'alimentation via JST


    console.println("------------------------------------------NEW START------------------------------------------");

#ifdef tsimcam
    console.println("T-SIMCAM ESP32-S3 LTE(SIM7600E)");
    console.println("                         Board: https://www.lilygo.cc/products/t-simcam");
    console.println("                  Board Github: https://github.com/Xinyuan-LilyGO/LilyGo-Camera-Series");
#endif

    console.println("           SIM7600 Command Set: https://github.com/Makerfabs/Maduino-Zero-4G-LTE/blob/main/A76XX%20Series_AT_Command_Manual_V1.05.pdf");
    console.println("               Bouygues SIM APN: ebouygtel.com");
    console.println("---------------------------------------------------------------------------------------------");

    bool is_sd_ready = hw.init_sd();
    bool is_module_on = hw.turn_on_module();
    bool is_module_configured = is_module_on && hw.init_module();

    String sd_status = is_sd_ready ? "YES" : "NO";
    String module_on_status = is_module_on ? "YES" : "NO";
    String module_configured_status = is_module_configured ? "YES" : "NO";

    console.println("        SD Storage Initialized: " + sd_status);
    console.println("             SIM7600 Module On: " + module_on_status);
    console.println("     SIM7600 Module Configured: " + module_configured_status);
 
    if (!is_sd_ready || !is_module_on || !is_module_configured)
    {
        console.println("\n\n!!!!!! HALTING EXECUTION - BOARD NOT READY !!!!!!");
        while (true) {}
    }

    console.println("                  Manufacturer: " + hw.manufacturer);
    console.println("                         Model: " + hw.model);
    console.println("                          IMEI: " + hw.imei);

    // Wait for GPS fix before proceeding
    console.println("---------------------------------------------------------------------------------------------");


    console.println(" Initialisation date via modem avec attente de réseau...");

    String datetime = date_modem.get_datetime_string();





// Heure réelle du réveil
if (!datetime.startsWith("Non disponible")) {
    String filename = datetime;
    filename.replace(":", "_");
    filename.replace(" ", "_");

    photo_filename_sd = "/photo_" + filename + ".jpg";
    photo_filename_http = "photo_" + filename + ".jpg";

    console.println(" Nom de la photo défini : " + photo_filename_sd);
} else {
    photo_filename_sd = "/photo.jpg";
    photo_filename_http = "photo.jpg";
    console.println(" Date inconnue. Nom par défaut utilisé.");
}



    console.println("---------------------------------------------------------------------------------------------");
    console.println("Battery Data");
    file_helper.print_all_lines(battery_data_file_name); 
    if (false || file_helper.line_count(battery_data_file_name) > 500)
    {
        file_helper.remove(battery_data_file_name);
    }

    console.println("---------------------------------------------------------------------------------------------");

    console.println(sleep_helper.wakeup_reason());
    camera.initialize();
    console.println("\n");
    console.println("------------------------------TOP-OF-THE-Setup Photo------------------------------------------------");

    // Monitor the current battery voltage and state of charge
    battery.refresh();

    String new_line = battery.to_csv();
    if (!file_helper.append(battery_data_file_name, new_line))
    {
        console.println("Failed to append battery data.");
        console.println(new_line);
    }

    console.println(battery.to_string());
    console.println("\n");

    moteur.init(moteur_pin); // Initialize the motor if you want to use it
    moteur.ouvrir(); // Open the motor (e.g., turn to 90 degrees)
    console.println("Moteur: Ouverture du moteur pour prendre la photo.");


    if (rlc::Config::is_camera_save_to_sd || rlc::Config::is_camera_upload_to_api)
    {
        
        if (camera.is_initialized && camera.take_photo())
        {
            console.println("Camera: Photo was taken.");


        if (rlc::Config::is_camera_save_to_sd)
            {
                if (file_helper.write(photo_filename_sd, camera.photo_buffer, camera.photo_buffer_size))
                {
                    console.println("Camera: Photo saved to SD card as " + photo_filename_sd);
                }
                else
                {
                    console.println("Camera: Failed to write photo to SD card.");
                }
            }
                         
         if (rlc::Config::is_camera_upload_to_api)
            {
                if (hw.is_cellular_connected(true))
                {
                    if (http.post_file_from_sd(rlc::Config::api_url, photo_filename_sd, photo_filename_http))
                    {
                        console.println("Camera: Photo uploaded to API");
                    }
                    else
                    {
                        console.println("Camera: Failed to upload photo to API.");
                    }
                }
                else
                {
                    console.println("Photo Upload: No cellular found.");
                }
            }

            camera.return_buffer();
        }
        else
        {
            console.println("Camera Error: " + camera.last_error);
        }
    }

    moteur.fermer(); // Close the motor (e.g., return to 0 degrees)
    console.println("Moteur: Fermeture du moteur après la photo.");



    // Inter serial port communications
    //
    hw.send_module_output_to_console_out();

    //hw.send_console_input_to_module();


// À la fin du setup une fois la photo envoyée
    //int sleep_duration_ms = 160 * 1000; // 160 secondes
  //  sleep_helper.mcu_deep_sleep_module_off(sleep_duration_ms);

      String datetime2 = date_modem.get_datetime_string();


  if (!datetime2.startsWith("Non disponible")) {
    console.println(" Heure actuelle modem : " + datetime2);

    // Calcul du moment où il faut se réveiller (par exemple 12h00)

    long sleep_ms = rlc::PriseHeure::calcul_sleep_ms(datetime2, hour, minute);
    console.println(" Mise en veille pour " + String(sleep_ms / 1000) + " secondes");
    console.println(" Mise en veille pour " + String(sleep_ms / 60000) + " minutes");
    console.println(" Mise en veille pour " + String(sleep_ms / 3600000) + " heurre");


    sleep_helper.mcu_deep_sleep_module_off(sleep_ms);  // → Va se réveiller à l'heure cible
} else {
    console.println(" Impossible d'obtenir la date. Pas de sleep.");
}



}

// loop() remains unchanged


void loop()
{

}