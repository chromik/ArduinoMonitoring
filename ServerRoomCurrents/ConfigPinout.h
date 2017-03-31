#ifndef CONFIG_PINOUT_H_
#define CONFIG_PINOUT_H_

// LCD pins
#define LCD_SCE 41
#define LCD_RESET 43
#define LCD_DC 45
#define LCD_SDIN 47
#define LCD_SCLK 49
#define LCD_BACKLIGHT 3 // Analog pin ridici podsviceni LCD

/* KEYBOARD PINY */
#define BT_AUTO 35 // Tlacitko AUTO
#define BT_MENU 33 // Tlacitko MENU
#define BT_LEFT 31 // Tlacitko LEFT
#define BT_RIGHT 29 // Tlacitko RIGHT
#define BT_SELECT 27 // Tlacitko SELECT
#define BT_ONOFF 25 // Tlacitko ON/OFF
#define BT_LED 23 // LED na klavesnici (sviti pri LOW)
#define BT_COMMON 37 // COMMON KEY

#endif
