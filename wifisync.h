#pragma once
#include <stdint.h>

// Wi-Fi time and weather. The board's RTC has no backup cell, so a flat battery
// or a pulled cable sends it back to the 1 January seed; this puts the real time
// back from the internet, and fetches the weather while it is connected.
//
// Set up once from the web installer (serial WIFI / TZ / GEO). The details live
// in their own NVS namespace, "wifi", so they never end up in an EXPORT backup.
// The connection runs on a background task and the radio is switched off again
// afterwards; the ESP-NOW link and a sync never run at the same time.
//
// Stubbed out for the desktop emulator, which has no radio.

void wifiBegin();                               // loads the settings; first sync ~15 s later
void wifiTick(uint32_t nowMs, bool radioBusy);  // starts a sync when one is due
bool wifiBusy();                                // a sync holds the radio right now
bool wifiTakeTime(uint32_t *localEpoch);        // a fresh local time from a finished sync, once

void wifiSetNetwork(const char *ssid, const char *pass);
void wifiForget();
void wifiSetTz(int16_t minutesEast);            // used when there is no place for the weather
void wifiSetPlace(float lat, float lon);
void wifiSyncNow();
void wifiSetNySnow(bool on);                   // the "Snow from New York" switch, saved

enum WifiState : uint8_t { WIFI_ST_NONE = 0, WIFI_ST_IDLE, WIFI_ST_BUSY, WIFI_ST_OK, WIFI_ST_FAIL };
uint8_t wifiState();
uint32_t wifiLastSync();                        // local epoch of the last good sync, 0 = never
const char *wifiNetwork();                      // the SSID, "" when not set up
#include <stddef.h>
void wifiDiag(char *out, size_t n);             // why the last sync stopped, for STATS
void wifiScanPrint();                           // serial WIFISCAN: the 2.4 GHz networks in reach
