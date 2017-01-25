#include <stdio.h>
#include "src/varint.h"
#include "src/malloc.h"
#include <string.h>
#include <stdlib.h>

struct Command {
	char const * name;
	char const * description;
};

static struct Command const commands[] = {
	{"--help", "Displays this text."},
	{"<number>",
		"outputs the given hexadecimal number. The number must have the following format (regex): '(+|-)?[0-9a-fA-F]+'."},
	{"<number> <op> <number>",
		"Binary operation on two hexadecimal integers. <op> can be one of the following:\n"
		"\t+\tAddition\n"
		"\t-\tSubtraction\n"
		"\tx\tMultiplication\n"
		"\t:\tDivision (with remainder). The output format is \"<number> | <number>\", where the first number is the quotient, and the second number is the remainder.\n"
		"\t^\tPower function"},
	{"<number> (shr|shl) <decimal>",
		"Performs a bitwise unsigned right/left shift on the given hexadecimal number by an amount of the given decimal number. The decimal number must fit into an integer." },
	{"<number> ^ <number> % <number>",
		"Computes pow(a,b) mod n, where a, b, and n are the given hexadecimal numbers."},
	{"isprime <number>", "Outputs 1 if the given hexadecimal integer is a prime number, otherwise 0."},
	{"nextprime <number>", "Outputs the closest prime number greater than or equal to the given hexadecimal number."},
	{"rand <decimal>", "Generates a random number with the requested length (in bytes). The decimal number must fit into an integer."},
	{"randprime <decimal>", "Generates a random prime number with the requested length (in bytes). The decimal number must fit into an integer."}
};

void help(char const * progname, FILE * file)
{
	for(size_t i = 0; i < sizeof(commands) / sizeof(*commands); i++)
	{
		fprintf(
			file,
			"command: %s %s\nexplanation: %s\n",
			progname,
			commands[i].name,
			commands[i].description);
	}
}


int main(
	int argc,
	char ** argv)
{
	// allocate 4KB heaps by default.
	vi_set_default_heap_size(4096);
	if(argc == 2)
	{
		if(!strcmp(argv[1], "--help"))
		{
			help(argv[0], stdout);
		} else
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
		}
	} else if(argc == 3)
	{
		if(!strcmp(argv[1], "rand"))
		{
			size_t len;
			if(1 != sscanf(argv[2], "%zu", &len))
			{
				fputs("second argument to 'rand' should be a decimal number!", stderr);
				help(*argv, stderr);
				exit(EXIT_FAILURE);
			}

			VarInt rand;
			vi_create_random_VarInt(&rand, len);

			char * str = vi_to_string_VarInt(&rand);
			puts(str);

			vi_free((void**)&str);
			vi_destroy_VarInt(&rand);
		} else if(!strcmp(argv[1], "randprime"))
		{
			size_t len;
			if(1 != sscanf(argv[2], "%zu", &len))
			{
				fputs("second argument should be a decimal number!", stderr);
				help(*argv, stderr);
				exit(EXIT_FAILURE);
			}

			VarInt rand;
			vi_create_random_VarInt(&rand, len);

			vi_next_prime_assign_VarInt(&rand, &rand);

			char * str = vi_to_string_VarInt(&rand);
			puts(str);

			vi_free((void**)&str);
			vi_destroy_VarInt(&rand);
		}  else if(!strcmp(argv[1], "text"))
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
		} else if(!strcmp(argv[1], "isprime"))
		{
			VarInt p;
			vi_create_from_hex_VarInt(
				&p,
				argv[2],
				strlen(argv[2]));

			puts(vi_is_prime_quick_VarInt(&p) ? "1":"0");
			vi_destroy_VarInt(&p);

		} else if(!strcmp(argv[1], "nextprime"))
		{
			VarInt p;
			vi_create_from_hex_VarInt(
				&p,
				argv[2],
				strlen(argv[2]));

			vi_next_prime_assign_VarInt(
				&p, &p);

			char * str = vi_to_string_VarInt(&p);
			puts(str);
			vi_free((void**)&str);
			vi_destroy_VarInt(&p);
		} else
		{
			fputs("invalid arguments.", stderr);
			help(*argv, stderr);
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
		} else if(!strcmp(argv[2], "shl"))
		{
			vi_create_VarInt(&result);
			vi_shl_assign_VarInt(&result, &a, atoi(argv[3]));
		} else
		{
			fputs("invalid arguments", stderr);
			help(*argv, stderr);
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
		} else
		{
			fputs("invalid arguments", stderr);
			help(*argv, stderr);
		}
	}

	vi_destroy_heap();

	return 0;
}
