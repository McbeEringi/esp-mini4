#include <Arduino.h>
#include <driver/ledc.h>
#include <LittleFS.h>
#define FSYS LittleFS
#include <ArduinoOTA.h>
#include <ESPAsyncWebSrv.h>
#include <M5UnitGLASS2.h>
#include <M5Unified.h>
#include <Avatar.h>

#define NAME "mini4"
#define PASS "mcbeeringi"

#define CFG_PATH "/config.bin"
#define ICON_PATH "/icon.bmp"
#define GITEKI_PATH "/giteki.bmp"
#define GITEKI "R 201-220555\n\n\nESP32-C3-WROOM-02\n"

#define I2CD 9
#define I2CC 8
#define NPDI 0

#define LFPIN 4
#define LBPIN 6
#define RFPIN 5
#define RBPIN 7

#define LFCH 0
#define LBCH 1
#define RFCH 2
#define RBCH 3


