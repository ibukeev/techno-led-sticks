#include <Adafruit_STMPE610.h>


/*

This code was built based on the example code developed by @CaseyJScalf. Saved me quite a bit of time figuring out ArtNet & Teensy details. Huge thanks! 
https://github.com/CaseyJScalf/Teensy-4.1-as-ArtNet-Node-for-5v-WS2812-LED/tree/main

*/


#include <Artnet.h>
#include <Wire.h>  
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Ethernet.h>
//#include <NativeEthernet.h>
//#include <NativeEthernetUdp.h>
#include <SPI.h>
#include <OctoWS2811.h>
//#include <HttpClient.h>
//#include <HTTPClient.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>


//Test run definitions. Could remove later if needed
#define RED 0xFF0000
#define GREEN 0x00FF00
#define BLUE 0x0000FF
#define YELLOW 0xFFFF00
#define PINK 0xFF1088
#define ORANGE 0xE05800
#define WHITE 0xFFFFFF
#define HTTP_CONNECTION_INTERVALS 2000
#define DMX_CHECK_INTERVALS 1000



//Defining OLED screen parameters
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
String OLED_message = "";


int frame_number = 0;
unsigned long last_frame_TS = 0;

//Defining networking parameters of the ESP32 orchestrating controller running Google Home IoT
char serverAddress[] = "192.168.1.25";  // server address
int port = 80;
EthernetClient Ethclient;
HttpClient client = HttpClient(Ethclient, serverAddress, port);


// Definint LED sticks parameters
const int ledsPerStrip = 35;
const byte numStrips= 8; // change for your setup
const int numLeds = ledsPerStrip * numStrips;
const int numberOfChannels = numLeds * 3; // Total number of channels you want to receive (1 led = 3 channels)

DMAMEM int displayMemory[ledsPerStrip*8];
int drawingMemory[ledsPerStrip*8];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

//Defining parameters controlling frequency of data pull from orchestrating controller
unsigned long reference_time_HTTP;
unsigned long reference_time_DMX;

//Defining Ambient colors controlled via Google Home
bool ambient_mode = false;

struct {
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  uint8_t brightness_global = 0;
  
  } ambient_color;

// Defining Artnet settings
Artnet artnet;
const int startUniverse = 0;  // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;
char buffer[40];

// Defining Teensy Networking settings
byte ip[] = { 192, 168, 0, 123 };
byte broadcast[] = {192, 168, 1, 255};
byte mac[] = { 0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC };


// Helper variable to handle first run of Teensy controller after the restart. 
bool first_run = true;

void setup() {
  Serial.begin(115200);

  //Initializing the OLED DIsplay  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
  }
  
  display.display(); // Show initial display buffer contents on the screen
  delay(2000);       // Pause for 2 seconds
  display.clearDisplay(); // Clear the buffer  

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,1);
  
  String OLED_message =  "Attempting to connect to the Ethernet...";
  display.println(OLED_message);
  Serial.println(OLED_message);
  display.display(); // Show updated display

  display.clearDisplay(); // Clear the buffer
  display.setCursor(0,1);
  delay(1000);
  
  artnet.begin(mac, ip);

  OLED_message =  "IP address:";
  display.println(OLED_message);
  Serial.println(OLED_message);
  display.println(Ethernet.localIP());
  Serial.println(Ethernet.localIP());

  OLED_message =  "Led strips initializing...";
  Serial.println("");
  Serial.println(OLED_message);
  display.println(OLED_message);

  display.display(); // Show updated display
  display.clearDisplay(); // Clear the buffer
  display.setCursor(0,1);
  delay(2000);

  /*
  if (client.connect(server, 80)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    // Make a HTTP request:
    client.println("GET /search?q=arduino HTTP/1.1");
    client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("connection failed");
  }
  */
  
  
  leds.begin();
  Serial.println("Number of pixels in setup is:");
  Serial.println(leds.numPixels());
  leds.show();
  test_LEDs();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame_simplified);

  reference_time_HTTP = millis();
  reference_time_DMX = millis();
  //artnet.setArtDmxCallback(onDmxFrame);
}


int statusCode;
String response;
DynamicJsonDocument doc(1024);
bool SMTH_CHANGED = false;

void loop() {

  // we call the read function inside the loop

  if(millis() - reference_time_DMX > DMX_CHECK_INTERVALS)
        {
          
          bool DMX_read = digitalRead(24);
          OLED_message =  "IP address:";
          display.println(OLED_message);
          display.println(Ethernet.localIP());
          display.println("");
          OLED_message =  "Control mode:";
          Serial.println(OLED_message);
          display.println(OLED_message);


          //Debugging digital Pin read
          //Serial.println("Pin 24 value:");
          //Serial.println(DMX_read);
          
          if(DMX_read) {
            
            ambient_mode = true;
            OLED_message = "Ambient (Google Home)";
            Serial.println(OLED_message);
            display.println(OLED_message);

            //Serial.println("Mode is Ambient");
            } else {ambient_mode = false;
            //Serial.println("Mode is DMX");
            
            
            OLED_message = "Artnet DMX";
            Serial.println(OLED_message);
            display.println(OLED_message);            
            
            
            
            }

          SMTH_CHANGED = digitalRead(25);
          //Serial.println("Did smth change?");
          //Serial.println(SMTH_CHANGED);

          display.display(); // Show updated display
          display.clearDisplay(); // Clear the buffer
          display.setCursor(0,1);

          reference_time_DMX = millis();
          
          }  

  //If we running DMX control
  if (!ambient_mode)
      {

        //Serial.println("Reading Artnet");
        artnet.read();
        //delay(500);
        }

        else 

          {

              //if(millis() - reference_time_HTTP > HTTP_CONNECTION_INTERVALS)
              if(SMTH_CHANGED || first_run)
                    {   


                        call_ESP_Ethernet_and_make_adjustments();
                    }
          }


    first_run = false; 
}



void onDmxFrame_simplified(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  Serial.println("Universe number:");
  Serial.println(universe);
  Serial.println("Packet Length");
  Serial.println(length);

  if (universe == 1)
      {
        frame_number++;
        unsigned long delta_frame_time = millis() - last_frame_TS;
        last_frame_TS = millis();
        if (frame_number%30 == 0 )
          {
            display.println("CUrrent frame 1 delta:");
            display.println(delta_frame_time);
            display.display(); // Show updated display
            display.clearDisplay(); // Clear the buffer
            display.setCursor(0,1);



          }

      }
  
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
       leds.setPixel(i, 200, 200, 200);
       
       //delay(50);
      }
      leds.show();

  }
    

void ambient_color_fill(uint8_t red, uint8_t green, uint8_t blue)
    {
      for (int i = 0; i < leds.numPixels(); i++)
            { 
              leds.setPixel(i, blue, red , green); 
              //leds.setPixel(i, 255, 0, 0); 
              }

        leds.show();
      
      
      }


int round_to_10(uint8_t input_int)
{
    
    // Smaller multiple 
    int a = (input_int / 20) * 20; 

    // Larger multiple 
    int b = a + 20; 

    int closest_int = (input_int - a >= b - input_int)? b : a; 
    
    if (closest_int > 255) {closest_int = 255;}
    if (closest_int < 40) {closest_int = 0;}

    return closest_int;
  
  }


void measure_color()
     {
        ambient_color.red = round_to_10(map(analogRead(A12), 0, 1023, 0, 255));
        ambient_color.green = round_to_10(map(analogRead(A13), 0, 1023, 0, 255));
        ambient_color.blue = round_to_10(map(analogRead(A11), 0, 1023, 0, 255));
        ambient_color.brightness_global = round_to_10(map(analogRead(A10), 0, 1023, 0, 255));
        Serial.println("Brightness is:");
        Serial.println(ambient_color.brightness_global);
        

      
      
      }


void call_ESP_Ethernet_and_make_adjustments()
    {

      
                  Serial.println("making GET request");
                  client.get("/color");
                  int statusCode = client.responseStatusCode();
                  String response = client.responseBody();
                  
                  Serial.print("Status code: ");
                  Serial.println(statusCode);
                  Serial.print("Response: ");
                  Serial.println(response);
                  
                  
                  deserializeJson(doc, response);
                  
                  uint32_t resp = doc["value"];
                  //uint32_t blue_color = 
                  ambient_color.blue = (resp & 0x000000ff);
                  //uint32_t green_color = 
                  ambient_color.green = (resp & 0x0000ff00) >> 8;
                  //uint32_t red_color 
                  ambient_color.red = (resp & 0x00ff0000) >> 16;



                  client.get("/brightness");
                  statusCode = client.responseStatusCode();
                  response = client.responseBody();
                  deserializeJson(doc, response);
                  ambient_color.brightness_global = doc["value"];
                  
                  ambient_color_fill(ambient_color.red*ambient_color.brightness_global/255, ambient_color.green*ambient_color.brightness_global/255,ambient_color.blue*ambient_color.brightness_global/255);
                  
                  Serial.printf("Color is R:%d, G:%d, B:%d", ambient_color.red, ambient_color.green, ambient_color.blue);
                  delay(2000);


              reference_time_HTTP = millis();  
      
      
      
      }
