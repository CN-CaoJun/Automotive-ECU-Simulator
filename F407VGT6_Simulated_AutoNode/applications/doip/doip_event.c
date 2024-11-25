#include "doip_event.h"
#include "doip_utils.h"

#include <sys/select.h>
#include <sys/time.h>

void *thread_function(void *arg) {
    event_io_t *watcher = (event_io_t *)arg;
    fd_set read_fds;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100;

    while (watcher->active) {
        FD_ZERO(&read_fds);
        FD_SET(watcher->fd, &read_fds);
        if (select(watcher->fd+1, &read_fds, NULL, NULL, &timeout) < 0) {
            logd("Failed to select");
            break;
        }
        if (FD_ISSET(watcher->fd, &read_fds)) {
            watcher->cb(arg);
        }
    }
    return NULL;
}

void event_io_init(event_io_t *watcher, int fd, void *data, event_callback cb)
{
    watcher->fd = fd;
    watcher->data = data;
    watcher->cb = cb;
}

int event_io_start(event_io_t *watcher)
{
    if (pthread_create(&(watcher->id), NULL, thread_function, watcher) != 0) {
        logd("io event start failed!");
        return -1;
    }
    watcher->active = 1;
    return 0;
}

int event_io_stop(event_io_t *watcher)
{
    watcher->active = 0;
    if (pthread_join(watcher->id, NULL) != 0) {
        logd("io event stop failed!");
        return -1;
    }
    return 0;
}
