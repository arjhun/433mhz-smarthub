#include "secrets.h"
#include "autodiscoverlight.h"
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <sf501-remote.h> //https://github.com/arjhun/arduino-sf501remote
#include <AsyncMqtt_Generic.h>  //https://github.com/khoih-prog/AsyncMQTT_Generic#installation
#include <CircularBuffer.h> // https://github.com/rlogiacco/CircularBuffer

void connectToWifi();
void connectToMqtt();
void onMqttConnect(bool sessionPresent);
void sendDiscoveryHubSensors();
void sendDiscoveryLight(AutoDiscoverLight light);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void onMqttPublish(uint16_t packetId);

//create a file secrets.h with the following
/*

#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID "somessid"
#define WIFI_PASSWORD "somepassword"
#define MQTT_HOST IPAddress(###, ###, ###, ###) or MQTT_HOST "test.server.org"
#define MQTT_PORT 1883

#endif
*/

//<discovery_prefix>/<component>/[<node_id>/]<object_id>/config
const String discoveryTopic = "homeassistantArjenTest";
const String nodeId = "/433mhzhub";

const int status_interval = 300;
const String commandTopic = "set";
const String statusTopic = "stat"; 

AutoDiscoverLight light1;
AutoDiscoverLight light2;
AutoDiscoverLight light3;
AutoDiscoverLight light4;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Sf501Remote remote;
CircularBuffer<Sf501Packet, 20> packetBuffer;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  
  mqttClient.subscribe((discoveryTopic+"/light"+nodeId+"/#").c_str(), 0);
  //send 
  sendDiscoveryHubSensors();
  sendDiscoveryLight(light1);
  sendDiscoveryLight(light2);
  sendDiscoveryLight(light3);
  sendDiscoveryLight(light4);
}

void sendDiscoveryHubSensors(){
  Serial.println("Sending discovery for hub status sensors.");
  String baseTopic = discoveryTopic+"/sensor433mhz_hub_ip";
  StaticJsonDocument<200> doc;
  doc["name"] = "433mhz hub ip";
  doc["status_topic"] = baseTopic+"/"+statusTopic;
  char config[256];
  serializeJson(doc, config);
  mqttClient.publish((baseTopic+"/config").c_str(), 0, true, config); 
}

void sendDiscoveryLight(AutoDiscoverLight light){
  Serial.println("Sending discovery.");
  String baseTopic = discoveryTopic+"/light/"+light.uid;
  StaticJsonDocument<200> doc;
  doc["name"] = light.name;
  doc["cmd_t"] = baseTopic+"/"+commandTopic;
  char config[128];
  serializeJson(doc, config);
  mqttClient.publish((baseTopic+"/config").c_str(), 0, true, config); 
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  // Serial.println("receiving");
  // String sTopic = String(topic);
  // int i = sTopic.indexOf(sTopic);
  
  // if(i != -1){
    
  //   String sPayload;

  //   for(int i = index; i < len; i++){
  //     sPayload+=payload[i];
  //   }

  //   Sf501Packet packet;
  //   packet.remoteId = sTopic.substring(i+mainTopic.length(), sTopic.lastIndexOf("/")).toInt();
  //   if(sPayload == "ON") packet.state = 1;
  //   else if (sPayload == "OFF") packet.state = 0; 
  //   packet.channel = sTopic.substring( sTopic.lastIndexOf("/")+1 , sTopic.length()).toInt();
  //   packetBuffer.push(packet);
  //   }
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  remote.startTransmitter(D5);
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  
  light1.uid = "living_room_rf_light_1";
  light1.name = "livingroom light 1";
  light2.uid = "living_room_rf_light_2";
  light2.name = "livingroom light 2";
  light3.uid = "living_room_rf_light_3";
  light3.name = "livingroom light 3";
  light4.uid = "living_room_rf_light_4";
  light4.name = "livingroom light 4";

  connectToWifi();
}

const int wait = 500;

void loop() {

  static long lastTime = 0;
  
  if(millis() - lastTime > wait && !packetBuffer.isEmpty()){  
      Sf501Packet packet = packetBuffer.shift();  
      remote.sendPacket(packet, 3);
      lastTime = millis();
  }

}
