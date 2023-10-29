#include <Arduino.h>
#include <driver/ledc.h>
#include <LittleFS.h>
#define FSYS LittleFS
#include <ArduinoOTA.h>
#include <ESPAsyncWebSrv.h>
#define USE_OLED
#ifdef USE_OLED
	#include <M5UnitGLASS2.h>
	#include <M5Unified.h>
	#include <Avatar.h> // using ESP32C3 requires APP_CPU_NUM def
#endif

#define NAME "mini4"
#define PASS "mcbeeringi"

#define CFG_PATH "/config.bin"
#define ICON_PATH "/icon.bmp"
#define GITEKI_PATH "/giteki.bmp"

#define LFPIN 4
#define LBPIN 6
#define RFPIN 5
#define RBPIN 7

#define LFCH 0
#define LBCH 1
#define RFCH 2
#define RBCH 3

#ifdef USE_OLED
	#define I2CD 9
	#define I2CC 8

	#define GITEKI "R 201-220555\n\n\nESP32-C3-WROOM-02\n"
#endif
#define NPDI 0
