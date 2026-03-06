/*                                                     
 * $Id: hello.c,v 1.5 2004/10/26 03:32:21 corbet Exp $ 
 */                                                    
#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>

MODULE_LICENSE("Dual BSD/GPL");

#define VENDOR_ID  0x28bd  // example
#define PRODUCT_ID 0x0937

char get_button_val(unsigned char code) {
	switch (code) {
		case 0x05: return 1;
		case 0x08: return 2;
		case 0x04: return 3;
		case 0x2C: return 4;
		case 0x19: return 5;
		case 0x16: return 6;
		case 0x1D: return 7;
		case 0x11: return 8;
		case 0x57: return 9;
		case 0x56: return 10;
		default: return 0; // not found
	}
}

struct button_array {
	short no_pressed;
	char* buttons;
};



static const struct usb_device_id tablet_table[] = {
	{ USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
	{ }
};

MODULE_DEVICE_TABLE(usb, tablet_table);


struct tablet_dev {
	struct usb_device *udev;
	struct usb_interface *interface;
	unsigned char *buf;
	struct urb *urb;
	__u8 int_ep;
	size_t buf_size;
};

void get_buttons_pressed(unsigned char* data, u32 length, struct button_array* location) {
	int index = 0;
	char val;
	struct button_array* buttons = location;

	val = get_button_val(data[1]);
	if (val != 0 && val != 1) { // 8 causes the first byte to be 05 which is the code for button 1
		buttons->buttons[index] = val;
		index++;
		buttons->no_pressed++;
	}

	for (int i = 2; i < length; i++) {
		val = get_button_val(data[i]);
		if (val != 0) {
			buttons->buttons[index] = val;
			index++;
			buttons->no_pressed++;
		}
	}
}

static void tablet_irq_callback(struct urb *urb)
{
	struct tablet_dev *dev = urb->context;
	int i;

	if (urb->status == 0) {
		printk(KERN_INFO "tablet data:");
		if (dev->buf[0] == 6) {
			char buttons[7];
			struct button_array pressed = {
				0,
				buttons
			};
			get_buttons_pressed(dev->buf, urb->actual_length, &pressed);
			printk(KERN_ALERT "Button(s) ");
			if (pressed.no_pressed == 0) {
				printk(KERN_ALERT "Released \n");
			} else {
				for (i = 0; i < pressed.no_pressed; i++) {
					printk(KERN_ALERT "%d, ", pressed.buttons[i]);
				}
				printk(KERN_ALERT "Pressed \n");
			}
		}
		for (i = 0; i < urb->actual_length; i++)
			printk(KERN_CONT " %02x", dev->buf[i]);

		printk(KERN_CONT "\n");
	}

	/* Resubmit URB to keep receiving data */
	usb_submit_urb(urb, GFP_ATOMIC);
}

static int tablet_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct tablet_dev *dev;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	int i;

	iface_desc = interface->cur_altsetting;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	/* Find interrupt IN endpoint */
	for (int i = 0; i < iface_desc->desc.bNumEndpoints; i++) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (usb_endpoint_is_int_in(endpoint)) {
			dev->int_ep = endpoint->bEndpointAddress;
			dev->buf_size = usb_endpoint_maxp(endpoint);
			break;
		}
	}

	if (!dev->int_ep) {
		printk(KERN_ERR "No interrupt IN endpoint found\n");
		goto error;
	}

	dev->buf = kmalloc(dev->buf_size, GFP_KERNEL);
	dev->urb = usb_alloc_urb(0, GFP_KERNEL);

	usb_fill_int_urb(
		dev->urb,
		dev->udev,
		usb_rcvintpipe(dev->udev, dev->int_ep),
		dev->buf,
		dev->buf_size,
		tablet_irq_callback,
		dev,
		endpoint->bInterval
	);

	usb_set_intfdata(interface, dev);
	usb_submit_urb(dev->urb, GFP_KERNEL);

	printk(KERN_INFO "Raw tablet driver bound\n");
	return 0;

	error:
		kfree(dev);
	return -ENODEV;
}

static void tablet_disconnect(struct usb_interface *interface)
{
	struct tablet_dev *dev = usb_get_intfdata(interface);

	usb_kill_urb(dev->urb);
	usb_free_urb(dev->urb);
	kfree(dev->buf);
	usb_put_dev(dev->udev);
	kfree(dev);

	printk(KERN_INFO "Raw tablet driver disconnected\n");
}

static struct usb_driver tablet_driver = {
	.name = "raw_usb_tablet",
	.probe = tablet_probe,
	.disconnect = tablet_disconnect,
	.id_table = tablet_table,
};

module_usb_driver(tablet_driver);