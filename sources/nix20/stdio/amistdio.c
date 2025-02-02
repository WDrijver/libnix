#include "amistdio.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stabs.h>

#include <dos/dosextens.h>
#include <exec/execbase.h>
#include <proto/dos.h>
#include <proto/exec.h>

static int bsz = 128;
static char *buffer;
static char *end;
static char *last;

void __freeBuff() {
	if (buffer)
		FreeVec(buffer);
}
void __initBuff() {
	bsz += bsz;
	__freeBuff();
	buffer = (char*) AllocVec(bsz, MEMF_PUBLIC);
	end = bsz + buffer;
}
ADD2INIT(__initBuff, -42);
ADD2EXIT(__freeBuff, -42);

__saveds
static void pc(register char *ptr asm("a3")) {
	if (ptr >= end)
		return;
	last = ptr;
	asm volatile("move.b d0,(a3)+");
}

int amiprintf(char const *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int retval = amivfprintf(Output(), fmt, args);
	va_end(args);
	return retval;
}

int amifprintf(BPTR f, char const *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int retval = amivfprintf(f, fmt, args);
	va_end(args);
	return retval;
}

int amivfprintf(BPTR f, const char *fmt, va_list args) {
	for (;;) {
		RawDoFmt(fmt, args, pc, buffer);
		if (last < end)
			break;
		__initBuff();
	}
	FPuts(f, buffer);
	return last - buffer;
}

