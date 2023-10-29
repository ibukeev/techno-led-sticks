
#include <SinricPro.h>
#include <SinricProLight.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <Arduino.h>




//#define ENABLE_DEBUG
//#define SINRICPRO_NOSSL




#define WIFI_SSID         "rabbithole"
#define WIFI_PASS         "9269127120"
#define APP_KEY           "c5117452-1d75-48a7-88b8-7ab2fc03a036"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "0e0c15dd-42a6-47e4-96b7-2c87951e316c-99134b1e-898d-4c7b-a844-c8d7967353fa"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define LIGHT_ID          "653c4c513aabd5f508e0c323"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define SWITCH_ID_1       "653c8a6f3aabd5f508e0d30f"




//#define PIN 4
#define PINTEST 26
#define NUMPIXELS  20
#define BRIGHTNESS  240 // set max brightness
#define BAUD_RATE         19200                // Change baudrate to your need

// #define NEO_PTYPE  NEO_GRBW  // f.e. SK6812
//#define NEO_PTYPE NEO_GRB   // most NeoPixel products
#define GREEN    0x0000FF00 // --> GREEN
#define WHITE    0x00FFFFFF // --> WHITE
#define VIOLET    0x0000FFFF // --> WHITE



//Adafruit_NeoPixel pixels(NUMPIXELS, PINTEST, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PINTEST, NEO_GRB + NEO_KHZ800);


struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};


// Colortemperature lookup table
using ColorTemperatures = std::map<uint16_t, Color>;
ColorTemperatures colorTemperatures{
    //   {Temperature value, {color r, g, b}}
    {2000, {255, 138, 18}},
    {2200, {255, 147, 44}},
    {2700, {255, 169, 87}},
    {3000, {255, 180, 107}},
    {4000, {255, 209, 163}},
    {5000, {255, 228, 206}},
    {5500, {255, 236, 224}},
    {6000, {255, 243, 239}},
    {6500, {255, 249, 253}},
    {7000, {245, 243, 255}},
    {7500, {235, 238, 255}},
    {9000, {214, 225, 255}}};


struct DeviceState {                                   // Stores current device state with following initial values:
    bool  powerState       = false;                    // initial state is off
    Color color            = colorTemperatures[9000];  // color is set to white (9000k)
    int   colorTemperature = 9000;                     // color temperature is set to 9000k
    int   brightness       = 100;                      // brightness is set to 100
} device_state;
    


int colors[3] = {GREEN, VIOLET, WHITE};
bool powerState;
int globalBrightness = 100;
unsigned long myTime;





// the setup function runs once when you press reset or power the board


bool onPowerState(const String &deviceId, bool &state) {
  powerState = state;
  if (state) {
    strip.setBrightness(map(globalBrightness, 0, 100, 0, 255));
  } else {
    strip.setBrightness(0);
  }
  strip.show();
  return true; // request handled properly
  }

bool onBrightness(const String &deviceId, int &brightness) {
  globalBrightness = brightness;
  strip.setBrightness(map(globalBrightness, 0, 100, 0, 255));
  strip.show();
  return true;
}


bool onAdjustBrightness(const String &deviceId, int brightnessDelta) {
  globalBrightness += brightnessDelta;
  brightnessDelta = globalBrightness;
  strip.setBrightness(map(globalBrightness, 0, 100, 0, 255));
  strip.show();
  return true;
}

bool onColor(const String &deviceId, byte &r, byte &g, byte &b) {
  
  //fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
  Serial.println("Strip color:");
  Serial.println("Red");
  Serial.println(r);
  Serial.println("Green");
  Serial.println(g);
  Serial.println("Blue");
  Serial.println(b);
  uint32_t setting_color = strip.Color(r, g, b);
  Serial.println("RGB");
  Serial.println(setting_color);

  
  strip.fill(strip.Color(r, g, b), 0, NUMPIXELS);
  strip.show();
  return true;
}


bool onColorTemperature(const String& deviceId, int& colorTemperature) {
    //device_state.color            = colorTemperatures[colorTemperature];  // set rgb values from corresponding colortemperauture
    
    Serial.println("Color temperature:");

    Serial.println("Red");
    Serial.println(colorTemperatures[colorTemperature].r);
    Serial.println("Green");
    Serial.println(colorTemperatures[colorTemperature].g);
    Serial.println("Blue");
    Serial.println(colorTemperatures[colorTemperature].b);
    uint32_t setting_color = strip.Color(colorTemperatures[colorTemperature].r, colorTemperatures[colorTemperature].g, colorTemperatures[colorTemperature].b);
    Serial.println("RGB");
    Serial.println(setting_color);
//    Serial.println(strip.Color(colorTemperatures[colorTemperature].r, colorTemperatures[colorTemperature].g, colorTemperatures[colorTemperature].b));
    
    //strip.fill(setting_color, 0, NUMPIXELS);
    strip.fill(0x00FFF3F5, 0, NUMPIXELS);


    return true;
}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", localIP.toString().c_str());
}



void setupLEDStrip() {
  strip.begin(); // This initializes the NeoPixel library.
  strip.setBrightness(map(globalBrightness, 0, 100, 0, 255)); // set brightness
  strip.fill(0x00FFF3F5, 0, NUMPIXELS);
  strip.show();
}


void setupSinricPro() {
  // get a new Light device from SinricPro

  //SinricProSwitch& myLight = SinricPro[LIGHT_ID];
  //SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID_1];
  //SinricProSwitch& mySwitch = SinricPro.add<SinricProSwitch>(LIGHT_ID);

  SinricProLight &myLight = SinricPro[LIGHT_ID];

  // set callback function to device
  myLight.onPowerState(onPowerState);
  //mySwitch1.onPowerState(onPowerState1);
  myLight.onBrightness(onBrightness);
  myLight.onAdjustBrightness(onAdjustBrightness);
  
  myLight.onColor(onColor);
  myLight.onColorTemperature(onColorTemperature);                  // assign onColorTemperature callback

  // setup SinricPro
  SinricPro.onConnected([]() {
    Serial.printf("Connected to SinricPro\r\n");
  });
  SinricPro.onDisconnected([]() {
    Serial.printf("Disconnected from SinricPro\r\n");
  });
  SinricPro.restoreDeviceStates(true); // Uncomment to restore the last known state from the server.
  SinricPro.begin(APP_KEY, APP_SECRET);

}


void setup() {


  //while (!Serial);  // for Leonardo/Micro/Zero
  //Serial.begin(19200);
  Serial.begin(BAUD_RATE);

  Serial.println("Test");
  Serial.printf("\r\n\r\n");

  setupWiFi();
  setupLEDStrip();
  setupSinricPro();
  myTime = millis();
  //delay(2000);


}

int k = 0;



void loop() {

  SinricPro.handle();
  /*if (millis() - myTime > 2000)
      {
        set_pixels();
        myTime = millis();
        }*/
    




}


void set_pixels()
{
  if (k > 2)
  {
    k = 0;

  }
  else k++;


  for (int i = 0; i < 20 ; i++)
  {
    //strip.setPixelColor(i, strip.Color(0, 150, 0));
    strip.setPixelColor(i, colors[k]);
  }


  strip.show(); // Initialize all pixels to 'off'

  //delay(1000);


}
