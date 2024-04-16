//#include <Adafruit_NeoPixel.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h>


//LED settings
#define PIN         26
#define DATA_PIN 26 //The data pin that the WS2812 strips are connected to.
#define NUMPIXELS  60
const int numLeds = 60; // Change if your setup has more or less LED's
const int numberOfChannels = numLeds * 3; // Total number of DMX channels you want to receive (1 le
CRGB leds[numLeds];

//Wifi settings - be sure to replace these with the WiFi network that your computer is connected to

const char* ssid = "rabbithole";
const char* password = "9269127120";


// Artnet settings
ArtnetWifi artnet;
const int startUniverse = 0;


bool sendFrame = 1;
int previousDataLength = 0;

// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false;
      break;
    }
    i++;
  }
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }

  return state;
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  
  Serial.println("Universe: ");
  Serial.println(universe);
  //Serial.println(data[i * 3]);
  sendFrame = 1;
  // set brightness of the whole strip 
  if (universe == 15)
  {
    FastLED.setBrightness(data[0]);
  }
  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    int led = i;
    //  + (universe - startUniverse) * (previousDataLength / 3);
    if ( (led < numLeds) && (universe == 2))
    {
      //if(universe == 1)
        //{
          leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
      //  }
      


    }
  }
  previousDataLength = length;     
  FastLED.show();
}



//Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {

Serial.begin(115200);
ConnectWifi();
artnet.begin();
FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, numLeds);

// onDmxFrame will execute every time a packet is received by the ESP32
artnet.setArtDmxCallback(onDmxFrame);



/*  pixels.begin();
  pixels.setPixelColor(2, pixels.Color(0, 255, 0));
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.setPixelColor(1, pixels.Color(255, 0, 0));
  pixels.setPixelColor(3, pixels.Color(255, 255, 255));
  pixels.show();
*/

}

void loop() {
  // put your main code here, to run repeatedly:


  artnet.read();
  /*leds[0] = CRGB(255, 0, 0);
  leds[1] = CRGB(0, 255, 0);
  leds[2] = CRGB(0, 0, 255);
  leds[3] = CRGB(255, 255, 255);
  FastLED.show();*/

}
