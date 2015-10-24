#include "FastLED.h"

// How many leds in your strip?
#define LEDS_PER_STRIP 100
#define LEFT_FRONT_PIN 13
#define RIGHT_FRONT_PIN 14
#define LEFT_REAR_PIN 12
#define RIGHT_REAR_PIN 11

// Define the array of leds
CRGB leds[LEDS_PER_STRIP * 4];
int ledX[LEDS_PER_STRIP * 4];
int ledY[LEDS_PER_STRIP * 4];

static const int MAX_LEDS_PER_ROW = 7;
static const int ledsPerRow[] = {3, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 5, 4, 4, 5, 4, 4, 4};
static const int skipPerRow[] = {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, -1, 1, 0, 1};
static const int rowCount = sizeof(ledsPerRow)/sizeof(ledsPerRow[0]);

long startTime;

template<typename T>
class LedDraw {
public:
	static void drawFrontPart(int startLed, int startX, int startY, bool mirror = false, int t = 0) {
		int led = startLed;
		int x, y;
		x = startX;
		y = startY;
		int direction = 1;

		if (mirror) {
			direction = -1;
			x += MAX_LEDS_PER_ROW - 1;
		}

		for (int row = 0; row < rowCount; row++) {
			x += direction*skipPerRow[row];
			for (int i = 0; i < ledsPerRow[row]; i++) {
				leds[led] = T::pixel(x,y,t);
				led++;
				x += direction;
			}
			y++;
			direction = -1 * direction;
		}	
	}

	static void drawFront() {
		long time = millis() - startTime;
		drawFrontPart(0, 0, 0, false, time);
		drawFrontPart(100, 8, 0, true, time);
	}
};


void calculateVestPath(int startLed, int startX, int startY, bool mirror = false) {
	int led = startLed;
	int x, y;
	x = startX;
	y = startY;
	int direction = 1;

	if (mirror) {
		direction = -1;
		x += MAX_LEDS_PER_ROW - 1;
	}

	for (int row = 0; row < rowCount; row++) {
		x += direction*skipPerRow[row];
		for (int i = 0; i < ledsPerRow[row]; i++) {
			ledX[led] = x;
			ledY[led] = y;
			led++;
			x += direction;
		}
		y++;
		direction = -1 * direction;
	}	
}


void setup() { 
	Serial.begin(57600);
	Serial.println("resetting");
	LEDS.addLeds<NEOPIXEL,LEFT_FRONT_PIN>(leds, LEDS_PER_STRIP);
	LEDS.addLeds<NEOPIXEL,RIGHT_FRONT_PIN>(&leds[LEDS_PER_STRIP], LEDS_PER_STRIP);
	LEDS.setBrightness(0);

	calculateVestPath(0, 0, 0, false);
	calculateVestPath(100, 8, 0, true);

	startTime = millis();
}

void outlineWave() {
	uint8_t offset = 0;
	unsigned long now = millis();
	for(int i = 0; i < LEDS_PER_STRIP; i++) {
		// note about math: 1 cycle of a wave is in the range 0-255
		// so millis / divider will take 256*divider milliseconds for a complete cycle
		//uint8_t wave = quadwave8(millis()/13+i*11);
		offset += scale8(quadwave8(add8(now/117, i)), 37);
		offset += scale8(quadwave8(add8(now/29, i)), 23);
		uint8_t wave = quadwave8(offset);

		// Set the i'th led to red 
		leds[i] = CHSV(add8(i,now/27), 255, qsub8(wave,20));
		leds[i+LEDS_PER_STRIP] = CHSV(add8(i,now/27), 255, qsub8(wave,20));
		//leds[i + LEDS_PER_STRIP] = CHSV(sub8(255,add8(i,now/27)), 255, qsub8(wave,20));
	}
}

void outlineRandomAmplitude(uint16_t delta) {
	// elapsed time
	static uint16_t time = 0;
	time += delta;
	//for(int i = 0; i < )
}

CRGB plasma(int x, int y, int t) {
	byte r = sin8(sin8(x*7)+sin8(y*5) + t/11);
	byte g = sin8(sin8(x*11)+sin8(y*3) + t/13);
	byte b = sin8(sin8(x*13)+sin8(y*7) + t/23);
	return CRGB(r,g,b);
}

void plasma2d() {
	// TODO: create and initialize ledX, ledY
	for(int i = 0; i < 2 * LEDS_PER_STRIP; i++) {
		leds[i] = plasma(ledX[i], ledY[i], millis() - startTime);
	}
}

class Plasma {
public:
	static CRGB pixel(int x, int y, int t) {
		return plasma(x,y,t);
	}
};

class Diamond {
public:
	CRGB pixel(int x, int y, int t) {
		

	}

	int x;
	int y;
	int spawnTime;
};

void loop() { 
	static uint8_t hue = 0;
	Serial.print("x");
	long time = millis() - startTime;
	LEDS.setBrightness(time < 30000 ? time / 1000.f : 30);
/*
	for(int i = 0; i < OUTLINE_NUM_LEDS; i++) {
		// Set the i'th led to red 
		ledsOutline[i] = CHSV((i/10)*25, 255, 255);
	}
	//ledsOutline[5] = 0;*/
	//outlineWave();
	// OLD version
	//plasma2d();
	// NEW version
	LedDraw<Plasma>::drawFront();
	/*
	// if we're in an even minute, cylon man, otherwise, breathe
	if ((millis() / 60000) % 4 == 0) {
		fadeMan();
		uint8_t pos = triwave8(millis()/15);
		uint8_t offset = 255*(pos % MAN_NUM_LEDS) / MAN_NUM_LEDS;
		uint8_t led = map8(pos, 0, MAN_NUM_LEDS);
		ledsMan[led] = CHSV(millis()/237, 255, 255);
	} else {
		for(int i = 0; i < MAN_NUM_LEDS; i++) {
			// breathe. finish cycle through colors every 256*337 ms - ~86 seconds
			// 1 cycle of brightness is 10 seconds
			ledsMan[i] = CHSV(millis()/337, 255, qsub8(quadwave8(millis()*256/10000), 20));
		}
	}*/
	// for(int i = 0; i < LEDS_PER_STRIP; i++) {
	// 	leds[i] = 0;
	// }
	FastLED.show();
	//FastLED.delay(5);
	delay(10);
	/*
	// First slide the led in one direction
	for(int i = 0; i < OUTLINE_NUM_LEDS; i++) {
		// Set the i'th led to red 
		ledsOutline[i] = CHSV(hue++, 255, 255);
		ledsMan[i % MAN_NUM_LEDS] = CHSV(hue, 255, 255);
		// Show the leds
		FastLED.show(); 
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		fadeall();
		// Wait a little bit before we loop around and do it again
		delay(10);
	}
	Serial.print("x");
	// Now go in the other direction.  
	for(int i = (OUTLINE_NUM_LEDS)-1; i >= 0; i--) {
		// Set the i'th led to red 
		ledsOutline[i] = CHSV(hue++, 255, 255);
		ledsMan[i % MAN_NUM_LEDS] = CHSV(hue, 255, 255);
		// Show the leds
		FastLED.show();
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		fadeall();
		// Wait a little bit before we loop around and do it again
		delay(10);
	}*/
}