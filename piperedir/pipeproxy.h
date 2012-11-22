#ifndef PIPEPROXY_H
#define PIPEPROXY_H

void 
    *fifo_open(char *name);

int
    fifo_read(void *p, char *buffer, int len);

int 
    fifo_write(void *p, char *buffer, int len);

void
    fifo_close(void *p);


#endif
