#ifndef UDP_NTP_CLIENT_H
#define UDP_NTP_CLIENT_H


void sendNTPpacket(char* address);
bool jeLetniCas(unsigned long unixTime, struct tm *cas = NULL);
void time_sync_internNTP(unsigned long epoch);
unsigned long time_syncNTP();
unsigned long getTime();
char *prevedCas(unsigned long t, bool nl = false);
void rtc_setup();
void time_sync_intern(unsigned long epoch);
float getRTCTemp();


#endif


