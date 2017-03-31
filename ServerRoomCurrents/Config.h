#ifndef CONFIG_H_
#define CONFIG_H_
#include "Constants.h"
#include "ConfigPinout.h"

/* OPTIONS */
// Pokud bude namereny proud mimo tento rozsah, Jedna se o chybu mereni UNKWNOWN_VALUE a bude se to brat, jako by bylo cidlo odpojeno
#define SENSOR_MIN_CURR 0 // minimalni meritelny proud senzorem 
#define SENSOR_MAX_CURR 200 //  maximalni meritelny proud senzorem
#define UNKNOWN_VALUE 666 // hodnota signalizujici neznamou, musi byt vetsi nez absolutni hodnota SENSOR_MIN_CURR i SENSOR_MAX_CURR (slouží jako počáteční minimum i maximum při porovnávání hodnot)

#define SENSOR_COUNT 12 // POCET PRIPOJENYCH prouduNICH SENZORU
#define MEASURES_SPREE 1 // kolikrat za je treba zmerit za sebou alert/normal teplotu pro prejiti do alert/normal stavu

// - SENDMAIL_NORMAL_TRY_REPEATS * SENDMAIL_NORMAL_TRY_MILLIS_INTERVAL ~= jak dlouho se pokouset o znovuodeslani mailu 
#define SENDMAIL_NORMAL_TRY_REPEATS 3 // pokud se neporadi odeslat mail o navratu stavu do normalu, kolikrat opakovat pokus? ([-1] = donekonecna, [0] = vubec, [ostatni] = pocet pokusu)
#define SENDMAIL_NORMAL_TRY_MILLIS_INTERVAL 30000 // a jak casto se pokouset o opakovani (interval v milisekundach)

// - SENDMAIL_ALERT_TRY_REPEATS * SENDMAIL_ALERT_TRY_MS_INTERVAL ~= jak dlouho se pokouset o znovuodeslani mailu
#define SENDMAIL_ALERT_TRY_REPEATS 3 // pokud se neporadi odeslat alert mail, kolikrat opakovat pokus? ([-1] = donekonecna, [0] = vubec, [ostatni] = pocet pokusu)
#define SENDMAIL_ALERT_TRY_MS_INTERVAL 30000 // a jak casto  (interval v milisekundach) se pokouset o opakovani 

#define AJAX_UPDATE_INTERVAL 500 // Minimalni (milisekundy) interval mezi zpracovavanim ajax pozadavku 
#define AJAX_UPDATE_DELAY_AFTER_KEY_PRESSED 2000 // Doba (milisekundy) o kterou odlozit dalsi ajax pozadavek po stisku klavesy (aby ajax nezpomaloval behem obsluhy odezvu

#define DHCP_RENEW_S_INTERVAL 300 // DHCP Renew interval v sekundach 

#define LCD_SETUP_INTERVAL 60 // LCD setup interval v sekundach

#define CONTRAST_MIN 51 // Minimalni mozny nastavitelny kontrast LCD
#define CONTRAST_MAX 61 // Maximalni mozny nastavitelny kontrast LCD

// TIMEOUT (v milisekundach) PRO CEKANI NA ODPOVED Z SMTP SERVERU
#define MAIL_WAIT_FOR_RESPONSE_MS_DELAY 1600

// TIMEOUT PRO NORMALNI PRIPOJENI
#define ETH_CONN_NORMAL_TIMEOUT 0x4650 // = 1 800 ms ->  kazda jednotka je 100us
#define ETH_CONN_NORMAL_RETRY_COUNTS 3 // pocet opakovacich pokusu

// TIMEOUT PRO PRIPOJENI PRI ODESILANI MAILU
#define ETH_CONN_EMAIL_TIMEOUT 0x4650 // = 1 800 ms ->  kazda jednotka je 100us
#define ETH_CONN_EMAIL_RETRY_COUNTS 2 // pocet opakovacich pokusu

// TIMEOUT PRO PRIPOJENI K NTP SERVERU
#define ETH_CONN_NTP_TIMEOUT 0x4650 // = 1 800 ms ->  kazda jednotka je 100us
#define ETH_CONN_NTP_RETRY_COUNTS 2 // pocet opakovacich pokusu


/* Nastaveni po factory resetu */
#define FACTORY_CONTRAST (CONTRAST_MIN + CONTRAST_MAX) / 2 // defaultni kontrast po factory resetu
#define FACTORY_BRIGHTNESS 128 // defaultni jas po factory resetu
#define FACTORY_TIME 1451602800 // defaultni cas pro factory resetu (1.1.2016)

/* DEBUG ON/OFF */
#define BAS_DEBUG 0 /* Jednoduchy DEBUG */
#define DEBUG_ON 0 /* Podrobny DEBUG*/
#define DEBUG_COMPONENT 0 /* Component DEBUG*/
#define REQ_DEBUG 0 /* http request DEBUG */
#define DEBUG_LCD 0 /* LCD DEBUG */
#define DEBUG_WITHOUT_RTC 0 /* bez RTC clanku (odpojeny) */
#define DEBUG_WITHOUT_RTC_TEMP 0 /* bez mereni teploty na RTC clanku */

/* MODULES ON/OFF */
#define SL_CONTROL_ON 0 /* Zapnout klavesnice pres seriovou linku -> Ovladani: 'q' = AUTO, 'e' = ON/OFF, 's' = SELECT, 'w' = MENU, 'a' = LEFT, 'd' = RIGHT */
#define LOG_ON 1 /* zapisovat logy na SD kartu */
#define XML_ON 1 /* Zapnout export do XML - pamet navic */
#define LCD_LIBRARY 1 /*Pouzit LCD knihovnu Adafruit, 1=ANO/0=TANGER_LCD */
#define SENDMAILON 1 /* zapina odesilani mailu (pro testovani=0)*/

#endif
