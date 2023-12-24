/*
 * kbleds.c − Blink keyboard leds until the module is unloaded.(modified for > 6.5)
 */
#include <linux/module.h>
#include <linux/configfs.h>
#include <linux/fs.h> 
#include <linux/tty.h>          /* For fg_console, MAX_NR_CONSOLES */
#include <linux/kd.h>           /* For KDSETLED */
#include <linux/vt.h>
#include <linux/console_struct.h>       /* For vc_cons */
#include <linux/vt_kern.h>
#include <linux/timer.h>
//SYS
#include <linux/printk.h>      
#include <linux/kobject.h>      
#include <linux/sysfs.h>       
#include <linux/init.h>         
#include <linux/string.h>       
 
MODULE_DESCRIPTION("Example module illustrating the use of Keyboard LEDs.");
MODULE_AUTHOR("Nikolai Popov");
MODULE_LICENSE("GPL");

struct timer_list my_timer;
struct tty_driver *my_driver;
static int _kbledstatus = 0;
//static int test = 7;// cod
#define BLINK_DELAY   HZ/5
#define ALL_LEDS_ON   0x07
#define RESTORE_LEDS  0xFF

/*
 * Function my_timer_func blinks the keyboard LEDs periodically by invoking
 * command KDSETLED of ioctl() on the keyboard driver. To learn more on virtual
 * terminal ioctl operations, please see file:
 *     /usr/src/linux/drivers/char/vt_ioctl.c, function vt_ioctl().
 *
 * The argument to KDSETLED is alternatively set to 7 (thus causing the led
 * mode to be set to LED_SHOW_IOCTL, and all the leds are lit) and to 0xFF
 * (any value above 7 switches back the led mode to LED_SHOW_FLAGS, thus
 * the LEDs reflect the actual keyboard status). To learn more on this,
 * please see file:
 *     /usr/src/linux/drivers/char/keyboard.c, function setledstate().
 *
 */

static struct kobject *example_kobject;
static int kbleds_test;
 
static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", kbleds_test);
}
 
static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
                        const char *buf, size_t count) {
    sscanf(buf, "%du", &kbleds_test);
    return count;
}
  
static struct kobj_attribute foo_attribute = __ATTR(kbleds_test, 0775, foo_show, foo_store);

static void my_timer_func(struct timer_list *ptr) {
    int *pstatus = &_kbledstatus;
    if (*pstatus == kbleds_test)
        *pstatus = RESTORE_LEDS;
    else
        *pstatus = kbleds_test;
    (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, *pstatus);
    my_timer.expires = jiffies + BLINK_DELAY;
    add_timer(&my_timer);
}

static int __init kbleds_init(void) {
    int i;
    printk(KERN_INFO "kbleds: loading\n");
    printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);
    for (i = 0; i < MAX_NR_CONSOLES; i++) {
        if (!vc_cons[i].d)
            break;
        printk(KERN_INFO "poet_atkm: console[%i/%i] #%i, tty %lx\n", i,
        MAX_NR_CONSOLES, vc_cons[i].d->vc_num, (unsigned long)vc_cons[i].d->port.tty);
    }
    printk(KERN_INFO "kbleds: finished scanning consoles\n");
    my_driver = vc_cons[fg_console].d->port.tty->driver;
    /*
    * Set up the LED blink timer the first time
    */    
    timer_setup(&my_timer, my_timer_func, 0);
    my_timer.expires = jiffies + BLINK_DELAY;
    add_timer(&my_timer);
    //return 0;
    //SYS
    int error = 0;
    pr_debug("Module initialized successfully \n");
    example_kobject = kobject_create_and_add("systest", kernel_kobj);
    if(!example_kobject)
        return -ENOMEM;
    error = sysfs_create_file(example_kobject, &foo_attribute.attr);
    if(error) {
        pr_debug("failed to create the foo file in /sys/kernel/systest \n");
    }
    return error;
}

static void __exit kbleds_cleanup(void) {
    printk(KERN_INFO "kbleds: unloading...\n");
    del_timer(&my_timer);
    (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);
    //SYS
    pr_debug ("Module un initialized successfully \n");
    kobject_put(example_kobject);
}

module_init(kbleds_init);
module_exit(kbleds_cleanup);

