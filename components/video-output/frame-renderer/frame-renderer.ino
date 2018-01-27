#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

// Pin configs
#define TFT_CS     8
#define TFT_RST    7
#define TFT_DC     6
#define INPUT_DATA 9
#define INPUT_CLOCK 10
#define INPUT_LATCH 12

const byte interruptPin = 2;
const byte color1bit = A3;
const byte color2bit = A2;
const byte color3bit = A1;
const byte color4bit = A0;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

volatile byte frameBuffer[32][32];
volatile byte addressValue = 0;

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
    // Init input
    pinMode(INPUT_DATA, INPUT);
    pinMode(INPUT_LATCH, OUTPUT);
    pinMode(INPUT_CLOCK, OUTPUT); 
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

uint8_t fixedShiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
	uint8_t value = 0;
	uint8_t i;

	for (i = 0; i < 8; ++i) {
		digitalWrite(clockPin, LOW);
		if (bitOrder == LSBFIRST)
			value |= digitalRead(dataPin) << i;
		else
			value |= digitalRead(dataPin) << (7 - i);
		digitalWrite(clockPin, HIGH);
	}
	return value;
}

void updateFramebuffer() {
    // read from register
    digitalWrite(INPUT_LATCH, 1);
    delayMicroseconds(20);

    //Set latch pin to 0 to get data from the CD4021
    digitalWrite(INPUT_LATCH, 0);

    //Get CD4021 register data in byte variable
    addressValue = fixedShiftIn(INPUT_DATA, INPUT_CLOCK, MSBFIRST);
    Serial.print(addressValue);
    Serial.print("-");
    Serial.println(addressValue, BIN);
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
