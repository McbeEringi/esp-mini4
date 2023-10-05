#include "util.h"

AsyncWebServer svr(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient *op=NULL;
float v[2]={},cfg[4]={},t,pitch;
uint32_t dt;
#ifdef USE_OLED
	Adafruit_SSD1306 display(128,64,&Wire,-1);
#endif

float fract(float x){return x-floor(x);}
float clamp(float x,float a,float b){return fmin(fmax(x,a),b);}
float saturate(float x){return clamp(x,0.,1.);}
float mix(float x,float y,float a){return x*(1.-a)+y*a;}
float step(float a,float x){return x<a?0.:1.;}
float linearstep(float a,float b,float x){return saturate((x-a)/(b-a));}
float smoothstep(float a,float b,float x){x=linearstep(a,b,x);return x*x*(3.-2.*x);}

void servo_init(uint8_t ch,uint8_t pin){ledcSetup(ch,320,LEDC_TIMER_14_BIT);ledcAttachPin(pin,ch);}
void servo(uint8_t ch,float x){ledcWrite(ch,(x*2000+500)*5.24288);}// tick/us=(hz*(2^bit))/1000000
float walk(float x,float s){x=fract(x)*2.;s*=.3;return mix(.5,saturate(mix(mix(3.,-2.,linearstep(0.,.9,x)),linearstep(.9,2.,x),step(.9,x))),s);}// [0~1]

// [ offsetLF, offsetRF, offsetLB, offsetRB ]
void cfgsave(){File x=FSYS.open(CFG_PATH,FILE_WRITE);x.write((const uint8_t*)&cfg,sizeof(cfg));x.close();}
void cfgload(){if(FSYS.exists(CFG_PATH)){File x=FSYS.open(CFG_PATH);x.read((uint8_t*)&cfg,min(sizeof(cfg),x.size()));x.close();}}

void flush(AsyncWebSocket *ws){// op tx [1,op,...clis]
	uint8_t i=2,l=ws->count()+i,a[l]={1,(uint8_t)op->id()};
	for(auto cli=ws->getClients().begin();i<l;++cli)a[i++]=(*cli)->id();
	ws->binaryAll(a,l);
}
void onWS(AsyncWebSocket *ws,AsyncWebSocketClient *client,AwsEventType type,void *arg,uint8_t *data,size_t len){
	switch(type){
		case WS_EVT_CONNECT:{// init tx [0,id,LF...,RF...,LB...,Rb...] LittleEndian float
			uint8_t l=18,a[l]={0,(uint8_t)client->id()};for(uint8_t i=0;i<16;i++)a[i+2]=*(((uint8_t*)&cfg)+i);client->binary(a,l);if(op==NULL)op=client;flush(ws);
		}break;
		case WS_EVT_DISCONNECT:{
			if(ws->count()){if(op==client)op=*(ws->getClients().begin());flush(ws);}else op=NULL;
			v[0]=0;v[1]=0;
		}break;
		case WS_EVT_DATA:{
			AwsFrameInfo *info=(AwsFrameInfo*)arg;
			if(info->final&&info->index==0&&info->len==len){
				switch(data[0]){
					case 0:cfgsave();break;// cfgsave rx [0]
					case 1:{// op rx [1,op]
						if(op==client)for(auto cli=ws->getClients().begin();*cli;++cli)if((*cli)->id()==data[1]){op=*cli;flush(ws);break;}
					}break;
					case 2:{// velocity rxtx [2,L,L,L,L,R,R,R,R] LittleEndian float
						if(op==client){v[0]=*(float*)(data+1);v[1]=*(float*)(data+5);ws->binaryAll(data,9);}
					}break;
					case 3:{ws->binaryAll(data,info->len);}break;// txt rxtx [3,...txt]
					case 4:{// offset rxtx [4,LF...,RF...,LB...,Rb...] LittleEndian float
						if(op==client){for(uint8_t i=0;i<4;i++)cfg[i]=*(float*)(data+1+(i<<2));ws->binaryAll(data,17);}
					}break;
				}
			}
		}break;
	}
}

void setup(){
	servo_init(LFCH,LFPIN);servo_init(RBCH,RBPIN);servo_init(LBCH,LBPIN);servo_init(RFCH,RFPIN);
	FSYS.begin();cfgload();

	#ifdef USE_OLED
		Wire.begin(I2CD,I2CC);
		display.begin(SSD1306_SWITCHCAPVCC,0x3c);display.setTextColor(SSD1306_WHITE);
		display.clearDisplay();
		display.drawBitmap(32,0,icon,64,64,SSD1306_WHITE);
		display.display();
	#else
		neopixelWrite(NPDI,16,0,0);
	#endif
	delay(1000);
	
	#ifdef USE_OLED
		display.clearDisplay();display.setCursor(32,8);
		display.drawBitmap(0,0,giteki,24,24,SSD1306_WHITE);
		display.printf(GITEKI);
		display.display();
		display.printf("\nWiFi...");
		display.display();
	#else
		neopixelWrite(NPDI,16,16,0);
	#endif

	WiFi.begin();
	for(uint8_t i=0;WiFi.status()!=WL_CONNECTED;i++){
		if(i>20){
			#ifdef USE_OLED
				display.printf("CFG...");display.display();
			#else
				neopixelWrite(NPDI,16,0,16);
			#endif
			WiFi.beginSmartConfig();while(!WiFi.smartConfigDone());
		}
		delay(500);
	}
	#ifdef USE_OLED
		display.printf("OK!\n");display.display();
	#else
		neopixelWrite(NPDI,0,16,0);
	#endif

	ArduinoOTA
		.setHostname(NAME).setPassword(PASS)
		.onStart([](){FSYS.end();ws.enable(false);ws.textAll("OTA update started.");ws.closeAll();})
		.onProgress([](unsigned int x,unsigned int a){
			#ifdef USE_OLED
				display.clearDisplay();display.drawBitmap(32,0,icon,64,64,SSD1306_WHITE);display.drawFastHLine(0,62,128,SSD1306_WHITE);display.fillRect(1,61,x*126/a,3,SSD1306_WHITE);display.display();
			#else
				neopixelWrite(NPDI,(a-x)*64/a,x*64/a,0);
			#endif
		})
		.onError([](ota_error_t e){
			#ifdef USE_OLED
				display.clearDisplay();display.setCursor(0,0);display.printf("OTA %s\nErr[%u]: %s_ERROR",ArduinoOTA.getCommand()==U_FLASH?"Flash":"FSYS",e,e==0?"AUTH":e==1?"BEGIN":e==2?"CONNECT":e==3?"RECIEVE":e==4?"END":"UNKNOWN");display.display();delay(5000);
			#else
				neopixelWrite(NPDI,64,0,64);
			#endif
		})
		.begin();
	ws.onEvent(onWS);svr.addHandler(&ws);
	svr.onNotFound([](AsyncWebServerRequest *request){request->redirect("/");});
	svr.serveStatic("/",FSYS,"/").setDefaultFile("index.html");
	svr.begin();
	delay(1000);
}

void loop(){
	ArduinoOTA.handle();
	#ifdef USE_OLED
		display.clearDisplay();display.setCursor(0,0);
		if(ws.count())display.printf("\n%f\n\n%f\n\n%u",v[0],v[1],ws.count());
		else display.printf("\n%s\n\n%s.local\n\n ( %s )",WiFi.SSID().c_str(),NAME,WiFi.localIP().toString().c_str());
		display.display();
	#else
		if(ws.count()){uint8_t x=(sin(millis()/1000.*3.1415)*.5+.5)*16;neopixelWrite(NPDI,x,x,x);}
		else if((uint32_t)WiFi.localIP())neopixelWrite(NPDI,0,16,0);else neopixelWrite(NPDI,16,16,0);
	#endif
	t+=(pitch=fmax(fmax(fabs(v[0]),fabs(v[1])),.3))/(millis()-dt)*.25;
	dt=millis();
	servo(LFCH,walk(t+.5,v[0]/pitch)+cfg[0]);servo(RFCH,walk(t   ,-v[1]/pitch)+cfg[1]);
	servo(LBCH,walk(t   ,v[0]/pitch)+cfg[2]);servo(RBCH,walk(t+.5,-v[1]/pitch)+cfg[3]);
	delay(10);
}
