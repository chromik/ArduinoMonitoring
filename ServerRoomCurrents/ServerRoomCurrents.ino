#include <math.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Bridge.h>
#include <utility/w5100.h>

#include "Config.h"
#include "GlobalFunctions.h"
#include "MailSend.h"
#include "Display.h"
#include "DisplayControl.h"
#include "MailSend.h"
#include "Storage.h"
#include "Types.h"
#include "Time.h"
#include "Base64.h"
#include "WebServer.h"
#include "SDCard.h"
//#include "InfixPostfixSolve.h"



/* millis global compare vars */
unsigned long last_ms_button_click = 0; // millis() cas posledniho stisku na klavesnici
unsigned long last_ms_ajax_run = 0; // millis() cas od posledniho zaslani AJAX pozadavku
unsigned long last_ms_normal_try = 0; // millis() cas od posledniho pokusu o zaslani mailu o navratu stavu do normalu
unsigned long last_ms_alert_try = 0; // millis() cas od posledniho pokusu o zaslani alert mailu
unsigned long last_ms_measure = 0; // millis() od posledniho mereni hodnot na cidlech

/* time global compare vars */
unsigned long last_time_dhcp_renew = 0; // pocet sekund od posledniho DHCP Renew
unsigned long last_time_ntp_sync = 0; // pocet sekund od posledni NTP synchronizace
unsigned long last_time_lcd_setup = 0; // pocet sekund od posledni inicializace LCD

char try_test_mail = 0; // priznak pro pokus o zaslani testovaciho emailu

#if LCD_LIBRARY
  #include <Adafruit_GFX.h>
  #include <Adafruit_PCD8544.h>
#else 
  #include "TangerLCD.h"
#endif

#if LCD_LIBRARY
  Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_SCLK, LCD_SDIN, LCD_DC, LCD_SCE, LCD_RESET); // PINY se konfiguruji v ConfigPinout.h
#else
  TangerLCD display = TangerLCD();
#endif

/* ETHERNET SETTINGS */
struct str_eth ethernet = {
  {0xB8, 0x27, 0xEB, 0x72, 0x1C, 0xA8}, // MAC adresa arduina
  "x", // Public IP - predava se SMTP serveru pri posilani mailu (v nasem pripade obsah nepodstatny, server nedela verifikaci)
  80 // Port, na kterem bezi webserver
};

/* EMAIL SEND SETTINGS */
struct str_email email;

struct str_sensor sensor[SENSOR_COUNT];

struct str_ntp ntp = {
  "", // NTP server - !!prepise se hodnotou z SD karty!!
  123, // NTP port - !!prepise se hodnotou z EEPROM!!
  8888, // local port pro prijimani UDP paketu
  5, // interval synchronizace casu s NTP v minutach - !!prepise se hodnotou z EEPROM!!
};

struct str_time times = {
  false, 0, 0, 0, 0, {0}, {0}, 0, 0, 0, 0, 0, 0, "N/A"
};

/*==========================================================================================================*/
/* Upozorneni na proudy */


struct str_alert alert = {
  15, // minimalni kriticka proud - !!prepise se hodnotou z EEPROM!!
  28, // maximalni kriticka proud - !!prepise se hodnotou z EEPROM!!
  0, // hysterze, hranice o kterou jsou limity posunute pri navratu do normal stavu  - !!prepise se hodnotou z EEPROM!!
  {false}, // identifiku u sensoru, zda je v alert stavu
  false, // Identifikuje, zda je nejaky ze senzoru mimo rozsah povolene proudy
  {true} // identifikuje, zda byl pro dany senzor jiz zaslan mail
};

struct str_measure measure;

/* Zde se uchovavaji namerene hodnoty */
struct str_tepoloty mt = {
  {0}, 0, "", 0, "", 0, "", 0, "", 0, ""
};



// ethernet initial setup
void eth_setup(void) {
  if (ethernet.static_ip) { // pouzivam statickou IP
    Serial.print("s:");
    Ethernet.begin(ethernet.mac, ethernet.ip, ethernet.dns_server, ethernet.gateway, ethernet.subnet);
  } else { // Pouzivam nastaveni DHCP serveru
    Serial.print("d:");
    
    display.println(">Cekam na");
    display.println(" DHCP");
    display.display();
    Ethernet.begin(ethernet.mac);
  }

  Serial.println(getIpAdress()); // ukaz IP adresu Arduina pres Seriovou linku

  // SPUSTENI ETHERNET SERVERU
  ethernet.server = new EthernetServer(ethernet.port);
  ethernet.server->begin();
  ethernet.client = new EthernetClient;

  
  // connection timeout
  W5100.setRetransmissionTime(ETH_CONN_NORMAL_TIMEOUT);
  W5100.setRetransmissionCount(ETH_CONN_NORMAL_RETRY_COUNTS);
}


/** Pošle varovný email
 *  @param s cislo sensoru jako int
 *  @param sensor cislo sensoru jako string
 *  @param t aktualni teplota senzoru
 *  @param subject Předmět e-mailu
 *  @param stat Typ zprávy (alert upozornění/návrat do normálu)
 */
char send_alert(int s, char *sensor, float t, String subject, const char *stat) {
  char ret = 1;
  // connection timeout pro mail
  W5100.setRetransmissionTime(ETH_CONN_EMAIL_TIMEOUT);
  W5100.setRetransmissionCount(ETH_CONN_EMAIL_RETRY_COUNTS);
  
  
  char curr[10];
  if (mt.curr[s] != UNKNOWN_VALUE) {
    floatToString(curr, mt.curr[s]);
  } else {
    strcpy(curr, "OFF");
  }

  
  String text = String(curr) + "<br/>" + String(sensor) + "<br/>Time: " + String( prevedCas(getTime()) ) + "<br/>" + getSenzorLabel(s);
  
  turnLED(true);
  display.setCursor(0, 0);
  display.print("SENSOR");
  display.println(sensor);
  display.print(stat);
  display.println("\n");
  display.println("Sending mail");
  display.display();
  if (sendmail_via_smtp(email.to, subject, text)) {
    alert.sended_alert[s] = true;
    display.println("OK");
  } else {
    alert.sended_alert[s] = false;
    display.println("fail");
    ret = 0;
  }
  display.display();
  delay(800);
  display.clearDisplay();
  delay(20);
  turnLED(false);

  /* vratim timeout zpet */
  W5100.setRetransmissionTime(ETH_CONN_NORMAL_TIMEOUT);
  W5100.setRetransmissionCount(ETH_CONN_NORMAL_RETRY_COUNTS);
  return ret;
}

/**
 * Odeslat varovny mail o prekroceni teploty
 * @param s Cislo sensoru
 * @param t Aktualni teplota na sensoru
 */
char send_alert_state(int s, float t) {
  char sensor[5];
  itoa(s, sensor, 10);
  
  char s_curr[15];
  floatToString(s_curr, t);

  String subject = getSenzorLabel(s) + " (s" + String(sensor) + ") proud varovani! I = " + s_curr + " A";

  return send_alert(s, sensor, t, subject, "alert");
}

/**
 * Odeslat informacni mail o navratu teploty do normalu
 * @param s Cislo sensoru
 * @param t Aktualni teplota na sensoru
 */
char send_normal_state(int s, float t) {
  char sensor[5];
  itoa(s, sensor, 10);
  
  char s_curr[15];
  floatToString(s_curr, t);
  
  String subject = getSenzorLabel(s) + " (s" + String(sensor) + ") proud v norme. I = " + s_curr + " A";

  return send_alert(s, sensor, t, subject, "normal");
}

/**
 * Zjisti, zda je teplota daneho senzoru v poradku nebo ne
 * @param sens cislo sensoru
 * @param temp teplota tohoto sensoru
 */
bool isInternalBad(int sens, float curr) {
  if (curr < 0) curr = 0; // ignorovat zaporne proudy
  
  if (!alert.is_sensor_alert[sens]) { // pokud neni alert
      if (curr < (float)alert.critic_min || curr > (float)alert.critic_max) { // zjisti, zda je proud v povolenem limitu
        return true;
      }
      return false;
  } else { // pokud je alert
      if (curr > (float)(alert.critic_min + alert.curr_bound) && curr < (float)(alert.critic_max - alert.curr_bound)) { // zjisti, zda je teplota v povolenem limitu s hysterzi
        return false;
      }
      return true;
  }
}

/**
 * Zaloguje konec vypadku pro dany sensor
 * @param sens Cislo sensoru
 */
void log_vypadek(int sens) {
  times.last_crit_end[sens] = getTime(); // ulozit konec vypadku
  save_last_crit(sens); // ulozit posledni vypadek do EEPROM
  #if LOG_ON
    log_vypadky_ulozVypadek(sens, times.last_crit_start[sens], times.last_crit_end[sens]);
  #endif
}

/** Zjisti, zda je teplota v poradku (neprekracuje povoleny limit)
 *  @param sens Cislo sensoru
 *  @param temp Teplota sensoru
 */
bool isTemperatureOK(int sens, float curr) {

  static unsigned char fail_count[SENSOR_COUNT] = {0}; // kolikrat po sobe byla namerena hodnota mimo limit
  static unsigned char ok_count[SENSOR_COUNT] = {MEASURES_SPREE}; // kolikrat po sobe byla namerena hodnota v limitu


  // KONTROLA ZDA JE MERENI V LIMITU NEBO MIMO LIMIT
  if (isInternalBad(sens, curr)) { // je PROUD MIMO LIMIT?
    /* teplota je mimo povoleny limit */
    ok_count[sens] = 0; // vynulovat pocitadlo "V LIMITU" mereni
    if (fail_count[sens] < MEASURES_SPREE)
      ++fail_count[sens]; // inkrementovat pocitadlo "MIMO LIMIT" mereni
  } else {
    /* teplota je v povolenem limitu */
    fail_count[sens] = 0; // vynulovat pocitadlo "MIMO LIMIT" mereni
    if (ok_count[sens] < MEASURES_SPREE)
      ++ok_count[sens]; // inkrementovat pocitadlo "V LIMITU" mereni
  }


  

  // NASTAVENI ALERT/NORMAL REZIMU PRO CIDLO
  if (!alert.is_sensor_alert[sens]) {
    if (fail_count[sens] >= MEASURES_SPREE) {
      // ================================ //
      // PRECHOD Z NORMAL DO ALERT REZIMU
      // ================================ //
      alert.is_sensor_alert[sens] = true; // zapnout alert
      send_alert_state(sens, curr); // pokusit se odeslat mail
      times.last_crit_start[sens] = getTime(); // zaznamenat zacatek vypadku
      return false;

    } else {
      // ================================ //
      // ZNOVU NORMAL REZIM
      // ================================ //
      alert.is_sensor_alert[sens] = false; // vypnout alert

      
      if (SENDMAIL_NORMAL_TRY_REPEATS && !alert.sended_alert[sens] && millis() > last_ms_normal_try + SENDMAIL_NORMAL_TRY_MILLIS_INTERVAL) { // pokud mail nebyl jeste uspesne odeslan, pokusi se jej odeslat cca co 30 sekund
        send_normal_state(sens, curr); // poslat mail nyni
        last_ms_normal_try = millis();

        static char alert_normal_repeats_remain = SENDMAIL_NORMAL_TRY_REPEATS;
        if (!alert_normal_repeats_remain) {
          alert.sended_alert[sens] = true;
          alert_normal_repeats_remain = SENDMAIL_NORMAL_TRY_REPEATS;
        } else {
          --alert_normal_repeats_remain;
        }
      }
      return true;
    }
  } else {
    if (ok_count[sens] >= MEASURES_SPREE) {
      // ================================ //
      // PRECHOD Z ALERT DO NORMAL REZIMU
      // ================================ //
      alert.is_sensor_alert[sens] = false; // vypnout alert
      if (alert.sended_alert[sens]) { // pokud se behem alertu stacil odeslat mail 
        send_normal_state(sens, curr); // odesli mail o navratu do normalu
      } else {
        alert.sended_alert[sens] = true; // pokud se behem prekroceni proudy nepodarilo odeslat mail o prekroceni, neposilej ani ted mail o vraceni do normalu
      }

      /* LOGOVANI VYPADKU */
      log_vypadek(sens);


      return true;

    } else {
      // ================================ //
      // ZNOVU ALERT REZIM
      // ================================ //
      alert.is_sensor_alert[sens] = true; // zapnout alert
      
      if (SENDMAIL_ALERT_TRY_REPEATS && !alert.sended_alert[sens] && millis() > last_ms_alert_try + SENDMAIL_ALERT_TRY_MS_INTERVAL) { // pokud dosud nebyl odeslan mail, pokusi se jej odeslat cca co 30 sekund
        send_alert_state(sens, curr); // poslat mail nyni
        static char repeats_remain = SENDMAIL_ALERT_TRY_REPEATS;
        if (!repeats_remain) {
          alert.sended_alert[sens] = true;
          repeats_remain = SENDMAIL_ALERT_TRY_REPEATS;
        } else {
          --repeats_remain;
        }
        last_ms_alert_try = millis();
      }
      return false;
    }
  }
}

/* Measures currents and updates data*/
void update_currents() {

  mt.phasA = 0; // soucet proudu na fázi A
  mt.phasB = 0; // soucet proudu na fázi B
  mt.phasC = 0; // soucet proudu na fázi C

  mt.aktiv_sensors = 0; // pocet aktivnich cidel
  mt.curr_warnings = 0; // pocet cidel mimo limit

  for (int i = 0; i < SENSOR_COUNT; i++) { // prochazet vsechny senzory
    
    if (sensor[i].pin == PIN_OFF) {
      alert.is_sensor_alert[i] = false;
      mt.curr[i] = UNKNOWN_VALUE;
      continue;
    }
    
    // cteni napeti senzoru
    
    float sensor_value = analogRead(sensor[i].pin);
    float volt = sensor_value * (2.56 / 1023.0);
    
    mt.curr[i] = ((volt + sensor[i].posun_A) * sensor[i].posun_B);

    float positive_curr = mt.curr[i];
    if (positive_curr < 0) positive_curr = 0;

    // POCITANI PROUDU NA JEDNOTLIVYCH FAZICH
    if (i >= 0 && i <= 3) { // fáze A
      mt.phasA += positive_curr;
    } else if (i >= 4 && i <= 7) { // fáze B
      mt.phasB += positive_curr;
    } else if (i >= 8 && i <= 11) { // fáze C
      mt.phasC += positive_curr;
    }
    
//    // prevedu napeti na string
//    char s_volt[10];
//    floatToString(s_volt, volt);
//    
//    // nactu si POSTFIX vzorec
//    String s_postfix = sensor[i].formula;
//    
//    // nahradim symbol [V] namerenym napetim
//    s_postfix.replace("V", s_volt);
//    
//    // prevedu string na pole charu
//    char postfix[40];
//    s_postfix.toCharArray(postfix, 40);
//    
//    // prevedu pomoci postfix vzorce napeti na proud
//    if (!solve_postfix(postfix, &mt.curr[i])) { // pokus se vyresit postfix vzorec
//      mt.curr[i] = volt * 10; // pokud se nepovede, provedu standartni vypocet
//    }
    
    ++mt.aktiv_sensors; // zvysi se pocet aktivnich senzoru

    if (!isTemperatureOK(i, positive_curr)) { // proud je mimo nastaveny limit
      ++mt.curr_warnings; // inkrementuje se pocet varovani
    }
  }

  if (mt.curr_warnings) { // proudy JSOU FAIL
    if (!alert.is_global_alert) { // PRETIM BYLY OK
      #if DEBUG_ON
        Serial.println("GLOBAL ALERT!");
      #endif
      times.global_warn_start = getTime(); // zacatek globalniho vypadku
      
    }
    alert.is_global_alert = true;
  } else { // proudy JSOU OK
    if (alert.is_global_alert) { // PREDTIM BYLY FAIL
      #if DEBUG_ON
        Serial.println("GLOBAL NORMAL");
      #endif
      times.global_warn_end = getTime(); // konec globalniho vypadku
      save_last_crit();
    }
    alert.is_global_alert = false;
  }

      
    floatToString(mt.s_phasC, mt.phasC);
    floatToString(mt.s_phasB, mt.phasB);
    floatToString(mt.s_phasA, mt.phasA);
    itoa(mt.aktiv_sensors, mt.saktiv_senzor, 10);

}

/* Inicializuje klavesnici */
void keyboard_setup() {
  pinMode(BT_COMMON, OUTPUT); //common key
  digitalWrite(BT_COMMON, LOW); //common key na OV
  pinMode(BT_LED, OUTPUT); // LED (sviti pri LOW)

  /* Tlacitka pripojena pres PULLUP odpory */
  pinMode(BT_AUTO, INPUT_PULLUP);
  pinMode(BT_MENU, INPUT_PULLUP);
  pinMode(BT_LEFT, INPUT_PULLUP);
  pinMode(BT_RIGHT, INPUT_PULLUP);
  pinMode(BT_SELECT, INPUT_PULLUP);
  pinMode(BT_ONOFF, INPUT_PULLUP);
}

void setup(void) {
  Serial.begin(9600);

  // Napetova reference pro analogove cteni pinu
  analogReference(INTERNAL2V56);
  
  /* CONFIGURATION LOAD */
  /* Nacte vzorecky pro mereni proudu */
  load_pinout();
  load_sensors();
  // nacte proudove limity
  load_currlimit();
  // interval refreshe casu z NTP serveru
  load_time_interval();

  // ntp server
  load_ntp_server();



  // Nacte interval mezi jednotlivymi merenimi
  load_measure_interval();
  
  // Nacte konfiguraci pro zasilani mailu
  load_email_auth(); // autorizace "auth login"
  load_email_address(); // e-mail prijemce, odesilatele
  load_email_smtp(); // SMTP server 

  // nastavit pocatecni alert cidel
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    alert.sended_alert[i] = true; // true znamena, ze se nebudu pokouset odesilat e-mail
  }

  // nacte display konfiguraci
  lcd_setup(); // konfigurace
  delay(100); // prodleva pro provedeni inicializace displeje
  load_display_conf(); // nastaveni displeje -> nacte jas a kontrast

  keyboard_setup(); // konfigurace klavesnice
  
  turnLED(true); // LED na klavesnici symbolizujici, ze probiha jeste "boot"

  // moznost FACTORY RESET pri SETUPU - aby slo obnovit tovarni nastaveni jeste pred nastavenim IP adresy
  /* pokud uzivatel drzi AUTO, MENU i SELECT 2 sekundy soucasne, dojde k tovarnimu nastaveni */
  if (is_reset_pressed()) {
    delay(50);
    if (is_reset_pressed()) {
      delay(1950);
      if (is_reset_pressed()) {
        factory_reset();
      }
    }
  }
  
  //RTC inicializace
  rtc_setup();
  times.boot_time = getTime(); // cas zapnuti Arduina

  
  // nacte posledni vypadky
  load_last_crits();

  /* KONFIGURACE ETHERNETU */
  load_ethernet_settings(); // nacist konfiguraci z EEPROM
  eth_setup(); // nastavit ethernet
  
  turnLED(false); // konec setupu, zhasnout LED na klavesnici
}

void loop(void) {
  static unsigned long loop_start = 0; // pro ukadani millis
  loop_start = millis();
  
  #if DEBUG_COMPONENT
      Serial.println("<LOOP>");
  #endif
  
  unsigned long now_time = getTime(); // aktualni cas (epoch)

  // DHCP RENEW
  if (!ethernet.static_ip && now_time > last_time_dhcp_renew + DHCP_RENEW_S_INTERVAL) {
    // renew IP adresy, pokud ji mam od DHCP serveru
    Ethernet.maintain();
    last_time_dhcp_renew = now_time;
  }

  // LCD SETUP REPEAT
  if (now_time > last_time_lcd_setup + LCD_SETUP_INTERVAL) {
    pinMode(LCD_BACKLIGHT, OUTPUT); //podsviceni displeje jako vystup
    display.begin(); // inicializace
    delay(5);
    load_display_conf();
    display.clearDisplay(); // smazani obrazovky
    last_time_lcd_setup = now_time;
  }


  // SYNCHRONIZACE CASU PRES NTP
  if (ntp.sync_interval && now_time > last_time_ntp_sync + ntp.sync_interval) {
    turnLED(true);
    ethernet.client->stop();
    time_syncNTP();
    last_time_ntp_sync = getTime();
    turnLED(false);
  }

  // ZPRACOVANI CIDEL
  if (millis() > last_ms_measure + measure.interval) { // pokud ubehl interval od posledniho mereni
    #if DEBUG_COMPONENT
      Serial.println("<MEAS PROCESS>");
    #endif
    
    update_currents(); // zpracuj cidla
    last_ms_measure = millis(); // update millis posledniho zpracovani cidel
    
    #if DEBUG_COMPONENT
      Serial.println("</MEAS PROCESS>");
    #endif
  }

  
  // ZPRACOVANI KLIENTA
  #if DEBUG_COMPONENT
    Serial.println("<CLIENT PROCESS>");
  #endif
  
  ethernet.client->stop();
  client_processing();  
  #if DEBUG_COMPONENT
    Serial.println("</CLIENT PROCESS>");
  #endif
  /* zpracovani LCD */
  #if DEBUG_COMPONENT
    Serial.println("<LCD PROCESS>");
  #endif
  zpracovani_lcd();
  #if DEBUG_COMPONENT
    Serial.println("</LCD PROCESS>");
  #endif

  // ZASLANI TEST MAILU VYZADANEHO Z ADMINISTRACE
  if (try_test_mail == 1) { // pokusit se odeslat testovani email o Prekroceni proudu?
    float curr = mt.curr[0]; // proud na senzoru0
    if (curr < 0) curr = 0; // zaporne proudy na 0.0
    
    if (send_alert_state(0, curr)) { // pokus o odeslani
      try_test_mail = 0; // odeslani OK
    } else {
      try_test_mail = -1; // odeslani FAIL
    }
    
  } else if (try_test_mail == 2) { // pokusit se odeslat testovaci email o Navratu proudu do normalu?
    float curr = mt.curr[0]; // teplota na senzoru0
    if (curr < 0) curr = 0;
    if (send_normal_state(0, curr)) { // pokus o odeslani
      try_test_mail = 0; // odeslani OK
    } else {
      try_test_mail = -1; // odeslani FAIL
    }
  }

  
  // KONTROLA PRETECENI millis()
  unsigned long loop_end = millis(); // end millis na konci smycky
  
  if (loop_start < loop_end) { // pokud je milis na konci smycky vyssi nez na zacatku (= miilis z konce predchozi smycky)
    loop_start = loop_end; // mastav start millis pro dalsi smycku
    
  } else { // doslo k preteceni millis()
    loop_start = 0; // zacatek start millis pro pristi smycku nastav na 0

    /* Vynulovat po preteceni millis() */
    last_ms_button_click = 0; // millis() cas posledniho stisku na klavesnici
    last_ms_ajax_run = 0; // millis() cas od posledniho zaslani AJAX pozadavku
    last_ms_normal_try = 0; // millis() cas od posledniho pokusu o zaslani mailu o navratu stavu do normalu
    last_ms_alert_try = 0; // millis() cas od posledniho pokusu o zaslani alert mailu
    last_ms_measure = 0; // millis() od posledniho mereni hodnot na cidlech
  } 
}
