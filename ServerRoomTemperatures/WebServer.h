#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include "Types.h"


/* functions */
void html_redirect_home(EthernetClient *client);
int eth_client_setup(void);
char * html_usermenu(void);
void html_statistics(EthernetClient *client);
void html_monitoring_temperatures(EthernetClient *client);
void html_temp_limits(EthernetClient *client);
void html_tempTable(EthernetClient *client);
bool temp_limits_update(EthernetClient *client);
void client_processing(void);
void html_posledni_(EthernetClient *client);
bool nastaveni_casu_interval_update(EthernetClient *client);
#endif


