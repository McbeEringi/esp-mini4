#include <driver/ledc.h>
#include <LittleFS.h>
#define FSYS LittleFS
#include <ArduinoOTA.h>
#include <ESPAsyncWebSrv.h>
// #define USE_OLED
#ifdef USE_OLED
	#include <Adafruit_SSD1306.h>
	#define SSD1306_NO_SPLASH
#endif

#define NAME "mini4"
#define PASS "mcbeeringi"

#define CFG_PATH "/config.bin"

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

	const uint8_t PROGMEM icon[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x98,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x80,0x00,0x00,0x00,0x00,0x00,0x01,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x07,0xde,0x01,0xe0,0x00,0x00,0x00,0x00,0x0e,0x30,0x00,0x38,0x00,0x00,0x00,0x00,0x38,0x00,0x00,0x3c,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x06,0x00,0x00,0x00,0x01,0xc0,0x00,0x00,0x01,0x80,0x00,0x00,0x03,0xb0,0x00,0x00,0x01,0xe0,0x00,0x00,0x07,0x60,0x00,0x00,0x0c,0x20,0x00,0x00,0x0e,0xc0,0x00,0x01,0xf8,0xb0,0x00,0x00,0x1f,0x80,0x00,0x00,0x07,0x38,0x00,0x00,0x3d,0x00,0x00,0x00,0x00,0x14,0x00,0x00,0x3c,0x00,0x00,0x20,0x00,0x1e,0x00,0x00,0x54,0x00,0x00,0x40,0x00,0x0a,0x00,0x00,0x70,0x00,0x31,0x80,0x00,0x07,0x00,0x00,0xa0,0x01,0xf3,0x0e,0x00,0x0f,0x00,0x00,0xa0,0x06,0x7c,0x03,0xfe,0x07,0x00,0x01,0xe0,0x18,0x00,0x00,0x00,0x07,0x80,0x01,0x40,0x30,0x00,0x00,0x03,0x05,0x80,0x01,0x40,0x60,0x00,0x00,0x01,0x87,0x80,0x01,0x40,0xc0,0x00,0x00,0x00,0x43,0x80,0x01,0x40,0x80,0x08,0x00,0x00,0x61,0x80,0x01,0x41,0x00,0x78,0x07,0xf0,0x21,0x80,0x01,0x41,0x01,0x80,0x00,0x18,0x21,0x80,0x01,0x41,0x02,0x00,0x00,0x00,0x31,0x80,0x00,0x41,0x04,0x10,0x01,0x00,0x12,0x80,0x00,0x41,0x00,0x10,0x01,0x00,0x0b,0x00,0x00,0x20,0x00,0x10,0x01,0x00,0x03,0x00,0x00,0x20,0x00,0x10,0x01,0x00,0x06,0x00,0x00,0x30,0x00,0x10,0x01,0x00,0x0c,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x07,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x01,0xc0,0x00,0x00,0x00,0x00,0x30,0x00,0x01,0xe0,0x00,0x00,0x00,0x00,0x14,0x00,0x01,0xc0,0x00,0x00,0x00,0x00,0x18,0x00,0x03,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x0e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	const uint8_t PROGMEM giteki[]={0x00,0xfe,0x00,0x03,0xff,0xc0,0x07,0x00,0xe0,0x0c,0x00,0x30,0x18,0x00,0x18,0x30,0x00,0x0c,0x60,0x00,0x06,0x60,0x78,0x06,0x47,0xf8,0x3e,0xfe,0x1b,0xf8,0xe0,0x1f,0x80,0xc0,0x18,0x00,0xc0,0x00,0x00,0xc3,0xff,0xc0,0xc3,0xff,0xc0,0x60,0x00,0x00,0x63,0xff,0xc6,0x33,0xff,0xcc,0x30,0x18,0x0c,0x18,0x18,0x18,0x0c,0x18,0x30,0x07,0x00,0xe0,0x01,0xff,0x80,0x00,0x7e,0x00};
#else
	#define NPDI 8
#endif