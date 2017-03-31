#include "Types.h"
#include "MailSend.h"
#include "WebServer.h"
#include "GlobalFunctions.h"
#include <Ethernet.h>
#if LCD_LIBRARY
  #include <Adafruit_GFX.h>
  #include <Adafruit_PCD8544.h>
#else 
  #include "TangerLCD.h"
#endif

extern EthernetClient client;

extern struct str_email email;
extern struct str_eth ethernet;
extern struct str_tepoloty mt;
extern struct str_alert alert;

#if LCD_LIBRARY
  extern Adafruit_PCD8544 display;
#else
  extern TangerLCD display;
#endif

/**
 * Posle e-mail pres smtp server
 * @param to Adresa prijemce
 * @param subject Predmet e-mailu
 * @param text Text e-mailu
 */
int sendmail_via_smtp(String to, String subject, String text) {
  #if SENDMAILON != 1
    return 1;
  #endif
  
  if (to == "") { // prijemnce neni nastaveny
    #if BAS_DEBUG
      Serial.println(F("Mail not configured"));
    #endif
    return 0;
  }
  
  #if BAS_DEBUG
    Serial.println(F("Sending mail"));
  #endif

  #if BAS_DEBUG
    Serial.print("server: ");
    Serial.print(email.smtp_server);
    Serial.print(", port: ");
    Serial.println(email.smtp_port);
  #endif
  if (ethernet.client->connect(email.smtp_server, email.smtp_port) < 0) {
    
    display.println("connect fail");
    display.display();
    #if BAS_DEBUG
      Serial.println(F("Failed - conn problem"));
    #endif
    return 0;
  }

  if(!eRcv()) return 0;

  ethernet.client->print(F("helo "));
  ethernet.client->println(ethernet.public_ip); // HELO po navazani spojeni s SMTP (public IP je pevne definovana v ServerRoomCurrents.ino; pokud server neoveruje IP, muze byt libovolny text)
  if(!eRcv()) return 0; // helo 94.113.228.184

  if (email.auth) { // JE ZAPNUTA AUTORIZACE?
    ethernet.client->println(F("auth login")); // AUTH LOGIN
    #if DEBUG_ON
      Serial.println("Client: auth login");
    #endif
    if(!eRcv()) return 0; // auth login
    
    ethernet.client->println(email.auth_login); // LOGIN V BASE64
    #if DEBUG_ON
      Serial.print("Client: ");
      Serial.println(email.auth_login);
    #endif
    if(!eRcv()) return 0; // Z3JhaGcxMkBzZXpuYW0uY3o= 
    
    ethernet.client->println(email.auth_pass); // HESLO V BASE64
    #if DEBUG_ON
      Serial.print("Client: ");
      Serial.println(email.auth_pass);
    #endif
    
    if(!eRcv()) return 0; // MDIwMTE5OTI=
  }

  // MAIL FROM
  ethernet.client->print("MAIL From: <");
  ethernet.client->print(email.from);
  ethernet.client->println(">");
  if(!eRcv()) return 0;
  // MAIL From: <xxx@xxx.xx>

  // RCPT TO 
  ethernet.client->print("RCPT To: <");
  ethernet.client->print(to);
  ethernet.client->println(">");
  if(!eRcv()) return 0;
  // RCPT To: <xxx@xxx.xx>

  /// DATA
  ethernet.client->println("DATA");
  if(!eRcv()) return 0;
  // DATA

  #if DEBUG_ON
    Serial.println(F("Sending email"));
  #endif

  // Predmet emailu
  ethernet.client->print("Subject: ");
  ethernet.client->println(subject);

  // Prijemce emailu
  ethernet.client->print("To: You <");
  ethernet.client->print(to);
  ethernet.client->println(">");
  // To: You <xxx@xxx.xx>
  
  // Odesilatel emailu
  ethernet.client->print("From: TANGER Arduino <");
  ethernet.client->print(email.from);
  ethernet.client->println(">");
  // From: Me <xxx@xxx.xx>

  // Kodovani emailu
  ethernet.client->println("Content-Type: text/html; charset=\"UTF-8\"");
  //ethernet.client->println("MIME-Version: 1.0");

  // Odradkovani
  ethernet.client->println("\r\n");

  // Samotny text emailu
  ethernet.client->println(text);
  
  // Tecka na novy radek pro potvrzeni a odeslani emailu
  ethernet.client->println(".");
  if(!eRcv()) return 0;

  #if DEBUG_ON
    Serial.println(F("QUIT"));
  #endif

  // Ukonceni spojeni
  ethernet.client->println("QUIT");
  if(!eRcv()) return 0;

  // Ukoncit fyzicky spojeni
  ethernet.client->stop();
  
  #if BAS_DEBUG
    Serial.println(F("Mail sended."));
  #endif
  
  return 1;
}

/* Ukoncit spojeni s mailserverem */
void efail(void)
{
  unsigned char thisByte = 0;
  int loopCount = 0;

  ethernet.client->println(F("QUIT"));

  while(!ethernet.client->available()) { // Dokud neni klient pripojen (neni odpoved od SMTP serveru)
    delay(1); // cekej milisekundu
    loopCount++; // pocitadlo milisekund zvetsi

    // po dane dobe ukoncit cekani na odpoved -> TIMEOUT
    if(loopCount > MAIL_WAIT_FOR_RESPONSE_MS_DELAY) {
      ethernet.client->stop();
      display.println("timeout");
      display.display();
      #if DEBUG_ON
        Serial.println(F("\r\nFailed - Timeout"));
      #endif
      
      return;
    }
  }
  #if DEBUG_ON
    Serial.print(F("Server:"));
  #endif
  while(ethernet.client->available())  // ctu klientovou (chybovou) odoved, dokud neco odesila
  {  
    thisByte = ethernet.client->read(); // ziskat znak od klienta    
    #if DEBUG_ON
      Serial.write(thisByte);
    #endif
  }

  ethernet.client->stop(); // ukonceni spojeni
  #if BAS_DEBUG
    Serial.println(F("\nDisconnected"));
  #endif
}

/* Snazi se prijmout data od serveru */
unsigned char eRcv(void)
{
  unsigned char respCode;
  unsigned char thisByte;
  int loopCount = 0;

  while(!ethernet.client->available()) {
    delay(1);
    loopCount++;

    // po dane dobe ukoncit cekani na odpoved -> TIMEOUT
    if(loopCount > MAIL_WAIT_FOR_RESPONSE_MS_DELAY) {
      ethernet.client->stop();
      
      display.println("timeout");
      display.display();
      #if BAS_DEBUG
        Serial.println(F("\r\nFailed - Timeout"));
      #endif
      return 0;
    }
  }

  respCode = ethernet.client->peek(); // ziskat znak od klienta (odpoved)

  while(ethernet.client->available()) // ctu klientovou odpoved, dokud neco odesila
  {  
    thisByte = ethernet.client->read(); // ziskat znak od klienta
    #if DEBUG_ON
      Serial.write(thisByte);
    #endif
  }

  if(respCode >= '4') // pokud odpoved byla cislo vetsi nebo rovno 4 -> FAIL
  {
    efail(); // ziskat chybovou hlasku od serveru
    return 0;  // NEUSPESNE ODESLANI MAILU
  }

  return 1; // MAIL USPESNE ODESLAN
}

