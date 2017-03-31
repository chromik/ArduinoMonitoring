#include "Types.h"
#include <SPI.h>
#include <Ethernet.h>
#include <time.h>
#include <Wire.h>
#include <Bridge.h>
#include <utility/w5100.h>
#include "Sodaq_DS3231.h"
#include "Time.h"
#include "Storage.h"

extern struct str_ntp ntp;
extern struct str_eth ethernet;
extern struct str_time times;
extern struct str_alert alert;

/* Vrati aktualni teplotu RTC clanku */
float getRTCTemp() {
  #if DEBUG_WITHOUT_RTC
    return 0;
  #endif
  #if DEBUG_WITHOUT_RTC_TEMP
    return -1;
  #endif
  rtc.convertTemperature(); //convert current current into registers
  return rtc.getTemperature();
}

// mesic v roce
const char monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// zjisti, co je za den v tydnu
int dayofweek(int d, int m, int y)
{
  const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
  y -= m < 3;
  return ( y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}


const int NTP_PACKET_SIZE = 48; // NTP time stamp se nachazi v prvnich 48 bytech zpravy

byte packetBuffer[NTP_PACKET_SIZE]; //buffer pro prijimane a odesilane UDP pakety

// UDP socket, ktery slouzi k prijmani a odesilani paketu pres UDP
EthernetUDP Udp;



// send an NTP request to the time server at the given address
void sendNTPpacket(char* address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, ntp.ntp_port); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}



/* Funkce ktera zjisti, zda je letni cas v CR */
bool jeLetniCas(unsigned long unixTime, struct tm *cas) {
  // epochy pro jednotliva data
  unsigned long dstNow, dstStart , dstEnd; //data zacatku a konce letniho casu pro tento rok
  struct tm * te_now;

  // pro tm strukturu je epoch pocet sekund od roku 2000, takze prictu 30 let;
  unixTime += THIRTY_YERS_IN_SECONDS;
  if (cas == NULL) {
    // ulozim si aktualni cas (epoch) do struktury
    te_now = gmtime(&unixTime);
  } else {
    te_now = cas;
  }

  // zacatek letniho casu (posledni nedele v breznu, po 1:59:59 nasleduje 3:00:00)
  struct tm te_start = *te_now; // nastavim si pomocne datum na aktualni
  te_start.tm_mon = 2; // brezen (indexace od 0)
  te_start.tm_mday = 1; // 1.
  te_start.tm_hour = 1; // 1h GTM (= 2 hod GTM+1)
  te_start.tm_min = 0; // 0 min
  te_start.tm_sec = 0; // 0 sec


  int dayname = dayofweek(te_start.tm_year, te_start.tm_mday, te_start.tm_mon); // aktualni den v tydnu 0 Ned, 1 Pon, 2 Ute, 3 Str, 4 Ctv, 5 Pat, 6 Sob
  te_start.tm_mday += 21 - dayname; // prictu k prvnimu dni v mesici 3 tydny a pokud je to nejaky den, co nasleduje az po nedeli, tak se vratim zpet k nejblizsi nedeli


  // konec letniho casu (posledni nedele v rijnu, po 2:59:59 nasleduje 2:00:00)
  struct tm te_end = *te_now; // nastavim si pomocne datum na aktualni
  te_end.tm_mon = 9; //rijen (indexace od 0)
  te_end.tm_mday = 1; // 1.
  te_end.tm_hour = 1; // 1h GTM = 2 hod GTM+1 -> (3h rano v letnim case u nas)
  te_end.tm_min = 0;
  te_end.tm_sec = 0;

  /* prvni nedele */
  dayname = dayofweek(te_start.tm_year, te_end.tm_mday, te_end.tm_mon); // aktualni den v tydnu 0 Ned, 1 Pon, 2 Ute, 3 Str, 4 Ctv, 5 Pat, 6 Sob
  te_end.tm_mday += 21 - dayname; // prictu k prvnimu dni v mesici 3 tydny a pokud je to nejaky den, co nasleduje az po nedeli, tak se vratim zpet k nejblizsi nedeli

  // prevedu struktury na epoch
  dstNow = mktime(te_now);
  dstStart = mktime(&te_start);
  dstEnd = mktime(&te_end);

  // zjistim, zda je aktualni datum v letnim case
  if (dstNow >= dstStart && dstNow < dstEnd) {
    return true;
  } else {
    return false;
  }
}




/* Prevede epoch na formatovany cas pro CR (vcetne nastaveni na letni/zimni cas */
char* prevedCas(unsigned long t, bool nl) {
  // Pokud neni ziskat cas, prevod neprovadim
  if (!t) {
    return NULL;
  }

  if (jeLetniCas(t, NULL)) { // pokud je letni cas
    t += 3600; // pridam jeste jednu hodinu navic
  }
  t += 3600; // pridam jednu hodinu navic kvuli nasemu pasmu +1 GTM

  times.second = t % 60;
  t /= 60;
  times.minute = t % 60;
  t /= 60;
  times.hour = t % 24;
  t /= 24;

  int year = 0;
  int days = 0;

  // spocita se, kolik rok roku ubehlo od 1970
  while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= t)
    ++year;

  // pripoctu ubehle roky k 1970 a mam aktualni rok
  times.year = 1970 + year;


  days -= LEAP_YEAR(year) ? 366 : 365;
  t -= days; // now it is days in this year, starting at 0

  days = 0;
  int month = 0;
  char monthLength = 0;
  for (month = 0; month < 12; ++month) {
    if (month == 1) // february
      monthLength = LEAP_YEAR(year) ? 29 : 28;
    else
      monthLength = monthDays[month];

    if (t >= monthLength)
      t -= monthLength;
    else
      break;
  }
  times.month = month + 1;  // jan is month 1
  times.day = t + 1;     // day of month
  if (nl) {
    sprintf(times.formated_now, "%02d:%02d:%02d\n%02d.%02d.%04d", times.hour, times.minute, times.second, times.day, times.month, times.year);
  } else {
    sprintf(times.formated_now, "%02d.%02d.%04d %02d:%02d:%02d", times.day, times.month, times.year, times.hour, times.minute, times.second);
  }
  return times.formated_now;

}

/* Provede update casovych promennych, aby sedeli k nove nastavenemu epochu a na tento epoch nastavi i RTC clanek */
void time_sync_intern(unsigned long epoch) {
  unsigned long old_epoch = getTime();
  rtc.setEpoch(epoch);
  
  times.boot_time += (epoch - old_epoch);
  times.global_warn_start += (epoch - old_epoch);
  times.global_warn_end += (epoch - old_epoch);
  save_last_crit();
  
  for (int i = 0; i < SENSOR_COUNT; ++i) {
    times.last_crit_start[i] += (epoch - old_epoch);
    times.last_crit_end[i] += (epoch - old_epoch);
    save_last_crit(i);
  }
  
  times.last_sync = epoch;
  times.synced = true;
}

/* Ziska aktualni cas z NTP serveru a synchronizuje s nim RTC clanek */
unsigned long time_syncNTP() {
  W5100.setRetransmissionTime(ETH_CONN_NTP_TIMEOUT);
  W5100.setRetransmissionCount(ETH_CONN_NTP_RETRY_COUNTS);
  
  Udp.begin(ntp.localPort);
  unsigned long epoch = 0;
  
  #if BAS_DEBUG
    Serial.println("Time syncing...'");
  #endif 
  
  sendNTPpacket(ntp.ntp_server); // send an NTP packet to a time server
  
  // wait to see if a reply is available
  delay(1000);
  if (Udp.parsePacket()) {
    // Pokud prijde UDP paket, ctu z nej data do bufferu
    Udp.read(packetBuffer, NTP_PACKET_SIZE);

    // casova hodnota zacina na 40tem bytu a je to 32 bitove cislo, takze musim spojit 4x8bitu

    // ze 4 bytu co jsem prijal od NTP serveru, poskladam 32 bitovou unsigned long hodnotu, ktera udava pocet sekund od roku 1900
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    
    epoch = secsSince1900 - SEVENTY_YEAR_IN_SECONDS; // pocet sekund od 1970
    if (epoch) {
      time_sync_intern(epoch);
      #if BAS_DEBUG
        Serial.print("Time sync success. Epoch=");
        Serial.println(epoch);
      #endif 
      return epoch;
    } else { 
      #if BAS_DEBUG
        Serial.println("Time sync fail! Epoch=0");
      #endif
      return 0;
    }
  } else {
    #if BAS_DEBUG
      Serial.println("Time sync fail!. Received packed size=0");
    #endif
    return 0;
  }
  

  /* vratim timeout zpet */
  W5100.setRetransmissionTime(ETH_CONN_NORMAL_TIMEOUT);
  W5100.setRetransmissionCount(ETH_CONN_NORMAL_RETRY_COUNTS);
  return epoch;
}

void rtc_setup() {
  rtc.begin();
}


// Ziskat aktualni cas (jako epoch)
unsigned long getTime() {
  #if DEBUG_WITHOUT_RTC
    static unsigned long cas = 0;
    return cas++;
  #endif
  unsigned long epoch = rtc.now().getEpoch();
  return epoch; // vratim aktualni cas
}

