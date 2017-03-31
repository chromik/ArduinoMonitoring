#ifndef CONFIG_PINOUT_H_
#define CONFIG_PINOUT_H_

// KLIMATIZACE ECHO pinin
#define ECHO_PIN_COND0 12 /* ECHO PIN indentifikace ZAPNUTI/VYPNUTI KLIMATIZACE A */
#define ECHO_PIN_COND1 13 /* ECHO PIN indentifikace ZAPNUTI/VYPNUTI KLIMATIZACE B */

// LCD pins
#define LCD_SCE 41
#define LCD_RESET 43
#define LCD_DC 45
#define LCD_SDIN 47
#define LCD_SCLK 49
#define LCD_BACKLIGHT 3 // Analog pin ridici podsviceni LCD

// KEYBOARD pins
#define BT_AUTO 35 // Tlacitko AUTO
#define BT_MENU 33 // Tlacitko MENU
#define BT_LEFT 31 // Tlacitko LEFT
#define BT_RIGHT 29 // Tlacitko RIGHT
#define BT_SELECT 27 // Tlacitko SELECT
#define BT_ONOFF 25 // Tlacitko ON/OFF
#define BT_LED 23 // LED na klavesnici (sviti pri LOW)
#define BT_COMMON 37 // COMMON KEY

/**
 * Arduino Mega 2560 má 6 timeru a 15PWM vystupu vazanych na ne:
 * 
 * Mozna nastaveni jsou:
 * Timer0: PIN04 - Channel B, PIN13 - Channel A
 * Timer1: PIN11 - Channel A, PIN12 - Channel B
 * Timer2: PIN09 - Channel B, PIN10 - Channel A
 * Timer3: PIN02 - Channel B, PIN03 - Channel C, PIN05 - Channel A
 * Timer4: PIN06 - Channel A, PIN07 - Channel B, PIN08 - Channel C
 * Timer5: PIN44 - Channel A, PIN45 - Channel B, PIM46 - Channel C
 */

// A = Timer 2 (8 bit), PIN9 => IR_USE_TIMER2
#define A_TIMER_PWM_PIN       9 // muze byt 9, 10
#define A_TIMER_DISABLE_INTR  (TIMSK2 = 0)
#define A_TIMER_ENABLE_PWM    (TCCR2A |= _BV(COM2B1))
#define A_TIMER_DISABLE_PWM   (TCCR2A &= ~(_BV(COM2B1)))
#define A_TIMER_CONFIG_KHZ(val) ({ \
  const uint8_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR2A               = _BV(WGM20); \
  TCCR2B               = _BV(WGM22) | _BV(CS20); \
  OCR2A                = pwmval; \
  OCR2B                = pwmval / 3; \
})

/*Nastaveni druheho B citace/casovace pro PWM modulaci*/
// B = Timer 1, PIN11  => IR_USE_TIMER1 
#define B_TIMER_PWM_PIN      11 // muze byt 11, 12
#define B_TIMER_DISABLE_INTR  (TIMSK1 = 0)
#define B_TIMER_ENABLE_PWM   (TCCR1A |= _BV(COM1A1))
#define B_TIMER_DISABLE_PWM  (TCCR1A &= ~(_BV(COM1A1)))
#define B_TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR1A                = _BV(WGM11); \
  TCCR1B                = _BV(WGM13) | _BV(CS10); \
  ICR1                  = pwmval; \
  OCR1A                 = pwmval / 3; \
})

/* OSTATNI CITACE, KTERE JE MOZNO VYUZIT K PWM, v pripade potreby je mozno prepsat makra vyse na tyto citace.
/* TIMER0
#define !!!A_NEBO_B!!!_TIMER_PWM_PIN  4 //muze byt 4, 13
#define !!!A_NEBO_B!!!_TIMER_DISABLE_INTR 
#define !!!A_NEBO_B!!!_TIMER_ENABLE_PWM
#define !!!A_NEBO_B!!!_TIMER_DISABLE_PWM
#define !!!A_NEBO_B!!!_TIMER_CONFIG_KHZ(val) ({ \

})

/* TIMER3
#define !!!A_NEBO_B!!!_TIMER_PWM_PIN  2 //muze byt 2, 3, 5
#define !!!A_NEBO_B!!!_TIMER_DISABLE_INTR    (TIMSK3 = 0)
#define !!!A_NEBO_B!!!_TIMER_ENABLE_PWM     (TCCR3A |= _BV(COM3A1))
#define !!!A_NEBO_B!!!_TIMER_DISABLE_PWM    (TCCR3A &= ~(_BV(COM3A1)))
#define !!!A_NEBO_B!!!_TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR3A = _BV(WGM31); \
  TCCR3B = _BV(WGM33) | _BV(CS30); \
  ICR3 = pwmval; \
  OCR3A = pwmval / 3; \
})

/* TIMER4
#define !!!A_NEBO_B!!!_TIMER_PWM_PIN  6 //muze byt 6, 7, 8
#define !!!A_NEBO_B!!!_TIMER_DISABLE_INTR  (TIMSK4 = 0) 
#define !!!A_NEBO_B!!!_TIMER_ENABLE_PWM    (TCCR4A |= _BV(COM4A1))
#define !!!A_NEBO_B!!!_TIMER_DISABLE_PWM   (TCCR4A &= ~(_BV(COM4A1)))
#define !!!A_NEBO_B!!!_TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR4A = _BV(WGM41); \
  TCCR4B = _BV(WGM43) | _BV(CS40); \
  ICR4 = pwmval; \
  OCR4A = pwmval / 3; \
})

/* TIMER5
#define !!!A_NEBO_B!!!_TIMER_PWM_PIN  44 //muze byt 44, 45, 46
#define !!!A_NEBO_B!!!_TIMER_DISABLE_INTR  (TIMSK5 = 0)
#define !!!A_NEBO_B!!!_TIMER_ENABLE_PWM    (TCCR5A |= _BV(COM5A1)) 
#define !!!A_NEBO_B!!!_TIMER_DISABLE_PWM   (TCCR5A &= ~(_BV(COM5A1)))
#define !!!A_NEBO_B!!!_TIMER_CONFIG_KHZ(val) ({ \
  const uint16_t pwmval = SYSCLOCK / 2000 / (val); \
  TCCR5A = _BV(WGM51); \
  TCCR5B = _BV(WGM53) | _BV(CS50); \
  ICR5 = pwmval; \
  OCR5A = pwmval / 3; \
})
*/

#endif
