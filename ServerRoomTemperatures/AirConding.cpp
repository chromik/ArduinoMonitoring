#include "AirConding.h"
#include "Types.h"
#include <avr/pgmspace.h>

extern struct str_tepoloty mt;
extern struct str_alert alert;
extern struct str_time times;

struct air_conditioning comfee[2] = {{
  -1 // defaultne nastavena teplota pro klimu - vypnuto
}};

// frekvence pro IR vysilani
int khz = IR_SEND_FREQUENCY;

// IR KODY
const unsigned char sig_part1[20] =    {'B', 'B', 'D', 'C', 'D', 'D', 'D', 'C', 'D', 'C', 'D', 'D', 'D', 'D', 'D', 'C', 'D', 'D', 'D', 'D'};
const unsigned char sig_part2[15] =     {'D', 'C', 'D', 'D', 'D', 'D', 'D', 'C', 'D', 'C', 'D', 'D', 'D', 'C', 'D'};
const unsigned char sig_part2_t[5] =     {'C', 'D', 'D', 'D', 'C'};
const unsigned char sig_part2_off[5] =   {'D', 'D', 'C', 'D', 'C'};
const unsigned char sig_part3_begin[5] = {'D', 'C', 'D', 'C', 'D'};
const unsigned char sig_part3_t[9] = {'C', 'D', 'C', 'D', 'C', 'D', 'D', 'D', 'C'};
const unsigned char sig_part3_off[9] = {'D', 'D', 'C', 'D', 'C', 'D', 'C', 'D', 'D'};
const unsigned char sig_part3_end[6] = {'D', 'D', 'D', 'D', 'D', 'D'};
const unsigned char sig_part4_begin[7] = {'D', 'D', 'D', 'D', 'D', 'D', 'D'};
const unsigned char sig_part4_off[14] = {'D', 'C', 'D', 'D', 'D', 'D', 'D', 'C', 'D', 'C', 'D', 'C', 'D', 'D'};
/* Cast s nastavovanou teplotou  */
const unsigned char sig_part4_t[14][7] = {
  {'D', 'D', 'D', 'D', 'D', 'D', 'D'}, // 17 °C
  {'D', 'D', 'D', 'D', 'D', 'D', 'C'}, // 18 °C
  {'D', 'D', 'D', 'D', 'C', 'D', 'C'}, // 19 °C
  {'D', 'D', 'D', 'D', 'C', 'D', 'D'}, // 20 °C
  {'D', 'D', 'C', 'D', 'C', 'D', 'D'}, // 21 °C
  {'D', 'D', 'C', 'D', 'C', 'D', 'C'}, // 22 °C
  {'D', 'D', 'C', 'D', 'D', 'D', 'C'}, // 23 °C
  {'D', 'D', 'C', 'D', 'D', 'D', 'D'}, // 24 °C
  {'C', 'D', 'C', 'D', 'D', 'D', 'D'}, // 25 °C
  {'C', 'D', 'C', 'D', 'D', 'D', 'C'}, // 26 °C
  {'C', 'D', 'D', 'D', 'D', 'D', 'C'}, // 27 °C
  {'C', 'D', 'D', 'D', 'D', 'D', 'D'}, // 28 °C
  {'C', 'D', 'D', 'D', 'C', 'D', 'D'}, // 29 °C
  {'C', 'D', 'D', 'D', 'C', 'D', 'C'}, // 30 °C
};
const unsigned char sig_part4_end[6] = {'D', 'D', 'D', 'D', 'D', 'D'};
const unsigned char sig_part5_begin[3] = {'D', 'D', 'D'};
/* Cast s nastavovvanou teplotou 2 */
const unsigned char sig_part5_t[14][7] = {
  {'C', 'D', 'C', 'D', 'C', 'D', 'C'}, // 17 °C
  {'C', 'D', 'C', 'D', 'C', 'D', 'D'}, // 18 °C
  {'C', 'D', 'C', 'D', 'D', 'D', 'D'}, // 19 °C
  {'C', 'D', 'C', 'D', 'D', 'D', 'C'}, // 20 °C
  {'C', 'D', 'D', 'D', 'D', 'D', 'C'}, // 21 °C
  {'C', 'D', 'D', 'D', 'D', 'D', 'D'}, // 22 °C
  {'C', 'D', 'D', 'D', 'C', 'D', 'D'}, // 23 °C
  {'C', 'D', 'D', 'D', 'C', 'D', 'C'}, // 24 °C
  {'D', 'D', 'D', 'D', 'C', 'D', 'C'}, // 25 °C
  {'D', 'D', 'D', 'D', 'C', 'D', 'D'}, // 26 °C
  {'D', 'D', 'C', 'D', 'C', 'D', 'D'}, // 27 °C
  {'D', 'D', 'C', 'D', 'C', 'D', 'C'}, // 28 °C
  {'D', 'D', 'C', 'D', 'D', 'D', 'C'}, // 29 °C
  {'D', 'D', 'C', 'D', 'D', 'D', 'D'}, // 30 °C
};
const unsigned char sig_part5_off[7] = {'D', 'D', 'D', 'D', 'D', 'D', 'C'};
const unsigned char sig_part5_end[9] = {'D', 'C', 'D', 'C', 'D', 'C', 'D', 'C', 'D'};
const unsigned char sig_delim = 'A';

unsigned int signall_trans(unsigned char c) {
  switch (c) {
    case 'A':
      return 5300; // delka signalu v mikroSekundach
    case 'B':
      return 4400; // delka signalu v mikroSekundach
    case 'C':
      return 1660; // delka signalu v mikroSekundach
    case 'D':
      return 550; // delka signalu v mikroSekundach
    default:
      return 0;
  }
}

/**
 * Delay v mikrosekundach
 * @param uSecs delka v mikrosekundach
 */
void ac_custom_delay_usec(unsigned long uSecs) {
  if (uSecs > 4) {
    unsigned long start = micros();
    unsigned long endMicros = start + uSecs - 4;
    if (endMicros < start) { // pokud jeste nedoslo k preteceni
      while ( micros() > start ) {} // cekat, az k nemu dojde
    }
    while ( micros() < endMicros ) {} // normalni delay
  }
}

/**
 * Nastavi PWM pro IR sender A
 * @param khz Frekcence v kHz
 */
void acA_enableIROut(int khz) {
  A_TIMER_DISABLE_INTR;
  pinMode(A_TIMER_PWM_PIN, OUTPUT);
  digitalWrite(A_TIMER_PWM_PIN, LOW);
  A_TIMER_CONFIG_KHZ(khz);
}

/**
 * Nastavi PWM  pro IR sender B
 * @param khz Frekcence v kHz
 */
void acB_enableIROut(int khz) {
  B_TIMER_DISABLE_INTR;
  pinMode(B_TIMER_PWM_PIN, OUTPUT);
  digitalWrite(B_TIMER_PWM_PIN, LOW);
  B_TIMER_CONFIG_KHZ(khz);
}

/** Nastaví PWM pro vybraný IR sender
 *  @param id Cislo IR diody (0 = IR sender A, 1 = IR sender B)
 *  @param khz Frekvence v kHz
 */
void ac_enableIROut(int id, int khz) {
  switch(id) {
    case 0:
      acA_enableIROut(khz);
      break;
    case 1:
      acB_enableIROut(khz);
      break;
  }
}

/** Vyslat sensorem logickou "1"
 *  @param id Cislo sensoru
 *  @param time Doba vysilani v mikrosekundach
 */
void ac_mark(int id, unsigned int time) {
  if (id == 0) {
    A_TIMER_ENABLE_PWM;
  } else {
    B_TIMER_ENABLE_PWM;
  }
  if (time > 0) ac_custom_delay_usec(time);
}

/** Vyslat sensorem logickou "0"
 *  @param id Cislo sensoru
 *  @param time Doba vysilani v mikrosekundach
 */
void ac_space(int id, unsigned int time) {  
  if (id == 0) {
    A_TIMER_DISABLE_PWM;
  } else {
    B_TIMER_DISABLE_PWM;
  }
  if (time > 0) ac_custom_delay_usec(time);
}

/** Vyslat sensorem RAW hodnoty
 *  @param id Cislo sensoru
 *  @param buf Pole s RAW hodnotami (vysilacimi kody)
 *  @param len Delka pole s RAW hodnotami
 *  @param khz Frekvence v kHz
 */
void air_cond_sendRaw(int id, const unsigned char buf[], unsigned int len, unsigned int hz) {
  ac_enableIROut(id, hz);
  for (unsigned int i = 0; i < len; ++i) {
    unsigned int value = signall_trans(buf[i]); // ziskej delku SPACE=0/MARK=1 signalu (v mikrosekundach)
    
    if (i & 1) ac_space(id, value); // licha iterace (i==1, i==3, i==5, i==7, ...) cyklu => SPACE (logicka 0) po dobu "value" [us]
    else ac_mark(id, value); // suda iterace (i==0, i==2, i==4, i==6) cyklu => MARK (logicka 1) po dobu "value" [us]
  }
  ac_space(id, 0); // SPACE=0 po konci kodu
}
/*
void air_cond_sendRaw(int id, const unsigned long buf[], unsigned int len, unsigned int hz) {
  ac_enableIROut(id, hz);
  for (unsigned int i = 0; i < len; ++i) {
    if (i & 1) ac_space(id, buf[i]);
    else ac_mark(id, buf[i]);
  }
  ac_space(id, 0);
}
*/


/** Vypnout klimatizaci
 *  @param id Cislo klimatizace (0 = klima A, 1 = klima B)
 */
void air_cond_turn_off(unsigned char id) {
  unsigned char Signall[IR_SIGNAL_LENGTH];
  int index = 0;
  
  // skladani signalu pro vypnuti klimatizace (2x, podruhe pro paritni cast)
  for (int i = 0; i < 2; ++i) { 
    if (i == 1) {
      Signall[index++] = sig_delim;
    }
    for (int j = 0; j < 20; ++j) {
      Signall[index++] = sig_part1[j];
    }
    for (int j = 0; j < 15; ++j) {
      Signall[index++] = sig_part2[j];
    }
    for (int j = 0; j < 5; ++j) {
      Signall[index++] = sig_part2_off[j];
    }
    for (int j = 0; j < 5; ++j) {
      Signall[index++] = sig_part3_begin[j];
    }
    for (int j = 0; j < 9; ++j) {
      Signall[index++] = sig_part3_off[j];
    }
    for (int j = 0; j < 6; ++j) {
      Signall[index++] = sig_part3_end[j];
    }    
    for (int j = 0; j < 14; ++j) {
      Signall[index++] = sig_part4_off[j];
    }
    for (int j = 0; j < 6; ++j) {
      Signall[index++] = sig_part4_end[j];
    }    
    for (int j = 0; j < 3; ++j) {
      Signall[index++] = sig_part5_begin[j];
    }
    for (int j = 0; j < 7; ++j) {
      Signall[index++] = sig_part5_off[j];
    }
    for (int j = 0; j < 9; ++j) {
      Signall[index++] = sig_part5_end[j];
    }
  }

  // odeslani slozeneho signalu
  air_cond_sendRaw(id, Signall, IR_SIGNAL_LENGTH, khz);
  
  // nastavit klimu jako vypnutou
  comfee[id].temp = 0;
}

/**
 * Vytvori signal pro nastaveni teploty na klimatizaci
 * @param Signall Ukazatel na pole, do kterého bude sestavený signál uložen
 * @param temp Teplota, pro kterou se vytváří signál
 */
void create_Signall(unsigned char *Signall, char temp) {
  int index = 0;
  // skladani signalu pro nastaveni teploty (2x, podruhe pro paritni cast)
  for (int i = 0; i < 2; ++i) { 
    if (i == 1) {
      Signall[index++] = sig_delim;
    }
    for (int j = 0; j < 20; ++j) {
      Signall[index++] = sig_part1[j];
    }
    for (int j = 0; j < 15; ++j) {
      Signall[index++] = sig_part2[j];
    }
    for (int j = 0; j < 5; ++j) {
      Signall[index++] = sig_part2_t[j];
    }
    for (int j = 0; j < 5; ++j) {
      Signall[index++] = sig_part3_begin[j];
    }
    for (int j = 0; j < 9; ++j) {
      Signall[index++] = sig_part3_t[j];
    }
    for (int j = 0; j < 6; ++j) {
      Signall[index++] = sig_part3_end[j];
    }
    for (int j = 0; j < 7; ++j) {
      Signall[index++] = sig_part4_begin[j];
    }
    for (int j = 0; j < 7; ++j) {
      Signall[index++] = sig_part4_t[temp - AIRCOND_MIN_TEMP][j];
    }
    for (int j = 0; j < 6; ++j) {
      Signall[index++] = sig_part4_end[j];
    }
    for (int j = 0; j < 3; ++j) {
      Signall[index++] = sig_part5_begin[j];
    }
    for (int j = 0; j < 7; ++j) {
      Signall[index++] = sig_part5_t[temp - AIRCOND_MIN_TEMP][j];
    }
    for (int j = 0; j < 9; ++j) {
      Signall[index++] = sig_part5_end[j];
    }
  }
}

/**
 * Zjisti, zda klimatizace bezi
 * @param id cislo klimatizace
 * @return true - klimatizace zapnuta, false - klimatizace je vypnuta
 */
bool is_aircond_on(unsigned char id) {
  switch (id) {
    case 0: // prvni klima
      if (digitalRead(ECHO_PIN_COND0) == LOW) {
        return true;
      }
      return false;
    case 1: // druha klima
      if (digitalRead(ECHO_PIN_COND1) == LOW) {
        return true;
      }
      return false;
  }
}
/**
 * Nastavi teplotu klimatizaci
 * @param id cislo klimatizace
 * @param temp teplota, kterou chci nastavit (od 17 °C do 30 °C, 0 pro vypnuti klimatizace)
 */
void air_cond_set_temperature(unsigned char id,char temp) {
  unsigned char Signall[IR_SIGNAL_LENGTH];
  
  if (temp >= AIRCOND_MIN_TEMP && temp <= AIRCOND_MAX_TEMP) {
    create_Signall(Signall, temp);
    air_cond_sendRaw(id, Signall, IR_SIGNAL_LENGTH, khz);
    comfee[id].temp = temp;
    
  } else if (temp == 0) {
    air_cond_turn_off(id);
  }
}


/**
 * Zjisti teplotu, na kterou je klimatizace nastavena
 * @param id cislo klimatizace
 * @return teplota, ktera byla nastavena (17 az 30 - teplota v °C, 0 - klimatizace je vypnuta)
 */
char air_cond_get_temperature(unsigned char id) {
  if (is_aircond_on(id)) { // pokud je klima zapnuta (ECHO PIN dava zpetnou odezvu)
    if (comfee[id].temp > 0) { // a mam ulozenou naposledy nastavenou validni hodnotu
      return comfee[id].temp; // vrat posledne nastavenou hodnotu
    } else {
      return -1; // klima je zapnuta, ale nevim na co muze byt nastavena (nenastavovala se pres webove rozhrani)
    }
  } else {
    return 0; // klima je vypnuta
  }
}

