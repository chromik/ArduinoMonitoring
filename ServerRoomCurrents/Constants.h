#ifndef CONSTANTS_H_
#define CONSTANTS_H_

/* PERMA PINOUT */
// Serial Monitor pins
#define SERIAL0_RX 0
#define SERIAL0_TX 1

// Ethernet Shield SPI pins
#define EHT_SHIELD_MISO 50
#define ETH_SHIELD_MOSI 51
#define ETH_SHIELD_SCK 52
#define ETH_SHIELD_SS 53

//ETHERNET SHIELD CONTROL pinin
#define SDCARD 4 // LOW enable, HIGH disable
#define ETHERNET 10 // LOW enable, HIGH disable

/* TYPY OBRAZOVEK */
#define SCR_MENU 0 // menu obrazovka
#define SCR_CURR 1 // obrazovka s aktualnimi proudmi
#define SCR_LIMS 2 // obrazovka s nastavenim limitnich proudu
#define SCR_LCDSET 3 // obrazovka s nastavenim lcd (konstrast + jas)
#define SCR_ETHSET 4 // obrazovka s informacemi o ethernetu (mac + ip adresa)
#define SCR_TIME 5
#define SCR_TIME_MANUAL 6
#define SCR_EDIT_ADDRESS 7
#define SCR_FACTORY_RESET 8

// Vupnute cidlo
#define PIN_OFF 255

// casove konstanty
#define THIRTY_YERS_IN_SECONDS 946684800
#define SEVENTY_YEAR_IN_SECONDS 2208988800UL

// prestupny rok
#define LEAP_YEAR(Y) ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

#endif

