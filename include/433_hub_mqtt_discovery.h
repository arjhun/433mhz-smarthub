#include "autodiscoverlight.h"
#include "secrets.h"
#include <AsyncMqtt_Generic.h>  //https://github.com/khoih-prog/AsyncMQTT_Generic#installation

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