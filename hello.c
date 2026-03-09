#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>

#define SCREEN_MAX_X 1920
#define SCREEN_MAX_Y 1080

static int interval_ms = 16;
module_param(interval_ms, int, 0644);

static int start_x = 0;
static int start_y = 0;
module_param(start_x, int, 0644);
module_param(start_y, int, 0644);

static struct input_dev *virt_dev;
static struct delayed_work cursor_work;
static struct workqueue_struct *cursor_wq;

static int cur_x;
static int cur_y;

static void cursor_work_fn(struct work_struct *work)
{
    /* --- Update your movement logic here --- */
    cur_x += 2;
    cur_y += 2;
    if (cur_x >= SCREEN_MAX_X) cur_x = 0;
    if (cur_y >= SCREEN_MAX_Y) cur_y = 0;
    /* --------------------------------------- */

    input_report_abs(virt_dev, ABS_X, cur_x); // Move cursor to absolute position
    input_report_abs(virt_dev, ABS_Y, cur_y);
    input_report_key(virt_dev, BTN_TOUCH, 1);
    input_sync(virt_dev);

    input_report_key(virt_dev, BTN_TOUCH, 0);
    input_sync(virt_dev);

    /* Reschedule */
    queue_delayed_work(cursor_wq, &cursor_work, msecs_to_jiffies(interval_ms));
}

int init_module(void)
{
    int error;

    pr_info("cursor_move: loading\n");

    cur_x = start_x;
    cur_y = start_y;

    virt_dev = input_allocate_device();
    if (!virt_dev) {
        pr_err("cursor_move: failed to allocate input device\n");
        return -ENOMEM;
    }

    virt_dev->name       = "cursor_warp";
    virt_dev->phys       = "cursor_warp/input0";
    virt_dev->id.bustype = BUS_VIRTUAL;
    virt_dev->id.vendor  = 0x0000;
    virt_dev->id.product = 0x0000;
    virt_dev->id.version = 0x0001;

    virt_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
    input_set_abs_params(virt_dev, ABS_X, 0, SCREEN_MAX_X, 0, 0);
    input_set_abs_params(virt_dev, ABS_Y, 0, SCREEN_MAX_Y, 0, 0);
    input_set_abs_params(virt_dev, ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
    input_set_abs_params(virt_dev, ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
    setbit(INPUT_PROP_POINTER, virt_dev->propbit);
    virt_dev->keybit[BIT_WORD(BTN_TOUCH)] |= BIT_MASK(BTN_TOUCH);

    error = input_register_device(virt_dev);
    if (error) {
        pr_err("cursor_move: failed to register device: %d\n", error);
        input_free_device(virt_dev);
        return error;
    }

    /* Create a single-threaded workqueue */
    cursor_wq = create_singlethread_workqueue("cursor_wq");
    if (!cursor_wq) {
        pr_err("cursor_move: failed to create workqueue\n");
        input_unregister_device(virt_dev);
        return -ENOMEM;
    }

    INIT_DELAYED_WORK(&cursor_work, cursor_work_fn);
    queue_delayed_work(cursor_wq, &cursor_work, msecs_to_jiffies(interval_ms));

    pr_info("cursor_move: started, updating every %dms\n", interval_ms);
    return 0;
}

void cleanup_module(void)
{
    pr_info("cursor_move: unloading\n");
    cancel_delayed_work_sync(&cursor_work);  /* stop work before freeing */
    destroy_workqueue(cursor_wq);
    input_unregister_device(virt_dev);
}

MODULE_AUTHOR("Gary Li");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Moves cursor on a workqueue");