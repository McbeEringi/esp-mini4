#include "util.h"

Adafruit_SSD1306 display(128,64,&Wire,-1);
float tmp0,tmp1;

void servo_init(uint8_t ch,uint8_t pin){ledcSetup(ch,400,LEDC_TIMER_14_BIT);ledcAttachPin(pin,ch);}
void servo(uint8_t ch,float x){ledcWrite(ch,(x*1900+500)*6.5536);}

float fract(float x){return x-floor(x);}
float clamp(float x,float a,float b){return min(max(x,a),b);}
float saturate(float x){return clamp(x,0.,1.);}
float mix(float x,float y,float a){return x*(1.-a)+y*a;}
float step(float a,float x){return x<a?0.:1.;}
float smoothstep(float a,float b,float x){x=saturate((x-a)/(b-a));return x*x*(3.-2.*x);}

float walk(float x,bool sub){if(sub)x+=.5;x=fract(x)*2.;return smoothstep(0.,1.,mix(saturate((1.-x)*4.-1.5),x-1.,step(1.,x)));}// [0~1]

void setup(){
	Wire.begin(I2CD,I2CC);

	display.begin(SSD1306_SWITCHCAPVCC,0x3c);
	display.setTextColor(SSD1306_WHITE);
	display.setRotation(0);
	display.clearDisplay();
	display.drawBitmap(32,0,icon,64,64,SSD1306_WHITE);
	display.display();

	delay(500);
	WiFi.begin();
	for(uint8_t i=0;WiFi.status()!=WL_CONNECTED;i++){
		if(i>10){
			display.clearDisplay();display.setCursor(0,0);display.printf("smart config\n");display.display();
			WiFi.beginSmartConfig();while(!WiFi.smartConfigDone());display.printf("done\n");display.display();
		}
		delay(500);
	}
	ArduinoOTA
		.setHostname(NAME).setPassword(PASS)
		.onProgress([](unsigned int x,unsigned int a){display.clearDisplay();display.drawBitmap(32,0,icon,64,64,SSD1306_WHITE);display.drawFastHLine(0,62,128,SSD1306_WHITE);display.fillRect(1,61,x*126/a,3,SSD1306_WHITE);display.display();})
		.onError([](ota_error_t e){display.clearDisplay();display.setCursor(0,0);display.printf("%s update\nErr[%u]: %s_ERROR",ArduinoOTA.getCommand()==U_FLASH?"flash":"spiffs",e,e==0?"AUTH":e==1?"BEGIN":e==2?"CONNECT":e==3?"RECIEVE":e==4?"END":"UNKNOWN");display.display();delay(5000);})
		.begin();
	delay(500);


	servo_init(LFCH,LFPIN);servo(LFCH,.5);delay(250);
	servo_init(LBCH,LBPIN);servo(LBCH,.5);delay(250);
	servo_init(RFCH,RFPIN);servo(RFCH,.5);delay(250);
	servo_init(RBCH,RBPIN);servo(RBCH,.5);delay(250);
}

void loop(){
	ArduinoOTA.handle();
	display.clearDisplay();	display.setCursor(0,0);
	tmp0=1.5;
	display.printf("%f\n\n%s",tmp0,WiFi.localIP().toString().c_str());
	tmp0*=millis()*.001;
	tmp1=walk(tmp0,true)*.3+.35;
	tmp0=walk(tmp0,false)*.3+.35;
	display.display();
	servo(LFCH,tmp0);servo(RBCH,1.-tmp0);
	servo(LBCH,tmp1);servo(RFCH,1.-tmp1);

	delay(20);
}
