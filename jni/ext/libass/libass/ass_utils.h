/*
 * Copyright (C) 2006 Evgeniy Stepanov <eugeni.stepanov@gmail.com>
 *
 * This file is part of libass.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef LIBASS_UTILS_H
#define LIBASS_UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef CONFIG_ENCA
#include <enca.h>
#endif

#include "ass.h"

#define MSGL_FATAL 0
#define MSGL_ERR 1
#define MSGL_WARN 2
#define MSGL_INFO 4
#define MSGL_V 6
#define MSGL_DBG2 7

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMINMAX(c,a,b) FFMIN(FFMAX(c, a), b)

int mystrtoi(char **p, int *res);
int mystrtoll(char **p, long long *res);
int mystrtou32(char **p, int base, uint32_t *res);
int mystrtod(char **p, double *res);
int strtocolor(ASS_Library *library, char **q, uint32_t *res, int hex);
char parse_bool(char *str);
unsigned ass_utf8_get_char(char **str);
void ass_msg(ASS_Library *priv, int lvl, char *fmt, ...);
#ifdef CONFIG_ENCA
void *ass_guess_buffer_cp(ASS_Library *library, unsigned char *buffer,
                          int buflen, char *preferred_language,
                          char *fallback);
#endif

/* defined in ass_strtod.c */
double ass_strtod(const char *string, char **endPtr);

static inline int d6_to_int(int x)
{
    return (x + 32) >> 6;
}
static inline int d16_to_int(int x)
{
    return (x + 32768) >> 16;
}
static inline int int_to_d6(int x)
{
    return x << 6;
}
static inline int int_to_d16(int x)
{
    return x << 16;
}
static inline int d16_to_d6(int x)
{
    return (x + 512) >> 10;
}
static inline int d6_to_d16(int x)
{
    return x << 10;
}
static inline double d6_to_double(int x)
{
    return x / 64.;
}
static inline int double_to_d6(double x)
{
    return (int) (x * 64);
}
static inline double d16_to_double(int x)
{
    return ((double) x) / 0x10000;
}
static inline int double_to_d16(double x)
{
    return (int) (x * 0x10000);
}
static inline double d22_to_double(int x)
{
    return ((double) x) / 0x400000;
}
static inline int double_to_d22(double x)
{
    return (int) (x * 0x400000);
}

// Calculate cache key for a rotational angle in degrees
static inline int rot_key(double a)
{
    const int m = double_to_d22(360.0);
    return double_to_d22(a) % m;
}

#define FNV1_32A_INIT (unsigned)0x811c9dc5

static inline unsigned fnv_32a_buf(void *buf, size_t len, unsigned hval)
{
    unsigned char *bp = buf;
    unsigned char *be = bp + len;
    while (bp < be) {
        hval ^= (unsigned) *bp++;
        hval +=
            (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) +
            (hval << 24);
    }
    return hval;
}
static inline unsigned fnv_32a_str(char *str, unsigned hval)
{
    unsigned char *s = (unsigned char *) str;
    while (*s) {
        hval ^= (unsigned) *s++;
        hval +=
            (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) +
            (hval << 24);
    }
    return hval;
}

#endif                          /* LIBASS_UTILS_H */
