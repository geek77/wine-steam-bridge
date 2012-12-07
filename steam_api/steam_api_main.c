/*
 * steam_api.dll
 *
 * Generated from steam_api.dll by winedump.
 *
 * DO NOT SUBMIT GENERATED DLLS FOR INCLUSION INTO WINE!
 *
 */

#define __WINESRC__
#include "config.h"
#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>
#include "windef.h"
#include "winbase.h"
#include "winnt.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(steam_api);

void stubFn(void) {
    FIXME("STUB\n");
}

#define STUB stubFn

#include "auto.incl"

void * libsteam_api(void) {    
    char *c;
    void *h;
    c = getenv("STEAM_API_SO");
    if (c == NULL) {
	ERR("environment variable 'STEAM_API_SO' not defined\n");
	return NULL;
    }
    h = dlopen(c, RTLD_NOW);
    return h;
}

void (*g_pSteamClientGameServer)(void);

static int table_filled = 0;

void fill_table(void *lib) {
    int i = 0;
    if (table_filled || lib==NULL) return;
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
    // fix g_pSteamClientGameServer export
    g_pSteamClientGameServer = dlsym(lib,"g_pSteamClientGameServer");
    if (g_pSteamClientGameServer==NULL) {
	TRACE("g_pSteamClientGameServer not found - set NULL\n");
    } else {
	TRACE("g_pSteamClientGameServer = %x\n",(unsigned int)g_pSteamClientGameServer);
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
            return TRUE;    
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDLL);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}
