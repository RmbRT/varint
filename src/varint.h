#ifndef __varint_varint_h_defined
#define __varint_varint_h_defined

#include <limits.h>
#include "defines.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char digit_t;
#define DIGIT_MAX UCHAR_MAX
#define DIGIT_BITS (sizeof(digit_t) * 8)

typedef enum {
	kPos,
	kNeg
} sign_t;

typedef struct
{
	digit_t * digits;
	size_t size;
	size_t capacity;
	sign_t sign;
} VarInt;

void vi_create_VarInt(
	VarInt * this);

void vi_create_from_hex_VarInt(
	VarInt * this,
	char const * str,
	size_t length);
char * vi_to_string_VarInt(
	VarInt const * this);

void vi_create_from_int_VarInt(
	VarInt * this,
	int value);

void vi_assign_random_VarInt(
	VarInt * this,
	size_t length);
void vi_create_random_VarInt(
	VarInt * this,
	size_t length);

void vi_copy_create_VarInt(
	VarInt * dest,
	VarInt const * src);

void vi_copy_assign_VarInt(
	VarInt * dest,
	VarInt const * src);

void vi_destroy_VarInt(
	VarInt * this);


void vi_add_assign_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb);
void vi_add_create_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb);

void vi_sub_assign_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb);
void vi_sub_create_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb);

void vi_mul_create_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb);
void vi_mul_assign_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb);

void vi_div_mod_create_VarInt(
	VarInt * quo,
	VarInt * rem,
	VarInt const * srca,
	VarInt const * srcb);
void vi_div_mod_assign_VarInt(
	VarInt * quo,
	VarInt * rem,
	VarInt const * srca,
	VarInt const * srcb);

void vi_dec_assign_VarInt(
	VarInt * dest,
	VarInt const * src);

void vi_inc_assign_VarInt(
	VarInt * dest,
	VarInt const * src);

int vi_compare_VarInt(
	VarInt const * srca,
	VarInt const * srcb);

int vi_abs_compare_VarInt(
	VarInt const * srca,
	VarInt const * srcb);

void vi_pow_assign_VarInt(
	VarInt * dest,
	VarInt const * base,
	VarInt const * exp);
void vi_pow_create_VarInt(
	VarInt * dest,
	VarInt const * base,
	VarInt const * exp);

void vi_pow_mod_assign_VarInt(
	VarInt * dest,
	VarInt const * base,
	VarInt const * exp,
	VarInt const * mod);

void vi_pow_mod_create_VarInt(
	VarInt * dest,
	VarInt const * base,
	VarInt const * exp,
	VarInt const * mod);

void vi_shr_assign_VarInt(
	VarInt * dest,
	VarInt const * src,
	int distance);

void vi_shl_assign_VarInt(
	VarInt * dest,
	VarInt const * src,
	int distance);

int vi_is_prime_quick_VarInt(
	VarInt const * this);

void vi_next_prime_assign_VarInt(
	VarInt * dest,
	VarInt const * start);

int vi_is_even_VarInt(
	VarInt const * this);

void vi_calloc_digit(
	digit_t ** ptr,
	size_t count);

void vi_free_digit(
	digit_t ** ptr);

void vi_realloc_digit(
	digit_t ** ptr,
	size_t count);

void vi_copy_digit(
	digit_t ** ptr,
	digit_t const * src,
	size_t count);

#ifdef __cplusplus
}
#endif

#endif