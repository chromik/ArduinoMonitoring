#ifndef MAIL_SEND_H_
#define MAIL_SEND_H_

int sendmail_via_smtp(String to, String subject, String text);
void floatToString(char *buffer, float value);
void efail(void);
unsigned char eRcv(void);

#endif
