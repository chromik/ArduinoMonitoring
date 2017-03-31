#ifndef TYPES_H_
#define TYPES_H_

#include <Ethernet.h>
#include "Config.h"

struct str_display {
  int kontrast;
  int jas;
};

struct str_measure {
  unsigned long int interval;
};

struct str_tepoloty {
  float curr[SENSOR_COUNT];
  float phasC; // Faze C
  char s_phasC[15]; 
  float phasB; // Faze B
  char s_phasB[15];
  float phasA; // Faze A
  char s_phasA[15];
  int aktiv_sensors; // pocet aktivnich senzoru
  char saktiv_senzor[3];
  int curr_warnings; // pocet senzoru, na kterych je proud prekrocen
  char scurr_warnings[3];
};

struct str_obrazovka {
  struct str_obrazovka* left; // na kterou obrazovku prejit pri stisku LEFT
  struct str_obrazovka* right; // na kterou obrazovku prejit pri stisku RIGHT
  struct str_obrazovka* select; // na kterou obrazovku prejit pri stisku SELECT
  struct str_obrazovka* menu; // na kterou obrazovku prejit pri stisku MENU
  int typ; // typ obrazovky
  const char *nazev;
  char **text;
  int prop[6];
  int cislo;
  int offset;
  int property1;
  int property2;
};

struct str_email {
  IPAddress smtp_server; // adresa smtp serveru
  unsigned int smtp_port; // smtp port
  bool auth; // pouzit autorizaci TRUE/FALSE
  char auth_login[30]; // login uzivatele v BASE64
  char auth_pass[30]; // heslo uzivatele v BASE64
  char from[40]; // adresa odesilatele
  char to[40]; // adresa prijemce
};

struct str_ntp{
  char ntp_server[50]; // adresa NTP serveru
  int ntp_port; // port NTP serveru
  unsigned int localPort; // lokalni port pro prijem NTP paketu
  unsigned long sync_interval;
};

struct str_eth {
  unsigned char mac[6]; // MAC adresa Arduina
  /* STATICKA ADRESA NASTAVENI */
  /* KONEC STATICKA ADRESA NASTAVENI */
  String public_ip; // verejna ip adresa Arduina (pouziva se pri odesilani mailu pres SMTP)
  unsigned int port; // port, na terem bude bezet NTP server
  bool static_ip; // pouzit staticke adresovani: ANO - true, NE - false
  IPAddress ip;
  IPAddress subnet;
  IPAddress gateway;
  IPAddress dns_server;
  EthernetServer *server; // socket pro server
  EthernetClient *client; // socket pro klienta
};




struct str_time {
  /* promenne updatnute pri funkci ziskejCas */
  bool synced; // udava, zda je cas synchronizovan s NTP serverem
  unsigned long last_sync; // cas posledni uspesne aktualizace casu z NTP
  unsigned long boot_time;
  unsigned long global_warn_start; // zacatek global vypadku
  unsigned long global_warn_end; // konec globa vypadku
  unsigned long last_crit_start[SENSOR_COUNT];
  unsigned long last_crit_end[SENSOR_COUNT];
  /* promenne updatnute pri funci prevedCas */
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  char formated_now[50];
};

struct str_alert {
  float critic_min;
  float critic_max;
  float curr_bound;
  bool is_sensor_alert[SENSOR_COUNT];
  bool is_global_alert;
  bool sended_alert[SENSOR_COUNT];
};

struct str_sensor {
  String formula;
  unsigned char pin;
  float posun_A;
  float posun_B;
};

#endif


