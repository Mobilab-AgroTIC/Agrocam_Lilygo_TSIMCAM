#ifndef Config_h
#define Config_h



#ifdef tsimcam

#define LTE_RESET_PIN 6
#define LTE_PWRKEY_PIN 48
#define LTE_FLIGHT_PIN 7
#define PCIE_TX_PIN 45
#define PCIE_RX_PIN 46

#define SD_MISO_PIN 40
#define SD_MOSI_PIN 38
#define SD_SCLK_PIN 39
#define SD_CS_PIN 47


#define BATTERY_PIN 3

#define CAM_PWDN_PIN -1


#define alim_jst 1
#define moteur_pin 21 // Pin du moteur servo

#endif

#include <Arduino.h>

namespace rlc
{
    class Config
    {
    public:



        static const char ftp_server[] ;
        static const int ftp_port ;
        static const char ftp_user[] ;
        static const char ftp_pass[] ;
        static const char ftp_target_path[] ; // répertoire distant




    
        static const float battery_zero_point_voltage;
        static const float battery_max_voltage;
        static const float battery_low_mode_percent;

        static const char api_url[];
        static const unsigned long api_num_gps_points_in_payload;
        static const int api_max_points_per_post;

        static const unsigned long gps_refresh_period_smallest_sec;
        static const unsigned long gps_refresh_period_default_sec;
        static const unsigned long gps_refresh_period_low_battery_sec;
        static const float gps_ideal_distance_between_points_feet;
        static const float gps_distance_threshold_feet;
        static const float gps_max_time_threshold_seconds;

        static const bool is_camera_save_to_sd;
        static const bool is_camera_upload_to_api;

    private:
    };
}

#endif