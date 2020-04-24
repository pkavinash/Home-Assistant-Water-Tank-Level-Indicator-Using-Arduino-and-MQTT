
# Home-Assistant-Water-Tank-Level-Indicator
I have been using this project to measure sump (under ground) water tank water level with NodeMCU and ultrasonic sensor, implemented using Arduino, MQTT and Home Assistant.  

## Features
1) Calculates how many liters of water available in tank (Here we used rectangular tank)
2) Calculates Percentage of water in tank
3) Calculates distance of water level in cm
4) Calculates distance of water level in feet
5) Shows historical record of water level in cm, percentage and in liters

## Requirements
1) [NodeMCU](http://s.click.aliexpress.com/e/nlefJ4PI)
2) [Ultrasonic sensor (I used HCSRO4)](http://s.click.aliexpress.com/e/e6xt0wEy)
3) [Home Assistant App](https://play.google.com/store/apps/details?id=io.homeassistant.companion.android&hl=en)

## Installation

 - Open <code>secret.h</code> and change Wifi settings, server settings and few other parameters as per your project requirement.
 - Open <code>settings.h</code> - Usually you don't need to change any values here, but if you need any customization feel free play with it.

## Hardware Connection

### Ultrasonic Sensor to NodeMCU Connection
5V/VCC to Vin </br>
Trig to D6/12 </br>
Echo to D5/14 </br>
GND to GND </br>

## Home Assistant Configuration

### Add in configuration.yaml
```python
mqtt:
  discovery: true
  discovery_prefix: homeassistant
  broker: xxx.xxx.xxx.xxx
  port: xxxx
  client_id: Home-Assistant
  username: Your-MQTT-Username
  password: Your-MQTT-Password
    
  - platform: mqtt
    name: "Water Sump Firmware Update"
    state_topic: "/house/watersump/Confirmfirmware/"
    command_topic: "/house/watersump/firmware/"
    payload_on: "1"
    payload_off: "0"
    optimistic: true
    qos: 0
```

### Add in sensors.yaml
```python
  - platform: mqtt
    name: "Distance in feet"
    state_topic: "/house/watersump/"
    unit_of_measurement: "ft"
    value_template: '{{ value_json.distanceInFeet }}'

  - platform: mqtt
    name: "Distance in cm"
    state_topic: "/house/watersump/"
    unit_of_measurement: "cm"
    value_template: '{{ value_json.distanceInCm }}'

  - platform: mqtt
    name: "Liters"
    state_topic: "/house/watersump/"
    unit_of_measurement: "L"
    value_template: '{{ value_json.volume }}'

  - platform: mqtt
    name: "Percentage"
    state_topic: "/house/watersump/"
    unit_of_measurement: "%"
    value_template: '{{ value_json.percentage }}'
    
  - platform: mqtt
    name: "Water Sump Build Number"
    state_topic: "/house/watersump/"
    value_template: '{{ value_json.buildNumber }}'
```

## Software Setup

 - Download and install the Home Assistant  App for iOS or Android.
 - Download this repo and copy the files in to your sketches directory. Open the sketch in Arduino IDE.
 - Go to the <code>settings.h</code> tab. This is where all the customisable settings are. You should be able to change almost everything from there before compiling.
 - Go to the <code>secret.h</code> tab. Here you change Wifi settings, server settings and few other parameters as per your project requirement.
 
## Screenshot
<img src="/Images/1.jpg" width="" height="" style="max-width:100%;"></br>
<img src="/Images/2.jpg" width="" height="" style="max-width:100%;"></br>
<img src="/Images/3.png" width="350" height="" style="max-width:100%;">&nbsp; &nbsp; &nbsp; &nbsp; 
<img src="/Images/4.png" width="350" height="" style="max-width:100%;">&nbsp; &nbsp; &nbsp; &nbsp;
<img src="/Images/4.png" width="350" height="" style="max-width:100%;">
