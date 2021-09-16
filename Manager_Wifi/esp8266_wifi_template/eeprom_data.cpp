#include <EEPROM.h>
#include "eeprom_data.h"
#include "hth_console_dbg.h"

#define EEPROM_DATA_PORT CONSOLE_PORT
#define EEPROM_DATA_CONSOLE(...) CONSOLE_LOGI(__VA_ARGS__)
#define EEPROM_DATA_TAG_CONSOLE(...) CONSOLE_TAG_LOGI("[EEPROM] ", __VA_ARGS__)

eeprom_data::eeprom_data(/* args */)
{
    memset(&_eepData, 0, sizeof(_eepData));
}

eeprom_data::~eeprom_data()
{
}

uint8_t eeprom_data::isDeviceActivated()
{
    return ((uint8_t)DEVICE_ACTIVATED == _eepData.activeStatus);
}

void eeprom_data::deviceActive()
{
    activePerform(ACTIVE_ENABLE_CM);
}

void eeprom_data::deviceInactive()
{
    activePerform(ACTIVE_DISABLE_CM);
}

void eeprom_data::begin()
{
    uint64_t chipID;
#ifdef ESP32
    chipID = ESP.getEfuseMac();
#elif defined(ESP8266)
    chipID = ESP.getChipId();
#endif
    EEPROM_DATA_TAG_CONSOLE("Chip ID = %04X", (uint16_t)(chipID >> 32)); //print High 2 bytes
    EEPROM_DATA_CONSOLE("%08X\r\n", (uint32_t)chipID);                         //print Low 4bytes.

    EEPROM.begin(EEPROM_BYTE_SIZE);
    EEPROM.get(0, _eepData);
    EEPROM_DATA_TAG_CONSOLE("Eeprom Info:");
    EEPROM_DATA_TAG_CONSOLE("- autoCfg: %u", _eepData.autoCfg);
    EEPROM_DATA_TAG_CONSOLE("- activeStatus: %u", _eepData.activeStatus);
    EEPROM_DATA_TAG_CONSOLE("- Chipid: %04X%08X", _eepData.block1_2[1], _eepData.block1_2[0]);

    constexpr uint8_t AUTO_CONFIG_IDENTIFICATION_VALUE = 128;
    if (AUTO_CONFIG_IDENTIFICATION_VALUE != _eepData.autoCfg)
    {
        _eepData.autoCfg = AUTO_CONFIG_IDENTIFICATION_VALUE;
        _eepData.activeStatus = 0;
        _eepData.chipID = 0;
        EEPROM.put(0, _eepData);
    }
    EEPROM.end();

    if (_eepData.chipID != chipID)
    {
        _eepData.activeStatus = DEVICE_INACTIVE;
        EEPROM_DATA_TAG_CONSOLE("Device inactive");
    }
    else
    {
        _eepData.activeStatus = DEVICE_ACTIVATED;
        EEPROM_DATA_TAG_CONSOLE("Device activated");
    }
}

void eeprom_data::activePerform(active_cmd_t Cmd)
{
    uint64_t chipID;
#ifdef ESP32
    chipID = ESP.getEfuseMac();
#elif defined(ESP8266)
    chipID = ESP.getChipId();
#endif
    EEPROM_DATA_TAG_CONSOLE("Chip ID = %04X", (uint16_t)(chipID >> 32)); //print 2 bytes High
    EEPROM_DATA_CONSOLE("%08X\r\n", (uint32_t)chipID);                   //print 4bytes Low.

    if (ACTIVE_ENABLE_CM == Cmd)
    {
        _eepData.chipID = chipID;
        _eepData.activeStatus = EEPROM_DEVICE_ACTIVE;
    }

    if (ACTIVE_DISABLE_CM == Cmd)
    {
        _eepData.chipID = 0;
        _eepData.activeStatus = EEPROM_DEVICE_INACTIVE;
    }

    EEPROM.begin(EEPROM_BYTE_SIZE);
    EEPROM.put(0, _eepData);
    EEPROM.end();
}

eeprom_data HTH_espEEPROM;