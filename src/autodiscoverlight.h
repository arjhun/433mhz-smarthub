#include <Arduino.h>

struct AutoDiscoverLight {
  char* name;
  char* uid;
  uint8_t remote_id;
  uint8_t channel_id;
  boolean state; 
};