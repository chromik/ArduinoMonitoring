#include "DisplayControl.h"
#include "Display.h"
#include "Types.h"
#include "Storage.h"
#include "Time.h"
#include "GlobalFunctions.h"
#include "AirConding.h"
#include <Arduino.h>

#if LCD_LIBRARY
  #include <Adafruit_GFX.h>
  #include <Adafruit_PCD8544.h>
#endif

extern unsigned long last_ms_ajax_run; // cas, kdy se naposledy provedl update hodnot u klienta pres ajax
extern unsigned long last_ms_button_click; // pro zjisteni, jestli se klavesa drzi delsi dobu
extern struct str_obrazovka *scr2render; // aktualne zobrazovana obrazovka
extern struct str_obrazovka *menu; // ukazatel na menu obrazovku
extern struct str_display display_settings; // nataveni displeje (jas, kontrast)
extern struct str_eth ethernet; // ethernet konfigurace
#if LCD_LIBRARY
extern Adafruit_PCD8544 display;
#else
extern TangerLCD display;
#endif

/* funkce pro otevreni menu */
struct str_obrazovka* open_menu_screen(void) {
  /* nastavim navrat z menu na aktualni obrazovku */
  menu->menu = scr2render;
  /* a otevru menu jako aktualni obrazovku */
  return menu;
}

/* Dealokuje aktualne zobrazovanou obrazovku a vrati se zpet na predeslou obrazovku, ze ktere byla aktualne zobrazovana obrazovka volana  */
void screen_dealloc_back() {
  display.clearDisplay();
  struct str_obrazovka *tmp = scr2render;
  scr2render = scr2render->menu;
  free(tmp);
}

/** Zjisti, zda je tlacitko stisknuto + osetruje zakmit 
 *  @param btn PIN, na kterem je tlacitko umisteno
 *  @return true - tlacitko bylo stisknuto, false - tlacitko stisknuto nebylo
 */
bool button_click(int btn) {
  static int multiple = 0; // pocet stisknuti za sebou (=drzeni tlacitka)
  
  // pokud se nedrzi, vratim false;
  if (digitalRead(btn) != LOW) return false;

  // pokud se drzelo, cekam chvili; pokud se drzi uz delsi dobu, delay je mensi kvuli rychlejsimu prochazeni pri drzeni tlacitka
  if (millis() < last_ms_button_click + 200) {
    ++multiple;
  } else {
    multiple = 0;
  }

  switch (multiple) { // cim dele se tlacitko drzi, tim rychlejsi je prochazeni (zmensuje se delay mezi stisky)
    case 0:
    case 1:
      delay(120);
      break;
    case 2:
      delay(90);
      break;
    case 3:
      delay(60);
      break;
    case 4:
      delay(22);
      break;
    case 5:
      delay(5);
      break;
    default:
      delay(0);
      break;
  }

  last_ms_button_click = millis();

  // a pokud se uz nedrzi, vratim false
  if (digitalRead(btn) != LOW) return false;

  last_ms_ajax_run = millis() + AJAX_UPDATE_DELAY_AFTER_KEY_PRESSED; // po stisku tlacitka nastavim delay na AJAXove volani, takze behem obsluhy klavesnice nebudou ajax pozadavky zpomalovat odezvu
  return true;
}

void control_teplota(int stisknuto) {
  if (stisknuto == BT_LEFT) {
    if (scr2render->left) {
      scr2render->left->prop[0] = scr2render->prop[0];
      scr2render->left->offset = scr2render->offset; // prenest aktualni offset do dalsiho zobrazeni
      scr2render = scr2render->left;
    }
  } else if (stisknuto == BT_RIGHT) {
    if (scr2render->right) {
      scr2render->right->prop[0] = scr2render->prop[0];
      scr2render->right->offset = scr2render->offset; // prenest aktualni offset do dalsiho zobrazeni
      scr2render = scr2render->right;
    }
  } else if (stisknuto == BT_MENU) { // Navrat do menu
    scr2render = open_menu_screen();
    
  } else if (stisknuto == BT_SELECT) { // Prochazeni mezi jednotlivymi zobrazenimi
    if (scr2render->offset < 2) {
      ++scr2render->offset;
    } else if (scr2render->offset == 2) {
      scr2render->offset = 0;
    }
  } else if (stisknuto == BT_AUTO) { // zobrazit podrobnosti (
    if (scr2render->prop[0]) {
      scr2render->prop[0] = 0; // podrobnosti OFF
    } else {
      scr2render->prop[0] = 1; // podrobnosti ON
    }
  }
}

void control_edit_address(int stisknuto) {
  if (stisknuto == BT_MENU) {
    if (scr2render->cislo == 0) { // pokud stisknuto na zacatku
      screen_dealloc_back(); // vratit se na predchozi obrazovku
    } else {
      --scr2render->cislo; // vratit se dozadu (--offset)
    }

  } else if (stisknuto == BT_SELECT) {
    if (scr2render->cislo == 3) { // pokud stisknuto na konci
      screen_dealloc_back(); // vratit se na predchozi obrazovku
    } else {
      ++scr2render->cislo; // posunout se dopredu (++offset)
    }

  } else if (stisknuto == BT_LEFT && scr2render->cislo < 4) {
    --scr2render->text[scr2render->offset][scr2render->cislo]; // zmensuj hodnotu daneho offsetu ( [offset1].[offset2].[offset3].[offset4] -> example: 192.168.1.1 )

  } else if (stisknuto == BT_RIGHT && scr2render->cislo < 4) {
    ++scr2render->text[scr2render->offset][scr2render->cislo]; // zvetsuj hodnotu daneho offsetu ( [offset1].[offset2].[offset3].[offset4] -> example: 192.168.1.1 )
  }
}

void control_aircond(int stisknuto) {
  if (scr2render->cislo == 0 || scr2render->cislo == 2) { // pokud mam vybranou klimatizaci A nebo B, ale jeste nenastavuji hodnotu
    if (stisknuto == BT_MENU) {
      screen_dealloc_back();
      
    } else if (stisknuto == BT_SELECT) {
      ++scr2render->cislo;
      scr2render->offset = air_cond_get_temperature(scr2render->cislo / 2); // nacti si aktualni teplotu
      if (scr2render->offset > 0) { // a pokud neni klima vypnuta
        scr2render->offset -= 16; // posun ji (posun je, protoze kdyz OFF = 0, tak kdyz stisknu right -> 1=17°C
      } else {
        scr2render->offset = 0; 
      }
      
    } else if (stisknuto == BT_LEFT) { // Vyber klimu A
      scr2render->cislo = 0;
    } else if (stisknuto == BT_RIGHT) { // Vyber klimu B
      scr2render->cislo = 2;
    }
    
  } else { // nastavuji hodnotu u klimatizace A nebo B
    if (stisknuto == BT_LEFT) { 
      if (scr2render->offset > 0) {
        --scr2render->offset;
      }
    } else if (stisknuto == BT_RIGHT) {
      if (scr2render->offset < 14) {
        ++scr2render->offset;
      }
    } else if (stisknuto == BT_SELECT) {
      if (scr2render->offset <= 0) {
        air_cond_turn_off(scr2render->cislo / 2);
      } else {
        air_cond_set_temperature(scr2render->cislo / 2, scr2render->offset + 16);
      }
      --scr2render->cislo;
      
    } else if (stisknuto == BT_MENU) {
      --scr2render->cislo;
    }
  }
}

// Ovladani hlavniho menu
void control_menu(int stisknuto) { 
  // pokud v menu stisknu menu
  if (stisknuto == BT_MENU) { // zavru menu
    if (scr2render->menu) {
      scr2render = scr2render->menu;
    }
  }
  else if (stisknuto == BT_LEFT) { // predchozi polozka
    if (scr2render->cislo > 0) {
      --scr2render->cislo;
    }
  }

  else if (stisknuto == BT_RIGHT) { // nasledujici polozka
    if (scr2render->cislo < (MENU_POLOZEK - 1)) {
      ++scr2render->cislo;
    }
  }

  scr2render->offset = scr2render->cislo / 5;

  if (stisknuto == BT_SELECT) { // vyber polozky v menu
    switch (scr2render->cislo) {
      case 0: // Nastaveni teplotnich limitu
        scr2render = create_screen_teplotni_limity();
        break;
      case 1: // Nataveni LCD
        scr2render = create_screen_set_lcd();
        break;
      case 2: // Nastavei ETHERNETU
        scr2render = create_screen_set_eth();
        break;
      case 3: // NASTAVENI CASU
        scr2render = create_screen_set_time();
        break;
      case 4: // TOVARNI NASTAVENI
        scr2render = create_screen_factory_reset();
        break;
      case 5: // OVLADANI KLIMATIZACE
        scr2render = create_screen_aircond();
        break;
      case 6:// ZAVRIT MENU
        scr2render->cislo = 0; // po znovuotevreni menu zobrazovat jako aktivni prvni polozku
        scr2render = scr2render->menu; // prejdu na obrazovku teplot (menu odkazuje na obrazovky teplot);
        break;
    }
  }
}

// Ovladani obrazovky s dialogem potvrzeni navratu do tovarniho nastaveni
void control_factory_reset(int stisknuto) {
  if (stisknuto == BT_LEFT) {
    scr2render->cislo = 0;  // Nastavit odpoved na NE
  } else if (stisknuto == BT_RIGHT) {
    scr2render->cislo = 1; // Natavit odpoved na ANO
  } else if (stisknuto == BT_MENU || (scr2render->cislo == 0 && stisknuto == BT_SELECT)) {
    screen_dealloc_back();  // Neprovade Factory reset a vratit se zpet
  } else if (stisknuto == BT_SELECT && scr2render->cislo == 1) { 
    factory_reset(); // provest facotry reset
  }
}

// Ovladani menu pro nastavovani casu
void control_set_time(int stisknuto) {
  if (stisknuto == BT_LEFT) {
    if (scr2render->cislo > 0) {
      --scr2render->cislo;
    }
  } else if (stisknuto == BT_RIGHT) {
    if (scr2render->cislo < 1) {
      ++scr2render->cislo;
    }
  } else if (stisknuto == BT_SELECT) {
    if (scr2render->cislo == 0) { // Manualni nastaveni casu
      struct str_obrazovka * tmp = scr2render;
      scr2render = create_screen_set_time_manual();
      scr2render->menu = tmp;

    } else if (scr2render->cislo == 1) { // NTP synchronizace casu
      // nastavi se hlaska, ze probiha NTP sync
      scr2render->prop[0] = -2;
      screen_show(scr2render);
      // synchronizuje se s NTP a nastavi se status
      if (scr2render->prop[0] <= 0) {
        scr2render->prop[0] = time_syncNTP();
        screen_show(scr2render);
      }
    }
  } else if (stisknuto == BT_MENU) { // vratit se do hl. menu
    screen_dealloc_back();
  }
}

// Ovladani menu pro manualni nastavovani casu
void control_set_time_manual(int stisknuto) {
  if (scr2render->offset == 0) { // vybiram polozku (den X mesic X rok X hodina X minuta X sekunda X tlacitko SAVE)
    if (stisknuto == BT_LEFT) {
      if (scr2render->cislo > 0) {
        --scr2render->cislo;  // predchozi polozka
      }

    } else if (stisknuto == BT_RIGHT) {
      if (scr2render->cislo < 6) {
        ++scr2render->cislo;  // dalsi polozka
      }

    } else if (stisknuto == BT_SELECT) { // vybrat polozku
      if (scr2render->cislo == 6) { // pokud je vybrana polozka SAVE
        unsigned long epoch = tmConvert_t(scr2render->prop[2], scr2render->prop[1], scr2render->prop[0], scr2render->prop[3], scr2render->prop[4], scr2render->prop[5]); // preved nastaveny cas na epoch
        if (jeLetniCas(epoch - 3600, NULL)) { // pokud je letni cas
          epoch -= 3600; // odeberu jeste jednu hodinu
        }
        time_sync_intern(epoch); // a nastav RTC na tento epoch
        screen_dealloc_back(); // vrat se na predchozi obrazovku

      } else {
        scr2render->offset = 1; // nastavim, ze mam polozku vybranou
      }

    } else if (stisknuto == BT_MENU) {
      screen_dealloc_back(); // vrat se zpet na predchozi obrazovku
    }

  } else if (scr2render->offset == 1) { // mam vybranou polozku
    if (stisknuto == BT_LEFT) { // dekrementace polozky
      int limit = 0;
      switch (scr2render->cislo) {
        case 0: // den
        case 1: // mesic
          limit = 1;
          break;
        case 2: // rok
          limit = 2000;
          break;
        case 3: // hodina
        case 4: // minuta
        case 5: // sekunda
        default:
          limit = 0;
      }
      if (scr2render->prop[scr2render->cislo] > limit) { // pokud je polozka vetsi nez 0
        --scr2render->prop[scr2render->cislo]; // zmensi ji
      }

    } else if (stisknuto == BT_RIGHT) { // inktrementace polozky
      int limit = 0;
      
      switch (scr2render->cislo) {
        case 0: // den
          limit = 31;
          break;
        case 1: // mesic
          limit = 12;
          break;
        case 2: // rok
          limit = 2500;
          break;
        case 3: // hohina
          limit = 23;
          break;
        case 4: // minuta
        case 5: // sekunda
          limit = 60;
          break;
      }
      if (scr2render->prop[scr2render->cislo] < limit) {
        ++scr2render->prop[scr2render->cislo];
      }

    } else if (stisknuto == BT_SELECT || stisknuto == BT_MENU) {
      scr2render->offset = 0; // zrusit nastavovani polozky a prejit zase do vyberu polozky
    }
  }
}

// Ovladani menu pro nastavovani LCD
void control_set_lcd(int stisknuto) {
  /* NASTAVENI KONTRASTU DISPLEJE */
  if (scr2render->cislo == 0) {
    // SNIZOVANI KONTRASTU
    if (stisknuto == BT_LEFT) {
      if (display_settings.kontrast > CONTRAST_MIN) {
        --display_settings.kontrast;
        display.setContrast(display_settings.kontrast);
      }
    }
    // ZVYSOVANI KONTRASTU
    else if (stisknuto == BT_RIGHT) {
      if (display_settings.kontrast < CONTRAST_MAX) {
        ++display_settings.kontrast;
        display.setContrast(display_settings.kontrast);
      }
    }
    // PREJDE Z NASTAVOVANI KONTRASTU NA JAS
    else if (stisknuto == BT_SELECT) {
      scr2render->cislo = 1;
    }
    // UKONCI SE NASTAVOVANI DISPLEJE
    else if (stisknuto == BT_MENU) {
      /* vrati se predesle nastaveni (pokud se neco nastavilo a neulozilo, zmeny se ztrati) */
      load_display_conf();
      screen_dealloc_back();
    }

    /* NASTAVENI JASU DISPLEJE */
  } else if (scr2render->cislo == 1) {
    // SNIZOVANI JASU
    if (stisknuto == BT_LEFT) {
      if (display_settings.jas > 0) {
        --display_settings.jas;
        analogWrite(LCD_BACKLIGHT, display_settings.jas);
      }

      // ZVYSOVANI JASU
    } else if (stisknuto == BT_RIGHT) {
      if (display_settings.jas < 255) {
        ++display_settings.jas;
        analogWrite(LCD_BACKLIGHT, display_settings.jas);
      }

      // ULOZENI NASTAVENI DISPLEJE (jak JAS, tak KONTRAST)
    } else if (stisknuto == BT_SELECT) {
      save_display_conf();
      screen_dealloc_back();

      // PREJITI ZPET NA NASTAVENI KONTRASTU
    } else if (stisknuto == BT_MENU) {
      scr2render->cislo = 0;
    }
  }
}

// Ovladani menu pro nastavovani Ethernetu
void control_set_eth(int stisknuto) {

  /* Ethernet info */
  if (scr2render->offset == 0) {
    if (stisknuto == BT_MENU) { // EXIT

      /* delete array with IP, MAC, Gateway and DNS settings variable */
      for (int i = 0; i < 4; ++i) {
        free(scr2render->text[i]);
      }
      free(scr2render->text);

      /* smazat samotnout obrazovku */
      screen_dealloc_back();

    } else if (stisknuto == BT_SELECT) { // Prejit na Ethernet settings
      ++scr2render->offset;
    }

    /* Ethernet settings */
  } else if (scr2render->offset > 0 ) {
    if (stisknuto == BT_LEFT) { // previous item
      if (scr2render->cislo > 0) {
        --scr2render->cislo;
      }

    } else if (stisknuto == BT_RIGHT) { // next item
      if (scr2render->cislo < 5) {
        ++scr2render->cislo;
      }

    } else if (stisknuto == BT_MENU) { // back to Ethernet info
      scr2render->cislo = 0;
      scr2render->offset = 0;

    } else if (stisknuto == BT_SELECT) {
      if (scr2render->cislo == 0) { // ENABLE/DISABLE static IP routing
        if (scr2render->property1 == 0) {
          scr2render->property1 = 1;
        } else {
          scr2render->property1 = 0;
        }

      } else if (scr2render->cislo == 5) { // SAVE current settings

        ethernet.static_ip = scr2render->property1;
        for (int i = 0; i < 4; ++i) {
          ethernet.ip[i] = (unsigned char)scr2render->text[0][i];
          ethernet.subnet[i] = (unsigned char)scr2render->text[1][i];
          ethernet.gateway[i] = (unsigned char)scr2render->text[2][i];
          ethernet.dns_server[i] = (unsigned char)scr2render->text[3][i];
        }
        save_ethernet_settings();
        software_Reset();
      } else {
        scr2render = create_screen_edit_address(scr2render->cislo - 1, scr2render->text); // create and open screen with address edit
      }
    }

    switch (scr2render->cislo) {
      case 1:
        scr2render->offset = 1;  // polozky 0 (Static/Dynamic IP) a 1 (IP address) je na strance 1
        break;
      case 2:
      case 3:
        scr2render->offset = 2; // polozky 2 (MASK) a 3 (GATEWAY) jsou na strance 2
        break;
      case 4:
      case 5:
        scr2render->offset = 3; // polozky 4 (DNS) a 5 (button SAVE) jsou na strance 3
    }
  }
}

/*  */
void control_teplotni_limity(int stisknuto) {
  switch (stisknuto) {
    case BT_LEFT:
      scr2render->cislo = 0;
      break;

    case BT_RIGHT:
      scr2render->cislo = 1;
      break;

    case BT_SELECT:
      if (scr2render->cislo == 0) {
        scr2render = create_screen_teplotni_limity_uniq(menu->menu->cislo);
      } else if (scr2render->cislo == 1) {
        scr2render = create_screen_teplotni_limity_glob();
      }
      break;
    case BT_MENU:
      screen_dealloc_back();
      break;
  }
}


void control_teplotni_limity_uniq(int stisknuto) {
  switch (scr2render->cislo) {
    case 0: // SENSOR SELECT
      if (stisknuto == BT_LEFT && scr2render->offset > 0) { // predchozi senzor
        scr2render = create_screen_teplotni_limity_uniq(scr2render->offset - 1);

      } else if (stisknuto == BT_RIGHT && scr2render->offset < SENSOR_COUNT - 1) { // nasledujici senzor
        scr2render = create_screen_teplotni_limity_uniq(scr2render->offset + 1);

      } else if (stisknuto == BT_SELECT) { // vybrat senzor a prejit na dalsi polozku
        ++scr2render->cislo;

      } else if (stisknuto == BT_MENU) { // zpet do hl. menu
        screen_dealloc_back();
      }
      break;

    case 1: // UNIQUE/GLOBAL setting
      if (stisknuto == BT_LEFT || stisknuto == BT_RIGHT) { // zmenit typ limitu (Unikatni / Globalni)
        if (scr2render->prop[0]) {
          scr2render->prop[0] = 0;
        } else {
          scr2render->prop[0] = 1;
        }

      } else if (stisknuto == BT_SELECT) { // vybrat typ limitu a prejit na dalsi polozku
        if (scr2render->prop[0]) {
          ++scr2render->cislo;
        } else {
          alert.sens_lim[scr2render->offset] = false;
          alert.sens_max[scr2render->offset] = alert.critic_max;
          alert.sens_min[scr2render->offset] = alert.critic_min;
          save_templimit_invidual();
          screen_dealloc_back();
        }

      } else if (stisknuto == BT_MENU) { // vratit se na vyber senzoru
        --scr2render->cislo;
      }
      break;

    case 2: // MAX limit
      if (stisknuto == BT_LEFT) { // dekrementace
        if (scr2render->property1 > SENSOR_MIN_TEMP && (scr2render->property1 - 1) > scr2render->property2) {
          --scr2render->property1;
        }

      } else if (stisknuto == BT_RIGHT) { // inkrementace
        if (scr2render->property1 < SENSOR_MAX_TEMP && (scr2render->property1 + 1) > scr2render->property2) {
          ++scr2render->property1;
        }

      } else if (stisknuto == BT_SELECT) { // potvrzeni MAX limitu
        ++scr2render->cislo;

      } else if (stisknuto == BT_MENU) { // vraceni se na predchozi polozku
        --scr2render->cislo;
      }
      break;

    case 3: // MIN limit
      if (stisknuto == BT_LEFT) { // dekrementace
        if (scr2render->property2 > SENSOR_MIN_TEMP && (scr2render->property2 - 1) < scr2render->property1) {
          --scr2render->property2;
        }

      } else if (stisknuto == BT_RIGHT) { // inkrementace
        if (scr2render->property2 < SENSOR_MAX_TEMP && (scr2render->property2 + 1) < scr2render->property1) {
          ++scr2render->property2;
        }

      } else if (stisknuto == BT_SELECT) { // potvrzeni MIN limitu + ulozeni
        alert.sens_lim[scr2render->offset] = true;
        alert.sens_max[scr2render->offset] = scr2render->property1;
        alert.sens_min[scr2render->offset] = scr2render->property2;
        save_templimit_invidual();
        screen_dealloc_back();

      } else if (stisknuto == BT_MENU) { // vraceni se na predchozi polozku
        --scr2render->cislo;
      }
      break;
  }
}

void control_teplotni_limity_glob(int stisknuto) {

  // NASTAVENI MAX LIMITU
  if (scr2render->cislo == 0) {
    // SNIZOVANI MAX LIMITU
    if (stisknuto == BT_LEFT) {
      if ((scr2render->property1 - 1) > scr2render->property2 ) {
        --scr2render->property1;
      }
    }

    // ZVYSOVANI MAX LIMITU
    else if (stisknuto == BT_RIGHT) {
      if (scr2render->property1 < SENSOR_MAX_TEMP) {
        ++scr2render->property1;
      }
    }

    // PREJIT NA NASTAVENI MIN LIMITU
    else if (stisknuto == BT_SELECT) {
      scr2render->cislo = 1;
    }

    // VRATIT SE DO MENU a neukladat nastavene teploty
    else if (stisknuto == BT_MENU) {
      screen_dealloc_back();
    }

    // NASTAVENI MIN LIMITU
  } else if (scr2render->cislo == 1) {
    // SNIZOVANI MIN LIMITU
    if (stisknuto == BT_LEFT) {
      if (scr2render->property2 > SENSOR_MIN_TEMP) {
        --scr2render->property2;
      }
    }
    // ZVYSOVANI MIN LIMITU
    else if (stisknuto == BT_RIGHT) {
      if ((scr2render->property2 + 1) < scr2render->property1) {
        ++scr2render->property2;
      }
    }
    // PREJIT K NASTAVENI HRANICE PRO NAVRAT
    else if (stisknuto == BT_SELECT) {
      ++scr2render->cislo;
    }
    // VRATIT SE ZPET K NASTAVENI MAX LIMITU
    else if (stisknuto == BT_MENU) {
      --scr2render->cislo;
    }

    // NASTAVENI HRANICE PRO NAVRAT
  } else if (scr2render->cislo == 2) {
    // DEKREMENTACE HRANICE
    if (stisknuto == BT_LEFT) {
      if (scr2render->prop[0] > 0) {
        scr2render->prop[0] -= 0.5;
      }

      // INKREMENTACE HRANICE
    } else if (stisknuto == BT_RIGHT) {
      if (scr2render->prop[0] < 20) {
        scr2render->prop[0] += 0.5;
      }

      // ULOZIT NASTAVENE LIMITY (MIN + MAX + BOUND)
    } else if (stisknuto == BT_SELECT) {
      if (scr2render->property2 < scr2render->property1) {
        save_templimit_global(scr2render->property2, scr2render->property1, scr2render->prop[0]);
        screen_dealloc_back();
      }

      // VRATIT SE ZPET K NASTAVENI MIN LIMITU
    } else if (stisknuto == BT_MENU) {
      --scr2render->cislo;
    }
  }
}


// zjisti, zda je stisknut trojklik pro tovarni nastaveni Arduina
bool is_reset_pressed() {
  if ((digitalRead(BT_AUTO) == LOW) && (digitalRead(BT_MENU) == LOW) && (digitalRead(BT_SELECT) == LOW)) {
    return true;
  } else {
    return false;
  }
}


/* Zpracovava vstup z klavesnice */
void button_controlling(void) {
  int stisknuto = 0;
  #if SL_CONTROL_ON
    // Ovladani displeje pres Seriovou linku
    char s;
    if (Serial.available()) {
      s = Serial.read();
      if (s == 'a') {
        stisknuto = BT_LEFT;
      }
      if (s == 'd') {
        stisknuto = BT_RIGHT;
      }
      if (s == 's') {
        stisknuto = BT_SELECT;
      }
      if (s == 'w') {
        stisknuto = BT_MENU;
      }
      if (s == 'q') {
        stisknuto = BT_AUTO;
      }
      if (s == 'e') {
        stisknuto = BT_ONOFF;
      } 
      if (s == '@') {
        factory_reset();
      }
    }
  #endif

  if (button_click(BT_AUTO)) {
    stisknuto = BT_AUTO;
  } else if (button_click(BT_MENU)) {
    stisknuto = BT_MENU;
  } else if (button_click(BT_LEFT)) {
    stisknuto = BT_LEFT;
  } else if (button_click(BT_RIGHT)) {
    stisknuto = BT_RIGHT;
  } else if (button_click(BT_SELECT)) {
    stisknuto = BT_SELECT;
  } else if (button_click(BT_ONOFF)) {
    stisknuto = BT_ONOFF;
  }

  // ZAPNOUT/VYPNOUT Podsviceni displeje
  if (stisknuto == BT_ONOFF) {
    static int podsviceni = 1;
    if (podsviceni) {
      analogWrite(LCD_BACKLIGHT, 0);
      podsviceni = 0;
    } else {
      analogWrite(LCD_BACKLIGHT, display_settings.jas);
      podsviceni = 1;
    }
  }

  // pokud uzivatel drzi AUTO, MENU i SELECT 2 sekundy soucasne, dojde k tovarnimu nastaveni
  if (is_reset_pressed()) {
    delay(50);
    if (is_reset_pressed()) {
      delay(1950);
      if (is_reset_pressed()) {
        factory_reset();
      }
    }
  }
  
  display.clearDisplay();

  // V zavislosti na typu obrazovky vyber funkci pro ovladani
  switch (scr2render->typ) {
    case SCR_MENU:
      control_menu(stisknuto);
      break;
    case SCR_TEMP:
      control_teplota(stisknuto);
      break;
    case SCR_LIMS:
      control_teplotni_limity(stisknuto);
      break;
    case SCR_LIMS_GLOB:
      control_teplotni_limity_glob(stisknuto);
      break;
    case SCR_LIMS_UNIQ:
      control_teplotni_limity_uniq(stisknuto);
      break;
    case SCR_LCDSET:
      control_set_lcd(stisknuto);
      break;
    case SCR_ETHSET:
      control_set_eth(stisknuto);
      break;
    case SCR_TIME:
      control_set_time(stisknuto);
      break;
    case SCR_TIME_MANUAL:
      control_set_time_manual(stisknuto);
      break;
    case SCR_EDIT_ADDRESS:
      control_edit_address(stisknuto);
      break;
    case SCR_FACTORY_RESET:
      control_factory_reset(stisknuto);
      break;
    case SCR_AIRCOND:
      control_aircond(stisknuto);
      break;
  }
  delay(20);
}


/* Provadi vykreslovani na LCD display */
void zpracovani_lcd(void) {
  static char count = 0;  //pocitadlo cisteni displeje
  if (count > 99) {
    display.clearDisplay();
    count = 0;
  }
  ++count;


  #if DEBUG_LCD
    Serial.println("<SRC SHOW>");
  #endif
  /* Vykresluje obrazovku */
  screen_show(scr2render);
  #if DEBUG_LCD
    Serial.println("</SRC SHOW>");
  #endif

  #if DEBUG_LCD
    Serial.println("<BTN CONTROLL>");
  #endif
  /* Zpracovava prikazy z klavesnice k ovladani obrazovky */
  button_controlling();
  #if DEBUG_LCD
    Serial.println("</BTN CONTROLL>");
  #endif
}
