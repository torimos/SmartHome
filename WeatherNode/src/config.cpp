#include "config.h"
#include <EEPROM.h>

void MasterConfig::save()
{
  EEPROM.begin(sizeof(MasterConfig));
  EEPROM.put(0, *this);
  EEPROM.commit();
}

bool MasterConfig::load()
{
  EEPROM.begin(sizeof(MasterConfig));
  EEPROM.get(0, *this);

  if (this->crc != 0xDEADBEAF)
  {
    this->crc = 0xDEADBEAF;
    memset(this->host_mac, 0, 6);
    return false;
  }
  return true;
}