#include <SdFat.h>
#include "Types.h"
#include "SDCard.h"
#include "Base64.h"

SdFat SD;
extern struct str_email email;

// prepise soubor na pametove karte
void sdcard_overwrite(String name, String text) {
  digitalWrite(SDCARD, LOW); // povolit kartu
  SD.begin(4);

  char filename[20];
  name.toCharArray(filename, 20);
  
  SD.remove(filename);
  File file = SD.open(filename, FILE_WRITE);
  file.println(text);
  file.close();
  digitalWrite(SDCARD, HIGH);
}

// odstrani soubor z pametove karty
void sdcard_removefile(char *name) {
  digitalWrite(SDCARD, LOW);
  SD.begin(4);
  SD.remove(name);
  digitalWrite(SDCARD, HIGH);
}

// ziska obsah souboru na pametove karte
String sdcard_getfile(char *name) {
  digitalWrite(SDCARD, LOW); // povolit kartu
  SD.begin(4);
  String text = "";
  File file = SD.open(name);
  if (file) {
    while (file.available()) {
      char c = file.read();
      text += c;
    }
  }
  file.close();
  digitalWrite(SDCARD, HIGH);
  delay(1);
  return text;
}

// naplni buffer obsahem souboru z pametove karty
void fill_chararray_sdfile(char *buffer, int size, char *filename) {
  String text = sdcard_getfile(filename);
  text.toCharArray(buffer, size);
}

