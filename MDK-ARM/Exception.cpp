#include <inttypes.h>
#include <stdio.h>
#include "main.h"

#ifndef NDEBUG
void FullStop(const char *why, const char *func, const char *file, int line)
{
    DebugPrintf("FULLSTOP(%s) in %s() [%s@%d]\r\n", why, func, file, line);
    for(;;);
}

void PrintExceptionChain(PExceptionInfo pInfo)
{
    unsigned int counter = 0;
    for(PExceptionInfo ptr = pInfo; ptr != NULL; ptr = ptr->inner)
    {
        counter++;
    }
    DebugPrintf("EXCEPTION (Depth %02d):\r\n", counter);
    counter--;
    for(PExceptionInfo ptr = pInfo; ptr != NULL; ptr = ptr->inner)
    {
        DebugPrintf("(%02d) Code 0x%08x in %s() [%s@%d]: %s\r\n", counter--, ptr->code, ptr->func, ptr->file, ptr->line, ptr->auxInfo);
    }
}
#endif
