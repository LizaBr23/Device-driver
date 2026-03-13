/*
 * test_inject.c — simulate tablet button presses without hardware.
 *
 * Usage: sudo ./test_inject
 *
 * Opens /dev/tablet and uses the TABLET_INJECT_EVENT ioctl to send button
 * events directly into the driver. The driver will look up the binding and
 * fire the matching keypress, just as if the physical button was pressed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

/*
 * Include the shared header. In userspace the ioctl macros come from
 * <sys/ioctl.h> (already included above), which defines _IO/_IOR/_IOW
 * the same way the kernel does.
 */
#include "tablet.h"

#define DEVICE "/dev/tablet"

/* Human-readable description of each button's default binding */
static const char *button_labels[] = {
    "",                          /* slot 0 — unused */
    "Button  1 — Ctrl+Z  (Undo)",
    "Button  2 — Ctrl+C  (Copy)",
    "Button  3 — Ctrl+V  (Paste)",
    "Button  4 — Ctrl+S  (Save)",
    "Button  5 — Ctrl+Y  (Redo)",
    "Button  6 — Ctrl+-  (Zoom Out)",
    "Button  7 — Ctrl+=  (Zoom In)",
    "Button  8 — Caps Lock",
    "Button  9 — Volume Up",
    "Button 10 — Volume Down",
};

static void print_menu(void)
{
    int i;
    printf("\nTablet button tester\n");
    printf("-------------------------------------------\n");
    for (i = 1; i <= 10; i++)
        printf("  %2d: %s\n", i, button_labels[i]);
    printf("   0: quit\n\n");
}

int main(void)
{
    int fd;
    int choice;
    struct tablet_event ev;

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open " DEVICE);
        fprintf(stderr, "Make sure the driver is loaded and you have "
                        "permission (try sudo).\n");
        return 1;
    }

    print_menu();

    while (1) {
        printf("Button> ");
        fflush(stdout);

        if (scanf("%d", &choice) != 1)
            break;

        if (choice == 0)
            break;

        if (choice < 1 || choice > 10) {
            printf("Invalid choice. Enter 1–10 or 0 to quit.\n");
            continue;
        }

        /* Build a minimal event with just the button number set */
        ev.x          = 0;
        ev.y          = 0;
        ev.pressure   = 0;
        ev.button     = choice;
        ev.pen_button = 0;

        if (ioctl(fd, TABLET_INJECT_EVENT, &ev) < 0) {
            perror("ioctl TABLET_INJECT_EVENT");
        } else {
            printf("Injected: %s\n", button_labels[choice]);
        }
    }

    close(fd);
    return 0;
}
