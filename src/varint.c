#include "varint.h"
#include "malloc.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static digit_t const
	digit_one = 1,
	digit_two = 2,
	digit_four = 4;
static VarInt const
	varint_one = {
		(digit_t*)&digit_one,
		1,
		1,
		kPos
	}, varint_minus_one = {
		(digit_t*)&digit_one,
		1,
		1,
		kNeg
	}, varint_two = {
		(digit_t*)&digit_two,
		1,
		1,
		kPos
	}, varint_zero = {
		NULL,
		0,
		0,
		kPos
	}, varint_four = {
		(digit_t*)&digit_four,
		1,
		1,
		kPos
	};

void vi_create_VarInt(
	VarInt * this)
{
	assert(this != NULL);

	this->digits = NULL;
	this->size = 0;
	this->capacity = 0;
	this->sign = kPos;
}

void vi_copy_create_VarInt(
	VarInt * dest,
	VarInt const * src)
{
	assert(dest != NULL);
	assert(src != NULL);

	vi_create_VarInt(dest);
	vi_copy_assign_VarInt(dest, src);
}

void vi_destroy_VarInt(
	VarInt * this)
{
	assert(this != NULL);

	if(this->digits)
	{
		vi_free_digit(&this->digits);
	}
	this->size = 0;
	this->capacity = 0;
	this->sign = 0;
}

void vi_copy_assign_VarInt(
	VarInt * dest,
	VarInt const * src)
{
	assert(dest != NULL);
	assert(src != NULL);

	if(dest->capacity < src->size && src->size)
	{
		vi_realloc_digit(
			&dest->digits,
			dest->capacity = src->size);
	}

	dest->size = src->size;
	for(size_t i = 0; i < dest->size; i++)
		dest->digits[i] = src->digits[i];

	dest->sign = src->sign;
}

void vi_create_random_VarInt(
	VarInt * this,
	size_t length)
{
	assert(this != NULL);
	vi_create_VarInt(this);
	vi_assign_random_VarInt(this, length);
}

void vi_assign_random_VarInt(
	VarInt * this,
	size_t length)
{
	assert(this != NULL);

	size_t const min_cap = length / sizeof(digit_t)
			+ (length & ((1<<sizeof(digit_t)) -1) ? 1 : 0);

	this->sign = kPos;
	this->size = 0;

	if(!min_cap)
	{
		return;
	}

	if(this->capacity < min_cap)
	{
		vi_realloc_digit(
			&this->digits,
			this->capacity = min_cap);
	}
	else
	{
		for(size_t i = min_cap - 1; i < this->capacity; i++)
			this->digits[i] = 0;
	}

	FILE * random = fopen("/dev/urandom", "r");
	if(!random)
	{
		fputs("could not open /dev/urandom", stderr);
		exit(EXIT_FAILURE);
	}

	if(length > fread(this->digits, 1, length, random))
	{
		fputs("error reading from /dev/urandom", stderr);
		exit(EXIT_FAILURE);
	}

	fclose(random);

	for(size_t i = min_cap; i--;)
		if(this->digits[i])
		{
			this->size = i + 1;
			break;
		}

}

void vi_calloc_digit(
	digit_t ** ptr,
	size_t count)
{
	vi_calloc(
		(void**)ptr,
		sizeof(digit_t),
		count);
}

void vi_free_digit(
	digit_t ** ptr)
{
	vi_free((void**)ptr);
}

void vi_realloc_digit(
	digit_t ** ptr,
	size_t count)
{
	vi_realloc(
		(void**)ptr,
		sizeof(digit_t),
		count);
}

void vi_copy_digit(
	digit_t ** ptr,
	digit_t const * src,
	size_t count)
{
	vi_copy(
		(void**)ptr,
		src,
		sizeof(digit_t),
		count);
}


void digit_add(
	digit_t a,
	digit_t b,
	digit_t c_in,
	digit_t * out,
	digit_t * carry)
{
	assert(out != NULL);
	assert(carry != NULL);
	assert(c_in == 0 || c_in == 1);

	*carry = (b > DIGIT_MAX - a || a+b > DIGIT_MAX - c_in) ? 1 : 0;
	*out = a + b + c_in;
}

void digit_sub(
	digit_t a,
	digit_t b,
	digit_t c_in,
	digit_t * out,
	digit_t * carry)
{
	assert(out != NULL);
	assert(carry != NULL);
	assert(c_in == 0 || c_in == 1);

	*carry = (a < b || b > DIGIT_MAX - c_in || a < b + c_in) ? 1 : 0;
	*out = a - (b+c_in);
}

static void internal_add_assign_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb)
{
	assert(dest != NULL);
	assert(srca != NULL);
	assert(srcb != NULL);

	if(dest == srca)
	{
		VarInt copy_srca;
		vi_copy_create_VarInt(&copy_srca, srca);
		internal_add_assign_VarInt(dest, &copy_srca, srcb);
		vi_destroy_VarInt(&copy_srca);
		return;
	}

	if(dest == srcb)
	{
		VarInt copy_srcb;
		vi_copy_create_VarInt(&copy_srcb, srcb);
		internal_add_assign_VarInt(dest, srca, &copy_srcb);
		vi_destroy_VarInt(&copy_srcb);
		return;
	}

	VarInt const * longer, * shorter;

	if(srca->size >= srcb->size)
	{
		longer = srca;
		shorter = srcb;
	} else {
		longer = srcb;
		shorter = srca;
	}

	if(!shorter->size)
	{
		vi_copy_assign_VarInt(dest, longer);
		return;
	}

	digit_t res, carry;
	digit_add(
		longer->digits[longer->size-1],
		(shorter->size == longer->size)
			? shorter->digits[shorter->size-1]
			: 0,
		1,
		&res,
		&carry);

	size_t new_cap = longer->size + carry;
	if(dest->capacity < new_cap)
	{
		vi_realloc_digit(
			&dest->digits,
			dest->capacity = longer->size + carry);
	}

	dest->size = 0;

	carry = 0;
	for(size_t i = 0; i < shorter->size; i++)
	{
		digit_add(
			longer->digits[i],
			shorter->digits[i],
			carry,
			&dest->digits[i],
			&carry);
		if(dest->digits[i])
			dest->size = i+1;
	}
	for(size_t i = shorter->size; i < longer->size; i++)
	{
		digit_add(
			longer->digits[i],
			0,
			carry,
			&dest->digits[i],
			&carry);
		if(dest->digits[i])
			dest->size = i+1;
	}

	if(carry)
	{
		dest->size = longer->size+1;
		dest->digits[dest->size-1] = carry;
	}
}

static void digit_mul(
	digit_t x,
	digit_t y,
	digit_t * low,
	digit_t * high)
{
	assert(low != NULL);
	assert(high != NULL);

	static digit_t const
		kHalf = sizeof(digit_t) * 4;
	static digit_t const
		kHalfMask = ((digit_t)1 << (sizeof(digit_t) * 4)) - 1;

	digit_t const b = (x & kHalfMask);
	digit_t const a = (x >> kHalf);
	digit_t const d = (y & kHalfMask);
	digit_t const c = (y >> kHalf);

	digit_t const ac = a * c;
	digit_t const ad = a * d;
	digit_t const bc = b * c;
	digit_t const bd = b * d;

	*low = bd;
	*high = 0;

	digit_t carry;
	digit_add(
		bd,
		(ad << kHalf),
		0,
		low,
		&carry);
	digit_t carry2;
	digit_add(
		*low,
		(bc << kHalf),
		0,
		low,
		&carry2);
	digit_add(
		ac,
		ad >> kHalf,
		carry,
		high,
		&carry);
	digit_add(
		*high,
		bc >> kHalf,
		carry2,
		high,
		&carry2);

	assert(carry | carry2 == 0);
}

void vi_mul_create_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb)
{
	assert(dest != NULL);
	assert(srca != NULL);
	assert(srcb != NULL);
	assert(dest != srca);
	assert(dest != srcb);

	vi_create_VarInt(dest);
	vi_mul_assign_VarInt(dest, srca, srcb);
}
void vi_mul_assign_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb)
{
	assert(dest != NULL);
	assert(srca != NULL);
	assert(srcb != NULL);

	if(dest == srca)
	{
		VarInt copy_srca;
		vi_copy_create_VarInt(&copy_srca, srca);
		vi_mul_assign_VarInt(dest, &copy_srca, srcb);
		vi_destroy_VarInt(&copy_srca);
		return;
	}

	if(dest == srcb)
	{
		VarInt copy_srcb;
		vi_copy_create_VarInt(&copy_srcb, srcb);
		vi_mul_assign_VarInt(dest, srca, &copy_srcb);
		vi_destroy_VarInt(&copy_srcb);
		return;
	}

	VarInt const * longer, * shorter;

	if(srca->size >= srcb->size)
	{
		longer = srca;
		shorter = srcb;
	} else
	{
		longer = srcb;
		shorter = srca;
	}

	dest->sign = kPos;
	dest->size = 0;

	for(size_t i = 0; i < shorter->size; i++)
	{
		VarInt temp;
		vi_create_VarInt(&temp);
		vi_calloc_digit(
			&temp.digits,
			temp.capacity = i + longer->size + 1);
		for(size_t x = 0; x < longer->size; x++)
		{
			digit_t low, high;
			digit_mul(
				shorter->digits[i],
				longer->digits[x],
				&low,
				&high);

			digit_t carry;
			size_t target = i+x;
			digit_add(
				temp.digits[target],
				low,
				0,
				&temp.digits[target],
				&carry);
			digit_add(
				temp.digits[target+1],
				high,
				carry,
				&temp.digits[target+1],
				&carry);

				target += 2;

			while(carry)
			{
				++target;
				assert(target < temp.capacity);

				digit_add(
					temp.digits[target],
					0,
					carry,
					&temp.digits[target],
					&carry);
			}

		}

		for(size_t j = 0; j < temp.capacity; j++)
			if(temp.digits[j])
				temp.size = j + 1;

		// dest += temp;
		vi_add_assign_VarInt(
			dest,
			dest,
			&temp);

		vi_destroy_VarInt(&temp);
	}

	dest->sign = srca->sign != srcb->sign;
}

void vi_div_mod_create_VarInt(
	VarInt * quo,
	VarInt * rem,
	VarInt const * srca,
	VarInt const * srcb)
{
	assert(srca != NULL);
	assert(srcb != NULL);
	assert(srcb->size != 0 && "cannot divide by zero");
	if(quo || rem)
		assert(quo != rem);
	assert(quo != srca);
	assert(quo != srcb);
	assert(rem != srca);
	assert(rem != srcb);

	if(quo)
		vi_create_VarInt(quo);
	if(rem)
		vi_create_VarInt(rem);
	vi_div_mod_assign_VarInt(quo, rem, srca, srcb);
}

void vi_div_mod_assign_VarInt(
	VarInt * quo,
	VarInt * rem,
	VarInt const * srca,
	VarInt const * srcb)
{
	if(quo == NULL)
	{
		VarInt _quo = varint_zero;
		vi_div_mod_assign_VarInt(
			&_quo,
			rem,
			srca,
			srcb);
		vi_destroy_VarInt(&_quo);
		return;
	}
	if(rem == NULL)
	{
		VarInt _rem = varint_zero;
		vi_div_mod_assign_VarInt(
			quo,
			&_rem,
			srca,
			srcb);
		vi_destroy_VarInt(&_rem);
		return;
	}

	assert(quo != NULL);
	assert(rem != NULL);
	assert(srca != NULL);
	assert(srcb != NULL);
	assert(srcb->size != 0 && "cannot divide by zero");
	assert(quo != rem);

	if(rem == srca || quo == srca)
	{
		VarInt copy_srca;
		vi_copy_create_VarInt(&copy_srca, srca);
		vi_div_mod_assign_VarInt(quo, rem, &copy_srca, srcb);
		vi_destroy_VarInt(&copy_srca);
		return;
	}

	if(rem == srcb || quo == srca)
	{
		VarInt copy_srcb;
		vi_copy_create_VarInt(&copy_srcb, srcb);
		vi_div_mod_assign_VarInt(quo, rem, srca, &copy_srcb);
		vi_destroy_VarInt(&copy_srcb);
		return;
	}

	// quo = 0
	quo->size = 0;

	if(srcb->size > srca->size)
	{
		vi_copy_assign_VarInt(rem, srca);
		return;
	}


	VarInt count_down;
	vi_copy_create_VarInt(&count_down, srca);
	VarInt factor;
	vi_copy_create_VarInt(&factor, (VarInt*)&varint_one);

	int min_factor = 1;

	for(int finished = 0; !finished;)
	{

		// dec_amnt = factor * srcb
		VarInt dec_amnt;
		vi_mul_create_VarInt(&dec_amnt, &factor, srcb);

		// decreased = count_down - dec_amnt;
		VarInt decreased;
		if(srca->sign == srcb->sign)
			vi_sub_create_VarInt(&decreased, &count_down, &dec_amnt);
		else
			vi_add_create_VarInt(&decreased, &count_down, &dec_amnt);

		vi_destroy_VarInt(&dec_amnt);

		if(!decreased.size) // finished and no remainder?
		{
			// rem = 0
			rem->size = 0;

			// quo += factor;
			vi_add_assign_VarInt(quo, quo, &factor);

			finished = 1;
		} // still not finished?
		else if(decreased.sign == count_down.sign)
		{
			// quo += factor;
			vi_add_assign_VarInt(quo, quo, &factor);

			// factor *= 2;
			vi_shl_assign_VarInt(&factor, &factor, 1);

			min_factor = 0;

			// count_down = decreased;
			vi_copy_assign_VarInt(&count_down, &decreased);

		} else if(!min_factor) // too large subtraction?
		{
			// factor = 1
			vi_copy_assign_VarInt(&factor, (VarInt*)&varint_one);
			min_factor = 1;
		} else // finished this one, with remainder
		{
			vi_copy_assign_VarInt(rem, &count_down);
			finished = 1;
		}

		vi_destroy_VarInt(&decreased);
	}

	vi_destroy_VarInt(&factor);
	vi_destroy_VarInt(&count_down);

	rem->sign = srca->sign;
	quo->sign = (srca->sign != srcb->sign);
}

void vi_dec_assign_VarInt(
	VarInt * dest,
	VarInt const * src)
{
	assert(dest != NULL);
	assert(src != NULL);

	vi_add_assign_VarInt(dest, src, &varint_minus_one);
}

void vi_inc_assign_VarInt(
	VarInt * dest,
	VarInt const * src)
{
	assert(dest != NULL);
	assert(src != NULL);

	vi_add_assign_VarInt(dest, src, &varint_one);
}

int vi_compare_VarInt(
	VarInt const * srca,
	VarInt const * srcb)
{
	assert(srca != NULL);
	assert(srcb != NULL);

	if(!srca->size)
		return srcb->size
			? srcb->sign == kPos
				? -1
				: 1
			: 0;
	if(!srcb->size)
		return srca->sign == kPos
			? 1
			: -1;

	if(srca->sign != srcb->sign)
		return srca->sign == kPos
			? 1
			: -1;

	if(srca->size < srcb->size)
		return srca->sign == kPos
			? -1
			: 1;
	if(srca->size > srcb->size)
		return srca->sign == kPos
			? 1
			: -1;

	for(size_t i = srca->size; i--;)
		if(srca->digits[i] != srcb->digits[i])
		{
			if(srca->digits[i] < srcb->digits[i])
				return srca->sign == kPos
					? -1
					: 1;
			else
				return srca->sign == kPos
					? 1
					: -1;
		}

	return 0;
}

void vi_pow_assign_VarInt(
	VarInt * dest,
	VarInt const * base,
	VarInt const * exp)
{
	assert(dest != NULL);
	assert(base != NULL);
	assert(exp != NULL);

	if(dest == base || dest == exp)
	{
		VarInt dest_copy;
		vi_create_VarInt(&dest_copy);
		vi_pow_assign_VarInt(&dest_copy, base, exp);
		vi_destroy_VarInt(dest);
		*dest = dest_copy;
	}


	if(!exp->size)
	{
		vi_copy_assign_VarInt(dest, &varint_one);
		return;
	}

	if(!base->size)
	{
		dest->size = 0;
		return;
	}

	if(vi_compare_VarInt(base, &varint_one) == 0)
	{
		vi_copy_assign_VarInt(dest, &varint_one);
		return;
	}

	if(exp->sign == kNeg)
	{
		dest->size = 0;
		return;
	}

	vi_copy_assign_VarInt(dest, &varint_one);
	VarInt mul;
	vi_copy_create_VarInt(&mul, base);


	for(size_t d = 0; d < exp->size; d++)
	{
		for(size_t b = 0; b < sizeof(digit_t) * 8; b++)
		{
			if(exp->digits[d] & (digit_t)((digit_t)1 << b))
			{
				vi_mul_assign_VarInt(dest, dest, &mul);
				if(d == exp->size - 1)
				{
					digit_t mask = 1;
					mask <<= b;
					--mask;
					if(!(exp->digits[d] & ~mask))
						break;
				}
			}

			vi_mul_assign_VarInt(&mul, &mul, &mul);
		}
	}

	vi_destroy_VarInt(&mul);
}

void vi_pow_mod_assign_VarInt(
	VarInt * dest,
	VarInt const * base,
	VarInt const * exp,
	VarInt const * mod)
{
	assert(dest != NULL);
	assert(base != NULL);
	assert(exp != NULL);
	assert(mod != NULL);

	if(dest == base || dest == exp || dest == mod)
	{
		VarInt result;
		vi_create_VarInt(&result);
		vi_pow_mod_assign_VarInt(&result, base, exp, mod);
		vi_destroy_VarInt(dest);
		*dest = result;
		return;
	}


	if(!exp->size)
	{
		vi_div_mod_assign_VarInt(NULL, dest, &varint_one, mod);
		return;
	}

	if(!base->size)
	{
		dest->size = 0;
		return;
	}

	if(vi_compare_VarInt(base, &varint_one) == 0)
	{
		vi_div_mod_assign_VarInt(NULL, dest, &varint_one, mod);
		return;
	}

	if(exp->sign == kNeg)
	{
		dest->size = 0;
		return;
	}

	vi_copy_assign_VarInt(dest, &varint_one);
	VarInt mul;
	vi_div_mod_create_VarInt(NULL, &mul, base, mod);

	for(size_t d = 0; d < exp->size; d++)
	{
		for(size_t b = 0; b < sizeof(digit_t) * 8; b++)
		{
			if(exp->digits[d] & (digit_t)((digit_t)1 << b))
			{
				vi_mul_assign_VarInt(dest, dest, &mul);
				vi_div_mod_assign_VarInt(NULL, dest, dest, mod);
				if(d == exp->size - 1)
				{
					digit_t mask = 1;
					mask <<= b;
					--mask;
					if(!(exp->digits[d] & ~mask))
						break;
				}
			}

			vi_mul_assign_VarInt(&mul, &mul, &mul);
			vi_div_mod_assign_VarInt(NULL, &mul, &mul, mod);
		}
	}

	vi_destroy_VarInt(&mul);
}

void vi_pow_create_VarInt(
	VarInt * dest,
	VarInt const * base,
	VarInt const * exp)
{
	assert(dest != NULL);
	assert(base != NULL);
	assert(exp != NULL);
	assert(dest != base);
	assert(dest != exp);

	vi_create_VarInt(dest);
	vi_pow_assign_VarInt(dest, base, exp);
}
void vi_pow_mod_create_VarInt(
	VarInt * dest,
	VarInt const * base,
	VarInt const * exp,
	VarInt const * mod)
{
	assert(dest != NULL);
	assert(base != NULL);
	assert(exp != NULL);
	assert(mod != NULL);
	assert(dest != base);
	assert(dest != exp);
	assert(dest != mod);

	vi_create_VarInt(dest);
	vi_pow_mod_assign_VarInt(dest, base, exp, mod);
}

static sign_t internal_sub_assign_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb)
{
	assert(dest != NULL);
	assert(srca != NULL);
	assert(srcb != NULL);

	if(dest == srca)
	{
		VarInt copy_srca;
		vi_copy_create_VarInt(&copy_srca, srca);
		sign_t result = internal_sub_assign_VarInt(dest, &copy_srca, srcb);
		vi_destroy_VarInt(&copy_srca);
		return result;
	}

	if(dest == srcb)
	{
		VarInt copy_srcb;
		vi_copy_create_VarInt(&copy_srcb, srcb);
		sign_t result = internal_sub_assign_VarInt(dest, srca, &copy_srcb);
		vi_destroy_VarInt(&copy_srcb);
		return result;
	}

	VarInt const * longer, * shorter;

	if(srca->size >= srcb->size)
	{
		longer = srca;
		shorter = srcb;
	} else {
		longer = srcb;
		shorter = srca;
	}

	if(!shorter->size)
	{
		vi_copy_assign_VarInt(dest, longer);
		return srca == shorter
			? !longer->sign
			: shorter->sign;
	}

	digit_t carry;

	if(dest->capacity < longer->size)
	{
		vi_realloc_digit(
			&dest->digits,
			dest->capacity = longer->size);
	}
	dest->size = 0;

	carry = 0;
	for(size_t i = 0; i < shorter->size; i++)
	{
		digit_sub(
			longer->digits[i],
			shorter->digits[i],
			carry,
			&dest->digits[i],
			&carry);
		if(dest->digits[i])
			dest->size = i+1;
	}
	for(size_t i = shorter->size; i < longer->size; i++)
	{
		digit_sub(
			longer->digits[i],
			0,
			carry,
			&dest->digits[i],
			&carry);
		if(dest->digits[i])
			dest->size = i+1;
	}
	if(carry)
	{
		for(size_t i = 0; i < dest->size; i++)
			dest->digits[i] = DIGIT_MAX - dest->digits[i] + 1;
	}

	if(shorter == srca)
		assert(!carry);

	return (longer == srca)
		? (carry
			? !srca->sign
			: srca->sign)
		: !srca->sign;
}

void vi_add_assign_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb)
{
	assert(dest != NULL);
	assert(srca != NULL);
	assert(srcb != NULL);

	if(srca->sign == srcb->sign)
	{
		internal_add_assign_VarInt(dest,srca,srcb);
		dest->sign = srca->sign;
	} else
	{
		dest->sign = internal_sub_assign_VarInt(dest,srca,srcb);
	}
}

void vi_add_create_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb)
{
	assert(dest != NULL);
	assert(srca != NULL);
	assert(srcb != NULL);
	assert(dest != srca);
	assert(dest != srcb);

	vi_create_VarInt(dest);
	vi_add_assign_VarInt(dest, srca, srcb);
}

void vi_sub_assign_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb)
{
	assert(dest != NULL);
	assert(srca != NULL);
	assert(srcb != NULL);

	if(srca->sign != srcb->sign)
	{
		internal_add_assign_VarInt(dest,srca,srcb);
		dest->sign = srca->sign;
	} else
	{
		dest->sign = internal_sub_assign_VarInt(dest,srca,srcb);
	}
}

void vi_sub_create_VarInt(
	VarInt * dest,
	VarInt const * srca,
	VarInt const * srcb)
{
	assert(dest != NULL);
	assert(srca != NULL);
	assert(srcb != NULL);
	assert(dest != srca);
	assert(dest != srcb);

	vi_create_VarInt(dest);
	vi_sub_assign_VarInt(dest, srca, srcb);
}


char * vi_to_string_VarInt(
	VarInt const * this)
{
	char * str = NULL;
	vi_calloc(
		(void**)&str,
		sizeof(char),
		sizeof(digit_t) * 2 * this->size  + 2);
	char * const ret = str;

	if(this->size)
		if(this->sign == kNeg)
			*str++ = '-';
		//else *str++ = '+';

	int written = 0;

	for(size_t i = this->size; i--;)
	{
		digit_t d = this->digits[i];

		for(int j = sizeof(digit_t) * 2; j--;)
		{
			digit_t nibble = (d >> (j*4)) & 0xf;
			if(written || nibble)
			{
				*str++ = "0123456789abcdef"[nibble];
				written = 1;
			}
		}
	}

	if(!written)
	{
		*str='0';
	}

	return ret;
}

unsigned hex_digit(char c)
{
	if(c >= 'A' && c <= 'F')
	{
		return c - ('A' - 0xA);
	} else if(c >= 'a' && c <= 'f')
	{
		return c - ('a' - 0xa);
	} else if(c >= '0' && c <= '9')
	{
		return c - '0';
	} else assert(!"invalid character.");
	return 0;
}

void vi_create_from_hex_VarInt(
	VarInt * this,
	char const * str,
	size_t length)
{
	assert(this != NULL);
	assert(str != NULL);

	vi_create_VarInt(this);

	sign_t s = kPos;
	if(*str == '+' || *str == '-')
	{
		--length;
		s = *str == '-' ? kNeg : kPos;
		++str;
	}
	this->capacity = (length >> 1) + (length & 1);
	vi_calloc_digit(
		&this->digits,
		this->capacity);

	digit_t d = 0;

	size_t nibble = 0;
	size_t n = 0;

	for(size_t i = length; i--;)
	{
		d |= (digit_t)hex_digit(str[i]) << (nibble++<<2);
		if(nibble == sizeof(digit_t) * 2)
		{
			this->digits[n++] = d;
			if(d)
				this->size = n;
			nibble = 0;
			d = 0;
		}
	}

	if(nibble != sizeof(digit_t) * 2)
	{
		this->digits[n++] = d;
		if(d)
			this->size = n;
	}

	this->sign = s;
}

static int fermat(
	VarInt const * base,
	VarInt const * p)
{
	assert(base != NULL);
	assert(p != NULL);
	assert(vi_compare_VarInt(base, p) < 0);



	VarInt temp = varint_zero;
	vi_dec_assign_VarInt(
		&temp,
		p);

	vi_pow_mod_assign_VarInt(
		&temp,
		base,
		&temp,
		p);

	// the fermat test must result in a remainder of 1, or else the number is definitely not prime.
	// fermat = base ^ (p-1) % p
	int check = vi_compare_VarInt(&temp, &varint_one);

	vi_destroy_VarInt(&temp);

	return (check == 0);
}


int vi_is_prime_quick_VarInt(
	VarInt const * this)
{
	assert(this != NULL);

	int maybe_prime = 1;
	VarInt n;

	#pragma omp parallel
	#pragma omp single nowait
	for(vi_sub_create_VarInt(&n, this, &varint_one);
		maybe_prime && vi_compare_VarInt(&n, &varint_one) > 0;
		vi_shr_assign_VarInt(
			&n,
			&n,
			1))
	{
#ifdef _OPENMP
		VarInt temp_n;
		vi_copy_create_VarInt(&temp_n, &n);
#endif

		#pragma omp task shared(maybe_prime) firstprivate(temp_n)
		{

			if(maybe_prime && !fermat(
#ifdef _OPENMP
				&temp_n,
#else
				&n,
#endif
				this))
			{
				maybe_prime = 0;
			}
			vi_destroy_VarInt(&temp_n);
		}
	}

	#pragma omp taskwait

	vi_destroy_VarInt(&n);
	return maybe_prime;
}


void vi_shr_assign_VarInt(
	VarInt * dest,
	VarInt const * src,
	int distance)
{
	assert(src != NULL);
	assert(dest != NULL);


	dest->sign = src->sign;

	if(!distance)
	{
		if(dest != src)
			vi_copy_assign_VarInt(dest, src);
		return;
	}

	if(distance < 0)
		return vi_shl_assign_VarInt(dest, src, -distance);

	static size_t const digit_bits = sizeof(digit_t) * 8;

	size_t swallow = distance / digit_bits;
	size_t rest = distance % digit_bits;


	if(swallow >= src->size)
	{
		dest->sign = kPos;
		dest->size = 0;
		return;
	}

	if(dest != src)
	{
		if(dest->capacity < src->size - swallow)
		vi_realloc_digit(
			&dest->digits,
			dest->capacity = src->size - swallow);
	}

	for(size_t i = 0; i + 1 < src->size - swallow; i++)
	{
		dest->digits[i] = (src->digits[i + swallow ] >> rest)
						| (src->digits[i + swallow + 1] << (digit_bits - rest));
	}

	dest->digits[src->size - swallow - 1] = (src->digits[src->size - 1] >> rest);

	dest->size = src->size - swallow;

	if(dest->size && !dest->digits[dest->size-1])
		--dest->size;

	dest->sign = dest->size ? src->sign : kPos;
}


void vi_shl_assign_VarInt(
	VarInt * dest,
	VarInt const * src,
	int distance)
{
	assert(src != NULL);

	if(distance < 0)
		return vi_shr_assign_VarInt(dest, src, -distance);

	static size_t const digit_bits = sizeof(digit_t) * 8;

	size_t fill = distance / digit_bits;
	size_t rest = distance % digit_bits;

	if(dest->capacity < src->size + fill + 1)
	vi_realloc_digit(
		&dest->digits,
		dest->capacity = src->size + fill + 1);

	dest->digits[src->size + fill] = src->digits[src->size - 1] >> (digit_bits - rest);

	for(size_t i = src->size; i-->1;)
	{
		dest->digits[i + fill] = (src->digits[i] << rest)
						| (src->digits[i - 1] >> (digit_bits - rest));
	}

	dest->digits[fill] = (src->digits[0] << rest);

	for(size_t i = fill; i--;)
		dest->digits[i] = 0;

	dest->size = src->size + fill + 1;
	if(dest->size && !dest->digits[dest->size-1])
		--dest->size;

	dest->sign =  src->sign;
}