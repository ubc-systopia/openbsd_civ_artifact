#include "compat.h"
#include <stdlib.h>
#include <string.h>

/*
 *  * Overwrite memory with zeroes before freeing.
 *   * This prevents leaking sensitive data (e.g., passwords) via freed memory.
 *    */
void
freezero(void *ptr, size_t size)
{
		if (ptr == NULL)
					return;
			explicit_bzero(ptr, size);
				free(ptr);
}

