#include "util.h"

AsyncWebServer svr(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient *op=NULL;
float v[2]={},cfg[4]={},t,pitch;
uint32_t dt;
#ifdef USE_OLED
	m5avatar::Avatar avatar;
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
		auto cfg=M5.config();
		cfg.unit_glass2.pin_sda=9;
		cfg.unit_glass2.pin_scl=8;
		M5.begin(cfg);
		M5.setPrimaryDisplayType({m5::board_t::board_M5UnitGLASS2});
		M5.Lcd.setColorDepth(1);

		M5.Lcd.drawBmpFile(FSYS,ICON_PATH,32,0);
		M5.update();
	#endif
	neopixelWrite(NPDI,16,0,0);
	delay(1000);
	
	#ifdef USE_OLED
		M5.Lcd.fillScreen(TFT_BLACK);M5.Lcd.setCursor(32,8);
		M5.Lcd.drawBmpFile(FSYS,GITEKI_PATH,0,0);
		M5.Lcd.printf(GITEKI);
		M5.update();
		M5.Lcd.printf("\nWiFi...");
		M5.update();
	#endif
	neopixelWrite(NPDI,16,16,0);

	WiFi.begin();
	for(uint8_t i=0;WiFi.status()!=WL_CONNECTED;i++){
		if(i>20){
			#ifdef USE_OLED
				M5.Lcd.printf("CFG...");M5.update();
			#endif
			neopixelWrite(NPDI,16,0,16);
			WiFi.beginSmartConfig();while(!WiFi.smartConfigDone());
		}
		delay(500);
	}
	#ifdef USE_OLED
		M5.Lcd.printf("OK!\n");M5.update();
		avatar.setScale(.3);
		avatar.setPosition(-88,-96);
		avatar.init();
	#endif
	neopixelWrite(NPDI,0,16,0);

	ArduinoOTA
		.setHostname(NAME).setPassword(PASS)
		.onStart([](){
			#ifdef USE_OLED
				avatar.suspend();delay(1);
				M5.Lcd.fillScreen(TFT_BLACK);M5.Lcd.drawBmpFile(FSYS,ICON_PATH,32,0);M5.Lcd.drawFastHLine(0,62,128,TFT_WHITE);M5.update();
			#endif
			FSYS.end();ws.enable(false);ws.textAll("OTA update started.");ws.closeAll();
		})
		.onProgress([](unsigned int x,unsigned int a){
			#ifdef USE_OLED
				M5.Lcd.fillRect(1,61,x*126/a,3,TFT_WHITE);M5.update();
			#endif
			neopixelWrite(NPDI,(a-x)*64/a,x*64/a,0);
		})
		.onError([](ota_error_t e){
			#ifdef USE_OLED
				M5.Lcd.fillScreen(TFT_BLACK);M5.Lcd.setCursor(0,0);M5.Lcd.printf("OTA %s\nErr[%u]: %s_ERROR",ArduinoOTA.getCommand()==U_FLASH?"Flash":"FSYS",e,e==0?"AUTH":e==1?"BEGIN":e==2?"CONNECT":e==3?"RECIEVE":e==4?"END":"UNKNOWN");M5.update();delay(5000);
				avatar.resume();
			#endif
			neopixelWrite(NPDI,64,0,64);
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
	// #ifdef USE_OLED
	// 	M5.Lcd.fillScreen(TFT_BLACK);M5.Lcd.setCursor(0,0);
	// 	if(ws.count())M5.Lcd.printf("\n%f\n\n%f\n\n%u",v[0],v[1],ws.count());
	// 	else M5.Lcd.printf("\n%s\n\n%s.local\n\n ( %s )",WiFi.SSID().c_str(),NAME,WiFi.localIP().toString().c_str());
	// 	M5.update();
	// #endif
	if(ws.count()){uint8_t x=(sin(millis()/1000.*3.1415)*.5+.5)*16;neopixelWrite(NPDI,x,x,x);}
	else if((uint32_t)WiFi.localIP())neopixelWrite(NPDI,0,16,0);else neopixelWrite(NPDI,16,16,0);

	t+=(pitch=fmax(fmax(fabs(v[0]),fabs(v[1])),.3))/(millis()-dt)*.25;
	dt=millis();
	servo(LFCH,walk(t+.5,v[0]/pitch)+cfg[0]);servo(RFCH,walk(t   ,-v[1]/pitch)+cfg[1]);
	servo(LBCH,walk(t   ,v[0]/pitch)+cfg[2]);servo(RBCH,walk(t+.5,-v[1]/pitch)+cfg[3]);
	delay(10);
}
