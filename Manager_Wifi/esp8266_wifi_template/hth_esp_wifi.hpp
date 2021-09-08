#ifndef _HTH_ESP_WIFI_H
#define _HTH_ESP_WIFI_H

#include <Arduino.h>

class hth_esp_wifi
{
private:
    int getRSSIasQuality(int RSSI);
public:
    hth_esp_wifi(/* args */);
    ~hth_esp_wifi();
    void begin();
    void end();
    void connect(const char* name, const char* pass);
    int ssidScan(String &json);
};

#endif // _HTH_ESP_WIFI_H
