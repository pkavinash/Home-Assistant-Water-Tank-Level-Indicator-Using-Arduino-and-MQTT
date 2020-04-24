//////////////////////////////////////////////////////////// 
//             Secret Configuration                      //
////////////////////////////////////////////////////////// 


/***************************************************
          WiFi Settings
 **************************************************/

#define WIFI_SSID "Your WiFi Network Name"                                    
#define WIFI_PASS "Your Wifi Password"                                   


/***************************************************
          MQTT Server Settings
 **************************************************/
const char* mqtt_server = "xxx.xxx.xx.xxx";           //MQTT server ip address
const int mqttPort = xxxx;                            //MQTT broker port

const char* mqttUser = "Your MQTT Username";          //MQTT broker username
const char* mqttPassword = "Your MQTT Password";      //MQTT broker user password


/***************************************************
          Firmware URL
 **************************************************/ 
#define FIRMWARE_URL "http://www.example.com/Home-Assistant-Water-Tank-Level-Indicator.ino.generic.bin"    // Enter the firmware URL location. Only http Protocol is supported. https not supported. Used for FOTA
