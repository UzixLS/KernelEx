/*
 * strtox.c
 *
 * strto...() functions, by macro definition
 */

#include <stddef.h>
#include <stdint.h>

TYPE NAME(const char *nptr, char **endptr, int base)
{
	return (TYPE) strntoumax(nptr, endptr, base, ~(size_t) 0);
}
