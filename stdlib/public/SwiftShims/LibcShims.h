//===--- LibcShims.h - Access to POSIX for Swift's core stdlib --*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2020 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
//  Using the Darwin (or Glibc) module in the core stdlib would create a
//  circular dependency, so instead we import these declarations as part of
//  SwiftShims.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_STDLIB_SHIMS_LIBCSHIMS_H
#define SWIFT_STDLIB_SHIMS_LIBCSHIMS_H

#include "SwiftStdbool.h"
#include "SwiftStdint.h"
#include "SwiftStddef.h"
#include "Visibility.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

#ifdef __cplusplus
extern "C" {
#endif


// Input/output <stdio.h>
SWIFT_RUNTIME_STDLIB_INTERNAL
int _swift_stdlib_putchar_unlocked(int c);
SWIFT_RUNTIME_STDLIB_INTERNAL
__swift_size_t _swift_stdlib_fwrite_stdout(const void *ptr, __swift_size_t size,
                                           __swift_size_t nitems);

// General utilities <stdlib.h>
// Memory management functions
static inline void _swift_stdlib_free(void *_Nullable ptr) {
  extern void free(void *);
  free(ptr);
}

// <unistd.h>
SWIFT_RUNTIME_STDLIB_SPI
__swift_ssize_t _swift_stdlib_read(int fd, void *buf, __swift_size_t nbyte);
SWIFT_RUNTIME_STDLIB_SPI
__swift_ssize_t _swift_stdlib_write(int fd, const void *buf, __swift_size_t nbyte);
SWIFT_RUNTIME_STDLIB_SPI
int _swift_stdlib_close(int fd);

// String handling <string.h>
SWIFT_READONLY
static inline __swift_size_t _swift_stdlib_strlen(const char *s) {
  extern __swift_size_t strlen(const char *);
  return strlen(s);
}

SWIFT_READONLY
static inline __swift_size_t _swift_stdlib_strlen_unsigned(const unsigned char *s) {
  return _swift_stdlib_strlen((const char *)s);
}

SWIFT_READONLY
static inline int _swift_stdlib_memcmp(const void *s1, const void *s2,
                                       __swift_size_t n) {
  extern int memcmp(const void *, const void *, __swift_size_t);
  return memcmp(s1, s2, n);
}

// Casting helper. This code needs to work when included from C or C++.
// Casting away const with a C-style cast warns in C++. Use a const_cast
// there.
#ifdef __cplusplus
#define CONST_CAST(type, value) const_cast<type>(value)
#else
#define CONST_CAST(type, value) (type)value
#endif

// Non-standard extensions
#if defined(__APPLE__)
#define HAS_MALLOC_SIZE 1
static inline __swift_size_t _swift_stdlib_malloc_size(const void *ptr) {
  extern __swift_size_t malloc_size(const void *);
  return malloc_size(ptr);
}
#elif defined(__linux__) || defined(__CYGWIN__) || defined(__ANDROID__) \
   || defined(__HAIKU__) || defined(__FreeBSD__) || defined(__wasi__)
#define HAS_MALLOC_SIZE 1
static inline __swift_size_t _swift_stdlib_malloc_size(const void *ptr) {
#if defined(__ANDROID__)
#if !defined(__ANDROID_API__) || __ANDROID_API__ >= 17
  extern __swift_size_t malloc_usable_size(const void *ptr);
#endif
#else
  extern __swift_size_t malloc_usable_size(void *ptr);
#endif
  return malloc_usable_size(CONST_CAST(void *, ptr));
}
#elif defined(_WIN32)
#define HAS_MALLOC_SIZE 1
static inline __swift_size_t _swift_stdlib_malloc_size(const void *ptr) {
  extern __swift_size_t _msize(void *ptr);
  return _msize(CONST_CAST(void *, ptr));
}
#else
#define HAS_MALLOC_SIZE 0

static inline __swift_size_t _swift_stdlib_malloc_size(const void *ptr) {
  return 0;
}
#endif

static inline __swift_bool _swift_stdlib_has_malloc_size() {
  return HAS_MALLOC_SIZE != 0;
}

// Math library functions
static inline SWIFT_ALWAYS_INLINE
float _stdlib_remainderf(float _self, float _other) {
  return __builtin_remainderf(_self, _other);
}
  
static inline SWIFT_ALWAYS_INLINE
float _stdlib_squareRootf(float _self) {
#if defined(_WIN32) && (defined(_M_IX86) || defined(__i386__))
  typedef float __m128 __attribute__((__vector_size__(16), __aligned__(16)));
  return __builtin_ia32_sqrtss(__extension__ (__m128){ _self, 0, 0, 0 })[0];
#else
  return __builtin_sqrtf(_self);
#endif
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_remainder(double _self, double _other) {
  return __builtin_remainder(_self, _other);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_squareRoot(double _self) {
  return __builtin_sqrt(_self);
}

#if !defined _WIN32 && (defined __i386__ || defined __x86_64__)
static inline SWIFT_ALWAYS_INLINE
long double _stdlib_remainderl(long double _self, long double _other) {
  return __builtin_remainderl(_self, _other);
}
  
static inline SWIFT_ALWAYS_INLINE
long double _stdlib_squareRootl(long double _self) {
  return __builtin_sqrtl(_self);
}
#endif

// Apple's math.h does not declare lgamma_r() etc by default, but they're
// unconditionally exported by libsystem_m.dylib in all OS versions that
// support Swift development; we simply need to provide declarations here.
#if defined(__APPLE__)
float lgammaf_r(float x, int *psigngam);
double lgamma_r(double x, int *psigngam);
long double lgammal_r(long double x, int *psigngam);
#endif // defined(__APPLE__)

// SWIFT_ENABLE_TENSORFLOW
// These changes were part of `ElementaryFunctions`, which was removed from
// apple/swift master branch and moved to apple/swift-numerics.
// TF-1203 tracks eliminating these ad-hoc tensorflow branch changes.
static inline SWIFT_ALWAYS_INLINE
float _stdlib_tanf(float x) {
  return __builtin_tanf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_acosf(float x) {
  return __builtin_acosf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_asinf(float x) {
  return __builtin_asinf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_atanf(float x) {
  return __builtin_atanf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_atan2f(float y, float x) {
  return __builtin_atan2f(y, x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_coshf(float x) {
  return __builtin_coshf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_sinhf(float x) {
  return __builtin_sinhf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_tanhf(float x) {
  return __builtin_tanhf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_acoshf(float x) {
  return __builtin_acoshf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_asinhf(float x) {
  return __builtin_asinhf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_atanhf(float x) {
  return __builtin_atanhf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_exp10f(float x) {
#if defined __APPLE__
  extern float __exp10f(float);
  return __exp10f(x);
#else
  return __builtin_powf(10, x);
#endif
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_expm1f(float x) {
  return __builtin_expm1f(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_log1pf(float x) {
  return __builtin_log1pf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_hypotf(float x, float y) {
#if defined(_WIN32)
  extern float _hypotf(float, float);
  return _hypotf(x, y);
#else
  return __builtin_hypotf(x, y);
#endif
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_erff(float x) {
  return __builtin_erff(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_erfcf(float x) {
  return __builtin_erfcf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_tgammaf(float x) {
  return __builtin_tgammaf(x);
}

static inline SWIFT_ALWAYS_INLINE
float _stdlib_lgammaf(float x) {
  extern float lgammaf_r(float x, int *psigngam);
  int dontCare;
  return lgammaf_r(x, &dontCare);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_tan(double x) {
  return __builtin_tan(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_acos(double x) {
  return __builtin_acos(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_asin(double x) {
  return __builtin_asin(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_atan(double x) {
  return __builtin_atan(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_atan2(double y, double x) {
  return __builtin_atan2(y, x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_cosh(double x) {
  return __builtin_cosh(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_sinh(double x) {
  return __builtin_sinh(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_tanh(double x) {
  return __builtin_tanh(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_acosh(double x) {
  return __builtin_acosh(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_asinh(double x) {
  return __builtin_asinh(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_atanh(double x) {
  return __builtin_atanh(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_exp10(double x) {
#if defined __APPLE__
  extern double __exp10(double);
  return __exp10(x);
#else
  return __builtin_pow(10, x);
#endif
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_expm1(double x) {
  return __builtin_expm1(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_log1p(double x) {
  return __builtin_log1p(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_hypot(double x, double y) {
  return __builtin_hypot(x, y);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_erf(double x) {
  return __builtin_erf(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_erfc(double x) {
  return __builtin_erfc(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_tgamma(double x) {
  return __builtin_tgamma(x);
}

static inline SWIFT_ALWAYS_INLINE
double _stdlib_lgamma(double x) {
  extern double lgamma_r(double x, int *psigngam);
  int dontCare;
  return lgamma_r(x, &dontCare);
}

#if !(defined(_WIN32) || defined(ANDROID)) && (defined(__i386__) || defined(__x86_64__))
static inline SWIFT_ALWAYS_INLINE
long double _stdlib_tanl(long double x) {
  return __builtin_tanl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_acosl(long double x) {
  return __builtin_acosl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_asinl(long double x) {
  return __builtin_asinl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_atanl(long double x) {
  return __builtin_atanl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_atan2l(long double y, long double x) {
  return __builtin_atan2l(y, x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_coshl(long double x) {
  return __builtin_coshl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_sinhl(long double x) {
  return __builtin_sinhl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_tanhl(long double x) {
  return __builtin_tanhl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_acoshl(long double x) {
  return __builtin_acoshl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_asinhl(long double x) {
  return __builtin_asinhl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_atanhl(long double x) {
  return __builtin_atanhl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_exp10l(long double x) {
  return __builtin_powl(10, x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_expm1l(long double x) {
  return __builtin_expm1l(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_log1pl(long double x) {
  return __builtin_log1pl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_hypotl(long double x, long double y) {
  return __builtin_hypotl(x, y);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_erfl(long double x) {
  return __builtin_erfl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_erfcl(long double x) {
  return __builtin_erfcl(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_tgammal(long double x) {
  return __builtin_tgammal(x);
}

static inline SWIFT_ALWAYS_INLINE
long double _stdlib_lgammal(long double x) {
  extern long double lgammal_r(long double x, int *psigngam);
  int dontCare;
  return lgammal_r(x, &dontCare);
}
#endif // !(defined(_WIN32) || defined(ANDROID)) && (defined(__i386__) || defined(__x86_64__))
// SWIFT_ENABLE_TENSORFLOW END

#ifdef __cplusplus
} // extern "C"
#endif

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#endif // SWIFT_STDLIB_SHIMS_LIBCSHIMS_H
