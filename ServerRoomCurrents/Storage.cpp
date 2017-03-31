#include "Storage.h"
#include "Types.h"
#include "Base64.h"
#include "SDCard.h"
#include "GlobalFunctions.h"
#include "Time.h"

#include <EEPROM.h>
#if LCD_LIBRARY
  #include <Adafruit_GFX.h>
  #include <Adafruit_PCD8544.h>
#else 
  #include "TangerLCD.h"
#endif

#if LCD_LIBRARY
  extern Adafruit_PCD8544 display;
#else
  extern TangerLCD display;
#endif

extern struct str_display display_settings;
extern struct str_alert alert;
extern struct str_email email;
extern struct str_measure measure;
extern struct str_eth ethernet;
extern struct str_time times;
extern struct str_sensor sensor[SENSOR_COUNT];

/* Ulozi long (4B) promennou do EEPROM
 *  @param mem EEPROM adresa pro ulozeni
 *  @param v Ukladana hodnota
 */
void store_long(int mem, unsigned long v) {
  byte store[4] = {0};
  for (int i = 0; i < 4; ++i) {
    store[i] |= (v & 0xFF);
    EEPROM.update(mem + i, store[i]);
    v = v >> 8;
  }
}

/** Nascte long (4B) prommenou z EEPROM
 *  @param mem EEPROM adresa, ze které načítám proměnnou
 */
unsigned long load_long(int mem) {
  long unsigned v = 0;
  for (int i = 3; i >= 0; --i) {
    byte c = EEPROM.read(mem + i);
    v |= c;
    if (i) {v <<= 8;}
  }
  return v;
}

/* Ulozi int (2B) promennou do EEPROM
 *  @param mem EEPROM adresa pro ulozeni
 *  @param v Ukladana hodnota
 */
void store_int(int mem, long unsigned v) {
  unsigned long high = v >> 8;
  unsigned long low = v % 256;
  EEPROM.update(mem, high);
  EEPROM.update(mem + 1, low);
}

/** Nascte int (2B) prommenou z EEPROM
 *  @param mem EEPROM adresa, ze které načítám proměnnou
 */
long unsigned load_int(int mem) {
  unsigned long high = EEPROM.read(mem);
  unsigned long low = EEPROM.read(mem + 1);
  return (high << 8) | low;
}


// nacte a vrati float hodnotu v rozsahu <-273, 382.25>, presnost na 2 desetinna mista ulozenou v pameti EEPROM na adrese: [mem] až [mem+1]
float load_float_value(int mem) {
  long mem0 = EEPROM.read(mem);
  long mem1 = EEPROM.read(mem + 1);

  long val = mem0 << 8 | mem1;
  float ret = (val - 27300 ) * 0.01 ;
  return ret;
}


// ulozi float hodnotu [val] v rozsahu <-273, 382.25>, presnost na 2 desetinna mista do pameti EEPROM na adresu: [mem] až [mem+1]
bool save_float_value(int mem, float val) {
  long v = (long) (val + 273) * 100;
  if ( v < 65536 && v >= 0) {
    long mem0 = v >> 8;
    long mem1 = v % 256;

    EEPROM.update(mem, mem0);
    EEPROM.update(mem + 1, mem1);
  } else {
    return false;
  }
}

/* Ulozi posledni vypadky */
void save_last_crit(int sens) {
  if (sens != -1) {
    store_long(EP_LAST_CRIT + 8 * sens, times.last_crit_start[sens]);
    store_long(EP_LAST_CRIT + 8 * sens + 4, times.last_crit_end[sens]);
  } else {
    store_long(EP_LAST_CRIT + 8 * SENSOR_COUNT, times.global_warn_start);
    store_long(EP_LAST_CRIT + 8 * SENSOR_COUNT + 4, times.global_warn_end);
  }
}

/* Nacte posledni vypadky */
void load_last_crits() {
  // nacte posledni vypadky vsech senzoru
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    times.last_crit_start[i] = load_long(EP_LAST_CRIT + 8 * i);
    times.last_crit_end[i] = load_long(EP_LAST_CRIT + 8 * i + 4);
  }
  // nacte posledni globalni vypadek
  times.global_warn_start = getTime(); // load_long(EP_LAST_CRIT + 8 * SENSOR_COUNT);
  times.global_warn_end = load_long(EP_LAST_CRIT + 8 * SENSOR_COUNT + 4);
}

/* Ulozi nastavenou ip adressu */
void save_ip(int address, IPAddress ip) {
  store_long(address, ip);
}

/* nacte ulozenou ip adresu */
byte load_ip(int address, IPAddress *ip) {
  *ip = load_long(address);
}

/* Ulozi ethernet konfiguraci */
void save_ethernet_settings() {
  save_ip(EP_ETHERNET, ethernet.ip); // ip adresa
  save_ip(EP_ETHERNET + 4, ethernet.subnet); // maska
  save_ip(EP_ETHERNET + 8, ethernet.gateway); // brana
  save_ip(EP_ETHERNET + 12, ethernet.dns_server); // dns
  EEPROM.update(EP_ETHERNET + 16, ethernet.static_ip); // staticka/dynamicka ip
}

/* Nacte ethernet konfiguraci */
void load_ethernet_settings() {
  load_ip(EP_ETHERNET, &ethernet.ip); // ip adresa
  load_ip(EP_ETHERNET + 4, &ethernet.subnet); // maska
  load_ip(EP_ETHERNET + 8, &ethernet.gateway); // brana
  load_ip(EP_ETHERNET + 12, &ethernet.dns_server); // dns
  ethernet.static_ip = EEPROM.read(EP_ETHERNET + 16); // staticka/dynamicka ip
}

void load_ntp_server() {
  String ntpser = sdcard_getfile("ntpcf");
  ntpser.trim();
  ntpser.toCharArray(ntp.ntp_server, 50);
  
  ntp.ntp_port = load_int(EP_NTP_PORT);
}

void save_ntp_server() {
  sdcard_overwrite("ntpcf", ntp.ntp_server);
  store_int(EP_NTP_PORT, ntp.ntp_port);
}

/* Ulozi interval mezi jednotlivymi merenimi */
bool save_measure_interval(long unsigned interval) {
  if (interval >= 0) {
    measure.interval = interval;
    store_long(EP_MEASURE_INTERVAL, interval);
    return true;
  } else {
    return false;
  }
}

/* Nacte interval mezi jednotlivymi merenimi */
void load_measure_interval() {
  measure.interval = load_long(EP_MEASURE_INTERVAL);
}

/* Ulozi nastaveni prirazeni PINu k senzoru */
void save_pinout(int id) {
  if (id == -1) { // Natavuji se vsechny piny
    for (int i = 0; i < SENSOR_COUNT; ++i) {
      EEPROM.update(EP_PINOUT + i, sensor[i].pin);
    }
  } else { // nastavuji jeden konkretni pin
    EEPROM.update(EP_PINOUT + id, sensor[id].pin);
  }
}

/* Nacte konfiguraci prirazeni PINu k senzorum */
void load_pinout() {
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    sensor[i].pin = EEPROM.read(EP_PINOUT + i);
  }
}

/* Ulozi interval synchronizace s NTP serverem */
bool save_time_interval(long unsigned interval) {
  if (interval >= 0) {
    ntp.sync_interval = interval;
    store_long(EP_NTP_SYNC_INTERVAL, interval);
    return true;    
  } else {
    return false;
  }
}

/* Nacte interval synchronizace s NTP serverem */
void load_time_interval() {
  ntp.sync_interval = load_long(EP_NTP_SYNC_INTERVAL);
}

/* Ulozi adresu prijemne a odesilatele pro odesilani mailu */
void save_email_address(String from, String to) {
  // join strings
  String text = from + ";" + to + ";";
  sdcard_overwrite("confmail", text);
  
  from.toCharArray(email.from, 40);
  to.toCharArray(email.to, 40);
}


void save_vzor(int sens, float posA, float posB) {
  long mem = EP_VZOR + (sens * 2 * sizeof(float));
  EEPROM.put(mem, posA);
  EEPROM.put(mem + sizeof(float), posB);
}

void load_vzor() {
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    long mem = EP_VZOR + (i * 2 * sizeof(float));
    EEPROM.get(mem, sensor[i].posun_A);
    EEPROM.get(mem + sizeof(float), sensor[i].posun_B);
  }
}


/* Nacte adresu prijemce a odesilatele pro odesilani mailu */
void load_email_address() {
  // separate strings
  String file_text = sdcard_getfile("confmail"); 
  int ft_length = file_text.length();
  int divider = file_text.indexOf(';');
  
  String from, to;
  from = file_text.substring(0, divider);
  int divider2 = file_text.lastIndexOf(';');
  to = file_text.substring(divider + 1, divider2);
  
  from.toCharArray(email.from, 40);
  to.toCharArray(email.to, 40);
}

/* Ulozi login a heslo pro pripojeni k SMTP serveru */
void save_email_auth(bool auth, char * user, char * pass) {
  char b64_user[30];
  char b64_pass[30];

  base64_encode(b64_user, user, strlen(user)); // prevede username na BASE64
  base64_encode(b64_pass, pass, strlen(pass)); // prevede heslo na BASE64

  String text = String(b64_user) + ";" + String(b64_pass) + ";";
  
  // ULOZ KONFIGURACI
  EEPROM.update(EP_EMAIL_AUTH, auth);
  sdcard_overwrite("confauth", text);
  
  // a zaroveni ji nastav
  email.auth = auth;
  strcpy(email.auth_login, b64_user);
  strcpy(email.auth_pass, b64_pass);
}

/* Nahraje popisky k jednotlivym senzorum + vzorecky pro vypocet proudu */
void load_sensors() {
  load_vzor();
//  for (int i = 0; i < SENSOR_COUNT; ++i) {
//    char sens[3];
//    itoa(i, sens, 10);
//    
//    String name_infix = "vzor" + String(sens) + ".txt"; // jmeno souboru s infix vzorcem
//    String vzor = sdcard_getfile(name_infix);
//    
//    char infix[100]; // vzorecek infix jako chararray
//    char postfix[100];
//    vzor.toCharArray(infix, 100); 
//    
//    infix_to_postfix(infix, postfix);
//    sensor[i].formula = String(postfix);

//  }
}


/* Nacte login a heslo pro pripojeni k SMTP serveru */
void load_email_auth() {
  String file_text = sdcard_getfile("confauth"); 
  int ft_length = file_text.length();
  int divider = file_text.indexOf(';');
  int divider2 = file_text.lastIndexOf(';');

  String user, pass;
  user = file_text.substring(0, divider);
  pass = file_text.substring(divider + 1, divider2);

  email.auth = EEPROM.read(EP_EMAIL_AUTH);
  if (email.auth && user != "" && pass != "") {
    user.toCharArray(email.auth_login, 30);
    pass.toCharArray(email.auth_pass, 30);
  }
}

/* Ulozi adresu a port SMTP serveru */
void save_email_smtp(byte *ip, unsigned int port) {
  int mem = EP_EMAIL_SMTP;
  for (int i = 0; i < 4; ++i) {
    EEPROM.update(EP_EMAIL_SMTP + i, ip[i]);
    email.smtp_server[i] = ip[i]; // update
  }
  store_int(EP_EMAIL_SMTP + 4, port);
  email.smtp_port = port; // update
}

/* Nacte adresu a port SMTP serveru */
void load_email_smtp() {
  unsigned char ip[4];
  for (int i = 0; i < 4; ++i) {
    ip[i] = EEPROM.read(EP_EMAIL_SMTP + i);
  }
  email.smtp_server[0] = ip[0];
  email.smtp_server[1] = ip[1];
  email.smtp_server[2] = ip[2];
  email.smtp_server[3] = ip[3];

  email.smtp_port = load_int(EP_EMAIL_SMTP + 4);
}


/* Ulozi nastaveni displeje */
bool save_display_conf() {
  EEPROM.update(EP_DISP_CONTRAST, display_settings.kontrast); // kontrast
  EEPROM.update(EP_DISP_JAS, display_settings.jas); // jas
}

/* Nacte nastaveni displeje */
void load_display_conf() {
  display_settings.jas = EEPROM.read(EP_DISP_JAS);
  byte kontrast = EEPROM.read(EP_DISP_CONTRAST);
  if (kontrast >= CONTRAST_MIN && kontrast <= CONTRAST_MAX) {
     display_settings.kontrast = kontrast;
  } else {
    display_settings.kontrast = CONTRAST_MIN;
  }
  display.setContrast(display_settings.kontrast);
  analogWrite(LCD_BACKLIGHT, display_settings.jas);
}



/* Ulozi nastaveny globalni limit */
bool save_currlimit_global(float minim, float maxim, float navrat_rozdil) {
  if (minim < maxim && minim >= SENSOR_MIN_CURR && maxim <= SENSOR_MAX_CURR) {

      // nastavim v programu min, max a hranici
      alert.critic_max = maxim;
      alert.critic_min = minim;

      // ulozim min, max a hranici taktez do EEPROM pameti
      save_float_value(EP_CRIT_MIN, alert.critic_min);
      save_float_value(EP_CRIT_MAX, alert.critic_max);
      save_float_value(EP_NAVRAT, navrat_rozdil);


      // ulozim nastaveni pro senzory do EEPROM

      // Vsechno co jsem dosud ulozil znovu nactu
      alert.critic_min = minim;
      alert.critic_max = maxim;
      alert.curr_bound = navrat_rozdil;
      
      return true;
  } else {
    return false;
  }
}

void load_currlimit(void) {
  // nactu si globalni min, max a hranici
  alert.curr_bound = load_float_value(EP_NAVRAT);
  alert.critic_min = load_float_value(EP_CRIT_MIN);
  alert.critic_max = load_float_value(EP_CRIT_MAX);
}

/* Obnovi tovarni nastaveni */
void factory_reset() {
  Serial.println("FACTORY RESET");
  
  // nastavi defaultni globalni min a max + hranici pro navrat do normalu
  save_currlimit_global(0, 100, 2); // GLOBAL LIMIT
  
  // nastavi defaultni kontrast a jas LCD
  EEPROM.update(EP_DISP_CONTRAST, FACTORY_CONTRAST); // KONTRAST LCD
  EEPROM.update(EP_DISP_JAS, FACTORY_BRIGHTNESS); // JAS LCD

  // vynuluje nastaveny SMTP server
  for (int i = EP_EMAIL_AUTH; i < EP_EMAIL_SMTP + 6; ++i) { // SET MAIL SMTP
    EEPROM.update(i, 0);
  }
  // zakaze statickou IP a zapne dynamickou
  for (int i = EP_ETHERNET; i < EP_ETHERNET + 17; ++i) {
    EEPROM.update(i, 0); // DISABLE STATIC IP
  }
  // nastavi interval synchronizace s NTP na jednu hodinu
  store_long(EP_NTP_SYNC_INTERVAL, 3600);

  // nastavi interval mezi merenimi na 100ms
  store_long(EP_MEASURE_INTERVAL, 100);

  // nastavi sensor0 na A0
  EEPROM.update(EP_PINOUT, 0);
  // nastavi ostatni senzory na VYP
  for (int i = EP_PINOUT + 1; i < EP_PINOUT + 14; ++i) {
    EEPROM.update(i, 255);
  }

  // vymaze log s vypadky
  sdcard_removefile("vypadky.log");
  // odstrani ulozeny login a heslo k SMTP serveru
  sdcard_removefile("confauth");
  // odstrani nastaveneho odesilatele a prijemce mailu
  sdcard_removefile("confmail");

  for (int i = 0; i < SENSOR_COUNT; ++i) {
    // nastavi defaultni popisky cidel
    char name[50];
    char numb[5];
    itoa(i, numb, 10);
    strcpy(name, "lab");
    strcat(name, numb);
    strcat(name, ".txt");
    String text = "TANGER";
    text.concat(numb);
    sdcard_overwrite(name, text);
    
//    // nastavi defaultni vzorce pro vypocet
//    strcpy(name, "vzor");
//    strcat(name, numb);
//    strcat(name, ".txt");
//    text = "V * 10";
//    sdcard_overwrite(name, text);
      
      save_vzor(i, 0, 1);
  }

  // default ntp server
  sdcard_overwrite("ntpcf", "ntp.nic.cz");
  store_int(EP_NTP_PORT, 123);
  
  // vyresetuje cas
  time_sync_intern(FACTORY_TIME); // 1.1.2016 0:0:0

    // zresetuje posledni vypadky
  for (int i = 0; i <= SENSOR_COUNT; ++i) {
    store_long(EP_LAST_CRIT + 8 * i, FACTORY_TIME);
    store_long(EP_LAST_CRIT + 8 * i + 4, FACTORY_TIME);
  }
  
  // restartuje Arduino
  software_Reset();
}
