//
// Created by david-bol12 on 3/10/26.

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

struct button_array {
    short no_pressed;
    char buttons[7];
};

struct tablet_event {
    int x;
    int y;
    int pressure;
    struct button_array tab_buttons;
    int pen_button;
};

int main() {
    int fd = open("/dev/Tablet Character Device", O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    printf("Move the pen over the tablet...\n");



    while (1) {
        int test = 0;

        printf("Start Read");

        struct tablet_event ev;

        /* blocks until USB driver writes an event */
        usleep(2000);
        ssize_t n = read(fd, &ev, sizeof(ev));
        if (n < 0) { perror("read"); break; }

        printf("Done");

        printf("x=%-6d y=%-6d pressure=%-6d pen_button=%d\n",
               ev.x, ev.y, ev.pressure, ev.pen_button);
    }

    close(fd);
    return 0;
}