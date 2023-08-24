

#include <Adafruit_NeoPixel.h>

#define PIN 4
#define PINTEST 21
#define NUMPIXELS  20
#define BRIGHTNESS  240 // set max brightness

// #define NEO_PTYPE  NEO_GRBW  // f.e. SK6812
//#define NEO_PTYPE NEO_GRB   // most NeoPixel products
#define GREEN    0x0000FF00 // --> GREEN
#define WHITE    0x00FFFFFF // --> WHITE

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, LOW);

  //while (!Serial);  // for Leonardo/Micro/Zero
  Serial.begin(9600);
  Serial.println("Test message");
  

  pixels.begin();
  pixels.setPixelColor(2, pixels.Color(0, 70, 0));
  pixels.show();
  //strip.begin(); // This initializes the NeoPixel library.
  //strip.setBrightness(BRIGHTNESS); // set brightness
  //setAllPixels(WHITE);
  //strip.show(); // Initialize all pixels to 'off'
}


void loop() {
  

}
