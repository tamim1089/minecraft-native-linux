#include "stdafx.h"

#ifdef _LINUX
#include <pthread.h>
#endif


typedef struct tagTHREADNAME_INFO {
    DWORD dwType;     // Must be 0x1000
    LPCSTR szName;    // Pointer to name (in user address space)
    DWORD dwThreadID; // Thread ID (-1 for caller thread)
    DWORD dwFlags;    // Reserved for future use; must be zero
} THREADNAME_INFO;

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName )
{
#ifdef _LINUX
    // Linux: name the calling thread (pthread limits names to 16 bytes incl. NUL).
    (void)dwThreadID;
    if (szThreadName)
    {
        char buf[16];
        size_t n = 0;
        for (; n < 15 && szThreadName[n]; ++n) buf[n] = szThreadName[n];
        buf[n] = '\0';
        pthread_setname_np(pthread_self(), buf);
    }
    return;
#endif
#ifndef _LINUX
    THREADNAME_INFO info;

    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

#if ( defined _LINUX | defined _LINUX ) && !defined(_LINUX)
	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (ULONG_PTR *)&info );
	}
	__except( GetExceptionCode()==0x406D1388 ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_EXECUTE_HANDLER )
	{
	}
#endif
#ifdef _LINUX
    __try
    {
        RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD *)&info );
    }
    __except( GetExceptionCode()==0x406D1388 ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_EXECUTE_HANDLER )
    {
    }
#endif
#endif // _LINUX
}
