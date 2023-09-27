//

#ifndef HOLOC_WIFI_H

#define HOLOC_WIFI_H

#define AP_SSID  "holoC"
#define AP_PASS  "12345678"

void wifiInit();

char *wifiScanAP();

char *getIp();

void wifi_connect_ap(char *ssid, char *password);

#endif
