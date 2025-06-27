#include <Arduino.h> // Librairie principale pour l’environnement Arduino

// Inclusion des différents modules utilisés dans l’architecture de l’AgroCam
#include "Config.h"           // Paramètres globaux (serveur, clés, seuils, etc.)
#include "Console.h"          // Gestion de l'affichage des logs sur le port série
#include "AtCommand.h"        // Envoi des commandes AT au modem SIM7600
#include "Hardware.h"         // Gestion du matériel : modem, carte SD, etc.
#include "FileHelper.h"       // Outils pour lire/écrire les fichiers CSV sur la SD
#include "Gps.h"              // Classe de gestion GPS (actuellement non utilisée)
#include "Http.h"             // Envoi de fichiers/photo via HTTP(S)
#include "Sleep.h"            // Mise en veille profonde de l’ESP32
#include "Battery.h"          // Surveillance du niveau de la batterie
#include "DateTime.h"         // Structure de représentation de date/heure
#include "GpsPoint.h"         // Données GPS (latitude/longitude) – inutilisé ici
#include "MyMath.h"           // Fonctions mathématiques diverses
#include "Camera.h"           // Capture photo via caméra OV2640

// Modules personnalisés
#include "DateModem.h"        // Récupération de la date via modem
#include "PriseHeure.h"       // Calcul du temps jusqu’à l’heure suivante
#include "moteur.h"           // Contrôle du servo moteur de protection de la caméra
#include "Ftp.h"              // Prévu pour envoi FTP (non activé ici)

// Heure cible de déclenchement de la photo
#define hour 14
#define minute 25

// Console série de debug (USB)
rlc::Console console(Serial);

// Port série dédié au modem SIM7600
HardwareSerial SerialAT(1);

// Objet d’aide pour envoyer des commandes AT au modem
rlc::AtCommand command_helper(SerialAT, console, false);

// Chemin des fichiers sur la carte SD
const char content_type[] = "application/x-www-form-urlencoded";
String gps_data_file_name = "/gps_data.csv";
String battery_data_file_name = "/bat_data.csv";

// Noms de fichier photo (SD et pour l’envoi HTTP)
String photo_filename_sd = "/photo.jpg";
String photo_filename_http = "photo.jpg";

// Initialisation des objets principaux
rlc::Hardware hw(command_helper, console, false);
rlc::FileHelper file_helper(console, false);
rlc::Gps gps(command_helper); // Non utilisé dans cette version
rlc::Http http(command_helper, console);
rlc::Sleep sleep_helper(hw, console);
rlc::Battery battery(rlc::Config::battery_zero_point_voltage, rlc::Config::battery_max_voltage, rlc::Config::battery_low_mode_percent);
rlc::Camera camera;
rlc::DateModem date_modem(command_helper);
rlc::Moteur moteur; // Contrôle le servo moteur
// rlc::Ftp ftp; // Non utilisé dans ce programme

rlc::DateTime current_date; 
bool has_entered_low_power_mode = false;
rlc::GpsPoint new_point; // Structure GPS non utilisée

void setup()
{
    // Initialisation des périphériques de la carte
    hw.init();

    // Activation de l’alimentation via le connecteur JST (obligatoire pour alimenter le modem sans USB)
    pinMode(alim_jst, OUTPUT);
    digitalWrite(alim_jst, HIGH);

    // Affichage de bienvenue et d’infos de debug
    console.println("------------------------------------------NEW START------------------------------------------");

#ifdef tsimcam
    console.println("T-SIMCAM ESP32-S3 LTE(SIM7600E)");
    console.println("                         Board: https://www.lilygo.cc/products/t-simcam");
    console.println("                  Board Github: https://github.com/Xinyuan-LilyGO/LilyGo-Camera-Series");
#endif

    console.println("           SIM7600 Command Set: https://github.com/Makerfabs/Maduino-Zero-4G-LTE/blob/main/A76XX%20Series_AT_Command_Manual_V1.05.pdf");
    console.println("               Bouygues SIM APN: ebouygtel.com");
    console.println("---------------------------------------------------------------------------------------------");

    // Initialisation SD et modem
    bool is_sd_ready = hw.init_sd();
    bool is_module_on = hw.turn_on_module();
    bool is_module_configured = is_module_on && hw.init_module();

    // Affichage des statuts
    console.println("        SD Storage Initialized: " + String(is_sd_ready ? "YES" : "NO"));
    console.println("             SIM7600 Module On: " + String(is_module_on ? "YES" : "NO"));
    console.println("     SIM7600 Module Configured: " + String(is_module_configured ? "YES" : "NO"));

    if (!is_sd_ready || !is_module_on || !is_module_configured)
    {
        // Si erreur critique → arrêt complet
        console.println("\n\n!!!!!! HALTING EXECUTION - BOARD NOT READY !!!!!!");
        while (true) {}
    }

    // Affichage des infos modem
    console.println("                  Manufacturer: " + hw.manufacturer);
    console.println("                         Model: " + hw.model);
    console.println("                          IMEI: " + hw.imei);

    console.println("---------------------------------------------------------------------------------------------");
    console.println(" Initialisation date via modem avec attente de réseau...");

    // Récupération de la date du modem (utilisée pour nommer la photo)
    String datetime = date_modem.get_datetime_string();

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

    // Affichage de l’historique batterie
    console.println("---------------------------------------------------------------------------------------------");
    console.println("Battery Data");
    file_helper.print_all_lines(battery_data_file_name);
    if (file_helper.line_count(battery_data_file_name) > 500)
    {
        file_helper.remove(battery_data_file_name);
    }

    console.println("---------------------------------------------------------------------------------------------");

    // Raison du réveil
    console.println(sleep_helper.wakeup_reason());

    // Initialisation de la caméra OV2640
    camera.initialize();

    console.println("\n");
    console.println("------------------------------TOP-OF-THE-Setup Photo------------------------------------------------");

    // Mesure tension batterie et enregistrement
    battery.refresh();
    String new_line = battery.to_csv();
    if (!file_helper.append(battery_data_file_name, new_line))
    {
        console.println("Failed to append battery data.");
        console.println(new_line);
    }

    console.println(battery.to_string());
    console.println("\n");

    // Ouverture de la trappe de protection
    moteur.init(moteur_pin);
    moteur.ouvrir();
    console.println("Moteur: Ouverture du moteur pour prendre la photo.");

    // Prise de photo si activée
    if (rlc::Config::is_camera_save_to_sd || rlc::Config::is_camera_upload_to_api)
    {
        if (camera.is_initialized && camera.take_photo())
        {
            console.println("Camera: Photo was taken.");

            // Sauvegarde sur SD
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

            // Envoi HTTP multipart si activé
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

            // Libération du buffer
            camera.return_buffer();
        }
        else
        {
            console.println("Camera Error: " + camera.last_error);
        }
    }

    // Fermeture du capot moteur après capture
    moteur.fermer();
    console.println("Moteur: Fermeture du moteur après la photo.");

    // Impression des données du modem (debug)
    hw.send_module_output_to_console_out();

    // Détermination de l'heure cible suivante
    String datetime2 = date_modem.get_datetime_string();
    if (!datetime2.startsWith("Non disponible")) {
        console.println(" Heure actuelle modem : " + datetime2);

        long sleep_ms = rlc::PriseHeure::calcul_sleep_ms(datetime2, hour, minute);
        console.println(" Mise en veille pour " + String(sleep_ms / 1000) + " secondes");
        console.println(" Mise en veille pour " + String(sleep_ms / 60000) + " minutes");
        console.println(" Mise en veille pour " + String(sleep_ms / 3600000) + " heurre");

        // Mise en sommeil profond du système jusqu'à l'heure cible
        sleep_helper.mcu_deep_sleep_module_off(sleep_ms);
    } else {
        console.println(" Impossible d'obtenir la date. Pas de sleep.");
    }
}

// Boucle principale vide : tout est exécuté dans le setup()
void loop()
{
}
