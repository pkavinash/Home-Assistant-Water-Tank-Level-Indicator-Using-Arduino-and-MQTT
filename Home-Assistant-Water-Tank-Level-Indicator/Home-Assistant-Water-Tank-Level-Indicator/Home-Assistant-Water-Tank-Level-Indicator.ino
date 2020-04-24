#include "settings.h"
 //#include "secret.h"                   // <<--- UNCOMMENT this before you use and change values on config.h tab
#include "my_secret.h"                   // <<--- COMMENT-OUT or REMOVE this line before you use. This is my personal settings.
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

void callback(char * topic, byte * payload, unsigned int length);
BlynkTimer timer;                                                         //To call a method for every certain millis

WiFiClient wifiClient;
PubSubClient client(mqtt_server, mqttPort, callback, wifiClient);

int sensSmoothArray1[FILTERSAMPLES];                                      // array for holding raw sensor values for sensor1
int sensSmoothArray2[FILTERSAMPLES];                                      // array for holding raw sensor values for sensor2

int rawData1, smoothData1;                                                // variables for sensor1 data
int rawData2, smoothData2;                                                // variables for sensor2 data

int long count = 0;                                                       //to ignore first few filtered data (power fail, device reset)

int volume;                                                               //used to calculate volume of water tank
int percent;                                                              //used to calculate percent of water present in water tank
float distanceinFeet;                                                     //used to calculate distance in feet
float waterFillLevel;                                                     // to calculate at what height water was present in tank
int distance;
long duration, originalDistanceinCm;                                      // Duration used to calculate distance

char const * waterTankSensor = "/house/watersump/";
char const * firmwareUpdateTopic = "/house/watersump/firmware/";

void setup() {
  ArduinoOTA.setHostname(OTA_HOSTNAME);                                   // A name given to your ESP8266 module when discovering it as a port in ARDUINO IDE
  ArduinoOTA.begin();                                                     // OTA initialization

  Serial.begin(115200);                                                   // Start the serial line for debugging
  delay(100);

  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);

  WiFi.begin(ssid, password);                                             // Start wifi subsystem  
  reconnectWifi();                                                        // Attempt to connect to the WIFI network and then connect to the MQTT server
  delay(2000);                                                            // Wait a bit before starting the main loop
 
  timer.setInterval(1000L, MeasureCm);                                    // Setup a function to be called every 1sec
}

void loop() {
  timer.run();
  if (WiFi.status() != WL_CONNECTED) {                                    // Reconnect if connection is lost
    reconnectWifi();
  } else if (!client.connected()) {
    reconnectMQTT();
  } else {
    client.loop();                                                        // Maintain MQTT connection
    delay(10);                                                            // MUST delay to allow ESP8266 WIFI functions to run
    ArduinoOTA.handle();
  }
}

void MeasureCm() {  
  digitalWrite(TRIGPIN, LOW);                                             //  The following trigPin/echoPin cycle is used to determine the
  delayMicroseconds(2);                                                   // distance of the nearest object by bouncing soundwaves off of it.
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  duration = pulseIn(ECHOPIN, HIGH);

  originalDistanceinCm = duration / 58.2;                                //We get water level distance from top in cm. Calculate the distance (in cm) based on the speed of sound. Distance in centimeters = Time / 58

  smoothData1 = digitalSmooth(originalDistanceinCm, sensSmoothArray1);   // every sensor you use with digitalSmooth needs its own array

  count++;                                                               // we are using this counter to ignore first few filter data

  //  Serial.println(" ");
  //  Serial.print("Ignore Counter for Smooth Filter: ");
  //  Serial.print(count);
  //  Serial.println(" ");

  if (count > FILTERSAMPLES) {
    count = FILTERSAMPLES;                                              // I thought of not to increase counter when it's satisfies our condition
    distance = smoothData1;                                             // we assign fitered data only after ignoring first few samples so we can avoid water level starting from 0 cm
    //    Serial.print("Original Distance: ");
    //    Serial.print(originalDistanceinCm);
    //    Serial.println(" cm");
    //    Serial.print("Smooth Distance: ");
    //    Serial.print(distance);
    //    Serial.println(" cm");
  } else {
    distance = originalDistanceinCm;
  }

  distanceinFeet = (distance / 30.48);                                 //We get water level distance from top in feet. Divide the length value by 30.48 to convert cm to feet.
  waterFillLevel = (WATER_TANK_HEIGHT_IN_CM - distance);               //total water tank hight - fill water level hight

  {
    if (distance >= MINIMUMRANGE && distance <= WATER_TANK_HEIGHT_IN_CM) {                    // here we are finding tank volume
      volume = ((WATER_TANK_LENGTH_IN_CM * waterFillLevel * WATER_TANK_WIDTH_IN_CM) / 1000);  // Filled Volume = Length * Width * Fill Height (Liquid Height) and divide by 1000 because we are passing vlaues in cm so to convert to liters we need to divide
      percent = ((float) volume / FULLTANK) * 100;
    }

    if (distance >= WATER_TANK_HEIGHT_IN_CM) {                        //we don't want to display negative values
      volume = 0;
      percent = 0;
    }

    if (distance < MINIMUMRANGE) {                                    //we don't want to display wrong data
      volume = FULLTANK;
      percent = 100;
    }
  }

  //  Serial.println(" ");
  //  Serial.print("Percent of Water in Tank: ");
  //  Serial.print(percent);
  //  Serial.println("%");
  //  Serial.print("Liters of Water in Sump: ");
  //  Serial.print(volume);
  //  Serial.println(" Liters");
  //  Serial.print("Distance in cm to Reach Water Level: ");
  //  Serial.print(distance);
  //  Serial.println(" cm");
  //  Serial.print("Distance in Feet to Reach Water Level: ");
  //  Serial.print(distanceinFeet);
  //  Serial.println(" Feet");
  publishData(volume, percent, distance, distanceinFeet, BUILD_NUMBER); 
}


void publishData(int p_volume, int p_percent, int p_distanceInCm, float p_distanceinFeet, int p_buildNumber) {    // function called to publish the temperature and the humidity  
  StaticJsonDocument < 200 > jsonDocument;                            // create a JSON object  
  jsonDocument["volume"] = (String) p_volume;                         // INFO: the data must be converted into a string; a problem occurs when using floats...
  jsonDocument["percentage"] = (String) p_percent;
  jsonDocument["distanceInCm"] = (String) p_distanceInCm;
  jsonDocument["distanceInFeet"] = (String) p_distanceinFeet;
  jsonDocument["buildNumber"] = (String) p_buildNumber;
  char data[200];
  serializeJson(jsonDocument, data);
  client.publish(waterTankSensor, data, true);                        //Publishing data to MQTT server as Json
  yield();
}


int digitalSmooth(int rawIn, int * sensSmoothArray) {                 //Data Filtering: "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
  int j, k, temp, top, bottom;
  long total;
  static int i;
  static int sorted[FILTERSAMPLES];
  boolean done;

  i = (i + 1) % FILTERSAMPLES;                                        // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                                         // input new data into the oldest slot

  for (j = 0; j < FILTERSAMPLES; j++) {                               // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                                                           // flag to know when we're done sorting
  while (done != 1) {                                                 // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (FILTERSAMPLES - 1); j++) {
      if (sorted[j] > sorted[j + 1]) {                                // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted[j + 1] = sorted[j];
        sorted[j] = temp;
        done = 0;
      }
    }
  }

  
  bottom = max(((FILTERSAMPLES * 15) / 100), 1);                      // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  top = min((((FILTERSAMPLES * 85) / 100) + 1), (FILTERSAMPLES - 1)); // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for (j = bottom; j < top; j++) {
    total += sorted[j];                                               // total remaining indices
    k++;
  }
  return total / k;                                                   // divide by number of samples
}

void reconnectWifi() {
  Serial.print("");
  Serial.print("Wifi status = ");
  Serial.print(WiFi.status());
  if (WiFi.status() != WL_CONNECTED) {                                // Attempt to connect to the wifi if connection is lost
    Serial.print("Connecting to ");
    Serial.println(ssid);

    while (WiFi.status() != WL_CONNECTED) {                           // Loop while we wait for connection
      delay(500);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    reconnectMQTT();
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void reconnectMQTT() {
  delay(1000);
  if (WiFi.status() == WL_CONNECTED) {                                // Make sure we are connected to WIFI before attemping to reconnect to MQTT    
    while (!client.connected()) {                                     // Loop until we're reconnected to the MQTT server
      Serial.print("Attempting MQTT connection...");
      String clientName;                                              // Generate client name based on MAC address and last 8 bits of microsecond counter
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);

      if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {  // Delete "mqtt_username", and "mqtt_password" here if you are not using any
        Serial.print("\tMQTT Connected");
        client.subscribe(firmwareUpdateTopic);                        // If connected, subscribe to the topic(s) we want to be notified about
        client.subscribe(waterTankSensor);
        client.publish("/house/watersump/Confirmfirmware/", "0");     // Sending message to MQTT server to turn off MQTT firmware upgrade button if its on
      } else {
        Serial.println("\tFailed.");
        abort();
      }
    }
  } else {
    Serial.println("Wifi is not connected");
  }
}

void callback(char * topic, byte * payload, unsigned int length) {
  String topicStr = topic;                                            // Convert topic to string to make it easier to work with
  Serial.println("Callback update.");
  Serial.print("Topic: ");
  Serial.println(topicStr);                                           // Note:  the "topic" value gets overwritten everytime it receives confirmation (callback) message from MQTT

  if (topicStr == "/house/watersump/firmware/") {
    Serial.print("Firmware update calling ");
    if (payload[0] == '1') {                                          // Turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
      Serial.print("Firmware switch on ");
      checkforupdate();
    } else if (payload[0] == '0') {                                   // Turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
      Serial.print("Firmware switch off ");
    }
  } else {
    publishData(volume, percent, distance, distanceinFeet, BUILD_NUMBER);
  }
}

void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void checkforupdate() {
  Serial.println("OTA Update Request Received");
  Serial.print("Firmware URL: ");
  Serial.println(FIRMWARE_URL);

  HTTPClient httpClient;
  httpClient.begin(FIRMWARE_URL);
  int httpCode = httpClient.GET();

  if (httpCode == 200) {
    Serial.println("Update file found, starting update");
    //    ESPhttpUpdate.onStart(update_started);                      // Add optional callback notifiers if necessary
    //    ESPhttpUpdate.onEnd(update_finished);
    //    ESPhttpUpdate.onProgress(update_progress);
    //    ESPhttpUpdate.onError(update_error);
    t_httpUpdate_return ret = ESPhttpUpdate.update(FIRMWARE_URL);

    switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.println("[update] Update failed.");
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("[update] Update no Update.");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("[update] Update ok.");                          // May not called we reboot the ESP
      break;
    }
  } else {
    client.publish("/house/switch/Confirmfirmware/", "0");            // Sending message to MQTT server to turn off MQTT firmware upgrade button if its on
    Serial.print("Firmware check failed, got HTTP response code ");
    Serial.println(httpCode);
  }
  httpClient.end();
}

String macToStr(const uint8_t * mac) {                                // Generate unique name from MAC addr
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5) {
      result += ':';
    }
  }
  return result;
}
