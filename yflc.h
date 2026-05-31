#ifndef YFLC_H
#define YFLC_H

#ifdef __cplusplus
extern "C" {
#endif

char* yflc_encrypt(const char* plain_text);

char* yflc_decrypt(const char* cipher_text);

#ifdef __cplusplus
}
#endif

#endif
