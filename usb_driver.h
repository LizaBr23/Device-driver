//
// Created by david-bol12 on 3/6/26.
//

#ifndef DEVICEDRIVERPROJECT_USB_DRIVER_H
#define DEVICEDRIVERPROJECT_USB_DRIVER_H

struct tablet_usb_dev {
    struct usb_device *usb_dev;
    struct usb_interface *interface;
    unsigned char *buf;
    struct urb *urb;
    __u8 int_ep;
    size_t buf_size;
    struct tablet_event *tablet_data;
    struct input_dev *input_dev;
};

#endif //DEVICEDRIVERPROJECT_USB_DRIVER_H