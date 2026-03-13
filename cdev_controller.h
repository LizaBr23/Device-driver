//
// Created by david-bol12 on 3/7/26.
//

#ifndef DEVICEDRIVERPROJECT_CDEV_CONTROLLER_H
#define DEVICEDRIVERPROJECT_CDEV_CONTROLLER_H

#include "tablet.h"

int tablet_init(void);
void tablet_exit(void);

// Write an event into the driver buffer and fire its key binding (if any).
// Called by the USB driver when hardware sends data, and by the inject ioctl for testing.
int tablet_buffer_write(struct tablet_event *event);

// Read the next event from the driver buffer (non-blocking, returns -1 if empty).
int tablet_buffer_read(struct tablet_event *event);

#endif //DEVICEDRIVERPROJECT_CDEV_CONTROLLER_H