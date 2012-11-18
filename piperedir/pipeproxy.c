#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


struct _fifo {
    int fd;
    char *name;
};

void 
    *fifo_open(char *name)
{
    struct _fifo *f = (struct _fifo*)malloc(sizeof(struct _fifo));
    if (f) {
	f->name = name;
	if(f->fd = open(name,O_RDWR)<0) {
	    free(f);
	    return NULL;
	}
    }
    return f;
}

int
    fifo_read(void *p, char *buffer, int len)
{
    struct _fifo *f = (struct _fifo*)p;
    return (int)read(f->fd,buffer,len);
}

int 
    fifo_write(void *p, char *buffer, int len)
{
    struct _fifo *f = (struct _fifo*)p;
    return (int)write(f->fd,buffer,len);
}

void
    fifo_close(void *p)
{
    struct _fifo *f = (struct _fifo*)p;
    close(f->fd);
    free(f);
}
