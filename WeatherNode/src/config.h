
#ifndef Config_h
#define Config_h

#include <Arduino.h>
class MasterConfig
{
  public:
    uint32_t crc;
    uint8_t host_mac[6];
    void save();
    bool load();
};

#endif