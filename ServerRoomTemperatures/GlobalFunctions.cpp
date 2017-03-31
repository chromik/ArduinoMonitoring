#include "GlobalFunctions.h"
#include "Types.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <Arduino.h>
#include "Time.h"
#include "SDCard.h"
#include <SdFat.h>
#include <time.h>
#include "MailSend.h"
#include "Storage.h"

extern SdFat SD;
extern struct str_eth ethernet;
extern str_alert alert;
extern str_time times;
extern struct str_email email;

extern char try_test_mail;

char buffer[50]; // buffer pro ukladani hodnot pro funkce v tomto souboru

/** Prevede as na epoch
 *  @param YYYY rok
 *  @param MM mesic
 *  @param DD den
 *  @param hh hodina
 *  @param ss sekunda
 *  @return epoch
 */
unsigned long tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss)
{
  struct tm tmSet;
  tmSet.tm_year = YYYY - 1900; // jako rok se nastavuje kolik let ubehlo od roku 1900
  tmSet.tm_mon = MM - 1; // leden = 0; prosinec = 11
  tmSet.tm_mday = DD;
  tmSet.tm_hour = hh - 1; // pasmo v CR je +1 GTM
  tmSet.tm_min = mm;
  tmSet.tm_sec = ss;
  unsigned long epoch = mk_gmtime(&tmSet) + THIRTY_YERS_IN_SECONDS; // v AVR Standard C Time knihovne je reprezentovan epoch jako pocet sekund od roku 2000, pro prevod na klasicky epoch je treba pricist 30 let v sekundach k tomu
  return epoch; 
}

/**
 * Opravi znaky u stringu precteneho z GET requestu
 * @param s vstupni string
 */
void coding_fix(String *s) {
  s->replace('+',' ');
  s->replace("%20", " ");
  s->replace("%21", "!");
  s->replace("%22", "\"");
  s->replace("%23", "#");
  s->replace("%24", "$");
  s->replace("%25", "%");
  s->replace("%26", "&");
  s->replace("%27", "'");
  s->replace("%28", "(");
  s->replace("%29", ")");
  s->replace("%2A", "*");
  s->replace("%2B", "+");
  s->replace("%2C", ",");
  s->replace("%2D", "-");
  s->replace("%2E", ".");
  s->replace("%2F", "/");
  s->replace("%3A", ":");
  s->replace("%3B", ";");
  s->replace("%3C", "<");
  s->replace("%3D", "=");
  s->replace("%3E", ">");
  s->replace("%3F", "?");
  s->replace("%40", "@");
}

/* Softwarovy reset programu */
void software_Reset() {// Restarts program from beginning but does not reset the peripherals and registers
  asm volatile("  jmp 0");  
}


// zjisti velikost volne pameti SRAM
int freeRam() 
{
  extern int __heap_start, *__brkval; 
  int v; // vytvorim promennou
  return (int) &v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval); 
}

// vrati celkovou velikost pameti SRAM
int ramSize() {
  int v; // vytvorim promennou
  int a = (int)&v; // ulozim do a adresu promenne
  return a; // a tu vratim
}

// vrati, jak dlouho je prekrocene alespon nektere cidlo
unsigned long get_doba_prekroceni() {
  if (!alert.is_global_alert) {
    return 0;
  } else {
    long unsigned now_time = getTime();
    return get_timediff(times.global_warn_start, now_time);  
  }
}

// vrati, jak dlouho jsou vsechny cidla v norme
unsigned long get_doba_normal() {
  if (alert.is_global_alert) {
    return 0;
  } else {
    long unsigned now_time = getTime();
    return get_timediff(now_time, times.global_warn_end);
  }
}

// vrati popisek senzoru s danym id
String getSenzorLabel(int id) {
  char cislo[5];
  char label_file[10] = "lab";
  itoa(id, cislo, 10);
  strcat(label_file, cislo);
  
  String strLabel = sdcard_getfile(label_file);
  strLabel.trim();
  return strLabel;
}

// vycisti log s vypadky
void log_vypadky_clear() {
  digitalWrite(SDCARD, LOW);
  SD.begin(4);
  SD.remove("vypadky.log");
  digitalWrite(SDCARD, HIGH);

  /* smazat globalni vypadek */
  times.global_warn_start = times.global_warn_end = FACTORY_TIME; // resetovat promenne
  store_long(EP_LAST_CRIT + 8 * SENSOR_COUNT, FACTORY_TIME); // EEPROM reset
  store_long(EP_LAST_CRIT + 8 * SENSOR_COUNT + 4, FACTORY_TIME); // EEPROM reset

  /* smazat vypadky u jednotlivych cidel */
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    times.last_crit_start[i] = times.last_crit_end[i] = getTime(); // reset promenne
    save_last_crit(i); // EEPROM reset
  }
}

// zaloguje vypadek teploty
void log_vypadky_ulozVypadek(int sensor, unsigned long from, unsigned long to) {
  #if DEBUG_ON == 1
    Serial.println("logging.");
  #endif
  digitalWrite(SDCARD, LOW); // povolit kartu
  SD.begin(4);
  char sFrom[20];
  char sTo[20];

  strcpy(sFrom, prevedCas(from));
  strcpy(sTo, prevedCas(to));
    
  File logfile = SD.open("vypadky.log", FILE_WRITE);
  logfile.print(sensor);
  logfile.print("; ");
  logfile.print(sFrom);
  logfile.print("; ");
  logfile.print(sTo);
  logfile.println(";");
  logfile.close();
  digitalWrite(SDCARD, HIGH);
  delay(20);
}

// posle klientovi log s vypadky
void log_ukaz_vypadky(EthernetClient *client) {
  
  digitalWrite(SDCARD, LOW); // povolit kartu
  SD.begin(4);
  PROGMEM const char * fn_vypadky_log = "vypadky.log";
  File logfile = SD.open(fn_vypadky_log);
  
  if (logfile) {
    while (logfile.available()) {
      client->write(logfile.read());
    }
  } else {
    PROGMEM const char * str_zadne_vypadky = "Zadne vypadky.";
    client->write(str_zadne_vypadky);
  }
  
  delay(5);
  logfile.close();
  delay(5);
  digitalWrite(SDCARD, HIGH);
}

/**
 * Nacte html kod z pametovky, tagy v nem (@01, @02, @03, @04, etc..) nahradi parametry této funkce (v pořadí jakém se nacházejí -> 1. nalezeny tag v souboru == 1. parametr fce) a vyresli jej klientovi
 * @param client Client socket
 * @param file Jmeno souboru na SD karte
 * @param arg_count Pocet hodnot (argumentu), ktere prijdou vlozit do templatu na SD karte misto tagu
 * @param ... jednotlive hodnoty, kterymi se budou nahrazovat tagy
 */
void client_send_html(EthernetClient *client, const char * file, int arg_count, ...) {   

  /* muze bezet jen karta nebo jen ethernet, nikoliv oboji najednou */
  digitalWrite(SDCARD, LOW); // povolit kartu
  digitalWrite(ETHERNET, HIGH); // zakazat ethernet
  
  // buffer pro ukladani textu z pametovky
  String buffer = "";
  // inicializace pametove karty
  SD.begin(4);
  // otevru soubor na pametove karte
  File dataFile = SD.open(file);
  
  if (dataFile) { // Ukladam text do bufferu
    
    va_list p; // argument list
    va_start(p, arg_count); // inicializace listu argumenty 

    unsigned int param_order = 0;
    while (dataFile.available()) {
      char c = dataFile.read();

      // pokud je nejaky TAG k nahrazeni hodnotou
      if (c == '@') {
        char high = dataFile.read(); // TAG HIGH value 

        if (high != '@' && param_order < arg_count) {
          ++param_order;
          char low = dataFile.read(); // TAG LOW value

          if ((high - '0') * 10 + low - '0' == param_order) { // zjistim, zda sedi cislo tagu s poradim parametru
            String argument = String(va_arg(p, char*)); // ziskat argument a prejit na dalsi
            buffer += argument; //nahradit tag hodnotou z parametru
          } else {
            PROGMEM const char * str_template_error = "TEMPLATE SYNTAX ERROR!";
            buffer = str_template_error;
            break;
          }
        } else {  // escape znak, nejedna se o tag a vypise se jen @
          buffer += high;
        }
      } else {
        buffer += c;
      }
      
    }
    dataFile.close();
    va_end(p); // konec uzivani argument listu
  }

  
  #if BAS_DEBUG
  else {
    Serial.print("SD fail, cant open file: '");
    Serial.print(file);
    Serial.println("'");
    }
  #endif
  
  digitalWrite(SDCARD, HIGH);
  digitalWrite(ETHERNET, LOW);
  client->println(buffer);   
  delay(1);
}


/* zjisti mac adresu */
char * getMacAdress() {
  char tmp[3];
  
  strcpy(buffer, "");
  for (int i = 0; i < 6; ++i) {
    itoa(ethernet.mac[i], tmp, 16);
    strcat(buffer, tmp);
    if (i < 5) strcat(buffer, ".");
  }
  
  return buffer;
}


/* Zapne nebo vypne LED na klavesnici */
void turnLED(bool on) {
  if (on) {
    digitalWrite(BT_LED, LOW);
  } else {
    digitalWrite(BT_LED, HIGH);
  }
}


/* vprevede ip adresu nas string */
char * showIpAdress(unsigned char *ip) {
  char tmp[4];

  strcpy(buffer, "");
  for (int i = 0; i < 4; ++i) {
    itoa(ip[i], tmp, 10);
    strcat(buffer, tmp);
    if (i < 3) strcat(buffer, ".");
  }
  return buffer;
}

/* ziska ip adresu zarizeni jako string */
char * getIpAdress() {
  char tmp[4];
  IPAddress ip = Ethernet.localIP();

  strcpy(buffer, "");
  for (int i = 0; i < 4; ++i) {
    itoa(ip[i], tmp, 10);
    strcat(buffer, tmp);
    if (i < 3) strcat(buffer, ".");
  }
  return buffer;
}


// Prevede string na float
float StringToFloat(String s) {
  return s.toFloat();
}

// prevede float na string
void floatToString(char *buffer, float value) {
  char negative = 0;
  if (value < 0) {
    negative = 1;
    value = -value;
  }
  
  int d1 = value;
  float f2 = value - d1;
  int d2 = trunc(f2 * 100);

  if (negative) {
    PROGMEM const char * str_format1 = "-%d.%02d";
    sprintf(buffer, str_format1, d1, d2);
  } else {
    PROGMEM const char * str_format2 = "%d.%02d";
    sprintf(buffer, str_format2, d1, d2);
  }
}

// prevede epoch na formatovany cas
char * getFormatedTime(unsigned long t, bool shorted) {
  unsigned long seconds = t % 60;
  t /= 60;
  unsigned int minutes = (int)t % 60;
  t /= 60;
  unsigned int hours = (int)t % 24;
  t /= 24;
  unsigned int days = (int)t;
  char tmp[10];

  itoa(days, tmp, 10);
  strcpy(buffer, tmp);
  if (shorted) {
    strcat(buffer, "d");
  } else {
    strcat(buffer, " dní, ");
  }

  itoa(hours, tmp, 10);
  strcat(buffer, tmp);
  if (shorted) {
    strcat(buffer, "h");
  } else {
    strcat(buffer, " hodin, ");
  }

  itoa(minutes, tmp, 10);
  strcat(buffer, tmp); 
  if (shorted) {
    strcat(buffer, "m");
  } else {
    strcat(buffer, " minut, ");
  }

  itoa(seconds, tmp, 10);
  strcat(buffer, tmp);
  if (shorted) {
    strcat(buffer, "s");
  } else {
    strcat(buffer, " sekund");
  }

  return buffer;
}

// ziska rozdil mezi dvema casy
unsigned long get_timediff(unsigned long t1, unsigned long t2) {
  if (t1 < t2) {
    return t2 - t1;
  } else {
    return t1 - t2;
  }
}

