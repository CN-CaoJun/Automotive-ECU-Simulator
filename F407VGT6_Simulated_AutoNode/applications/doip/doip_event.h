#ifndef __DOIP_EVENT_H__
#define __DOIP_EVENT_H__

#include <pthread.h>

typedef void (*event_callback)(void *data);

typedef struct
{
    int fd;
    void *data;
    int active;
    pthread_t id;
    event_callback cb;
} event_io_t;


extern void event_io_init(event_io_t *watcher, int fd, void *data, event_callback cb);

extern int event_io_start(event_io_t *watcher);

extern int event_io_stop(event_io_t *watcher);

#endif  /* __DOIP_EVENT_H__ */