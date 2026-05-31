#include "yflc.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define WORD_LEN 6

static const char b64_chars[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static const char* fo_words[64] = {
    "如来", "菩萨", "罗汉", "般若", "菩提", "涅槃", "三昧", "禅定",
    "慈悲", "喜舍", "解脱", "自在", "观照", "修行", "精进", "布施",
    "持戒", "忍辱", "禅那", "智慧", "方便", "愿力", "功德", "福田",
    "缘起", "性空",
    "众生", "佛性", "法身", "报身", "化身", "真如", "实际", "法界",
    "真谛", "俗谛", "中道", "实相", "无明", "行识", "名色", "六入",
    "触受", "爱取", "有生", "老死", "苦集", "灭道", "四谛", "八正",
    "因缘", "道品",
    "虚空", "一乘", "二谛", "三宝", "四念", "五根", "六尘", "七宝",
    "八风", "九品",
    "十力", "百劫",
};

static int b64_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static int fo_index(const char* word) {
    int i;
    for (i = 0; i < 64; i++) {
        if (strcmp(word, fo_words[i]) == 0) return i;
    }
    return -1;
}

static char* b64_encode_nopad(const unsigned char* src, size_t src_len) {
    size_t out_cap = ((src_len + 2) / 3) * 4;
    char* out = (char*)malloc(out_cap + 1);
    size_t i, pos, pad;

    if (!out) return NULL;

    pos = 0;
    for (i = 0; i < src_len; i += 3) {
        uint32_t n;
        int remaining = (int)(src_len - i);
        pad = (remaining < 3) ? (size_t)(3 - remaining) : 0;

        n = ((uint32_t)src[i]) << 16;
        if (i + 1 < src_len) n |= ((uint32_t)src[i + 1]) << 8;
        if (i + 2 < src_len) n |= ((uint32_t)src[i + 2]);

        out[pos++] = b64_chars[(n >> 18) & 0x3F];
        out[pos++] = b64_chars[(n >> 12) & 0x3F];
        out[pos++] = (pad == 2) ? '=' : b64_chars[(n >> 6) & 0x3F];
        out[pos++] = (pad >= 1) ? '=' : b64_chars[n & 0x3F];
    }
    out[pos] = '\0';

    while (pos > 0 && out[pos - 1] == '=') pos--;
    out[pos] = '\0';

    return out;
}

static unsigned char* b64_decode(const char* b64, size_t* out_len) {
    size_t len = strlen(b64);
    size_t pad = (4 - len % 4) % 4;
    size_t padded_len, max_out, pos, i;
    char* padded;
    unsigned char* out;
    int idx[4];

    padded_len = len + pad;
    padded = (char*)malloc(padded_len + 1);
    if (!padded) return NULL;
    memcpy(padded, b64, len);
    memset(padded + len, '=', pad);
    padded[padded_len] = '\0';

    max_out = (padded_len / 4) * 3;
    out = (unsigned char*)malloc(max_out + 1);
    if (!out) { free(padded); return NULL; }

    pos = 0;
    for (i = 0; i < padded_len; i += 4) {
        idx[0] = b64_index(padded[i]);
        idx[1] = b64_index(padded[i + 1]);
        idx[2] = b64_index(padded[i + 2]);
        idx[3] = b64_index(padded[i + 3]);

        if (idx[0] < 0 || idx[1] < 0) { free(padded); free(out); return NULL; }

        out[pos++] = (unsigned char)((idx[0] << 2) | (idx[1] >> 4));
        if (padded[i + 2] != '=') {
            if (idx[2] < 0) { free(padded); free(out); return NULL; }
            out[pos++] = (unsigned char)((idx[1] << 4) | (idx[2] >> 2));
        }
        if (padded[i + 3] != '=') {
            if (idx[3] < 0) { free(padded); free(out); return NULL; }
            out[pos++] = (unsigned char)((idx[2] << 6) | idx[3]);
        }
    }
    free(padded);

    *out_len = pos;
    out[pos] = '\0';
    return out;
}

char* yflc_encrypt(const char* plain_text) {
    size_t src_len, b64_len, total, pos, i;
    char* b64;
    char* result;

    if (!plain_text) return NULL;

    src_len = strlen(plain_text);
    b64 = b64_encode_nopad((const unsigned char*)plain_text, src_len);
    if (!b64) return NULL;

    b64_len = strlen(b64);
    total = 0;
    for (i = 0; i < b64_len; i++) {
        int ix = b64_index(b64[i]);
        if (ix < 0) { free(b64); return NULL; }
        total += WORD_LEN;
    }

    result = (char*)malloc(total + 1);
    if (!result) { free(b64); return NULL; }

    pos = 0;
    for (i = 0; i < b64_len; i++) {
        int ix = b64_index(b64[i]);
        memcpy(result + pos, fo_words[ix], WORD_LEN);
        pos += WORD_LEN;
    }
    result[pos] = '\0';

    free(b64);
    return result;
}

char* yflc_decrypt(const char* cipher_text) {
    size_t cipher_len, word_count, b64_pos, i, out_len;
    char* b64;
    unsigned char* data;

    if (!cipher_text) return NULL;

    cipher_len = strlen(cipher_text);
    if (cipher_len % WORD_LEN != 0) return NULL;

    word_count = cipher_len / WORD_LEN;
    b64 = (char*)malloc(word_count + 5);
    if (!b64) return NULL;

    b64_pos = 0;
    for (i = 0; i < cipher_len; i += WORD_LEN) {
        char word[7];
        int ix;
        memcpy(word, cipher_text + i, WORD_LEN);
        word[WORD_LEN] = '\0';
        ix = fo_index(word);
        if (ix < 0) { free(b64); return NULL; }
        b64[b64_pos++] = b64_chars[ix];
    }
    b64[b64_pos] = '\0';

    data = b64_decode(b64, &out_len);
    free(b64);
    if (!data) return NULL;

    return (char*)data;
}
