
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
// #include <windef.h>
// #include <winbase.h>
// #include <wincon.h>
// #include <winnls.h>
// #include <winuser.h>


#include "pipeproxy.h"

// WINE_DEFAULT_DEBUG_CHANNEL(piperedir);

/*
 * use pipeproxy for reading and writing to target pipes
 */
 
typedef struct _params {
    char *pipe;
    char *fifo;
} params_t;

typedef struct _pipe {
    char *name;
    HANDLE w;
    void*  l;
} pipe_t;



#define return_with_error(...) fprintf(stdout,"error: %s at %s : ",__FUNCTION__,__FILE__); fprintf(stdout,__VA_ARGS__); fflush(stdout); return
#define WINE_TRACE(...) fprintf(stdout,"trace: %s at %s : ",__FUNCTION__,__FILE__); fprintf(stdout,__VA_ARGS__);  fflush(stdout)
#define strnlen(a,b) strlen(a)
/*

    pipe proc
*/
static const wchar_t WIN_CLIENT_SERVICE[] = L"\\\\.\\pipe\\steam-mstr-cs";
static const wchar_t WIN_BREAKPAD_HANDLER[] = L"\\\\.\\pipe\\SteamCrashHandler\\BreakpadServer";

char* last_error(void)
{
    char* cstr;
    FormatMessageA(
	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	NULL,
	GetLastError(),
	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
	(LPSTR) cstr,
	0,
	NULL);    
    return cstr;
}

LPWSTR tomb(char *s) {
    int size = 0;
     LPWSTR result = NULL;
     int required = MultiByteToWideChar(CP_UTF8, 0,  s, strnlen(s,1024), result,0);
     if (required>0) {
        size = required*sizeof(WCHAR)+2;
        result = (LPWSTR)malloc(size);
        memset(result,0,size);
        required = MultiByteToWideChar(CP_UTF8,0,s,strnlen(s,1024), result, required);
        if (required>0) return result;        
     }
     return NULL;
};

void 
    listen_pipe(char *name, void (*onconnect)(HANDLE client,void *params), void *args)
{
    LPWSTR _name = tomb(name);
    WINE_TRACE("begin '%s'->'%s'\n", name, (char*)_name);    
    
    HANDLE p;
    for (;;) {
	// hangs here
	p = CreateNamedPipeW(
	    _name,
	    PIPE_ACCESS_DUPLEX,
	    PIPE_TYPE_BYTE | PIPE_WAIT,
	    1,
	    1024,
	    1024,
	    0,
	    NULL);
	WINE_TRACE("pipe \"%s\" created\n",name);
	if (p==INVALID_HANDLE_VALUE) {
	    WINE_TRACE("could not create pipe \"%s\", error: %s\n",name,last_error());
	    break;
	}
	
	WINE_TRACE("pipe \"%s\" created, listen\n",name);
        if (ConnectNamedPipe(p,NULL)) {        
	    onconnect(p,args);
	    continue;
	} else {
	    CloseHandle(p);
	}
    }
    WINE_TRACE("exit...\n");
}

void dump_data(const char *s, char* pn, char *data, int len) {}

DWORD WINAPI
    read_pipe(LPVOID lpParam)
{
    char buffer[4096];
    pipe_t *pipe = (pipe_t*)lpParam;
    DWORD wasRead;
    WINE_TRACE("\n");
    for(;;) {
	if (ReadFile(pipe->w,buffer,4096,&wasRead,NULL)) {
	    if (wasRead>0) {
		dump_data("P->F",pipe->name,buffer,wasRead);
		fifo_write(pipe->l,buffer,wasRead);
	    } 
	} else {
	    printf("error reading pipe...\n");
	    break;
	}
    }
}

DWORD WINAPI
    read_fifo(LPVOID lpParam)
{
    char buffer[4096];
    pipe_t *pipe = (pipe_t*)lpParam;
    int wasRead;
    DWORD actualWritten;
    WINE_TRACE("\n");
    for(;;) {
	wasRead = fifo_read(pipe->l,buffer,4096);
	if (wasRead<0) {
	    printf("error reading fifo...\n");
	    break;
	}
	dump_data("%s, F->P",pipe->name,buffer,wasRead);
	if (!WriteFile(pipe->w,buffer,wasRead,&actualWritten,NULL)) {
	    printf("error writing to pipe...\n");
	    break;
	}
	if (wasRead!=actualWritten) {
	    printf("error writing data to pipe\n");
	    break;
	}
    }
}


void 
    onclient(HANDLE p, void *params)
{
    HANDLE hThreadP2F = NULL;
    DWORD  tidP2F = 0;
    DWORD  tidF2P = 0;
    HANDLE hThreadF2P = NULL;
    params_t *prms;
    void *f;    
    pipe_t *redirect = (pipe_t*)malloc(sizeof(pipe_t));
    WINE_TRACE("\n");
    prms = (params_t*)params;
    f = fifo_open(prms->fifo);
    if (f == NULL) return_with_error("could not open fifo '%s'\n", prms->fifo);
    redirect->w = p;
    redirect->l = f;    
    hThreadP2F = CreateThread(NULL,0,read_pipe,(LPVOID)redirect,0,&tidP2F);
    if (hThreadP2F==NULL) return_with_error("could not create read_pipe thread\n");
    hThreadF2P = CreateThread(NULL,0,read_fifo,(LPVOID)redirect,0,&tidF2P);
    if (hThreadF2P==NULL) return_with_error("could not create read_fifo thread\n");
}

void
    redirect_pipe_to_fifo(params_t *params)
{
    WINE_TRACE("\n");
    listen_pipe(params->pipe,onclient,params);
}

/*
    create named pipe
	on client connected - start threads:
	    read_pipe
	    read_fifo
	
	thread:
	    

 */

#define MAX_CONSOLE_LINES 500 
// #define _O_TEXT 0 
void RedirectIOToConsole()

{
    int hConHandle;
    long lStdHandle;

    CONSOLE_SCREEN_BUFFER_INFO coninfo;

    FILE *fp;

    // allocate a console for this app

    AllocConsole();

    // set the screen buffer to be big enough to let us scroll text

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

    coninfo.dwSize.Y = MAX_CONSOLE_LINES;

    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

// redirect unbuffered STDOUT to the console

    lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);

    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

    fp = _fdopen( hConHandle, "w" );

    *stdout = *fp;

    setvbuf( stdout, NULL, _IONBF, 0 );

    // redirect unbuffered STDIN to the console

    lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);

    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

    fp = _fdopen( hConHandle, "r" );

    *stdin = *fp;

    setvbuf( stdin, NULL, _IONBF, 0 );

// redirect unbuffered STDERR to the console

    lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);

    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

    fp = _fdopen( hConHandle, "w" );

    *stderr = *fp;

    setvbuf( stderr, NULL, _IONBF, 0 );



}

int main(int argc, char *argv[]) {
    // RedirectIOToConsole();
    fprintf(stderr,"hello,world\n");
    WINE_TRACE("\n");
    params_t params;
    if (argc<2) {
	printf("Usage: piperedir.exe \"\\\\.\\pipe\\<pipe-to-listen>\" \"/path/to/fifo\"\n");
	return 0;
    }
    params.pipe = argv[1];
    params.fifo = argv[2];
    redirect_pipe_to_fifo(&params);
    return 0;
}
