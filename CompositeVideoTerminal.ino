#include "CompositeVideoTerminal.h"
#include "font.h"

#define PIN_A 3
#define PIN_B 4
#define PIN_SUPPRESS 5
#define PIN_SYNC 9

extern "C" { void outputChars(uint8_t *pScreenRam, const uint8_t *fontSlice, uint16_t tcnt, uint16_t minTCNT); }

uint8_t screenRam[BYTES_PER_RASTER * CHARACTER_ROWS + 1] = 
"  MELO TEST ";

volatile uint16_t scanline = 0; 
ISR(TIMER1_OVF_vect) { 
	if (++scanline == 312) {
		OCR1A = 948; 
		scanline = 0;
		} else if (scanline == 8) {
		OCR1A = 74; 
		}	else if (scanline == TOP_EDGE) { 
		TIMSK1 |= _BV(OCIE1B);
	}
}

volatile uint16_t minTCNT = 0xFFFF;
volatile uint16_t maxTCNT = 0;

ISR(TIMER1_COMPB_vect) { 
	static uint8_t *pScreenRam;
	static const uint8_t *fontSlice;
	static uint8_t slice;
	
	uint16_t tcnt = TCNT1; 
	
	if (scanline == TOP_EDGE) { 
		slice = 0;
		pScreenRam = screenRam; 
		fontSlice = font; 
		} else {
		outputChars(pScreenRam, fontSlice, tcnt, minTCNT);
		if (tcnt > maxTCNT) maxTCNT = tcnt;
		if (tcnt < minTCNT) minTCNT = tcnt;

		if (scanline == BOTTOM_EDGE) {
			TIMSK1 &= ~_BV(OCIE1B); 
			} else if (++slice == PIXELS_PER_CHARACTER) {
			slice = 0;
			fontSlice = font;
			pScreenRam += BYTES_PER_RASTER;
			} else {
			fontSlice += 128;
		}
	}
}

void setup() {
  for (int i = 8 * BYTES_PER_RASTER; i < 10 * BYTES_PER_RASTER; ++i) {
    screenRam[i] |= 0x80;  
  }

	pinMode(PIN_SUPPRESS, INPUT);
	digitalWrite(PIN_SUPPRESS, LOW); 

	pinMode(PIN_A, INPUT_PULLUP);
	pinMode(PIN_B, INPUT_PULLUP);
		
	UCSR0A = _BV(U2X0);
	UCSR0B = _BV(TXEN0);
	UCSR0C = _BV(UMSEL01) | _BV(UMSEL00);
	UBRR0L = UBRR0H = 0x00;

	digitalWrite(PIN_SYNC, HIGH);
	pinMode(PIN_SYNC, OUTPUT);

	cli(); 
	TCCR1A =  _BV(COM1A1) | _BV(COM1A0) | _BV(WGM11) | _BV(WGM10); 
	TCCR1B = _BV(WGM12) | _BV(CS10); 
	OCR1A = 948; 
	OCR1B = LEFT_EDGE;
	TIMSK1 = _BV(TOIE1); 
	TCNT1 = 0x0000;
	sei(); 
  TIMSK0 &= ~_BV(TOIE0); 
}

uint32_t loopCounter = 0UL; 
void loop() {
  ++loopCounter;
  uint8_t buffer[11] = "0000000000"; 
  uint32_t n = loopCounter;
  uint8_t i = 9;
  while (n) {
    buffer[i--] = (n % 10UL) | '0';
    n /= 10UL;
  }
  for (uint8_t i = 0; i < 10; i++) {
    screenRam[BYTES_PER_RASTER * CHARACTER_ROWS - 10 + i] = buffer[i];
  }
}
