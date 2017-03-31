#include "Display.h"
#include "Types.h"
#include "GlobalFunctions.h"
#include "Time.h"
#include "AirConding.h"
#include <avr/pgmspace.h>

const char *label_ip = "IP:";
const char *label_gateway = "GATEWAY:";
const char *label_mask = "MASK:";
const char *label_dns = "DNS:";

#if LCD_LIBRARY
  #include <Adafruit_GFX.h>
  #include <Adafruit_PCD8544.h>
#endif

/* ukazatel na obrazovku, ktera se vykresluje */
struct str_obrazovka *scr2render = create_temperature_screens(SENSOR_COUNT); // ze zacatku inicializuju obrazovkou, na ktere jsou teploty
/* ukazatel na obrazovku, ktera funguje jako menu */
struct str_obrazovka *menu = create_menu_screen(); // inicializace pomoci funkce, ktera vytvori a vrati menu obrazovku
extern struct str_tepoloty mt; // struktura pro ukladani namerenych teplot
extern struct str_alert alert; // struktura pro ukladani upozorneni a limitnich teplot
extern struct str_time times;
extern struct str_eth ethernet;

#if LCD_LIBRARY
  extern Adafruit_PCD8544 display;
#else
  extern TangerLCD display;
#endif

struct str_display display_settings; // uchovava jas a kontrast displeje


/**
 * Zvyrazni polozku na displeji (např. [Menu], [Nastaveni]) z leve strany, pokud je zvolena
 * @param o ukazatel na obrazovku
 * @param i poradove cislo polozky na obrazovce
 * @param c uzaviraci znak pro zvyrazneni polozky
 */
void hoverL(struct str_obrazovka *o, int i, char c) {
  if (o->cislo == i) {
    display.print(c);
  }
}

/**
 * Zvyrazni polozku na displeji (např. [Menu], [Nastaveni]) z prave strany, pokud je zvolena
 * @param o ukazatel na obrazovku
 * @param i poradove cislo polozky na obrazovce
 * @param c otviraci znak pro zvyrazneni polozky
 */
void hoverR(struct str_obrazovka *o, int i, char c) {
  if (o->cislo == i) {
    display.print(c);
  }
}

/**
 * Zvyrazni polozku na displeji (např. [Menu], [Nastaveni]) z leve strany a prida odradkovani, pokud je zvolena
 * @param o ukazatel na obrazovku
 * @param i poradove cislo polozky na obrazovce
 * @param c uzaviraci znak pro zvyrazneni polozky
 */
void selectL(struct str_obrazovka *o, int i, char c) {
  hoverL(o, i, c);
}

/**
 * Zvyrazni polozku na displeji (např. [Menu], [Nastaveni]) z prave strany a prida odrazkovani, pokud je zvolena
 * @param o ukazatel na obrazovku
 * @param i poradove cislo polozky na obrazovce
 * @param c otviraci znak pro zvyrazneni polozky
 */
void selectR(struct str_obrazovka *o, int i, char c) {
  hoverR(o, i, c);
  display.println();
}

/* Zobrazi obrazovku s nastavenim kontrastu a jasu */
void screen_show_set_lcd(struct str_obrazovka *o) {
  char tmp[10];
  display.setCursor(0, 0);

  /* NADPIS */
  PROGMEM const char * str_display_set = "Display set";
  display.println(str_display_set);

  /* KONTRAST */
  PROGMEM const char * str_kontrast = "Kontrast:";
  display.println(str_kontrast);
  if (o->cislo == 0) { // pokud je KONTRAST vybrany
    display.print("< "); // zobrazit u nej zvyrazneni
  }
  floatToString(tmp, display_settings.kontrast - CONTRAST_MIN); // aby byl nejmensi mozny kontrast zobrazovan jako 0
  display.print(tmp);
  if (o->cislo == 0) { // pokud je KONTRAST vybrany
    display.println(" >"); // zobrazit u nej zvyrazneni
  } else {
    display.println("");
  }

  /* JAS */
  PROGMEM const char * str_jas = "Jas:";
  display.println(str_jas);
  if (o->cislo == 1) { // pokud je JAS vybrany
    display.print("< "); // zobrazit u nej zvyrazneni
  }
  floatToString(tmp, display_settings.jas);
  display.print(tmp);
  if (o->cislo == 1) { // pokud je JAS vybrany
    display.println(" >"); // zobrazit u nej zvyrazneni
  } else {
    display.println(""); // jinak pouze odradkovat
  }
  display.display();
}

/* Zobrazi obrazovku pro editaci adresy */
void screen_show_edit_address(struct str_obrazovka *o) {
  display.setCursor(0, 0);
  display.println(o->nazev);

  char address[4];
  for (int i = 0; i < 4; ++i) {
    hoverL(o, i, '<');
    unsigned char dig = o->text[o->offset][i];
    itoa(dig, address, 10);
    display.print(address);
    hoverR(o, i, '>');
    
    if (i == 3) {
      display.println("");
    } else {
      display.println(".");
    }
  }

  display.display();
}

/* Zobrazi obrazovku s natavenim Ethernetu */
void screen_show_set_eth(struct str_obrazovka *o) {
  char tmp[10];
  display.setCursor(0, 0);
  if (o->offset == 0) {
    PROGMEM const char * str_ip = "IP adresa:";
    PROGMEM const char * str_mac = "MAC adresa:";
    PROGMEM const char * str_setup = " [Setup] ";
    
    display.println(str_ip);
    display.println(getIpAdress());
    display.println(str_mac);
    display.println(getMacAdress());
    display.println(str_setup);
  
  } else if (o->offset == 1) {
    selectL(o, 0, '<');
    PROGMEM const char * str_static = "Static:";
    display.print(str_static);
    selectR(o, 0, '>');
    if (o->property1 == 1) {
      display.println("YES");
    } else {
      display.println("NO");
    }
    
    display.println("");
    
    selectL(o, 1, '<');
    display.print(label_ip);
    selectR(o, 1, '>');
    display.println(showIpAdress((unsigned char *)o->text[0]));
    
    display.println("     1/3");
    
    
  } else if (o->offset == 2) {

    selectL(o, 2, '<');
    display.print(label_mask);
    selectR(o, 2, '>');
    display.println(showIpAdress((unsigned char *)o->text[1]));

    display.println("");

    selectL(o, 3, '<');
    display.print(label_gateway);
    selectR(o, 3, '>');
    display.println(showIpAdress((unsigned char *)o->text[2]));
    
    display.println("     2/3");

  } else if (o->offset == 3) {
    selectL(o, 4, '<');
    display.print(label_dns);
    selectR(o, 4, '>');
    display.println(showIpAdress((unsigned char *)o->text[3]));
    display.println("");
  
    selectL(o, 5, '<');
    PROGMEM const char * str_save = "SAVE";
    display.print(str_save);
    selectR(o, 5, '>');
    
    display.println("\n     3/3");
    
  }
  display.display();
}



/* Zobrazi obrazovku s nastavenim teplotnich limitu - vyber mezi GLOB a UNIQ*/
void screen_show_temp_limits(struct str_obrazovka *o) {
  char tepl[10];
  display.setCursor(0, 0);
  /* NADPIS */
  PROGMEM const char * str_lim_temp = "Limity teplot";
  display.println(str_lim_temp);

  selectL(o, 0, '<');
  PROGMEM const char * str_sens_uniq = "SENS UNIQUE";
  display.print(str_sens_uniq);
  selectR(o, 0, '>');

  selectL(o, 1, '<');
  display.print("GLOBAL");
  selectR(o, 1, '>');
    
  display.display();
}

/* Zobrazi obrazovku s nastavenim globalnich teplotnich limitu */
void screen_show_temp_limits_glob(struct str_obrazovka *o) {
  char tepl[10];
  display.setCursor(0, 0);
  /* NADPIS */
  display.println("Limity teplot");

  /* MAXIMALNI LIMIT */
  PROGMEM const char * str_max = "Maximum:";
  display.println(str_max);
  selectL(o, 0, '<');
  floatToString(tepl, o->property1);
  display.print(tepl);
  selectR(o, 0, '>');

  /* MINIMALNI LIMIT */
  PROGMEM const char * str_min = "Minimum:";
  display.println(str_min);
  selectL(o, 1, '<');
  floatToString(tepl, o->property2);
  display.print(tepl);
  selectR(o, 1, '>');

  PROGMEM const char * str_bound = "Bound:";
  selectL(o, 2, '<');
  display.print(str_bound);
  floatToString(tepl, o->prop[0]);
  display.print(tepl);
  selectR(o, 2, '>');
  
  display.display();
}


/* Zobrazi obrazovku s nastavenim unikatnich i teplotnich limitu - zap/vyp UNIQUE + nataveni limitu */
void screen_show_temp_limits_uniq(struct str_obrazovka *o) {
  char tmp[10];
  display.setCursor(0, 0);
  
  selectL(o, 0, '<');
  display.print("Sensor");
  itoa(o->offset, tmp, 10);
  display.print(tmp);
  selectR(o, 0, '>');

  selectL(o, 1, '<');
  display.print("Limit: ");
  if (o->prop[0] == 0) {
    display.print("GLOB");
  } else {
    display.print("UNIQ");
  }
  selectR(o, 1, '>');

  if (o->prop[0]) {
    /* MAXIMALNI LIMIT */
    const char str_max[] PROGMEM  = "Maximum:";
    display.println(str_max);
    selectL(o, 2, '<');
    floatToString(tmp, o->property1);
    display.print(tmp);
    selectR(o, 2, '>');
  
    /* MINIMALNI LIMIT */
    const char str_min[] PROGMEM = "Minimum:";
    display.println(str_min);
    selectL(o, 3, '<');
    floatToString(tmp, o->property2);
    display.print(tmp);
    selectR(o, 3, '>');
  }
  
  display.display();
}

/* Zobrazi obrazovku s nastavenim casu (vyber mezi manualnim a NTP sync) */
void screen_show_set_time(struct str_obrazovka *o) {
  display.setCursor(0, 0);
  display.print("Cas: ");
  display.println(prevedCas(getTime(), true));
  display.println("");

  // Manualni nastaveni casu
  selectL(o, 0, '<');
  display.print("Ruc. nast.");
  selectR(o, 0, '>');
  
  // NTP synchronizace
  selectL(o, 1, '<');
  display.print("NTP sync");
  selectR(o, 1, '>');
  if (o->prop[0] == -1) {
    display.println("");
  } else if (o->prop[0] == 0) {
    display.println(" Failed");
  } else if (o->prop[0] == -2) {
    display.println(" waiting...");
  } else {
    display.println(" Success");
  }
  
  display.display();
}

/* Zobrazi obrazovku s natavenim casu manualnim */
void screen_show_set_time_manual(struct str_obrazovka *o) {
  display.setCursor(0, 0);
  display.println("Cas manual:");
  
  char t_beg;
  char t_end;

  if (o->offset == 0) {
    t_beg = '<';
    t_end = '>';
  } else if (o->offset == 1) {
    t_beg = '[';
    t_end = ']';
  }

  
  for (int i = 0; i <= 5; ++i) {
    
    hoverL(o, i, t_beg);
    display.print(o->prop[i]); // Cas manual nadpis
    hoverR(o, i, t_end);

    switch(i) {
      case 0: // den
      case 1: // mesic
        display.print("."); //nasleduje tecka
        break;
      case 2: // rok
      case 5: // sekunda
        display.println(""); // nasleduje novy radek
        break;
      case 3: // hodina
      case 4: // minuta
        display.print(":"); // nasleduje dvojtecka
        break;
    }
  }

  /* tlacitko pro nastaveni */
  hoverL(o, 6, t_beg);
  PROGMEM const char * str_nastavit = "Nastavit";
  display.print(str_nastavit);
  hoverR(o, 6, t_end);

  display.display();
}



char _s_buff[15]; /* Makro funkce pro vypsani textu a promenne na jeden radek */
#define SCREEN_PRINT_VALUE(TEXT, VALUE) \
  strcpy(_s_buff, TEXT); \
  strcat(_s_buff, VALUE); \
  display.println(_s_buff);

/* Zobrazi obrazovku s aktualnimi teplotami */
bool screen_show_teplota(struct str_obrazovka *o) {
  display.setCursor(0, 0);
  char tmp[10];

    
  itoa(o->cislo, tmp, 10); // cislo senzoru na text
  
  if (!o->prop[0]) {
    strcpy(_s_buff, "<Sens");
    strcat(_s_buff, tmp); // cislo senzoru
    

    strcat(_s_buff, "  ");
    itoa(o->offset + 1, tmp, 10); // cislo podobrazovky (kvuli cislovani od 0 pricitam jednicku
    strcat(_s_buff, tmp);
    strcat(_s_buff, "/3"); // ze tri podobrazovek
    
    strcat(_s_buff, ">");
    
    display.println(_s_buff); 
    
    display.print("   ");
    display.print(mt.s_temp[o->cislo]);
    if (alert.is_sensor_alert[o->cislo]) { // pokud je senzor alert
      display.println(" !!");
    } else {
      display.println("");
    }

    
    if (o->offset == 0) { // Podobrazovka 1
  
      char limit_type;
      if (alert.sens_lim[o->cislo]) {
        limit_type = 'U';
      } else {
        limit_type = 'G';
      }
  
      
      display.print(limit_type);
      display.print("Lmin: ");
      floatToString(tmp, alert.sens_min[o->cislo]);
      display.println(tmp);
      
      display.print(limit_type);
      display.print("Lmax: ");
      floatToString(tmp, alert.sens_max[o->cislo]);
      display.println(tmp);
  
      display.print("Popis: ");
      String sens_label = getSenzorLabel(o->cislo);
  
      if (sens_label.length() > 19) {
        sens_label = sens_label.substring(0, 17);
        sens_label += "..";
      }
      display.print(sens_label);
  
    } else if (o->offset == 1) { // Podobrazovka 2
      display.println(prevedCas(getTime(), true));
      PROGMEM const char * str_uptime = "Uptime: ";
      display.println(str_uptime);
      display.println(getFormatedTime(get_timediff(getTime(), times.boot_time), true)); 
      
    } else if (o->offset == 2) { // Podobrazovka 3
      PROGMEM const char * str_avg = "Avg: ";
      PROGMEM const char * str_max = "Max: ";
      PROGMEM const char * str_min = "Min: ";
      PROGMEM const char * str_alerts = "Alerts:   ";
      SCREEN_PRINT_VALUE(str_avg, mt.sprumerna)
      SCREEN_PRINT_VALUE(str_max, mt.smaximalni)
      SCREEN_PRINT_VALUE(str_min, mt.sminimalni)
      SCREEN_PRINT_VALUE(str_alerts, mt.stemp_warnings)
    }
    
  } else {  // Zobrazit vypadky
    
    display.print("S"); // sensor
    display.print(tmp); // id
    PROGMEM const char * str_posl_vyp = " Posl.vyp:";
    display.println(str_posl_vyp); // nadpis
    if (!alert.is_sensor_alert[o->cislo]) { // pokud neni zrovna vypadek
      if (times.last_crit_start[o->cislo] < times.last_crit_end[o->cislo]) { // ale nejaky uz byl
        display.print(prevedCas(times.last_crit_start[o->cislo], true));
        display.print(" - ");
        display.println(prevedCas(times.last_crit_end[o->cislo], true));
        display.print("(");
        display.print(getFormatedTime(get_timediff(times.last_crit_end[o->cislo], times.last_crit_start[o->cislo]), true));
        display.println(")");
      } else { // a zadny zatim ani nebyl
        display.println("[Zadne]");
      }
    } else { // pokud je vypadek
      display.print(prevedCas(times.last_crit_start[o->cislo], true));
      display.print(" - ");
      display.println(prevedCas(getTime(), true));
      display.print("(");
      display.print(getFormatedTime(get_timediff(getTime(), times.last_crit_start[o->cislo]), true));
      display.println(")");
    }
  }
  
  display.display();
}

/* Zobrazi obrazovku pro factory reset */
void screen_show_factory_reset(struct str_obrazovka *o) {
  display.setCursor(0, 0);
  PROGMEM const char * str_obnovit_tov = "Obnovit tovar.\n nastaveni?";
  display.println(str_obnovit_tov);
  
  display.println("");
  
  hoverL(o, 0, '<');
  display.print("NE");
  hoverR(o, 0, '>');

  display.print("  ");
  
  hoverL(o, 1, '<');
  display.print("ANO");
  hoverR(o, 1, '>');

  display.display();
}

/* Zobrazi menu obrazovku */
void screen_show_menu(struct str_obrazovka *o) {
  display.setCursor(0, 0);
  PROGMEM const char * str_menu = "MENU ";
  display.print(str_menu);
  display.print(o->offset + 1);
  display.println("/2");
  for (int i = 5 * o->offset; i < 5 * o->offset + 5; ++i) {
    if (i > MENU_POLOZEK - 1) break;
    
    if (i == o->cislo) {
      display.print(">");
    } else {
      display.print(" ");
    }
    display.println(o->text[i]);
  }
  display.display();
}

/* Zobrazi obrazovku s menu pro klimatizace */
void screen_show_aircond(struct str_obrazovka *o) {
  char ac_status[2][10];
  char temp;

  
  for (int i = 1, j = 0; i <= 3; i+= 2, ++j) {
      PROGMEM const char * str_off = "OFF";
      PROGMEM const char * str_na = "N/A";
      // Klimatizace
      if (o->cislo == i) {  // v menu je jako aktivni vybrana dana klimatizace a natavuji ji
        if (o->offset > 0) { // klima je zapnuta a nastavovala se hodnota
          itoa(o->offset + 16, ac_status[j], 10); // zobraz posledne nastavenou hodnotu
        } else {
          strcpy(ac_status[j], str_off); // klima je zapnuta, ale nevim nastaveni
        }
        
      } else { // KLIMATIZACE je v menu nevybrana
        temp = air_cond_get_temperature(j);
        if (temp > 0) {
          itoa(temp, ac_status[j], 10);
        } else if (temp == 0) {
          strcpy(ac_status[j], str_off);
        } else {
          strcpy(ac_status[j], str_na);
        }
      }
    
  }
  display.setCursor(0, 0);
  
  selectL(o, 0, '<');
  PROGMEM const char * str_klima0 = "Klima0";
  display.print(str_klima0);
  selectR(o, 0, '>');
  
  selectL(o, 1, '['); // pokud je vybrana, zvyraznit
  display.print(ac_status[0]);
  selectR(o, 1, ']'); // pokud je vybrana, zvyraznit
  
  display.println();
  
  selectL(o, 2, '<');
  PROGMEM const char * str_klima1 = "Klima1";
  display.print(str_klima1);
  selectR(o, 2, '>');
  
  selectL(o, 3, '[');
  display.print(ac_status[1]);
  selectR(o, 3, ']');
  
  display.display();
}

/** Funkce, ktera zjisti o jaky typ obrazovky jde a tuto obrazovku nasledne zobrazi
 *  @param o ukazatel na obrazovku, ktera prijde vykreslit
 */
bool screen_show(struct str_obrazovka *o) {

  switch(o->typ) {
    case SCR_TEMP:
      screen_show_teplota(o);
      break;
    case SCR_MENU:
      screen_show_menu(o);
      break;
    case SCR_LIMS:
      screen_show_temp_limits(o);
      break;
    case SCR_LIMS_UNIQ:
      screen_show_temp_limits_uniq(o);
      break;
    case SCR_LIMS_GLOB:
      screen_show_temp_limits_glob(o);
      break;
    case SCR_LCDSET:
      screen_show_set_lcd(o);
      break;
    case SCR_ETHSET:
      screen_show_set_eth(o);
      break;
    case SCR_TIME:
      screen_show_set_time(o);
      break;
    case SCR_TIME_MANUAL:
      screen_show_set_time_manual(o);
      break;
    case SCR_EDIT_ADDRESS:
      screen_show_edit_address(o);
      break;
    case SCR_FACTORY_RESET:
      screen_show_factory_reset(o);
      break;
    case SCR_AIRCOND:
      screen_show_aircond(o);
      break;
  }
}

/* Vytvori obrazovku s ovladanim klimatizace */
struct str_obrazovka * create_screen_aircond(void) {
  struct str_obrazovka *o = (struct str_obrazovka*) malloc (sizeof(struct str_obrazovka));
  o->typ = SCR_AIRCOND;
  o->cislo = 0;
  o->menu = scr2render;
  return o;
}

/**
 * Vytvori obrazovku pro zmenu IP adresy
 * @param cislo co chci zmenit (0 - ip zarizeni, 1 - maska, 2 - brana, 3 - dns server)
 * @param ip_array ukazatel na aktualne nastavenou adresu
 */
struct str_obrazovka * create_screen_edit_address(int cislo, char **ip_array) {
  struct str_obrazovka *o = (struct str_obrazovka*) malloc (sizeof(struct str_obrazovka));
  o->typ = SCR_EDIT_ADDRESS;
  o->cislo = 0;
  o->offset = cislo;
  switch(o->offset) {
    case 0:
      o->nazev = label_ip;
      break;
    case 1:
      o->nazev = label_mask;
      break;
    case 2:
      o->nazev = label_gateway;
      break;
    case 3:
      o->nazev = label_dns;
      break;
  }
  o->text = ip_array;
  o->menu = scr2render;
  return o;
}

/* Vytvori obrazovku, na ktere se nastavuji teplotni limity */
struct str_obrazovka * create_screen_teplotni_limity(void) {
  struct str_obrazovka *o = (struct str_obrazovka*) malloc (sizeof(struct str_obrazovka));
  o->typ = SCR_LIMS;
  o->cislo = 0;
  o->menu = scr2render;
  return o;
}

/* vytvori obrazovku, na ktere se nastavuje limit k pinum, ktere jsou nastavene jako unikatni */
struct str_obrazovka * create_screen_teplotni_limity_glob(void) {
  struct str_obrazovka *o = (struct str_obrazovka*) malloc (sizeof(struct str_obrazovka));
  o->typ = SCR_LIMS_GLOB;
  o->cislo = 0;
  o->menu = scr2render;
  o->property1 = alert.critic_max;
  o->property2 = alert.critic_min;
  o->prop[0] = alert.temp_bound;
  return o;
}

/** 
 * vytvori obrazovku, na ktere se nastavuji limit k sensoru
 * @param sensor cislo sensoru
*/
struct str_obrazovka * create_screen_teplotni_limity_uniq(int sensor) {
  struct str_obrazovka *o = (struct str_obrazovka*) malloc (sizeof(struct str_obrazovka));

  o->typ = SCR_LIMS_UNIQ;
  o->cislo = 0;
  o->offset = sensor;
  
  if (alert.sens_lim[sensor]) {
    o->prop[0] = true;
  } else {
    o->prop[0] = false;
  }
  o->property1 = alert.sens_max[sensor];
  o->property2 = alert.sens_min[sensor];

  if (sensor == 0) {
    o->menu = scr2render;
  } else {
    o->menu = scr2render->menu;
    free(scr2render);
  }
  return o;
}

/* Vytvori obrazovku, na ktere se nastavuje kontrast a jas displeje */
struct str_obrazovka * create_screen_set_lcd(void) {
  struct str_obrazovka *o = (struct str_obrazovka*) malloc (sizeof(struct str_obrazovka));
  o->typ = SCR_LCDSET; // typ obrazovky
  o->cislo = 0; // cislo obrazovky

  o->menu = scr2render;
  o->left = NULL;
  o->right = NULL;
  o->select = NULL;
  return o;
}

/* Vytvori obrazovku s nastavenim ethernetu */
struct str_obrazovka * create_screen_set_eth(void) {
  struct str_obrazovka *o = (struct str_obrazovka*) malloc (sizeof(struct str_obrazovka));
  o->typ = SCR_ETHSET; // typ obrazovky
  o->cislo = 0; // cislo obrazovky
  o->offset = 0;
  
  o->menu = scr2render;

  o->text = (char **) malloc (4 * sizeof(char *));
  for (int i = 0; i < 4; ++i) {
    o->text[i] = (char *) malloc (4 * sizeof(char));
  }

  if (ethernet.static_ip) {
    o->property1 = 1;
  } else {
    o->property1 = 0;
  }

  for (int i = 0; i < 4; ++i) {
    o->text[0][i] = ethernet.ip[i];
    o->text[1][i] = ethernet.subnet[i];
    o->text[2][i] = ethernet.gateway[i];
    o->text[3][i] = ethernet.dns_server[i];
  }
  return o;
}

/* Vytvori obrazovku pro manualni nastaveni casu */
struct str_obrazovka * create_screen_set_time_manual(void) {
  
  prevedCas(getTime());
  
  struct str_obrazovka *o = (struct str_obrazovka*) malloc (sizeof(struct str_obrazovka));

  o->typ = SCR_TIME_MANUAL;
  o->cislo = 0;
  o->offset = 0;

  o->menu = scr2render;

  o->prop[0] = times.day;
  o->prop[1] = times.month;
  o->prop[2] = times.year;
  o->prop[3] = times.hour;
  o->prop[4] = times.minute;
  o->prop[5] = times.second;

  return o;
}

/* Vytvori obrazovku pro nastavenim casu -> menu s vyberem mezi nastavenim manualne a synchronizaci s NTP*/
struct str_obrazovka * create_screen_set_time(void) {
  struct str_obrazovka *o = (struct str_obrazovka*) malloc (sizeof(struct str_obrazovka));
  o->typ = SCR_TIME;
  o->cislo = 0;
  o->prop[0] = -1;
  o->menu = scr2render;
  return o;
}

/* Vytvori obrazovku pro tovarni nastaveni */
struct str_obrazovka * create_screen_factory_reset(void) {
  struct str_obrazovka *o = (struct str_obrazovka*) malloc (sizeof(struct str_obrazovka));
  o->typ = SCR_FACTORY_RESET;
  o->cislo = 0;
  o->menu = scr2render;
  return o;
}

/* Vytvori list s obrazovkami, na kterych budou teploty jednotlivych cidel */
struct str_obrazovka * create_temperature_screens(int pocet) {
  struct str_obrazovka *obr = NULL, *obr_prvni = NULL;

  // vytvarim obrazovky (promenna "pocet" udava kolik)
  for (int i = 0; i < pocet; ++i) {
    if (!obr) {  /* pokud se vytvari prvni obrazovka */
      obr_prvni = obr = (struct str_obrazovka *) malloc ( sizeof(struct str_obrazovka));
      obr->left = NULL; // predchudce neni
    } else {
      obr->right = (struct str_obrazovka *) malloc (sizeof(struct str_obrazovka)); // vytvorim noveho nasledovnika
      obr->right->left = obr; // nasledovniku priradim predchudce
      obr = obr->right; // n
    }
    obr->right = NULL; // nasledovnik u nove vytvorene je NULL
    obr->select = obr;
    obr->menu = obr;
    obr->cislo = i;
    obr->offset = 0;
    obr->prop[0] = 0;
    obr->typ = SCR_TEMP;
    //strcpy(obr->typ, "teplotni");

    //inic_temperature_screen(obr);
  }
  return obr_prvni;
}

/* Pocatecni nastaveni LCD */
void lcd_setup(void) {
  pinMode(LCD_BACKLIGHT, OUTPUT); //podsviceni displeje jako vystup
  analogWrite(LCD_BACKLIGHT, display_settings.jas); // nastaveni intenzity podsviceni
  display.setContrast(display_settings.kontrast); // nastaveni intenzity kontrastu
  display.begin(); // inicializace
  display.clearDisplay(); // smazani obrazovky
  display.println("*BOOTING*"); // informace o bootovani Arduina
}

/* Vytvori obrazovku pro menu */
struct str_obrazovka *create_menu_screen(void) {
  struct str_obrazovka *main = (struct str_obrazovka *) malloc (sizeof(struct str_obrazovka));
  main->typ = SCR_MENU;

  /* alokuje stringove pole pro jednotlive polozky v menu */
  main->text = (char **) malloc (MENU_POLOZEK * sizeof(char *)); 
  for (int i = 0; i < MENU_POLOZEK; ++i) {
    main->text[i] = (char *) malloc(15 * sizeof(char)); 
  }

  /* Nastavi text k polozkam v menu */
  PROGMEM const char * str_menu0 = "Limity tepl.";
  PROGMEM const char * str_menu1 = "Nastav. LCD";
  PROGMEM const char * str_menu2 = "Ethernet";
  PROGMEM const char * str_menu3 = "Nastav. casu";
  PROGMEM const char * str_menu4 = "Fact. reset";
  PROGMEM const char * str_menu5 = "Klimatizace";
  PROGMEM const char * str_menu6 = "Zavrit";
  strcpy(main->text[0], str_menu0);
  strcpy(main->text[1], str_menu1);
  strcpy(main->text[2], str_menu2);
  strcpy(main->text[3], str_menu3);
  strcpy(main->text[4], str_menu4);
  strcpy(main->text[5], str_menu5);
  strcpy(main->text[6], str_menu6);
  
  main->cislo = 0; 
  main->offset = 0;
  return main;
}

