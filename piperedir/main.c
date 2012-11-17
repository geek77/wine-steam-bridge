#include <windows.h>
#include <stdio.h>
#include <string.h>

struct _entry {
    char *buffer;
    int   size;
    struct _entry *next;
};

struct _queue {
    HANDLE mutex;
    HANDLE event;
    struct _entry *first;
    struct _entry *last;
};

typedef struct _queue queue_t;
typedef struct _entry entry_t;
queue_t * 
    new_queue() 
{
    queue_t *q = (queue_t*)malloc(sizeof(queue_t));
    if (q==NULL) return NULL;
    q->mutex = CreateMutex(NULL, FALSE, NULL);
    if (q->mutex==NULL) goto error;    
    q->event = CreateEvent(NULL, TRUE, FALSE, TEXT("update"));
    if (q->event==NULL) goto error;
    q->first = NULL;
    q->last = NULL;
    return q;
    error:
	if (q->event!=NULL) CloseEvent(q->event);
	free(q);
	return NULL;
};

entry_t *
    new_entry(char *data, int len)
{
    entry_t *e = (entry_t*)malloc(sizeof(entry_t));
    if (e==NULL) return NULL;
    e->buffer = (char*)malloc(len);
    if (e->buffer==NULL) {
	free(e);
	return NULL;
    }
    memcpy(e->buffer,data,len);
    e->next = NULL;
    return;
}

void 
    entry_free(entry_t *e)
{
    if (e==NULL) return;
    if (e->buffer!=NULL) free(e->buffer);
    free(e);
}

int
    push_to_queue(queue_t *q, char *data, int len)
{
    entry_t *e;
    DWORD wait;
    if (q==NULL||data==NULL||len==0) return -1;
    e = new_entry(data,len);
    if (e==NULL) return -1;
    wait = WaitForSingleObject(q->mutex,INFINITE);
    if (WAIT_ABANDONED==wait) return -1;
    
    if (q->first==NULL) {
	q->first = e;
	q->last = e;
    } else {
	q->last = e;
    }
    
    ReleaseMutex(q->mutex);
    if (!SetEvent(q->event))
	return -1;	
}

int
    fetch_from_queue(queue_t *q, char **data, int *len)
{
    entry_t *e;
    DWORD wait;
    if (q==NULL) return -1;
    wait = WaitForSingleObject(q->mutex,INFINITE);
    if (WAIT_ABANDONED==wait) return -1;
    
    if (q->first == NULL) {
	ReleaseMutex(q->mutex);
	return -1;
    }
    e = q->first;
    if (e->next == NULL) {
	q->first = NULL;
	q->last = NULL;
    }
    ReleaseMutex(q->mutex);
    *data = e->buffer;
    *len = e->size;
    free(e);
    return 0;
}

/*

    pipe proc
*/
static const char WIN_CLIENT_SERVICE[] = L"\\\\.\\pipe\\steam-mstr-cs";
static const char WIN_BREAKPAD_HANDLER[] = L"\\\\.\\pipe\\SteamCrashHandler\\BreakpadServer";

void 
    listen_pipe(char *name, void (*onconnect)(HANDLE client))
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
	    onconnect(p);
	    continue;
	} else {
	    CloseHandle(p);
	}
    }
    
}












*/