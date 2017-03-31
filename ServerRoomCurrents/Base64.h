#ifndef _BASE64_H
#define _BASE64_H
extern const char b64_alphabet[];
inline void a3_to_a4(unsigned char *a4, unsigned char *a3);
inline void a4_to_a3(unsigned char *a3, unsigned char *a4);
inline unsigned char b64_lookup(char c);
int base64_encode(char *output, char *input, int inputLen);
int base64_decode(char *output, char *input, int inputLen);

#endif // _BASE64_H
