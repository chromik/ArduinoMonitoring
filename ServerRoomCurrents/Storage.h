#ifndef STORAGE_H_
#define STORAGE_H_

#include <Arduino.h>

/* EEPROM MAPA OBSAZENEHO MISTA, RANGE: 000-4096*/
/* adresa 000-001 */#define EP_CRIT_MIN 0           /* 2B */
/* adresa 002-003 */#define EP_CRIT_MAX 2           /* 2B */
/* adresa 004-007 */#define EP_NTP_SYNC_INTERVAL 4  /* 4B */
/* adresa 008-008 */#define EP_DISP_CONTRAST 8      /* 1B */
/* adresa 009-009 */#define EP_DISP_JAS 9           /* 1B */
/* adresa 010-013 */#define EP_NAVRAT 10            /* 2B */  
/* adresa 014-014 */#define EP_EMAIL_AUTH 14        /* 1B */
/* adresa 015-020 */#define EP_EMAIL_SMTP 15        /* 6B */ /*15-18 IP, 19-20 PORT */ 
/* adresa 021-024 */#define EP_MEASURE_INTERVAL 21  /* 4B */
/* adresa 025-041 */#define EP_ETHERNET 25          /* 17B */
/* adresa 300-XXX */#define EP_CUSTOM_CRIT 300      /* ?B */
/* adresa 514-633 */#define EP_LAST_CRIT 1022        /* 120B */
/* adresa 634-635 */#define EP_NTP_PORT 734       /* 2B */ /*634-635 PORT */ 
/* adresa 700-713 */#define EP_PINOUT 800          /* 14B */
/* adresa 714-??? */#define EP_VZOR 824

extern struct str_alert alert;
extern struct str_ntp ntp;

void load_time_interval();
void load_currlimit(void);
void load_currlimit_invidual();
bool save_currlimit_global(float minim, float maxim, float navrat_rozdil);

void save_currlimit_invidual();
bool save_time_interval(unsigned long interval);

bool save_display_conf(void);
void load_display_conf(void);

void save_email_auth(bool auth, char * user, char * pass);
void load_email_auth();

void save_email_smtp(unsigned char *ip, unsigned int port);
void load_email_smtp();

void save_email_address(String from, String to);
void load_email_address();

void save_pinout(int id = -1);
void load_pinout();

bool save_measure_interval(long unsigned interval);
void load_measure_interval();

void save_ethernet_settings();
void load_ethernet_settings();

void save_ntp_server();
void load_ntp_server();

void save_last_crit(int sens = -1);
void load_last_crits();

void load_sensors();
void factory_reset();
void store_long(int mem, unsigned long v);

void save_vzor(int sens, float posA, float posB);
void load_vzor();

#endif


