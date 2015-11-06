#include "FastLED.h"

// How many leds in your strip?
#define LEDS_PER_STRIP 100
#define LEFT_FRONT_PIN 13
#define RIGHT_FRONT_PIN 14
#define LEFT_REAR_PIN 11
#define RIGHT_REAR_PIN 12

// Define the array of leds
CRGB leds[LEDS_PER_STRIP * 4];
int ledX[LEDS_PER_STRIP * 4];
int ledY[LEDS_PER_STRIP * 4];

static const int MAX_LEDS_PER_ROW_FRONT = 7;
static const int ledsPerRow[] = {3, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 5, 4, 4, 5, 4, 4, 4};
static const int skipPerRow[] = {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, -1, 1, 0, 1};
static const int MAX_LEDS_PER_ROW_BACK = 6;
static const int ledsPerRowBackLeft[] = {4, 4, 4, 4, 4, 5, 5, 5, 6, 6, 5, 5, 5, 5, 5, 6, 6}; // total 84 LEDs
static const int skipPerRowBackLeft[] = {0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0};
static const int ledsPerRowBackRight[] = {4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6};
static const int skipPerRowBackRight[] = {0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0};

static const int rowCountFront = sizeof(ledsPerRow)/sizeof(ledsPerRow[0]);
static const int rowCountBack = sizeof(ledsPerRowBackLeft)/sizeof(ledsPerRowBackLeft[0]);

static const int LEDS_BACK_LEFT = 84;
static const int LEDS_BACK_RIGHT = 81;

long startTime;

template<typename T>
class LedDraw {
public:
	static void drawFrontPart(T& instance, int startLed, int startX, int startY, bool mirror = false, int t = 0) {
		int led = startLed;
		int x, y;
		x = startX;
		y = startY;
		int direction = 1;

		if (mirror) {
			direction = -1;
			x += MAX_LEDS_PER_ROW_FRONT - 1;
		}

		for (int row = 0; row < rowCountFront; row++) {
			x += direction*skipPerRow[row];
			for (int i = 0; i < ledsPerRow[row]; i++) {
				leds[led] = instance.pixel(x,y,t);
				led++;
				x += direction;
			}
			y++;
			direction = -1 * direction;
		}	
	}

	static void drawBackPart(T& instance, const int *ledCounts, const int *skipCounts, int startLed, int startX, int startY, bool mirror = false, int t = 0) {
		int led = startLed;
		int x, y;
		x = startX;
		y = startY;
		int direction = 1;

		if (mirror) {
			direction = -1;
			x += MAX_LEDS_PER_ROW_BACK - 1;
		}

		for (int row = 0; row < rowCountBack; row++) {
			x += direction*skipCounts[row];
			for (int i = 0; i < ledCounts[row]; i++) {
				leds[led] = instance.pixel(x,y,t);
				led++;
				x += direction;
			}
			y++;
			direction = -1 * direction;
		}	
	}



	static void drawFront(T& instance) {
		long time = (millis() - startTime);
		drawFrontPart(instance, 0, 0, 0, false, time);
		drawFrontPart(instance, LEDS_PER_STRIP, MAX_LEDS_PER_ROW_FRONT+1, 0, true, time);
	}
	static void drawBack(T& instance) {
		long time = (millis() - startTime);
		drawBackPart(instance, ledsPerRowBackLeft, skipPerRowBackLeft, LEDS_PER_STRIP*2, 0, 0, true, time);
		drawBackPart(instance, ledsPerRowBackRight, skipPerRowBackRight, LEDS_PER_STRIP*2 + LEDS_BACK_LEFT, MAX_LEDS_PER_ROW_BACK, 0, false, time);
	}
	static void draw(T& instance) {
		drawFront(instance);
		drawBack(instance);
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
		x += MAX_LEDS_PER_ROW_FRONT - 1;
	}

	for (int row = 0; row < rowCountFront; row++) {
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
	LEDS.addLeds<NEOPIXEL, LEFT_REAR_PIN>(&leds[LEDS_PER_STRIP*2], LEDS_BACK_LEFT);
	LEDS.addLeds<NEOPIXEL, RIGHT_REAR_PIN>(&leds[LEDS_PER_STRIP*2 + LEDS_BACK_LEFT], LEDS_BACK_RIGHT);
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
	CRGB pixel(int x, int y, int t) {
		return plasma(x,y,t);
	}
};

class PalettedPlasma {
private:
	CRGBPalette16 current;
	CRGBPalette16 target;
	CRGBPalette16 last;
	long startTime;
	long lastTime;
public:
	CRGB pixel(int x, int y, int t) {
		byte index = sin8(sin8(x*7/3)+sin8(y*5/3) + t/17);
		byte brightness = sin8(sin8(x*11/3)+sin8(y*3/3) + t/19);
		return ColorFromPalette(current, index, qsub8(brightness, 30));
	}

	PalettedPlasma() {
		lastTime = startTime = millis();
		last = RainbowColors_p;
		target = PartyColors_p;
	}



	void update() {
		int seconds = (millis() - lastTime);

		if (seconds > 5000) {
			last = target;
			lastTime = millis();
			switch(random8() % 4) {
			case 0:
				target = RainbowColors_p;
				break;
			case 1:
				target = PartyColors_p;
				break;
			case 2:
				target = LavaColors_p;
				break;
			case 3:
				target = ForestColors_p;
				break;
			}
			//current = target;
		} else {
			blend(last.entries, target.entries, current.entries, 16, seconds*255/5000);
		}


	}

	void draw() {
		LedDraw<PalettedPlasma>::draw(*this);
	}

};

class Diamond {
public:
	CRGB pixel(int x, int y, int t) {
		
		return 0;
	}

	int x;
	int y;
	int spawnTime;
};


class Animation {
public:
	virtual void update() = 0;
	virtual void draw() = 0;
};

DEFINE_GRADIENT_PALETTE(firePalette) {
	0, 0,0,0,
	32, 255,0,0,
	96, 255,128,0,
	128, 255,128,96,
	255, 255,128,96
};


class Fire : Animation {
private:
	int width;
	int height;

	uint8_t *buffer1;
	uint8_t *buffer2;

	uint8_t *src;
	uint8_t *dst;

	CRGBPalette256 palette;
public:
	CRGB pixel(int x, int y, int t) {
		return ColorFromPalette(palette, src[x + 1 + (y + 2)*width], 255, LINEARBLEND);
		//return ColorFromPalette(LavaColors_p, src[x + (y + 2)*width], 255, LINEARBLEND);
		
	}

	Fire(int width, int height) {
		palette = firePalette;
		this->width = width;
		this->height = height + 2;
		buffer1 = new uint8_t[width * height];
		buffer2 = new uint8_t[width * height];
		src = buffer1;
		dst = buffer2;
		memset(buffer1, 0, width*height*sizeof(buffer1[0]));
		memset(buffer2, 0, width*height*sizeof(buffer2[0]));
	}

	void update() {
		for(int x = 1; x < width - 1; x++) {
			for(int y = 0; y < 2; y++) {
				src[x + y*width] = (random8() + 128) / 2;// > 128 ? 255 : 0;
			}
		}
		for(int x = 1; x < width - 1; x++) {
			for(int y = 2; y < height; y++) {
				dst[x + y*width] = qsub8((src[x + (y-1)*width] + src[x + 1 + (y-1)*width] \
									+ src[x - 1 + (y-1	)*width] + src[x + (y-2)*width]) / 4, 10);
			}
		}
		uint8_t* tmp = src;
		src = dst;
		dst = tmp;
		delay(25);
	}	

	void draw() {
		LedDraw<Fire>::drawFront(*this);
	}
};
static Fire fire(18,18);
static Plasma plasmaState;
static PalettedPlasma palPlasma;

void loop() { 
	static uint8_t hue = 0;
	Serial.print("x");
	long time = millis() - startTime;
	LEDS.setBrightness(time < 15000 ? time / 500.f : 60);
	//fire.update();
	//fire.draw();
	palPlasma.update();
	palPlasma.draw();
	//LedDraw<Plasma>::drawFront(plasmaState);
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
	//LedDraw<Plasma>::drawFront();
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