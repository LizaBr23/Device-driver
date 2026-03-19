//
// Created by david-bol12 on 3/14/26.
//

#include "cdev_reader.h"

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

int init_reader() {
    int fd = open("/dev/Tablet Character Device", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }
    return fd;
}

void get_tablet_event(int fd, struct tablet_event* event_buf) {
    read(fd, event_buf, sizeof(struct tablet_event));
}

int set_binding(int fd, struct button_binding *binding) {
    return ioctl(fd, TABLET_SET_BINDING, binding);
}

void get_settings(int fd, struct tablet_settings *tablet_settings) {
    if (fd < 0) return;
    int ret = ioctl(fd, TABLET_GET_SETTING, tablet_settings);
    if (ret < 0) perror("ioctl failed");
}

void* cdev_read(void* reader_args) {
    struct reader_args *args = reader_args;
    int fd = args->fd;
    while (1) {
        get_tablet_event(fd, args->event_buf);
    }
}