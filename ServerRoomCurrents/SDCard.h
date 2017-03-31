#ifndef SD_CARD_H_
#define SD_CARD_H_

void sdcard_overwrite(String name, String text);
String sdcard_getfile(String name);
void fill_chararray_sdfile(char *buffer, int size, char *filename);
void sdcard_removefile(char *name);

#endif
