#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

// Pin configs
#define TFT_CS     8
#define TFT_RST    7
#define TFT_DC     6

const byte interruptPin = 2;
const byte color1bit = A3;
const byte color2bit = A2;
const byte color3bit = A1;
const byte color4bit = A0;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

byte frameBuffer[32][32];

const uint16_t colorMap[] = {
    0x0000,
    0x8000,
    0xF800,
    0xF81F,
    0x0410,
    0x0400,
    0x07E0,
    0x07FF,
    0x0010,
    0x8010,
    0x001F,
    0xC618,
    0x8410,
    0x8400,
    0xFFE0,
    0xFFFF
};

void setup() {
    // Init serial
    Serial.begin(9600);
    Serial.println("Hello! mx01 gpu");
    // Init display
    tft.initR(INITR_144GREENTAB);
    tft.fillScreen(ST7735_YELLOW);
    // Fille framebuffer with black
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            frameBuffer[i][j] = B00000000;
        }
    }
    // Attach write interrupt
    attachInterrupt(digitalPinToInterrupt(interruptPin), updateFramebuffer, FALLING);
}

void loop() {
    // put your main code here, to run repeatedly:
    renderBuffer();
}

byte boolArrayToByte(boolean* bits){
    int ret = 0;
    int tmp;
    for (int i = 0; i < 4; i++) {
        tmp = bits[i];
        ret |= tmp << (4 - i - 1);
    }
    return ret;
}

void updateFramebuffer() {
    const boolean bit1 = digitalRead(color1bit);
    const boolean bit2 = digitalRead(color2bit);
    const boolean bit3 = digitalRead(color3bit);
    const boolean bit4 = digitalRead(color4bit);
    const boolean bits[] = {bit4, bit3, bit2, bit1};
    frameBuffer[1][1] = boolArrayToByte(bits); // Fixed address while I wait for shift register to arrive
}

void renderBuffer() {
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            tft.fillRect(x * 4, y * 4 , 4, 4, colorMap[frameBuffer[x][y]]);
            // tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color2);
        }
    }
}
