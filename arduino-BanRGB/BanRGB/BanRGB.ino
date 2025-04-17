// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif


// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    0

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 64

#define START_BYTE 0xEA
#define DATA_LENGTH 4

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


byte receivedData[DATA_LENGTH]; // 用于存储接收到的四个字节
bool isReceiving = false;       // 标志是否正在接收数据
int dataIndex = 0;   

void setup() {

  Serial.begin(115200); // 初始化串口通信

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)
}


// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    // strip.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    strip.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

void loop() {

    if(isReceiving == false){
       strip.setPixelColor(receivedData[0],strip.Color(receivedData[1], receivedData[2], receivedData[3]));
       strip.show();
    }
    //rainbow(10); 
}

void serialEvent() {
  while (Serial.available() > 0) {
    byte incomingByte = Serial.read();

    if (!isReceiving) {
      // 检测到起始字节
      if (incomingByte == START_BYTE) {
        isReceiving = true;
        dataIndex = 0;
         Serial.println("Start byte detected, waiting for data...");
      }
    } else {
      // 接收后续的四个字节
      if (dataIndex < DATA_LENGTH) {
        receivedData[dataIndex] = incomingByte;
        dataIndex++;
        Serial.print("Received byte ");
        Serial.print(dataIndex);
        Serial.print(": ");
        Serial.println(receivedData[dataIndex - 1], HEX);
      }

      // 检查是否接收完成
      if (dataIndex >= DATA_LENGTH) {
        isReceiving = false;
        Serial.println("Data received complete:");
        for (int i = 0; i < DATA_LENGTH; i++) {
          Serial.print(receivedData[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
      }
    }
  }
}
