#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// Wifi credentials for OTA update
const char* ssid = "WIFI SSID for OTA";
const char* password = "PASSWORD";


#define R D2
#define G D1
#define B D3
#define W D4
#define button D7

int RGBWarray[4] = { 500, 500, 500, 500 };	// Brightness we want to display
int RGBWarrayReal[4] = { 0, 0, 0, 0 };		// Actual pwm value at the moment

bool onOff = 1;			// Is light on or off
int colorSelect = 0;	// Selected color number

long buttonTimer = 0;
long longPressTime = 400;
boolean buttonActive = false;
boolean longPressActive = false;
boolean singlePress = false;
boolean longPress = false;

unsigned long dimmerPreviousMillis = 0;
const long dimmerInterval = 0;

// Writes PWM values, used in debugging without dimming
void debugWritePwm()
{
	analogWrite(R, RGBWarray[0]);
	analogWrite(G, RGBWarray[1]);
	analogWrite(B, RGBWarray[2]);
	analogWrite(W, RGBWarray[3]);
}

// Writes PWM values
void writePwm() 
{
	analogWrite(R, RGBWarrayReal[0]);
	analogWrite(G, RGBWarrayReal[1]);
	analogWrite(B, RGBWarrayReal[2]);
	analogWrite(W, RGBWarrayReal[3]);
}

// This checks if we want to change the brightness of any channel, and changes the value smoothly if needed
void dimmer()
{
	unsigned long currentMillis = millis();
	if (currentMillis - dimmerPreviousMillis >= dimmerInterval)
	{
		dimmerPreviousMillis = currentMillis;
		for (int i = 0; i <= 3; i++)
		{
			if (RGBWarray[i] > RGBWarrayReal[i])
			{
				RGBWarrayReal[i] ++;
			}
			if (RGBWarray[i] < RGBWarrayReal[i])
			{
				RGBWarrayReal[i] --;
			}
		}
		writePwm();
	}
}

// Checks if any buttons pressed, and updates statuses
void buttonPress()
{
	longPress = false;
	singlePress = false;

	if (digitalRead(button) == HIGH) {
		//Serial.println("Button pressed");
		if (buttonActive == false) {
			buttonActive = true;
			buttonTimer = millis();
		}
		if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {
			longPressActive = true;
			longPress = true;
		}
	}
	else {
		if (buttonActive == true) {
			Serial.println("2");
			if (longPressActive == true) {
				longPressActive = false;
			}
			else {
				singlePress = true;
			}
			buttonActive = false;
		}
	}
}

// Ota mode waits for ota software update
void otaMode()
{
	Serial.begin(115200);
	Serial.println("Booting");
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("Connection Failed! Rebooting...");
		delay(5000);
		ESP.restart();
	}
	// Port defaults to 8266
	// ArduinoOTA.setPort(8266);

	// Hostname defaults to esp8266-[ChipID]
	// ArduinoOTA.setHostname("myesp8266");

	// No authentication by default
	// ArduinoOTA.setPassword("admin");

	// Password can be set with it's md5 value as well
	// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
	// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		else // U_SPIFFS
			type = "filesystem";

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		Serial.println("Start updating " + type);
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
		RGBWarray[0] = 100;
		RGBWarray[1] = 800;
		RGBWarray[2] = 100;
		RGBWarray[3] = 100;
		debugWritePwm();
		delay(2000);
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
		RGBWarray[0] = 100;
		RGBWarray[1] = 100;
		RGBWarray[2] = 800;
		RGBWarray[3] = 100;
		debugWritePwm();
		delay(200);
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
		RGBWarray[0] = 800;
		RGBWarray[1] = 100;
		RGBWarray[2] = 100;
		RGBWarray[3] = 100;
		debugWritePwm();
		delay(2000);
	});
	ArduinoOTA.begin();
	Serial.println("Ready");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	while (true)
	{
		Serial.println("Ota loop");
		//delay(500);
		ArduinoOTA.handle();
		RGBWarray[0] = 200;
		RGBWarray[1] = 200;
		RGBWarray[2] = 200;
		RGBWarray[3] = 200;
		debugWritePwm();
		delay(500);
		RGBWarray[0] = 10;
		RGBWarray[1] = 10;
		RGBWarray[2] = 10;
		RGBWarray[3] = 10;
		debugWritePwm();
		delay(500);
		if (digitalRead(button) == HIGH)
		{
			delay(500);
			if (digitalRead(button) == HIGH)
			{
				RGBWarray[0] = 800;
				RGBWarray[1] = 100;
				RGBWarray[2] = 100;
				RGBWarray[3] = 100;
				debugWritePwm();
				delay(2000);
				ESP.restart();
			}
		}
	}
}



void setup()
{
	Serial.begin(115200);
	delay(2000);
	Serial.println("Started");
	pinMode(R, OUTPUT);
	pinMode(G, OUTPUT);
	pinMode(B, OUTPUT);
	pinMode(W, OUTPUT);
	pinMode(button, INPUT);
	delay(500);
	if (digitalRead(button) == HIGH)
	{
		Serial.println("Ota mode!");
		delay(500);
		otaMode();
	}
}

void loop()
{
	dimmer();
	buttonPress();

	if (singlePress)
	{
		Serial.println("single press true");
		colorSelect++;
		if (colorSelect > 4)
		{
			colorSelect = 0;
		}
		Serial.println(colorSelect);
	}
	if (longPress)
	{
		onOff = !onOff;
	} 

	switch (colorSelect)
	{
	case 0:
		RGBWarray[0] = 1000;
		RGBWarray[1] = 0;
		RGBWarray[2] = 1000;
		RGBWarray[3] = 500;
		break;
	case 1:
		RGBWarray[0] = 200;
		RGBWarray[1] = 0;
		RGBWarray[2] = 200;
		RGBWarray[3] = 100;
		break;
	case 2:
		RGBWarray[0] = 1000;
		RGBWarray[1] = 0;
		RGBWarray[2] = 0;
		RGBWarray[3] = 100;
		break;
	case 3:
		RGBWarray[0] = 0;
		RGBWarray[1] = 0;
		RGBWarray[2] = 1000;
		RGBWarray[3] = 300;
		break;
	case 4:
		RGBWarray[0] = 0;
		RGBWarray[1] = 0;
		RGBWarray[2] = 0;
		RGBWarray[3] = 500;
		break;
	default:
		break;
	}

	if (onOff)
	{
		RGBWarray[0] = 0;
		RGBWarray[1] = 0;
		RGBWarray[2] = 0;
		RGBWarray[3] = 0;
	}
}
