/*
 * steamclient.dll
 *
 * Generated from steamclient.dll by winedump.
 *
 * DO NOT SUBMIT GENERATED DLLS FOR INCLUSION INTO WINE!
 *
 */

#include "config.h"
#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>
#include "windef.h"
#include "winbase.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(steamclient);

void stubFn(void) { 
    TRACE("\n");
}

#define STUB stubFn

#include "auto.incl"


void * libsteam_api(void) {
    void *h;
    void *c = getenv("STEAMCLIENT_SO");
    if (c==NULL) {
	ERR("environment variable STEAMCLIENT_SO not defined\n");
	return NULL;
    };
    h = dlopen(c, RTLD_NOW);
    return h;
}

static int table_filled = 0;

void fill_table(void *lib) {
    int i = 0;
    if (table_filled) return;
    table_filled = 1;
    TRACE("\n");       
    for (i=0; i<38; i++) {
	void *p = dlsym(lib,name_table[i]);
	if (p) {
	    addr_table[i] = p;
	    TRACE("%x -> '%s'\n",(unsigned int)p,name_table[i]);
	} else {
	    TRACE("unknown symbol '%s' - leave stub\n",name_table[i]);
	}
    }
}




BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    TRACE("(0x%p, %d, %p)\n", hinstDLL, fdwReason, lpvReserved);
    fill_table(libsteam_api());
    switch (fdwReason)
    {
        case DLL_WINE_PREATTACH:
    	    fill_table(libsteam_api());
            return TRUE;    /* prefer native version */
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDLL);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}
