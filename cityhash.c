// Copyright (c) 2011 Google, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// CityHash, by Geoff Pike and Jyrki Alakuijala
//
// This file provides cityhash64() and related functions.
//
// It's probably possible to create even faster hash functions by
// writing a program that systematically explores some of the space of
// possible hash functions, by using SIMD instructions, or by
// compromising on hash quality.

#include <assert.h>
#include <string.h>

#include "cityhash.h"

#define likely(x) (__builtin_expect(!!(x), 1))

#ifdef LITTLE_ENDIAN
#define uint32_t_in_expected_order(x) (x)
#define uint64_t_in_expected_order(x) (x)
#else
#define uint32_t_in_expected_order(x) (bswap32(x))
#define uint64_t_in_expected_order(x) (bswap64(x))
#endif

#define PERMUTE3_32(a, b, c)                                                  \
  do                                                                          \
    {                                                                         \
      swap32(a, b);                                                           \
      swap32(a, c);                                                           \
    }                                                                         \
  while (0)

#define PERMUTE3_64(a, b, c)                                                  \
  do                                                                          \
    {                                                                         \
      swap64(a, b);                                                           \
      swap64(a, c);                                                           \
    }                                                                         \
  while (0)

// some primes between 2^63 and 2^64 for various uses
const uint64_t k0 = 0xc3a5c85c97cb3127;
const uint64_t k1 = 0xb492b66fbe98f273;
const uint64_t k2 = 0x9ae16a3b2f90404f;

// magic numbers for 32-bit hashing, copied from murmur3
static const uint32_t c1 = 0xcc9e2d51;
static const uint32_t c2 = 0x1b873593;

static uint32_t
uload32(const uint8_t *p)
{
  uint32_t result;

  memcpy(&result, p, sizeof(result));

  return result;
}

static uint32_t
fetch32(const uint8_t *p)
{
  return uint32_t_in_expected_order(uload32(p));
}

static uint32_t
bswap32(const uint32_t x)
{

  uint32_t y = x;

  for (size_t i = 0; i<sizeof(uint32_t)>> 1; i++)
    {

      uint32_t d = sizeof(uint32_t) - i - 1;

      uint32_t mh = ((uint32_t)0xff) << (d << 3);
      uint32_t ml = ((uint32_t)0xff) << (i << 3);

      uint32_t h = x & mh;
      uint32_t l = x & ml;

      uint64_t t = (l << ((d - i) << 3)) | (h >> ((d - i) << 3));

      y = t | (y & ~(mh | ml));
    }

  return y;
}

static void
swap32(uint32_t *a, uint32_t *b)
{
  uint32_t t;

  t = *a;
  *a = *b;
  *b = t;
}

// a 32-bit to 32-bit integer hash copied from murmur3
static uint32_t
fmix(uint32_t h)
{

  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

static uint32_t
rotate32(uint32_t val, size_t shift)
{

  assert(shift < 32);
  return (val >> shift) | (val << (32 - shift));
}

// helper from murmur3 for combining two 32-bit values
static uint32_t
mur(uint32_t a, uint32_t h)
{

  a *= c1;
  a = rotate32(a, 17);
  a *= c2;
  h ^= a;
  h = rotate32(h, 19);

  return h * 5 + 0xe6546b64;
}

static uint32_t
hash32_13_to_24(const uint8_t *s, size_t len)
{

  uint32_t a = fetch32(s - 4 + (len >> 1));
  uint32_t b = fetch32(s + 4);
  uint32_t c = fetch32(s + len - 8);
  uint32_t d = fetch32(s + (len >> 1));
  uint32_t e = fetch32(s);
  uint32_t f = fetch32(s + len - 4);
  uint32_t h = len;

  return fmix(mur(f, mur(e, mur(d, mur(c, mur(b, mur(a, h)))))));
}

static uint32_t
hash32_0_to_4(const uint8_t *s, size_t len)
{

  uint32_t b = 0;
  uint32_t c = 9;

  for (size_t i = 0; i < len; i++)
    {

      int8_t v = (int8_t)s[i];

      b = b * c1 + v;
      c ^= b;
    }

  return fmix(mur(b, mur(len, c)));
}

static uint32_t
hash32_5_to_12(const uint8_t *s, size_t len)
{

  uint32_t a = len, b = len * 5, c = 9, d = b;

  a += fetch32(s);
  b += fetch32(s + len - 4);
  c += fetch32(s + ((len >> 1) & 4));

  return fmix(mur(c, mur(b, mur(a, d))));
}

uint32_t
cityhash32(const uint8_t *s, size_t len)
{

  if (len <= 24)
    {

      return len <= 12
                 ? (len <= 4 ? hash32_0_to_4(s, len) : hash32_5_to_12(s, len))
                 : hash32_13_to_24(s, len);
    }

  // len > 24
  uint32_t h = len, g = c1 * len, f = g;
  uint32_t a0 = rotate32(fetch32(s + len - 4) * c1, 17) * c2;
  uint32_t a1 = rotate32(fetch32(s + len - 8) * c1, 17) * c2;
  uint32_t a2 = rotate32(fetch32(s + len - 16) * c1, 17) * c2;
  uint32_t a3 = rotate32(fetch32(s + len - 12) * c1, 17) * c2;
  uint32_t a4 = rotate32(fetch32(s + len - 20) * c1, 17) * c2;

  h ^= a0;
  h = rotate32(h, 19);
  h = h * 5 + 0xe6546b64;
  h ^= a2;
  h = rotate32(h, 19);
  h = h * 5 + 0xe6546b64;
  g ^= a1;
  g = rotate32(g, 19);
  g = g * 5 + 0xe6546b64;
  g ^= a3;
  g = rotate32(g, 19);
  g = g * 5 + 0xe6546b64;
  f += a4;
  f = rotate32(f, 19);
  f = f * 5 + 0xe6546b64;

  size_t iters = (len - 1) / 20;

  do
    {

      uint32_t a0 = rotate32(fetch32(s) * c1, 17) * c2;
      uint32_t a1 = fetch32(s + 4);
      uint32_t a2 = rotate32(fetch32(s + 8) * c1, 17) * c2;
      uint32_t a3 = rotate32(fetch32(s + 12) * c1, 17) * c2;
      uint32_t a4 = fetch32(s + 16);

      h ^= a0;
      h = rotate32(h, 18);
      h = h * 5 + 0xe6546b64;
      f += a1;
      f = rotate32(f, 19);
      f = f * c1;
      g += a2;
      g = rotate32(g, 18);
      g = g * 5 + 0xe6546b64;
      h ^= a3 + a1;
      h = rotate32(h, 19);
      h = h * 5 + 0xe6546b64;
      g ^= a4;
      g = bswap32(g) * 5;
      h += a4 * 5;
      h = bswap32(h);
      f += a0;
      PERMUTE3_32(&f, &h, &g);
      s += 20;
    }
  while (--iters != 0);

  g = rotate32(g, 11) * c1;
  g = rotate32(g, 17) * c1;
  f = rotate32(f, 11) * c1;
  f = rotate32(f, 17) * c1;
  h = rotate32(h + g, 19);
  h = h * 5 + 0xe6546b64;
  h = rotate32(h, 17) * c1;
  h = rotate32(h + f, 19);
  h = h * 5 + 0xe6546b64;
  h = rotate32(h, 17) * c1;

  return h;
}
