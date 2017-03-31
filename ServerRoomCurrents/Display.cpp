#include "Display.h"
#include "Types.h"
#include "GlobalFunctions.h"
#include "Time.h"

const char *label_ip = "IP:";
const char *label_gateway = "GATEWAY:";
const char *label_mask = "MASK:";
const char *label_dns = "DNS:";

#if LCD_LIBRARY
  #include <Adafruit_GFX.h>
  #include <Adafruit_PCD8544.h>
#else
  #include "TangerLCD.h"
#endif

/* ukazatel na obrazovku, ktera se vykresluje */
struct str_obrazovka *scr2render = create_current_screens(SENSOR_COUNT); // ze zacatku inicializuju obrazovkou, na ktere jsou proudy
/* ukazatel na obrazovku, ktera funguje jako menu */
struct str_obrazovka *menu = create_menu_screen(); // inicializace pomoci funkce, ktera vytvori a vrati menu obrazovku
extern struct str_tepoloty mt; // struktura pro ukladani namerenych proudu
extern struct str_alert alert; // struktura pro ukladani upozorneni a limitnich proudu
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
  display.println("Display set");

  /* KONTRAST */
  display.println("Kontrast:");
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
  display.println("Jas:");
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
    display.println("IP adresa:");
    display.println(getIpAdress());
    display.println("MAC adresa:");
    display.println(getMacAdress());
    display.println(" [Setup] ");
  
  } else if (o->offset == 1) {
    selectL(o, 0, '<');
    display.print("Static:");
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
    display.print("SAVE");
    selectR(o, 5, '>');
    
    display.println("\n     3/3");
    
  }
  display.display();
}


/* Zobrazi obrazovku s nastavenim proudovech limitu */
void screen_show_curr_limits(struct str_obrazovka *o) {
  char tepl[10];
  display.setCursor(0, 0);
  /* NADPIS */
  display.println("Limity proudu");

  selectL(o, 0, '<');
  display.print("SENS UNIQUE");
  selectR(o, 0, '>');

  selectL(o, 1, '<');
  display.print("GLOBAL");
  selectR(o, 1, '>');
    
  display.display();
}

// Zobrazi obrazovku s globalnimi teplotnimi limity
void screen_show_curr_limits_glob(struct str_obrazovka *o) {
  char tepl[10];
  display.setCursor(0, 0);
  /* NADPIS */
  display.println("Limity proudu");

  /* MAXIMALNI LIMIT */
  display.println("Maximum:");
  selectL(o, 0, '<');
  floatToString(tepl, o->property1);
  display.print(tepl);
  selectR(o, 0, '>');

  /* MINIMALNI LIMIT */
  display.println("Minimum:");
  selectL(o, 1, '<');
  floatToString(tepl, o->property2);
  display.print(tepl);
  selectR(o, 1, '>');

  selectL(o, 2, '<');
  display.print("Bound:");
  floatToString(tepl, o->prop[0]);
  display.print(tepl);
  selectR(o, 2, '>');
  
  display.display();
}


/*
   int typ; // typ obrazovky
  const char *nazev;
  
  char **text;
  
  int prop[6];
  
  int cislo;
  int offset;
  
  int property1;
  int property2;
 */

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

    // tisk polozky menu
    hoverL(o, i, t_beg);  
    display.print(o->prop[i]); // Cas manual nadpis
    hoverR(o, i, t_end);

    // znak za polozkou
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

  /* tlacitko pro potvrzeni nastaveni */
  hoverL(o, 6, t_beg);
  display.print("Nastavit");
  hoverR(o, 6, t_end);

  display.display();
}

char _s_buff[15]; /* Makro funkce pro vypsani textu a promenne na jeden radek */
#define SCREEN_PRINT_VALUE(TEXT, VALUE) \
  strcpy(_s_buff, TEXT); \
  strcat(_s_buff, VALUE); \
  display.println(_s_buff);

/* Zobrazi obrazovku s aktualnimi proudmi */
bool screen_show_proud(struct str_obrazovka *o) {
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
    
    char curr[10];
    if (mt.curr[o->cislo] != UNKNOWN_VALUE) {
      floatToString(curr, mt.curr[o->cislo]);
    } else {
      strcpy(curr, "OFF");
    }
    display.print(curr);
    if (alert.is_sensor_alert[o->cislo]) { // pokud je senzor alert
      display.println(" !!");
    } else {
      display.println("");
    }

    
    if (o->offset == 0) { // Podobrazovka 1
      display.print("LMin: ");
      floatToString(tmp, alert.critic_min);
      display.println(tmp);
      
      display.print("LMax: ");
      floatToString(tmp, alert.critic_max);
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
      display.println("Uptime: ");
      display.println(getFormatedTime(get_timediff(getTime(), times.boot_time), true)); 
      
    } else if (o->offset == 2) { // Podobrazovka 3
      SCREEN_PRINT_VALUE("fazA: ", mt.s_phasA)
      SCREEN_PRINT_VALUE("fazB: ", mt.s_phasB)
      SCREEN_PRINT_VALUE("fazC: ", mt.s_phasC)
      SCREEN_PRINT_VALUE("Alerts:   ", mt.scurr_warnings)
    }
    
  } else {  // Zobrazit vypadky
    
    display.print("S"); // sensor
    display.print(tmp); // id
    
    display.println(" Posl.vyp:"); // nadpis
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
  display.println("Obnovit def.");
  display.println(" nastaveni?");
  
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
  display.print("MENU ");
  display.print(o->offset + 1);
  display.println("/2");
  for (int i = 5 * o->offset; i < 5 * o->offset + 5; ++i) {
    if (i > 5) break;
    if (i == o->cislo) {
      display.print(">");
    } else {
      display.print(" ");
    }
    display.println(o->text[i]);
  }
  display.display();
}

/* Funkce, ktera zjisti o jaky typ obrazovky jde a tuto obrazovku nasledne zobrazi */
bool screen_show(struct str_obrazovka *o) {
  switch(o->typ) {
    case SCR_CURR:
      screen_show_proud(o);
      break;
    case SCR_MENU:
      screen_show_menu(o);
      break;
    case SCR_LIMS:
      screen_show_curr_limits_glob(o);
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
  }
}


/* Vytvori brazovku pro editovani ip adres */
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


/* VYtvori obrazovku, na ktere se nastavuji globalni proudove limity */
struct str_obrazovka * create_screen_proudove_limity_glob(void) {
  struct str_obrazovka *o = (struct str_obrazovka*) malloc (sizeof(struct str_obrazovka));
  o->typ = SCR_LIMS;
  o->cislo = 0;
  o->menu = scr2render;
  o->property1 = alert.critic_max;
  o->property2 = alert.critic_min;
  o->prop[0] = alert.curr_bound;
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

/* Vytvori list s obrazovkami, na kterych budou proudy jednotlivych cidel */
struct str_obrazovka * create_current_screens(int pocet) {
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
    obr->typ = SCR_CURR;
    //strcpy(obr->typ, "proudove");

    //inic_current_screen(obr);
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
  
  main->text = (char **) malloc (6 * sizeof(char *));
  for (int i = 0; i < 6; ++i) {
    main->text[i] = (char *) malloc(15 * sizeof(char));
  }
  strcpy(main->text[0], "Limity tepl.");
  strcpy(main->text[1], "Nastav. LCD");
  strcpy(main->text[2], "Ethernet");
  strcpy(main->text[3], "Nastav. casu");
  strcpy(main->text[4], "Fact. reset");
  strcpy(main->text[5], "Zavrit");
  main->cislo = 0;
  main->offset = 0;
  return main;
}
