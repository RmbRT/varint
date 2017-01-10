#include <stdio.h>
#include "src/varint.h"
#include "src/malloc.h"
#include <string.h>
#include <stdlib.h>

void main(
	int argc,
	char ** argv)
{

	if(argc == 2)
	{
		VarInt a;
		vi_create_from_hex_VarInt(
			&a,
			argv[1],
			strlen(argv[1]));
		char * str = vi_to_string_VarInt(&a);
		puts(str);
		vi_free((void**)&str);
		vi_destroy_VarInt(&a);
	} else if(argc == 3)
	{
		if(!strcmp(argv[1], "rand"))
		{
			size_t len;
			if(1 != sscanf(argv[2], "%zu", &len))
			{
				fputs("second argument should be a decimal number!", stderr);
				exit(EXIT_FAILURE);
			}

			VarInt rand;
			vi_create_random_VarInt(&rand, len);

			char * str = vi_to_string_VarInt(&rand);
			puts(str);

			vi_free((void**)&str);
			vi_destroy_VarInt(&rand);
		} else if(!strcmp(argv[1], "text"))
		{
			size_t len = strlen(argv[2]);
			size_t cap = len / sizeof(digit_t) + (len % sizeof(digit_t) ? 1 : 0);
			VarInt x;
			vi_create_VarInt(&x);
			vi_calloc_digit(&x.digits,
				x.capacity = cap);

			for(size_t i = 1; i<=len; i++)
			{
				((char*)x.digits)[len - i] = argv[2][i-1];
			}

			for(size_t i = cap; i--;)
				if(x.digits[i])
				{
					x.size = i + 1;
					break;
				}
			char * str = vi_to_string_VarInt(&x);
			puts(str);
			vi_free((void**)&str);
			vi_destroy_VarInt(&x);
		} else if(!strcmp(argv[1], "prime"))
		{
			VarInt p;
			vi_create_from_hex_VarInt(
				&p,
				argv[2],
				strlen(argv[2]));

			puts(vi_is_prime_quick_VarInt(&p) ? "1":"0");
			vi_destroy_VarInt(&p);

		} else
		{
			fputs("invalid arguments.", stderr);
		}
	} else if(argc == 4)
	{
		VarInt a;
		VarInt b;

		vi_create_from_hex_VarInt(
			&a,
			argv[1],
			strlen(argv[1]));
		vi_create_from_hex_VarInt(
			&b,
			argv[3],
			strlen(argv[3]));

		VarInt result;
		VarInt rem;

		if(!strcmp(argv[2], "-"))
		{
			vi_sub_create_VarInt(
				&result,
				&a,
				&b);
		} else if(!strcmp(argv[2], "+"))
		{
			vi_add_create_VarInt(
				&result,
				&a,
				&b);
		} else if(!strcmp(argv[2], "x"))
		{
			vi_mul_create_VarInt(
				&result,
				&a,
				&b);
		} else if(!strcmp(argv[2], ":"))
		{
			vi_div_mod_create_VarInt(
				&result,
				&rem,
				&a,
				&b);
		} else if(!strcmp(argv[2], "^"))
		{
			vi_pow_create_VarInt(
				&result,
				&a,
				&b);
		} else if(!strcmp(argv[2], "shr"))
		{
			vi_create_VarInt(&result);

			vi_shr_assign_VarInt(&result, &a, atoi(argv[3]));
		} else
		{
			fprintf(
				stderr,
				"Invalid command line argument '%s'\n"
				"usage: %s [+|-]hex1 [(+|-|x|:|^) [+|-]hex2]\n",
				argv[2], argv[0]);
			goto destroy_1;
		}

		char * res_str = vi_to_string_VarInt(&result);
		if(!strcmp(argv[2], ":"))
		{
			char * rem_str = vi_to_string_VarInt(&rem);
			printf("%s | %s\n", res_str, rem_str);

			vi_free((void**)&rem_str);
			vi_destroy_VarInt(&rem);
		} else
		{
			puts(res_str);
		}
		vi_free((void**)&res_str);
		vi_destroy_VarInt(&result);
destroy_1:
		vi_destroy_VarInt(&a);
		vi_destroy_VarInt(&b);

	} else if(argc == 6)
	{
		if(!strcmp(argv[2], "^") && !strcmp(argv[4], "%"))
		{
			VarInt base, exp, mod, result;
			vi_create_from_hex_VarInt(&base, argv[1], strlen(argv[1]));
			vi_create_from_hex_VarInt(&exp, argv[3], strlen(argv[3]));
			vi_create_from_hex_VarInt(&mod, argv[5], strlen(argv[5]));

			vi_pow_mod_create_VarInt(&result, &base, &exp, &mod);
			char * res_str = vi_to_string_VarInt(&result);

			puts(res_str);

			vi_free((void**)&res_str);

			vi_destroy_VarInt(&base);
			vi_destroy_VarInt(&exp);
			vi_destroy_VarInt(&mod);
			vi_destroy_VarInt(&result);
		}
	}
}