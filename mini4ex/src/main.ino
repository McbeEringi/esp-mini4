#include "util.h"

AsyncWebServer svr(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient *op=NULL;
float v[2]={},t,pitch;
Adafruit_SSD1306 display(128,64,&Wire,-1);

float fract(float x){return x-floor(x);}
float clamp(float x,float a,float b){return min(max(x,a),b);}
float saturate(float x){return clamp(x,0.,1.);}
float mix(float x,float y,float a){return x*(1.-a)+y*a;}
float step(float a,float x){return x<a?0.:1.;}
float smoothstep(float a,float b,float x){x=saturate((x-a)/(b-a));return x*x*(3.-2.*x);}
float max(float a,float b){return a<b?b:a;}

void servo_init(uint8_t ch,uint8_t pin){ledcSetup(ch,320,LEDC_TIMER_16_BIT);ledcAttachPin(pin,ch);}
void servo(uint8_t ch,float x){ledcWrite(ch,(x*2000+500)*20.97152);}// tick/us=(hz*(2^bit))/1000000
float walkX(float x,float s){x=fract(x)*2.;s*=.4;return mix(.5,smoothstep(0.,1.,mix(saturate((1.-x)*4.-1.5),x-1.,step(1.,x))),s);}// [0~1]
float walkY(float x,float s){return .4;x=fract(x)*2.;s*=.4;return smoothstep(0.,1.,mix(saturate((1.-x)*4.-1.5),x-1.,step(1.,x)))*s+(1.-s)*.5;}// [0~1]

void flush(AsyncWebSocket *ws){// op tx [1,op,...clis]
	uint8_t l=ws->count()+2,a[l]={1,(uint8_t)op->id()};
	for(uint8_t i=2;i<l;i++)a[i]=(*(ws->getClients().nth(i-2)))->id();
	ws->binaryAll(a,l);
}
void onWS(AsyncWebSocket *ws,AsyncWebSocketClient *client,AwsEventType type,void *arg,uint8_t *data,size_t len){
	switch(type){
		case WS_EVT_CONNECT:{uint8_t l=3,a[l]={0,(uint8_t)client->id()};client->binary(a,l);if(op==NULL)op=client;flush(ws);}break;// init tx [0,id]
		case WS_EVT_DISCONNECT:
			if(ws->count()>0){if(op==client)op=*(ws->getClients().nth(ws->count()-1));flush(ws);}else op=NULL;
			v[0]=0;v[1]=0;
			break;
		case WS_EVT_DATA:{
			AwsFrameInfo *info=(AwsFrameInfo*)arg;
			if(info->final&&info->index==0&&info->len==len){
				switch(data[0]){
					case 1:{// op rx [1,op]
						if(op==client){
							AsyncWebSocketClient *tmp;
							for(uint8_t i=0;i<ws->count();i++){tmp=*(ws->getClients().nth(i));if(tmp->id()==data[1]){op=tmp;flush(ws);break;}}
						}
					}break;
					case 2:{// velocity rxtx [2,L,L,L,L,R,R,R,R] LittleEndian
						if(op==client){
							v[0]=*(float*)(data+1);
							v[1]=*(float*)(data+5);
							ws->binaryAll(data,9);
						}
					}break;
					case 3:{ws->binaryAll(data,info->len);}break;// txt rxtx [3,...txt]
				}
			}
		}break;
	}
}

void setup(){
	servo_init(LFXCH,LFXPIN);servo_init(LFYCH,LFYPIN);
	servo_init(LBXCH,LBXPIN);servo_init(LBYCH,LBYPIN);
	servo_init(RFXCH,RFXPIN);servo_init(RFYCH,RFYPIN);
	servo_init(RBXCH,RBXPIN);servo_init(RBYCH,RBYPIN);
	Wire.begin(I2CD,I2CC);

	display.begin(SSD1306_SWITCHCAPVCC,0x3c);display.setTextColor(SSD1306_WHITE);
	display.clearDisplay();
	display.drawBitmap(32,0,icon,64,64,SSD1306_WHITE);
	display.display();

	delay(1000);
	WiFi.begin();
	for(uint8_t i=0;WiFi.status()!=WL_CONNECTED;i++){
		if(i>20){
			display.clearDisplay();display.setCursor(0,0);display.printf("smart config\n");display.display();
			WiFi.beginSmartConfig();while(!WiFi.smartConfigDone());display.printf("done\n");display.display();
		}
		delay(500);
	}
	display.clearDisplay();display.setCursor(0,0);
	display.printf("\n%s\n\n%s",WiFi.SSID().c_str(),WiFi.localIP().toString().c_str());
	display.display();
	ws.onEvent(onWS);svr.addHandler(&ws);
	svr.on("/",HTTP_GET,[](AsyncWebServerRequest *request){request->send_P(200,"text/html",html);});
	svr.onNotFound([](AsyncWebServerRequest *request){request->send_P(302,"text/html",html);});
	svr.begin();
	ArduinoOTA
		.setHostname(NAME).setPassword(PASS)
		.onProgress([](unsigned int x,unsigned int a){display.clearDisplay();display.drawBitmap(32,0,icon,64,64,SSD1306_WHITE);display.drawFastHLine(0,62,128,SSD1306_WHITE);display.fillRect(1,61,x*126/a,3,SSD1306_WHITE);display.display();})
		.onError([](ota_error_t e){display.clearDisplay();display.setCursor(0,0);display.printf("%s update\nErr[%u]: %s_ERROR",ArduinoOTA.getCommand()==U_FLASH?"flash":"spiffs",e,e==0?"AUTH":e==1?"BEGIN":e==2?"CONNECT":e==3?"RECIEVE":e==4?"END":"UNKNOWN");display.display();delay(5000);})
		.begin();

	servo(LFXCH,.3);servo(LFYCH,.3);delay(250);servo(RBXCH,.3);servo(RBYCH,.3);delay(250);
	servo(LBXCH,.3);servo(LBYCH,.3);delay(250);servo(RFXCH,.3);servo(RFYCH,.3);delay(250);
}

void loop(){
	ArduinoOTA.handle();
	display.clearDisplay();display.setCursor(0,0);
	display.printf("\n%f\n\n%f\n\n%u",v[0],v[1],ws.count());
	display.display();
	t+=(pitch=max(max(fabs(v[0]),fabs(v[1])),.3))*.06;
	servo(LFXCH,walkX(t+.5,v[0]/pitch));servo(LFYCH,1.-walkY(t+.5,v[0]/pitch));servo(RFXCH,walkX(t   ,-v[1]/pitch));servo(RFYCH,walkY(t   ,v[1]/pitch));
	servo(LBXCH,walkX(t   ,v[0]/pitch));servo(LBYCH,walkY(t   ,v[0]/pitch));servo(RBXCH,walkX(t+.5,-v[1]/pitch));servo(RBYCH,1.-walkY(t+.5,v[1]/pitch));
	delay(10);
}
