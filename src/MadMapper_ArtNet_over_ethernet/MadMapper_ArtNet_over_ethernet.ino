
/*

This code was built based on the example code developed by @CaseyJScalf. Saved me quite a bit of time figuring out ArtNet & Teensy details. Huge thanks! 
https://github.com/CaseyJScalf/Teensy-4.1-as-ArtNet-Node-for-5v-WS2812-LED/tree/main

*/


#include <Artnet.h>
//#include <Ethernet.h>
//#include <NativeEthernet.h>
//#include <NativeEthernetUdp.h>
#include <SPI.h>
#include <OctoWS2811.h>




// LED strip settings
const int ledsPerStrip = 35;
const byte numStrips= 8; // change for your setup
const int numLeds = ledsPerStrip * numStrips;
const int numberOfChannels = numLeds * 3; // Total number of channels you want to receive (1 led = 3 channels)


DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

#define RED 0xFF0000
#define GREEN 0x00FF00
#define BLUE 0x0000FF
#define YELLOW 0xFFFF00
#define PINK 0xFF1088
#define ORANGE 0xE05800
#define WHITE 0xFFFFFF

// Artnet settings
Artnet artnet;
const int startUniverse = 0;  // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;
char buffer[40];

// Change ip and mac address for your setup
byte ip[] = { 192, 168, 1, 177 };
byte broadcast[] = {192, 168, 1, 255};
byte mac[] = { 0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC };




void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Test is running");
  //artnet.setBroadcast(broadcast);
  artnet.begin(mac, ip);

  Serial.println("ArtNet initializing...");

  
  leds.begin();
  Serial.println("Led strips initializing...");
  Serial.println("Number of pixels in setup is:");
  Serial.println(leds.numPixels());
  leds.show();
  /*
  Serial.println("Led strips test program is starting");
  initTest();



  //Serial.println("Hello, ArtNet Node Starting... Lalala");


    */

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame_simplified);
  //artnet.setArtDmxCallback(onDmxFrame);
  
  
}

void loop() {

  // we call the read function inside the loop

  //test_LEDs();

  artnet.read();
  //delay(2000);
  //artnet.printPacketHeader();
  //artnet.read_alt();
  //Serial.println("Reading Artnet");
}


void onDmxFrame_simplified(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  Serial.println("Universe number:");
  Serial.println(universe);
  Serial.println("Packet Length");
  Serial.println(length);
  
 // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    
    
    int led = i + (universe - startUniverse) * ledsPerStrip;
    //Serial.println(led);
    
    if (i < ledsPerStrip)
        {
    //          sprintf(buffer, "LED number is %d", led);
    //Serial.println(buffer);
    //sprintf(buffer, "Color is: (%d, %d, %d )", data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    //Serial.println(buffer);

      leds.setPixel(led, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
        }
  }
  leds.show();
  //delay(10000);
  //previousDataLength = length;
  
}


void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {

  Serial.println("Received callback...");
  sendFrame = 1;

  // Store which universe has got in
  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  Serial.println("Universe number received:");
  Serial.println(universe);



  // Store which universe has got in
  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  for (int i = 0 ; i < maxUniverses ; i++)
  {
    if (universesReceived[i] == 0)
    {
      sendFrame = 0;
      break;
    }
  }

 // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    int led = i + (universe - startUniverse) * ledsPerStrip;
    //int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < ledsPerStrip)
      leds.setPixel(led, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
  }
  previousDataLength = length;


  if (sendFrame) {
    leds.show();
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}


void initTest() {
  int microsec = 2000000 / leds.numPixels();
  colorWipe(RED, microsec);
  colorWipe(GREEN, microsec);
  //colorWipe(BLUE, microsec);
  //colorWipe(YELLOW, microsec);
  //colorWipe(PINK, microsec);
  //colorWipe(ORANGE, microsec);
  //colorWipe(WHITE, microsec);
}

void colorWipe(int color, int wait) {
  for (int i = 0; i < leds.numPixels(); i++) {
    leds.setPixel(i, color);
    leds.show();
    delayMicroseconds(wait);
  }
}


void test_LEDs()
  {
    //wipe everything black
    for (int i = 0; i < leds.numPixels(); i++)
      {leds.setPixel(i, 0, 0, 0);}
    leds.show();
 
    
    for (int i = 0; i < leds.numPixels(); i++)
    {
       leds.setPixel(i, 200, 0, 0);
       leds.show();
       delay(50);
      }

  }
    
    
