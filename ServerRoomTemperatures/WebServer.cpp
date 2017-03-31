#include "WebServer.h"
#include "Storage.h"
#include "Time.h"
#include "Base64.h"
#include "GlobalFunctions.h"
#include "SDCard.h"
#include "AirConding.h"
#include <avr/pgmspace.h>

/* variables */
extern struct str_alert alert;
extern int sensor_pin[SENSOR_COUNT];
extern struct str_eth ethernet;
extern struct str_tepoloty mt;
extern struct str_time times;
extern struct str_ntp ntp;
extern struct str_email email;
extern struct str_measure measure;
extern unsigned long last_ms_ajax_run;

extern char try_test_mail;

/** Odeslat klientovi informaci o uspesnem provedeni
 *  @param client Client socket
 *  @param text Volitelna hlaska pro vypis
 */
void html_redirect_ok(EthernetClient *client, const char *text = NULL) {
  client_send_html(client, "rdok", 1, text);
}

/** Odeslat klientovi informaci o neuspesnem provedeni 
 *  @param client Client socket
 *  @param text Volitelna hlaska pro vypis
 */
void html_redirect_fail(EthernetClient *client, const char *text = NULL) {
  client_send_html(client, "rdfa", 1, text);
}

/**
 * CSV vystup pro dany sensor
 * @param c Klient socket
 * @param numb cislo sensoru
 */
void csv_sensor(EthernetClient *c, int numb) {
  char teplota[10];
  if (mt.temp[numb] >= SENSOR_MIN_TEMP && mt.temp[numb] <= SENSOR_MAX_TEMP) {
    ltoa((unsigned long)(mt.temp[numb] * 10), teplota, 10);
  } else {
    strcpy(teplota, "N/A");
  }
  PROGMEM const char * str_http = "HTTP/1.1 200 OK";
  c->println(str_http);
  c->println("");
  c->println(teplota);
  c->println("0");
  c->print("TIME: ");
  c->println( prevedCas( getTime() ) );
  
  char label_file[20];
  char cidlo[5];
  itoa(numb, cidlo, 10);
  strcpy(label_file, "lab");
  strcat(label_file, cidlo);
  client_send_html(c, label_file, 0);
  c->println();
}

/* Stranka nenalezena */
void html_page404(EthernetClient *client) {
  PROGMEM const char * str_404 = "<h1>404</h1>";
  client->println(str_404);
}

/* Stranka s nastavenim sensoru (prirazeni PINU + nastaveni jeho popisku) */
void html_nastaveni_sensoru(EthernetClient *client) {
  
  client_send_html(client, "setsens0", 0); // zacatek HTML stranky
  for (int i = 0; i < SENSOR_COUNT; ++i) { // Vypis pro kazdy sensor
    char id[5];
    itoa(i, id, 10);

    /* Sensor popisek */
    char label_file[10] = "lab";
    char pin[5];
    itoa(sensor_pin[i], pin, 10);
    strcat(label_file, id);
    String text = sdcard_getfile(label_file);
    char popis[50];
    text.toCharArray(popis, 50);

    /* Dropdown list s PINY */
    client_send_html(client, "pSensID", 2, id, id); 
    for (int j = -1; j < 16; ++j) { // vkladani polozek do Dropdown listu 
      
        client->print("<option");
        if (sensor_pin[i] == j || (j == -1 && sensor_pin[i] == PIN_OFF)) {
          client->print(" selected>");
        } else {
          client->print(">");
        }
        if (j == -1) {
          client->print("vyp");
        } else {
          client->print("A");
          client->print(j);
        }
        client->println("</option>");

    } 

    /* Popis sensoru, proud a vzorec */
    client_send_html(client, "setsens", 2, id, popis); 
  }
  // ukonceni HTML stranky
  client_send_html(client, "setsens1", 0);
}


/* Tabulka se souhrnnymi informacemi  */
void html_statistics(EthernetClient *client) {
  PROGMEM const char * fn_stat1 = "stat1";
  PROGMEM const char * fn_stat2 = "stat2";
  
  client_send_html(client, fn_stat1, 0);
  client_send_html(client, fn_stat2, 0);
}

/* Ovladani klimatizace */
void html_airCondTable(EthernetClient *client) {
  PROGMEM const char * fn_aircond0 = "aircond0";
  PROGMEM const char * fn_aircond1 = "aircond1";
  client_send_html(client, fn_aircond0, 0);
  client_send_html(client, fn_aircond1, 0);
}


/* Home stranka */
void html_monitoring_temperatures(EthernetClient *client) {
  PROGMEM const char * fn_temp_nadp = "temp_nadp";
  client_send_html(client, fn_temp_nadp, 0);
  
  html_statistics(client);
  html_airCondTable(client);
  html_tempTable(client);
}


/* Stranka s nastavenim limitnich globalnich teplot */
void html_temp_limits(EthernetClient *client) {
  char sens_min[10];
  char sens_max[10];
  char alert_min[10];
  char alert_max[10];
  char temp_bound[10];
  itoa(SENSOR_MIN_TEMP, sens_min, 10);
  itoa(SENSOR_MAX_TEMP, sens_max, 10);
  itoa(alert.critic_min, alert_min, 10);
  itoa(alert.critic_max, alert_max, 10);
  itoa(alert.temp_bound, temp_bound, 10);
  PROGMEM const char * fn_setlimit = "setlimit";
  client_send_html(client, fn_setlimit, 5, sens_min, sens_max, alert_min, alert_max, temp_bound);
}

/* Stranka s nastavenim casu */
void html_nastaveni_casu(EthernetClient *client) {
 
  /* Pocatecni HTML kod */
  PROGMEM const char * fn_settime = "settime";
  client_send_html(client, fn_settime, 0);

  /* Ziskat aktualni cas */
  prevedCas(getTime());  char den[5];
  char mesic[5];
  char rok[5];
  char hod[5];
  char min[5];
  char sec[5];
  itoa(times.day, den, 10);
  itoa(times.month, mesic, 10);
  itoa(times.year, rok, 10);
  itoa(times.hour, hod, 10);
  itoa(times.minute, min, 10);
  itoa(times.second, sec, 10);

  /* INFO O AKTUALNE NASTVENEM CASU + TLACITKO PRO NTP SYNCHRONIZACI */
  unsigned long inter = ntp.sync_interval;
  char seconds[5];
  itoa(inter % 60, seconds, 10);
  inter /= 60;
  char minutes[5];
  itoa(inter % 60, minutes, 10);
  inter /= 60;
  char hours[5];
  itoa(inter % 24, hours, 10);
  inter /= 24;
  char days[5];
  itoa(inter, days, 10);

  PROGMEM const char fn_setadvint [] = "setAdvInt";
  PROGMEM const char str_text [] = "Interval NTP synchronizace";
  client_send_html(client, fn_setadvint, 6, "q4", str_text, days, hours, minutes, seconds);

  
  /* RUCNI NASTAVENI CASU + HTML kod pro konec stranky */
  PROGMEM const char fn_settime2 [] = "settime2";
  client_send_html(client, fn_settime2, 6, den, mesic, rok, hod, min, sec);
  
  /* NASTAVENI NTP SERVERU */
  char ntp_port[6];
  itoa(ntp.ntp_port, ntp_port, 10);
  client_send_html(client, "setntp", 2, ntp.ntp_server, ntp_port);
}

/* Stranka s nastavenim emailu */
void html_nastaveni_emailu(EthernetClient *client) {
  /* SMTP SERVER */
  char ip[4][4];
  char port[6];
  for (int i = 0; i < 4; ++i) {
    itoa(email.smtp_server[i], ip[i], 10);
  }
  itoa(email.smtp_port, port, 10);
  client_send_html(client, "setmail", 5, ip[0], ip[1], ip[2], ip[3], port);
  
  /* PRIHLASOVACI UDAJE PRO SMTP SERVER*/
  char checked[10] = "";
  char login[50];
  char pass[50];
  // set up
  if (email.auth) {
    strcpy(checked, "checked");
  }
  base64_decode(login, email.auth_login, strlen(email.auth_login)); // Nacti a preved login z BASE64
  base64_decode(pass, email.auth_pass, strlen(email.auth_pass)); // Nacti a preved heslo z BASE64
 
  /* ODESILATEL A PRIJEMCE */
  client_send_html(client, "setmail1", 3, checked, login, pass);
  client_send_html(client, "setmail2", 2, email.from, email.to);
}


void html_nastaveni_mereni(EthernetClient *client) {
  /* NASTAVENI INTERVALU MEZI MERENIMI */
  char interval[10];
  char konstanta[10];
  
  itoa(measure.interval, interval, 10);
  floatToString(konstanta, measure.konstanta);
  client_send_html(client, "mereni", 2, interval, konstanta);
}

/* Stranka s nastavenim ethernetu */
void html_nastaveni_ethernetu(EthernetClient *client) {
  /* INFO O MAC A IP */
  char act_mac[20];
  char act_ip[20];
  strcpy(act_mac, getMacAdress());
  strcpy(act_ip, getIpAdress());
  if (ethernet.static_ip) {
    client_send_html(client, "ethernet", 3, act_ip, act_mac, "checked");
  } else {
    client_send_html(client, "ethernet", 3, act_ip, act_mac, NULL);
  }
  /* KONFIGURACE IP ADRES */
  char ip[4][5];
  char mask[4][5];
  char gw[4][5];
  char dns[4][5];
  for (int i=0; i<4; ++i) {
    itoa(ethernet.ip[i], ip[i], 10);
    itoa(ethernet.subnet[i], mask[i], 10);
    itoa(ethernet.gateway[i], gw[i], 10);
    itoa(ethernet.dns_server[i], dns[i], 10);
  }
  client_send_html(client, "eth1", 4, ip[0], ip[1], ip[2], ip[3]); // vlastni ip adresa
  client_send_html(client, "eth2", 4, mask[0], mask[1], mask[2], mask[3]); // sitova maska
  client_send_html(client, "eth3", 4, gw[0], gw[1], gw[2], gw[3]); // gateway brana
  client_send_html(client, "eth4", 4, dns[0], dns[1], dns[2], dns[3]); // dns server
  client_send_html(client, "eth0", 0);
}

/* Tabulka s informacemi o jednotlivych cidlech */
void html_tempTable(EthernetClient *client) {

  /* ZACATEK HTML KODU */
  client_send_html(client, "lsnadp", 0);
  
  unsigned long now_time = getTime();

  /* VYPIS TABULKY */
  for (int i = 0; i < SENSOR_COUNT; ++i) { // Prochazim vsechna aktivni cidla
    if (sensor_pin[i] == PIN_OFF) continue;
    
    /* NAZEV CISLA, JEHO PIN, AKTUALNI PROUD A PROGRESSBAR */
    char cidlo[5];
    char cidlo_pin[5];
    char minim[10];
    char maxim[10];
    itoa(i, cidlo, 10);
    itoa(sensor_pin[i], cidlo_pin, 10);
    floatToString(minim, alert.sens_min[i]);
    floatToString(maxim, alert.sens_max[i]);
    client_send_html(client, "lbs", 7, cidlo, cidlo_pin, cidlo, mt.s_temp[i], cidlo, cidlo, cidlo);

    /* NASTAVENY LIMIT CIDLA */
    char min_ret[10];
    char max_ret[10];
    floatToString(min_ret, alert.sens_min[i] + alert.temp_bound);
    floatToString(max_ret, alert.sens_max[i] - alert.temp_bound);
    if (alert.sens_lim[i]) {
      client_send_html(client, "lbs2", 4/*3*/, minim, min_ret, maxim, max_ret);
    } else {
      client_send_html(client, "lbs2d", 6/*5*/, minim, minim, min_ret, maxim, maxim, max_ret);
    }
    
    /* POSLEDNI VYPADEK A POPIS CIDLA */
    client->print("</td><td id='vyp");
    client->print(cidlo);
    client->print("'></td><td id='pop");
    client->print(cidlo);
    client->println("'></td></tr>");
  }
  /* KONEC HTML KODU */
  client->println("</table>");
}

/* Stranka s vyberem cidel pro CSV export */
void csv_export(EthernetClient *c) {
  client_send_html(c, "csv0", 0);
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    if (sensor_pin[i] == PIN_OFF) continue;
    char id[3];
    itoa(i, id, 10);
    client_send_html(c, "csv1", 2, id, id);
  }
  client_send_html(c, "divEND", 0);
}


#if XML_ON
/* XML cast s udaji o jednotlivych sensorech */
void xml_sensor(EthernetClient *client, unsigned char i) {
  char alert_type[7];
  if (alert.sens_lim[i]) {
    strcpy(alert_type, "OWN");
  } else {
    strcpy(alert_type, "GLOBAL");
  }
  char alert_min[7];
  char alert_max[7];
  floatToString(alert_min, alert.sens_min[i]);
  floatToString(alert_max, alert.sens_max[i]);

  char alert_status[4];
  if (alert.is_sensor_alert[i]) {
    strcpy(alert_status, "YES");
  } else {
    strcpy(alert_status, "NO");
  }
  client_send_html(client, "xmls", 5, mt.s_temp[i], alert_type, alert_min, alert_max, alert_status);
}



/* XML export */
void xml_export(EthernetClient *client) {
  prevedCas(getTime());
  char tmp[8][10];
  itoa(times.year, tmp[0], 10);
  itoa(times.month, tmp[1], 10);
  itoa(times.day, tmp[2], 10);
  itoa(times.hour, tmp[3], 10);
  itoa(times.minute, tmp[4], 10);
  itoa(times.second, tmp[5], 10);
  itoa(alert.critic_min, tmp[6], 10);
  itoa(alert.critic_max, tmp[7], 10);

  char aktiv_cidel[5];
  char varovani[5];

  itoa(mt.aktiv_sensors, aktiv_cidel, 10);
  itoa(mt.temp_warnings, varovani, 10);
  
  client_send_html(client, "xml", 13, tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6], tmp[7], aktiv_cidel, varovani, mt.sprumerna, mt.smaximalni, mt.sminimalni );

  for (int i = 0; i < SENSOR_COUNT; ++i) {
    if (sensor_pin[i] == PIN_OFF) continue;
    
    client->print("<sensor id='");
    char tmp[2];
    itoa(i, tmp, 10);
    client->print(tmp);
    client->println("'>");
    xml_sensor(client, i);
    client->println("</sensor>");
  }
  client->println("</temperatures>");
}

#endif

/* Ajaxove odeslani aktualniho casu */
void ajax_time(EthernetClient *client) {
  char ntp_interval[40];
  char last_sync_date[25];
  char next_sync_countdown[40];
  unsigned long now_time = getTime();
  if (times.synced) {
    strcpy(next_sync_countdown, getFormatedTime(times.last_sync + ntp.sync_interval - now_time));
    strcpy(last_sync_date, prevedCas(times.last_sync));
  } else {
    strcpy(next_sync_countdown, "N/A");
    strcpy(last_sync_date, "N/A");
  }
  strcpy(ntp_interval, getFormatedTime(ntp.sync_interval));
  
  client_send_html(client, "tim.aj", 4, prevedCas(getTime()), last_sync_date, ntp_interval, next_sync_countdown);
}

/* Ajaxove odeslani vsech hodnot */
void ajax_summary(EthernetClient *client) { // Zpracovani ajax pozadavku
  unsigned long now_time = getTime();
  char infolab[60]; // teplota prekrocena/v norme
  char info[75]; // doba prekroceni/normy
  char uptime[50]; // uptime Arduina
  char rtctemp[15];
  
  if (alert.is_global_alert) {
    /* "teplota je prekrocena od" */
    fill_chararray_sdfile(infolab, 60, "lbTmF"); 
    /* doba prekroceni */
    strcpy(info, prevedCas(times.global_warn_start));
    strcat(info, " (");
    strcat(info, getFormatedTime(get_timediff(times.global_warn_start, now_time )));
    strcat(info, ")");
  } else {
    /* "teplota je v norme od" */
    fill_chararray_sdfile(infolab, 60, "lbTm0");
    /* doba normalu */
    if (times.global_warn_end > times.boot_time) {
      strcpy(info, prevedCas(times.global_warn_end));
      strcat(info, " (");
      strcat(info, getFormatedTime(get_timediff(now_time, times.global_warn_end)));
    } else {
      strcpy(info, prevedCas(times.boot_time));
      strcat(info, " (");
      strcat(info, getFormatedTime(get_timediff(now_time, times.boot_time)));
    }
    strcat(info, ")");
  }
  /* Arduino uptime */
  strcpy(uptime, getFormatedTime(get_timediff(now_time, times.boot_time)));
  /* RAM info */
  char sram[70]; // obsazenost RAM p
  char tmp[10];
  int ram_used = ramSize() - freeRam();
  itoa(ram_used, tmp, 10);
  strcpy(sram, tmp);
  strcat(sram, "B/");
  itoa(ramSize(), tmp, 10);
  strcat(sram, tmp);
  strcat(sram, "B (");
  long ram_usage = ((long)ram_used * 100) / (long)ramSize();
  itoa(ram_usage, tmp, 10);
  strcat(sram, tmp);
  strcat(sram, " %, ");
  itoa(freeRam(), tmp, 10);
  strcat(sram, tmp);
  strcat(sram, "B free)");
  /* RTC temp */
  float rtctempFloat = getRTCTemp();
  floatToString(rtctemp, rtctempFloat);
  client_send_html(client, "sum.ajax", 7, infolab, info, prevedCas(now_time), uptime, sram, mt.saktiv_senzor, rtctemp);

  char temp_avg[15];
  char temp_min[15];
  char temp_max[15];
  /* Prumerna teplota */
  if (mt.prumerna == UNKNOWN_VALUE) {
    strcpy(temp_avg, "N/A");
  } else {
    floatToString(temp_avg, mt.prumerna);
  }
  
  /* Minimalni teplota */
  if (mt.minimalni == UNKNOWN_VALUE) {
    strcpy(temp_min, "N/A");
  } else {
    floatToString(temp_min, mt.minimalni);
  }
  
  /* Maximalni teplota */
  if (mt.maximalni == UNKNOWN_VALUE) {
    strcpy(temp_max, "N/A");
  } else {
    floatToString(temp_max, mt.maximalni);
  }
  
  /* Globalni max limit */
  char crit_max[10];
  itoa(alert.critic_max, crit_max, 10);
  /* GLobalni min limit */
  char crit_min[10];
  itoa(alert.critic_min, crit_min, 10);
  char aircond[2][5];
  for (int i = 0; i < 2; ++i) {
    char ac_temp;
    ac_temp = air_cond_get_temperature(i);
    if ( ac_temp == -1) { // pokud nebyla od spusteni arduina natavena zadna teplota, ale klima bezi
      strcpy(aircond[i], "N/A"); // nevim, co je nastavene
    } else if (ac_temp >= 17 && ac_temp <= 30) { // pokud klima bezi a nastavoval jsem teplotu
      itoa(air_cond_get_temperature(i), aircond[i], 10); // zobrazim ji
    } else {
      strcpy(aircond[i], "OFF"); // jinak je klima vypnuta
    }
  }
  client_send_html(client, "sum2.ajax", 7, temp_avg, temp_max, temp_min, crit_max, crit_min, aircond[0], aircond[1]);
  
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    // prevedu si cislo cidla na String
    itoa(i, tmp, 10);
    /* Procenta progress baru */
    char proc_prog[10];
    strcpy(proc_prog, "");
    int procento;
    
    if (mt.temp[i] != (float)UNKNOWN_VALUE) {
      if (!alert.is_sensor_alert[i]) {
        procento = (mt.temp[i] - alert.sens_min[i]) * 100.0 / (alert.sens_max[i] - alert.sens_min[i]);
      } else {
        if (mt.temp[i] > (float)alert.sens_max[i] - alert.temp_bound) {
          procento = 100;
        } else if (mt.temp[i] < (float)alert.sens_min[i] + alert.temp_bound) {
          procento = 0;
        }
      }
      itoa(procento , proc_prog, 10);
      
      //itoa( (int) (float)(mt.temp[i] - alert.sens_min[i]) * 100.0 / (alert.sens_max[i] - alert.sens_min[i]), proc_prog, 10);
    } else {
      strcpy(proc_prog, "-1");
    }
    /* Posledni vypadek */
    char last_break[100];
    if (!alert.is_sensor_alert[i]) {

      if (times.last_crit_start[i] < times.last_crit_end[i]) {
        strcpy(last_break, prevedCas(times.last_crit_start[i]));
        strcat(last_break, " - ");
        strcat(last_break, prevedCas(times.last_crit_end[i]));
        strcat(last_break, " (");
        strcat(last_break, getFormatedTime(get_timediff(times.last_crit_end[i], times.last_crit_start[i])));
        strcat(last_break, ")");
      } else {
        strcpy(last_break, "-");
      }
    } else {
      
      if (mt.temp[i] > (float)alert.sens_max[i]) {
        strcpy(proc_prog, "100");
      } else {
        strcpy(proc_prog, "0");
      }
      
      strcpy(last_break, prevedCas(times.last_crit_start[i]));
      strcat(last_break, " - NYNÍ (");
      strcat(last_break, getFormatedTime(get_timediff(now_time, times.last_crit_start[i])));
      strcat(last_break, ")");
    }
    /* Popisek senzoru */
    char label_file[20];
    strcpy(label_file, "lab");
    strcat(label_file, tmp);
    fill_chararray_sdfile(infolab, 30, label_file);

    client_send_html(client, "sens.ajax", 12, tmp, mt.s_temp[i], tmp, tmp, last_break, tmp, tmp, infolab, tmp, tmp, proc_prog, tmp);

    
  }
  
  client->println("</ajax>");
}



/** Funkce pro ziskani udaje z GET pozadavku
 *  @param client Client socket
 *  @param propery Do teto promenne se ulozi nazev vlastnosti
 *  @return Vrati se hodnota vlastnosti
 */
String get_prop_value(EthernetClient *client, String *property) {
  char c;
  *property = "";
  String val = "";

  while ( (c = client->read()) != '=' && c != -1 && c != '\n') { // ctu nazev property
    *property += c;
  }
  if (c == '=') { // kontrola, zda nasleduje value
    // parsing value
    while ( (c = client->read()) != ' ' && c != '&' && c != -1 && c != '\n') { // cteni value
      val += c;
    }
  }
  #if REQ_DEBUG
    Serial.print("PROPERTY:");
    Serial.print(*property);
    Serial.print("=");
    Serial.println(val);
  #endif

  return val;
}

/**
 * Ziska hodnotu checkboxu, nazev checkboxu musi zacinat pismenem 'x' !!!!
 * @param client Client socket
 * @property Do teto promenne se ulozi nazev vlastnosti (bez pismena x)
 * @return vrati se, zda je checkbox zaskrknut nebo ne
 */
bool get_checkbox_value(EthernetClient *client, String *property) {
  if (char c = client->read() == 'x') {    // prectu jeden znak, abych zjistil, zda je checkobox zaskrkly (kdyby nebyl, neni vubec v requestu)
    get_prop_value(client, property); // pokud je checkbox pritomny, prectu jeho cely nazev i property
    return true; // CHECKBOX je zaskrkly
  } else {
    return false; // CHECKBOX neni zaskrkly
  }
}



bool nastaveni_ntp_serveru_update(EthernetClient *client) {
  String property;
  String address = get_prop_value(client, &property);
  int port = get_prop_value(client, &property).toInt();

  if (port < 0 || port > 65535) return false;

  address.toCharArray(ntp.ntp_server, 50);
  ntp.ntp_port = port;
  save_ntp_server();
  return true;
}


/* Updatuje nastaveny cas z GET pozadavku*/
bool nastaveni_casu_manual_update(EthernetClient *client) {
  String property;
  // den
  String value = get_prop_value(client, &property);
  int den = value.toInt();
  // mesic
  value = get_prop_value(client, &property);
  int mesic = value.toInt();
  // rok
  value = get_prop_value(client, &property);
  int rok = value.toInt();
  // hodina
  value = get_prop_value(client, &property);
  int hodina = value.toInt();
  // minuta
  value = get_prop_value(client, &property);
  int minuta = value.toInt();
  // sekunda
  value = get_prop_value(client, &property);
  int sekunda = value.toInt();

  if (den < 1 || den > 31 || mesic < 1 || mesic > 12 || rok < 1900 || 
  hodina < 0 || hodina > 23 || minuta < 0 || minuta > 59 || sekunda < 0 || sekunda > 59) {
    return false;
  } 
  
  unsigned long epoch = tmConvert_t(rok, mesic, den, hodina, minuta, sekunda); // epoch v GTM+1
  if (jeLetniCas(epoch - 3600, NULL)) { // pokud je letni cas
    epoch -= 3600; // odeberu jeste jednu hodinu
  }
  
  time_sync_intern(epoch); 
  return true;
}


/* zjisti z GET pozadavku cislo sensoru a vypise pro nej CSV */
void csv_export_sens(EthernetClient *client) {
  String property; 
  String value = get_prop_value(client, &property);

  int sens_numb = value.toInt();
  csv_sensor(client, sens_numb);
}


/* Aktualizuje z GET pozadavku nastaveni SMTP serveru pro zasilani emailu*/
bool email_smtp_update(EthernetClient *client) {
  String property;
  String ip1 = get_prop_value(client, &property);
  String ip2 = get_prop_value(client, &property);
  String ip3 = get_prop_value(client, &property);
  String ip4 = get_prop_value(client, &property);
  String p = get_prop_value(client, &property);

  unsigned int port = p.toInt();
  if (port < 1 && port > 65535) return false;
  
  byte ip[4];
  ip[0] = ip1.toInt();
  ip[1] = ip2.toInt();
  ip[2] = ip3.toInt();
  ip[3] = ip4.toInt();

  save_email_smtp(ip, port); // ulozim konfiguraci

  return true;
}

/* Aktualizuje z GET pozdavku nastaveni emailovych prihlasovacich udaju (SMTP auth) */
bool email_auth_update(EthernetClient *client) {
  String property;
  bool auth = get_checkbox_value(client, &property);
  
  String user = get_prop_value(client, &property); 
  String pass = get_prop_value(client, &property);
  coding_fix(&user); // oprava kodovani po projiti pres GET pozadavek
  coding_fix(&pass); // oprava kodovani po projiti pres GET pozadavek
  
  if (auth && (user == "" || pass == "")) {
    return false;
  } else if (!auth) {
    save_email_auth(auth, "", "");
    return true;
  } 
  
  char username[40];
  char password[40];
  user.toCharArray(username, 40);
  pass.toCharArray(password, 40);

  save_email_auth(auth, username, password); // ulozeni konfigurace
  return true;
}

/* Aktualizuje z GET pozadavku adresu prijemce a odesilatele pro zasilani emailu*/
bool email_address_update(EthernetClient *client) {
  String property;
  String from = get_prop_value(client, &property);
  String to = get_prop_value(client, &property);

  
  if (from == "" || to == "") {
    return false;
  }

  // hlavne kvuli prevodu zpet na '@'
  coding_fix(&to);
  coding_fix(&from);

  save_email_address(from, to);
  return true;
}

/* Aktualizuje z GET pozadavku  interval mezi jednotlivymi merenimi */
bool measure_interval_update(EthernetClient *client) {
  String property;
  String interval = get_prop_value(client, &property);
  String konstanta = get_prop_value(client, &property);
  
  long i = interval.toInt();
  float k = konstanta.toFloat();
  return save_measure_interval(i, k);
}

/* Aktualizuje z GET pozadavku rucni nataveni ethernetu */
bool ethernet_manual_update(EthernetClient *client) {
  String property;
  byte address[4][4];
  bool manual = get_checkbox_value(client, &property);

  /* PARSOVANI ADRES */
  for (int i = 0; i < 4; ++i) { // pro ip, masku, branu a dns
    for (int j = 0; j < 4; ++j) { // pro prvni, druhou, treti a ctvrtou cast adresy
      address[i][j] = get_prop_value(client, &property).toInt(); // ziskej danou cast adresy, preved ji na int a uloz do pole
      if (address[i][j] < 0 || address[i][j] > 255) {
        return false;
      }
    }
  }

  
  if (ethernet.static_ip = manual) {
    for (int i = 0; i < 4; ++i) {
      ethernet.ip[i] = address[0][i];
      ethernet.subnet[i] = address[1][i];
      ethernet.gateway[i] = address[2][i];
      ethernet.dns_server[i] = address[3][i];
    }
  }
  save_ethernet_settings(); // uloz nastaveni
  software_Reset(); // a resetuj Arduino
  return true;
}

/* Aktualizuje z GET pozadavku nastaveni unikatniho limitu pro dany sensor */
void templimit_invidual_update(EthernetClient *client) {
  String property;
  String id = get_prop_value(client, &property);
  bool enable = get_checkbox_value(client, &property);
  String min = get_prop_value(client, &property);
  String max = get_prop_value(client, &property);

  float minim = StringToFloat(min);
  float maxim = StringToFloat(max);
  int i = id.toInt();
  
  if (enable && minim < maxim && minim >= SENSOR_MIN_TEMP && maxim <= SENSOR_MAX_TEMP) {
    alert.sens_lim[i] = true;
    alert.sens_min[i] = minim;
    alert.sens_max[i] = maxim;
    save_templimit_invidual();
    load_templimit_invidual();
  } else {
    alert.sens_lim[i] = false;
    alert.sens_min[i] = alert.critic_min;
    alert.sens_max[i] = alert.critic_max;
    save_templimit_invidual();
    load_templimit_invidual();
  }
}

/* Aktualizuje z GET pozadavku nastaveni teploty klimatizace */
void aircond_set_update(EthernetClient *client, unsigned char id) {
  String property;
  String temp = get_prop_value(client, &property);
  air_cond_set_temperature(id, temp.toInt());
}



/* updatuje popis cidla z GET get requestu */
void pin_popis_update(EthernetClient *client) {
  /* nacist popisek */
  String property;
  String cidlo_id = get_prop_value(client, &property);
  String popis = get_prop_value(client, &property);
  
  coding_fix(&popis); // upravit jeho kodovani po projiti skrz GET pozadavek 
  String name = "lab" + cidlo_id; // nastavit nazev souboru
  sdcard_overwrite(name, popis); // ulozit popisek jako tento soubor
}

/* Aktualizuje z GET pozadavku pinout cisla*/
void  pin_pinout_update(EthernetClient *client) {

  String property;
  String cidlo_id = get_prop_value(client, &property);
  String pinout = get_prop_value(client, &property);

  int id = cidlo_id.toInt();
  if (pinout == "vyp") {
    sensor_pin[id] = PIN_OFF;
  } else {
    pinout.remove(0, 1); // remove 1 character at index 0
    int pin = pinout.toInt();
    sensor_pin[id] = pin;
  }
  save_pinout(id);
}

/* Aktualizuje z GET pozadavku interval updatovani casu pres NTP server */
bool nastaveni_casu_interval_update(EthernetClient *client) {
  String property;
  String days = get_prop_value(client, &property);
  String hours = get_prop_value(client, &property);
  String minutes = get_prop_value(client, &property);
  String seconds = get_prop_value(client, &property);
  
  long unsigned sync_interval = days.toInt() * 24;
  sync_interval += hours.toInt();
  sync_interval *= 60;
  sync_interval += minutes.toInt();
  sync_interval *= 60;
  sync_interval += seconds.toInt();

  if (save_time_interval(sync_interval)) {
    return true;
  } else {
    return false;
  }
}


/* Updatuje globalni teplotni limity z GET pozadavku */
bool temp_limits_update(EthernetClient *client) {
  char c;
  String property = "";
  String value_min, value_max;
  float crit_min, crit_max;

  value_min = get_prop_value(client, &property);
  value_max = get_prop_value(client, &property);

  crit_min = StringToFloat(value_min);
  crit_max = StringToFloat(value_max);
  
  String krok = get_prop_value(client, &property);
  
  float navrat_rozdil = krok.toFloat();
  
  if (save_templimit_global(crit_min, crit_max, navrat_rozdil)) {
    return true;
  } else {
    return false;
  }
}

/* Parsuje pozadavek od klienta */
String parse_client_command(EthernetClient *client) {
  String client_command = ""; // prikaz klienta
  char c;
  
  String req_start = ""; // zacatek requestu
  while ( (c = client->read() ) != -1 ) { // prochazim cely post pozadavek
    req_start += c;
    PROGMEM const char * str_favicon = "/favicon.ico";
    if (req_start.indexOf(str_favicon) != -1) { // pokud narazim na FAVICON REGUEST
      return "!"; // ukoncit
    }
    
    if (c == '_') { // pokud narazim na podtrzitko a zaroven se nejedna o REQUEST o favicon

      #if REQ_DEBUG
        Serial.print("CLIENT COMMAND:");
      #endif
      
      while ( (c = client->read() ) != '?' && c != ' '  && c != -1) // ctu dal az do mezery nebo otazniku
      {
        #if REQ_DEBUG
          Serial.print(c);
        #endif
        // a text mezi temito znaky si ulozim
        client_command.concat(c);
      }
      #if REQ_DEBUG
        Serial.println("");
      #endif
      
      #if DEBUG_ON
        Serial.print("client command: [");
        Serial.print(client_command);
        Serial.println("]");
      #endif
      
      break; // kdyz mam ulozeny, muzu vyskocit z cele smycky
    }
  }
  return client_command;
}

/* Cte a zpracovava klientuv pozadavek */
void client_processing(void) {

  // zjistim, zda je pripojen klient
  *(ethernet.client) = ethernet.server->available();

  if (*(ethernet.client)) { // klient je pripojen
    #if DEBUG_ON == 1
      Serial.println("client++");
    #endif
    
    /* dokud je klient pripojen a dostupny (HTTP pozadavek) */
    while (ethernet.client->connected()) {
      String client_command = parse_client_command(ethernet.client); // parsuj pozadavek

      /* ###[NEZADOUCI REQUEST O FAVICON]### */
      if (client_command == "!") { // pokud se jedna o nezadouci Request
        ethernet.client->stop(); // odpoj klienta
        return; // a vysmahni

      /* ###[AJAXOVA VOLANI]### */
      } else if (client_command == "ajs" || client_command == "ajaxtime") { // jedna se o Ajaxove volani
        // zpracovavat ajax pozadavky maximalne 1x za pulsekundu kvuli zamezeni freeznuti arduina (kdyby bylo pripojeno vice klientu)
        if (millis() > last_ms_ajax_run + AJAX_UPDATE_INTERVAL) {
          if (client_command == "ajs") {
            ajax_summary(ethernet.client);
          } else {
            ajax_time(ethernet.client);
          }
          last_ms_ajax_run = millis();
        }

      /* ###[CSV EXPORT SENSORU]### */
      } else if (client_command == "sens") { 
        csv_export_sens(ethernet.client);
      }

      /* ###[XML EXPORT]### */
      #if XML_ON
      // XML Export teplot
      else if (client_command == "xml") {
        xml_export(ethernet.client);
      }
      #endif  

      /* ###[LOG VYPADKU]### */
      else if (client_command == "log" || client_command == "log91587clear") {
        if (client_command == "log91587clear") log_vypadky_clear();
        log_ukaz_vypadky(ethernet.client);

      /* ###[ZOBRAZENI HTML STRANKY]### */
      } else {
          client_send_html(ethernet.client, "main0", 0); // Zacatek HTML kodu stranky
          client_send_html(ethernet.client, "main1", 0); // Leve menu

          /****************************************
           *  VYBER KONKRETNI STRANKY K ZOBRAZENI *
           ****************************************/
          const PROGMEM char * str_chlimit = "q0";
          const PROGMEM char * str_airc0 = "q1";
          const PROGMEM char * str_airc1 = "q2";
          const PROGMEM char * str_timesync = "q3";
          const PROGMEM char * str_q4 = "q4";
          const PROGMEM char * str_nastcasmanup = "q5";
          const PROGMEM char * str_nastntp = "q6";
          const PROGMEM char * str_chemstmp = "q7";
          const PROGMEM char * str_chemauth = "q8";
          const PROGMEM char * str_chemad = "q9";
          const PROGMEM char * str_emtesta = "q10";
          const PROGMEM char * str_emtestn = "q11";
          const PROGMEM char * str_testres = "q12";
          const PROGMEM char * str_chlabel = "q13";
          const PROGMEM char * str_chpinout = "q14";
          const PROGMEM char * str_tmplimup = "q15";
          const PROGMEM char * str_chmeasint = "q16";
          const PROGMEM char * str_chethip = "q17";
          
          /* MAINPAGE */
          if (client_command == "" || client_command == "home" || client_command == str_chlimit 
            || client_command == str_airc0 || client_command == str_airc1) {
              
            // nastaveni teplotniho limitu cidla
            if (client_command == str_chlimit) {
              templimit_invidual_update(ethernet.client);
            } else if (client_command == str_airc0) {
              aircond_set_update(ethernet.client, 0);
            } else if (client_command == str_airc1) {
              aircond_set_update(ethernet.client, 1);
            }

            // stranka s vypisem informaci
            html_monitoring_temperatures(ethernet.client);

            // Posle klientovi skripty pro pozadani o AJAX pozadavek a jeho zpracovani 
            client_send_html(ethernet.client, "sc0", 0);
            client_send_html(ethernet.client, "sc1", 0);
            client_send_html(ethernet.client, "sc2", 0);


          /* MENU PRO CSV VYPIS */
          } else if (client_command == "csv") {
            csv_export(ethernet.client);

          /* GLOBALNI LIMITY */
          }  else if (client_command == "tmpLim" || client_command == str_tmplimup) {

            /* Aktualizace limitu */
            if (client_command == str_tmplimup) {
              char * str_limitu = "limitu";
              if (temp_limits_update(ethernet.client)) {
                html_redirect_ok(ethernet.client, str_limitu); // limity aktualizovany
              } else {
                html_redirect_fail(ethernet.client, str_limitu); // chyba pri aktualizaci
              }
            }
            /* Teplotni limity */
            html_temp_limits(ethernet.client);

          /* NASTAVENI CASU */
          } else if (client_command == "nastCas" || client_command == str_timesync || client_command == str_q4 || client_command == str_nastcasmanup || client_command == str_nastntp) {

              /* Aktualizace casu pres NTP */
              if (client_command == str_timesync) {
                if (time_syncNTP()) {
                  PROGMEM const char * str_succes = "<h2 style='color:green'>Synchronizace času úspěšná</h2>";
                  ethernet.client->println(str_succes);
                } else {
                  PROGMEM const char * str_failed = "<h2 style='color:red'>Synchronizace času neúspěšná</h2>";
                  ethernet.client->println(str_failed);
                }

              /* Nastaveni intervalu synchronizace s NTP */
              } else if (client_command == str_q4) {
                PROGMEM const char * str_intervalu = "intervalu";
                if (nastaveni_casu_interval_update(ethernet.client)) {
                  html_redirect_ok(ethernet.client, str_intervalu);
                } else {
                  html_redirect_fail(ethernet.client, str_intervalu);
                }

              /* Manualni nastaveni casu */
              } else if (client_command == str_nastcasmanup) {
                PROGMEM const char * str_casu = "casu";
                if (nastaveni_casu_manual_update(ethernet.client)) { 
                  html_redirect_ok(ethernet.client, str_casu);
                } else {
                  html_redirect_fail(ethernet.client, str_casu);
                }
              } else if (client_command == str_nastntp) {
                PROGMEM const char * str_serveru = "serveru";
                if (nastaveni_ntp_serveru_update(ethernet.client)) {
                  html_redirect_ok(ethernet.client, str_serveru);
                } else {
                  html_redirect_fail(ethernet.client, str_serveru);
                }
              }
              html_nastaveni_casu(ethernet.client); // Stranka s nastavenim casu

          /* NASTAVENI SENSORU */

          } else if (client_command == "nastSens" || client_command == str_chlabel || client_command == str_chpinout) {
            
            if (client_command == str_chlabel) {
              pin_popis_update(ethernet.client); // zmena popisku senzoru 
            } else if (client_command == str_chpinout) {
              pin_pinout_update(ethernet.client); // zmena pinu senzoru 
            }
            // stranka s nastavenim senzoru
            html_nastaveni_sensoru(ethernet.client);

          /* NASTAVENI EMAILU */
          } else if (client_command == "nast-mail" || client_command == str_chemstmp 
            || client_command == str_chemauth || client_command == str_chemad || client_command == str_emtesta || client_command == str_emtestn || client_command == str_testres) {
              
            if (client_command == str_chemstmp) {
              email_smtp_update(ethernet.client); // Zmena SMTP serveru
              
            } else if (client_command == str_chemauth) {
              email_auth_update(ethernet.client); // zmena autorizacnich udaju
              
            } else if (client_command == str_chemad) {
              email_address_update(ethernet.client); // zmena odesilatele a prijemce
            
            } else if (client_command == str_emtesta) {
              try_test_mail = 1; // posle testovaci mail
              client_send_html(ethernet.client, "scrd", 1, "4"); // cekat 4 sekund na odeslani mailu pred zobrazenim statusu
              
            } else if (client_command == str_emtestn) {
              try_test_mail = 2; // posle testovaci mail
              client_send_html(ethernet.client, "scrd", 1, "4"); // cekat 4 sekund na odeslani mailu pred zobrazenim statusu
              
            } else if (client_command == str_testres) {
                if (try_test_mail == 0) {
                  PROGMEM const char * str_mail_ok = "<h2 style='color:green'>Testovaci mail byl uspesne odeslan</h2>";
                  ethernet.client->println(str_mail_ok);
                } else {
                  PROGMEM const char * str_mail_fail = "<h2 style='color:red'>Testovaci mail nebyl odeslan</h2>";
                  ethernet.client->println(str_mail_fail);
                }
            }
            
            html_nastaveni_emailu(ethernet.client); // Stranka s nasvenim mailu

          /* NASTAVENI INTERVALU MERENI */
          } else if (client_command == "nast-mer" || client_command == str_chmeasint) {

            if (client_command == str_chmeasint) {
              PROGMEM const char * str_intervalu = "intervalu";
              if (measure_interval_update(ethernet.client)) {  // pokus o zpracovani pripadneho pozadavku
                html_redirect_ok(ethernet.client, str_intervalu); // vypis o uspesnem zpracovani pozadavku
              } else {
                html_redirect_fail(ethernet.client, str_intervalu); // vypis o neuspesnem zpracovani pozadavku
              }
            }
            html_nastaveni_mereni(ethernet.client); // vypis pro klienta

            
          /* NASTAVENI ETHERNETU */
          } else if (client_command == "nast-eth" || client_command == str_chethip) {
            if (client_command == str_chethip) {
              ethernet_manual_update(ethernet.client); // zpracovani pripadneho pozadavku
            }
            html_nastaveni_ethernetu(ethernet.client); // vypis pro klienta
            
          } else {
            html_page404(ethernet.client); // stranka nenalezena
          }
          
          // zbytek html tagu k dokonceni stranky
          client_send_html(ethernet.client, "main2", 0);
      }
      
      // cas klienta na zpracovani pozadavku
      delay(10);
      ethernet.client->stop();
      #if DEBUG_ON
        Serial.println("client--");
      #endif
    }
  }
}
