#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <stdarg.h>
#include <inttypes.h>

#define F_CPU 1600000
#define BAUD 9600

#include <util/delay.h>
#include <util/setbaud.h>

#define PORT_VID    PORTD
#define DDR_VID     DDRD
#define PIN_VID     7

#define PORT_SYN    PORTB
#define DDR_SYN     DDRB
#define PIN_SYN     1

#define PORT_LED    PORTB
#define DDR_LED     DDRB
#define PIN_LED     5

#define NS_PER_TICK     250
#define NS_PER_US       1000
#define TICKS_PER_US    (NS_PER_US / NS_PER_TICK)

#define CYCLES_PER_SECOND   F_CPU
#define CYCLES_PER_MS       (CYCLES_PER_SECOND / 1000)
#define CYCLES_PER_US       (CYCLES_PER_MS / 1000)
#define CYCLES_PER_TICKS    (CYCLES_PER_US / TICKS_PER_US)

#define DELAY_TCNT(n) do { \
    cli(); \
    uint16_t _dts = TCNT1; \
    while ((TCNT1 - _dts) < (n-4)) {} \
    sei(); \
} while (0)

static void seinit() {
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBBRL_VALUE;
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    UCSR0B = _BV(TXEN0);
    UCSR0A = 0;
}

static void sewritec(char c) {
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

static void sewritestr(const char *str) {
    while (*str) {
        sewritec(*str);
        str++;
    }
}

static void seprintf(const char *fmt, ...) {
    char buff[512];
    va_list va_args;
    va_start(va_args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, va_args);
    sewritestr(buff);
    va_end(va_args);
}

int main(void) {
    seinit();

    TCCR1A = 0;
    TCCR1B = 1;

    DDR_LED |= (1 << PIN_LED);
    DDR_VID |= (1 << PIN_VID);
    DDR_SYN |= (1 << PIN_SYN);

    uint8_t hsc = 0;
    uint8_t tkc = 0;

    while (1) {
        cli();
        uint16_t start = TCNT1, end;

        if (hsc < 5) {
            for (uint8_t i = 0; i < 5; i++) {
                PORT_SYN &= ~(1 << PIN_SYN);
                _delay_us(125);
                _delay_us(750);
                _delay_us(1);
                _delay_us(20);
                _delay_us(5);
                _delay_ms(2);
                _delay_us(750);
                _delay_us(125);
                PORT_SYN |= (1 << PIN_SYN);
                _delay_us(1);
                _delay_us(750);
            }
        } else if (hsc < 10) {
            // Operazioni quando hsc è compreso tra 5 e 9
        } else if (hsc < 74) {
            // Operazioni quando hsc è compreso tra 10 e 73
        } else if (hsc < 554) {
            // Operazioni quando hsc è compreso tra 74 e 553
        } else if (hsc < 618) {
            // Operazioni quando hsc è compreso tra 554 e 617
        } else {
            // Operazioni quando hsc è maggiore o uguale a 618
        }

        tkc += 8;
        hsc += !!(tkc == 8);
        hsc = (hsc == 625) ? 0 : hsc;
        while ((end = TCNT1) - start < ((CYCLES_PER_TICKS * 8) - 8)) {}
        sei();
        seprintf("Took %" PRIu16 " cycles (%" PRIu8 ").\r\n", end - start - 8, hsc);
    }

    PORT_LED |= (1 << PIN_LED);

    return 0;
}
