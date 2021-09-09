#pragma once

#include "../bzip2-1.0.6//bzlib.h"
#include "../bzip2-1.0.6//bzlib_private.h"

/* Include the struct and definitions that only exist in the bzlib.c file
We don't want to include the actual C file, so here we hack. */

extern int TFZ2_bzRead(
	int*    bzerror,
	BZFILE* b,
	void*   buf,
	int     len,
	unsigned long long &readlen
);