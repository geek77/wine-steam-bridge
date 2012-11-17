#include <windows.h>
#include <stdio.h>
#include <string.h>

/*
 * use pipeproxy for reading and writing to target pipes
 */
 

struct _pipe {
    char *name;
    HANDLE w;
    void*  l;
} pipe_t;

/*

    pipe proc
*/
static const char WIN_CLIENT_SERVICE[] = L"\\\\.\\pipe\\steam-mstr-cs";
static const char WIN_BREAKPAD_HANDLER[] = L"\\\\.\\pipe\\SteamCrashHandler\\BreakpadServer";

void 
    listen_pipe(char *name, void (*onconnect)(HANDLE client,void *params), void *args)
{
    
    
    HANDLE p;
    for (;;) {
	p = CreateNamedPipe(
	    name,
	    PIPE_ACCESS_DUPLEX,
	    PIPE_TYPE_BYTE |
	    PIPE_READMODE_BYTE |
	    PIPE_WAIT,
	    PIPE_UNLIMITED_INSTANCES,
	    1024,
	    1024,
	    0,
	    NULL);
	
	if (p==INVALID_HANDLE_VALUE) break;
    
        if (ConnectNamedPipe(p,NULL)) {
	    onconnect(p,args);
	    continue;
	} else {
	    CloseHandle(p);
	}
    }
    
}



void 
    read_pipe(LPPARAM lpParam)
{
    char buffer[4096];
    pipe_t *pipe = (pipe_t)lpParam;
    DWORD wasRead;
    for(;;) {
	if (ReadFile(pipe->w,buffer,4096,&wasRead,NULL)) {
	    if (wasRead>0) {
		dump_data("%s,P->F",pipe->name,buffer,wasRead);
		fifo_write(pipe->l,buffer,wasRead);
	    } 
	} else {
	    printf("error reading pipe...\n");
	    break;
	}
    }
}

void
    read_fifo(LPPARAM lpParam)
{
    char buffer[4096];
    pipe_t *pipe = (pipe_t)lpParam;
    int wasRead;
    DWORD actualWritten;
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
    redirect_pipe_to_fifo(char *name, char *p, char *f)
{
    pipe_t *pipe = (pipe_t*)malloc(sizeof(pipe_t));
    if (pipe==NULL) return;
    
    void *fifo = fifo_open(f);
    if (fifo==NULL) {
	printf("can't open \"%s\", exiting\n", f);
    }
    
    
}

void 
    onclient(HANDLE p, void *params)
{
}

int _tmain(int argc, char *argv[]) {
    char *named_pipe;
    char *fifo;
    if (argc<2) {
	printf("Usage: piperedir.exe \"\\\\.\\pipe\\<pipe-to-listen>\" \"/path/to/fifo\"\n");
	return 0;
    }
    named_pipe = argv[1];
    fifo = argv[2];
    redirect_pipe_to_fifo(named_pipe,fifo);
}
