#include "Base64.h"
#include <avr/pgmspace.h>
const char PROGMEM b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
                                    "abcdefghijklmnopqrstuvwxyz" \
                                    "0123456789+/";
/** Prevod textu do Base64
 *  @param output Vystup v BASE64
 *  @param input Textovy vstup
 *  @param inputLen Delka textoveho vstupu
 */
int base64_encode(char *output, char *input, int inputLen) {
  int i = 0, j = 0;
  int encLen = 0;
  unsigned char a3[3];
  unsigned char a4[4];
  
  while (inputLen--) {
    a3[i++] = *(input++);
    if (i == 3) {
      a3_to_a4(a4, a3);
      for (i = 0; i < 4; i++) {
        output[encLen++] = pgm_read_byte(&b64_alphabet[a4[i]]);
      }
      i = 0;
    }
  }
  if (i) {
    for (j = i; j < 3; ++j) {
      a3[j] = '\0';
    }
    a3_to_a4(a4, a3);
    for (j = 0; j < i + 1; ++j) {
      output[encLen++] = pgm_read_byte(&b64_alphabet[a4[j]]);
    }
    while ((i++ < 3)) {
      output[encLen++] = '=';
    }
  }
  output[encLen] = '\0';
  return encLen;
}

/** Prevod BASE64 na text
 *  @param output Textovy vystup
 *  @param input BASE64 Vstup
 *  @param inputLen Delka BASE64 vstupu
 */
int base64_decode(char * output, char * input, int inputLen) {
  int i = 0, j = 0;
  int decLen = 0;
  unsigned char a3[3];
  unsigned char a4[4];
  
  while (inputLen--) {
    if (*input == '=') {
      break;
    }
    a4[i++] = *(input++);
    if (i == 4) {
      for (i = 0; i < 4; ++i) {
        a4[i] = b64_lookup(a4[i]);
      }
      a4_to_a3(a3, a4);
      for (i = 0; i < 3; ++i) {
        output[decLen] = a3[i];
        ++decLen;
      }
      i = 0;
    }
  }
  if (i) {
    for (j = i; j < 4; ++j) {
      a4[j] = '\0';
    }
    for (j = 0; j < 4; ++j) {
      a4[j] = b64_lookup(a4[j]);
    }
    a4_to_a3(a3, a4);
    for (j = 0; j < i - 1; ++j) {
      output[decLen] = a3[j];
      ++decLen;
    }
  }
  output[decLen] = '\0';
  return decLen;
}

int base64_encode_length(int plainLen) {
  int n = plainLen;
  return (n + 2 - ((n + 2) % 3)) / 3 * 4;
}

int base64_decode_length(char * input, int inputLen) {
  int i = 0;
  int numEq = 0;
  for (i = inputLen - 1; input[i] == '='; i--) {
    ++numEq;
  }
  return ((6 * inputLen) / 8) - numEq;
}

inline void a3_to_a4(unsigned char * a4, unsigned char * a3) {
  a4[0] = (a3[0] & 0xfc) >> 2;
  a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
  a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
  a4[3] = (a3[2] & 0x3f);
}

inline void a4_to_a3(unsigned char * a3, unsigned char * a4) {
  a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
  a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
  a3[2] = ((a4[2] & 0x3) << 6) + a4[3];
}

inline unsigned char b64_lookup(char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A';
  else if (c >= 'a' && c <= 'z') return c - 71;
  else if (c >= '0' && c <= '9') return c + 4;
  else if (c == '+') return 62;
  else if (c == '/') return 63;
  return -1;
}
