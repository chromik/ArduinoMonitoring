#ifndef GLOBAL_FUNCTIONS_H_
#define GLOBAL_FUNCTIONS_H_
#include <Arduino.h>
#include <Ethernet.h>

float StringToFloat(String s);
void floatToString(char *buffer, float value);
int freeRam();
int ramSize();
char * getMacAdress();
char * getIpAdress();
char * getFormatedTime(unsigned long t, bool shorted = false);
unsigned long get_timediff(unsigned long t1, unsigned long t2);
unsigned long get_doba_prekroceni();
unsigned long get_doba_normal();
void client_send_html(EthernetClient *client, const char * file, int c, ...);
void log_vypadky_ulozVypadek(int sensor, unsigned long from, unsigned long to);
void log_ukaz_vypadky(EthernetClient *client);
void log_vypadky_clear();
String getSenzorLabel(int id);
void coding_fix(String *s);
char * showIpAdress(unsigned char *ip);
void software_Reset();
void turnLED(bool on);
unsigned long tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss);

#endif

