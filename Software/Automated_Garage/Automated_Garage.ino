/****************************************************
* Project Name:  Automated Garage
* Project File:  Automated_Garage.ino
*
* Description:  This will be the code for the ESP8266-01
* device.  This code will provide via MQTT the esnsor
* information of open/close for the garage door
* as well as prvide the functionality of opening the 
* garage door as well as turning the onboard garage
* light on and off.

*
* Author:  M. Sperry
* Date:  12/15/18
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


// Update these with values suitable for your network.
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";  //IP address of your Home assistant server if hosting on HASS or the MQTT server you have
#define mqtt_user "homeassistant" //enter your MQTT username
#define mqtt_password "" //enter your MQTT password

//Define the RX Pin for GPIO
#define Gsense 3

WiFiClient espClient;
PubSubClient client(espClient);

//Declaire global variablwes
String strTopic;
String strPayload;
int state = 0;

// MQTT: topic for Garage sensor
const PROGMEM char* MQTT_SENSOR_TOPIC = "ha/Gsense";

void setup_wifi() {
  delay(2000);

// un-comment to show mac address for your device if you are
// using a mac address list in your router
//  Serial.println(WiFi.macAddress());

  // We start by connecting to a WiFi network
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  digitalWrite(2,HIGH);
  Serial.println(WiFi.localIP());

  
}

// function called to publish the garage door sensor value
void publishData() {
  // create a JSON object
  // doc : https://github.com/bblanchon/ArduinoJson/wiki/API%20Reference
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& obj = jsonBuffer.createObject();

  // Get the door sensor state
  if (!digitalRead(Gsense) && state == 1)
  {
    obj["Status"] = "Open";
    state = 0;

    //un-comment for debug but leave commented
    //to increase speed of switching
 //   obj.prettyPrintTo(Serial);
 //   Serial.println("");
  
    char data[200];
    obj.printTo(data, obj.measureLength() + 1);
    client.publish(MQTT_SENSOR_TOPIC, data, true);
  }
  else if (digitalRead(Gsense) && state == 0)
  {
    obj["Status"] = "Close";
    state = 1;
    
     //un-comment for debug but leave commented
    //to increase speed of switching
//    obj.prettyPrintTo(Serial);
//    Serial.println("");
  
    char data[200];
    obj.printTo(data, obj.measureLength() + 1);
    client.publish(MQTT_SENSOR_TOPIC, data, true);    
  }
  
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  strTopic = String((char*)topic);

  //check for the garage door topic
  if(strTopic == "ha/GDoor")
    {
      // check to see if the payload is either on or off
      // doesent really matter since a garage door opener
      // only needs a momenary pulse.
      if (String((char*)payload) == "ON" || String((char*)payload) == "OFF")
      {
        //turn relay 1 on
        Serial.write(0xA0);
        Serial.write(0x01);
        Serial.write(0x01);
        Serial.write(0xA2);
        delay(500);
        //turn relay 1 off
        Serial.write(0xA0);
        Serial.write(0x01);
        Serial.write(0x00);
        Serial.write(0xA1);
        
      }
      else
      {
        // Turn relay 1 off
        Serial.write(0xA0);
        Serial.write(0x01);
        Serial.write(0x00);
        Serial.write(0xA1);
      }
    }
    // Check to see if the topic for the garage light is called
    else if (strTopic == "ha/GLight")
    {
      
      // check to see if the payload is either on or off
      // doesent really matter since a garage door opener
      // only needs a momenary pulse.
      if (String((char*)payload) == "ON" || String((char*)payload) == "OFF")
      {
        //turn relay 2 on
        Serial.write(0xA0);
        Serial.write(0x02);
        Serial.write(0x01);
        Serial.write(0xA3);
        delay(500);
        //turn relay 2 off
        Serial.write(0xA0);
        Serial.write(0x02);
        Serial.write(0x00);
        Serial.write(0xA2);
      }
      else
      {
        // Turn relay 2 off
        Serial.write(0xA0);
        Serial.write(0x02);
        Serial.write(0x00);
        Serial.write(0xA2);
      }
    }
}
 
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("ha/#");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 
void setup()
{

  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(Gsense,INPUT);
  pinMode(2,OUTPUT);

  digitalWrite(2,LOW);
}
 
void loop()
{
  if (!client.connected()) {
    reconnect();
  }

// publish the sensor data to your MQTT server to feed Home Assistant
  publishData();
  
  client.loop();
  delay(100);
}
