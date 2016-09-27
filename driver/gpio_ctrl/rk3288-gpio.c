#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/timer.h>
#include <asm/atomic.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/ioctl.h>
#include <linux/jiffies.h>
#include <linux/delay.h>


#define	RK3288_GPIO_MAGIC		'k'
#define RK3288_REQ_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x20,int)//申请gpio口
#define RK3288_REL_GPIO			_IOWR(RK3288_GPIO_MAGIC,0x21,int)//释放gpio口
#define RK3288_INP_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x22,int)//设置gpio口为输入
#define RK3288_OUT_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x23,int)//设置gpio口为输出
#define RK3288_SET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x24,int)//设置gpio口的值
#define RK3288_GET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x25,int)//获取gpio口的值
#define RK3288_DISABEL_WATCHDOG _IOWR(RK3288_GPIO_MAGIC,0x26,int)//禁用系统喂狗
#define RK3288_GPIO_MAXNR       0x26
#define RK3288_GPIO_MINNR       0x20
#define FEED_WATCHDOG_INTERVAL  3000//3000ms
#define WATCHDOG_GPIO           10 //GPIO0_B2 = 0 + 8 + 2
#define SYSTEM_FEED_WATCHDOG    //由于看门狗定制器可以设置为边缘触发,所以不需要定义

#define RK3288_MAJOR 255  /*预设rk3288_major的主设备号*/
static int rk3288_major = 0;//RK3288_MAJOR;
struct class *rk3288_gpio_class;
static unsigned char feed_watchdog_flag = 1;//底层驱动定时喂狗标志
/*
 * 看门狗结构体定义
 */
struct rk3288_gpio_watchdog{
    struct delayed_work work;
};
struct rk3288_gpio_watchdog *rk3288_watchdog;
/*
    该结构体主要用于ioctl控制gpio口操作
*/
struct UserData{
    unsigned int gpio;  //用户的gpio口
    unsigned int state; //用户的gpio口状态
};

/*
    设备结构体,此结构体可以封装设备相关的一些信息等
    信号量等也可以封装在此结构中，后续的设备模块一般都
    应该封装一个这样的结构体，但此结构体中必须包含某些
    成员，对于字符设备来说，我们必须包含struct cdev cdev
*/
struct rk3288_gpio_dev
{
    struct cdev cdev;
};

struct rk3288_gpio_dev *rk3288_gpio_devp; /*设备结构体指针*/
/* rk3288定时喂狗任务 */
static void rk3288_feed_watchdog(struct work_struct *work)
{
    //喂狗操作
    //printk(KERN_NOTICE "%s:feed watchdog watchdog_state = %d\n",__func__,__gpio_get_value(WATCHDOG_GPIO));
    __gpio_set_value(WATCHDOG_GPIO,__gpio_get_value(WATCHDOG_GPIO)?0:1);

    //定时喂狗的调度
    if(feed_watchdog_flag)
    {
        schedule_delayed_work(&rk3288_watchdog->work,msecs_to_jiffies(FEED_WATCHDOG_INTERVAL));
    }
}
/* 初始化rk3288看门狗IO口 */
static int rk3288_watchdog_gpio(void)
{
    int err = -EINVAL;
    err = gpio_request(WATCHDOG_GPIO, NULL);
    if(err){
        printk(KERN_NOTICE "======== request watchdog gpio failed,That doesn't matter!========\n",err);
    }
    err = gpio_direction_output(WATCHDOG_GPIO,0);
    if(err){
        printk(KERN_NOTICE "======== set watchdog gpio direction and value failed,That doesn't matter!========\n",err);
    }
    return 0;
}

#if 0
/*rk3288_gpio口的打开,上层对此设备调用open时会执行*/
static int rk3288_gpio_open(struct inode *inode, struct file *filp)
{
    printk(KERN_NOTICE "======== rk3288_gpio_open ========\n");
    return 0;
}

/*rk3288_gpio口的关闭,上层对此设备调用close时会执行*/
static int rk3288_gpio_close(struct inode *inode, struct file *filp)
{
    printk(KERN_NOTICE "======== rk3288_gpio_close ========\n");
    return 0;
}
#endif
/*rk3288_gpio口的控制操作,上层对此设备调用ioctl时会执行*/
static long rk3288_gpio_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    int err = 0;
    struct UserData data;
    //char gpio_label[16] = {0};
    /* 检测命令的有效性 */
    if (_IOC_TYPE(cmd) != RK3288_GPIO_MAGIC)
        return -EINVAL;
    if (_IOC_NR(cmd) > RK3288_GPIO_MAXNR || _IOC_NR(cmd) < RK3288_GPIO_MINNR)
        return -EINVAL;

    /* 根据命令类型，检测参数空间是否可以访问 */
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
    if (err)
        return -EFAULT;
    //printk(KERN_NOTICE "======== rk3288_gpio_ioctl ========\n");
    if(copy_from_user(&data,argp,sizeof(struct UserData)))
    {
        printk(KERN_ERR "input argument is invalid!\n");
        return -EINVAL;
    }
	//printk(KERN_NOTICE "======== cmd = %d ========\n",_IOC_NR(cmd));
    switch(cmd)
    {
        case RK3288_REQ_GPIO://申请gpio口
            //printk(KERN_NOTICE "======== request <gpio%d> operator! ========\r\n",data.gpio);
            if(!gpio_is_valid(data.gpio))
            {
                printk(KERN_ERR "========   gpio%d is invalid! ========\n",data.gpio);
                return -EBUSY;
            }
            //sprintf(gpio_label,"rk3288-gpio%d",data.gpio);
            if(gpio_request(data.gpio, NULL))
            {
                printk(KERN_ERR "========   gpio%d request is failed! ========\n",data.gpio);
                return -EBUSY;
            }
            break;
        case RK3288_REL_GPIO://释放gpio口
            //printk(KERN_NOTICE "======== release <gpio%d> operator! ========\n",data.gpio);
			gpio_free(data.gpio);
            break;
        case RK3288_INP_GPIO://设置gpio口为输入
            //printk(KERN_NOTICE "======== input <gpio%d>  operator! ========\n",data.gpio);
            if(gpio_direction_input(data.gpio))
            {
                printk(KERN_ERR "========   gpio%d set input is failed! ========\n",data.gpio);
                return -EBUSY;
            }
            break;
		case RK3288_OUT_GPIO://设置gpio口为输出
            //printk(KERN_NOTICE "======== output <gpio%d> operator ========\n",data.gpio);
            if(gpio_direction_output(data.gpio,data.state))
            {
                printk(KERN_ERR "========   gpio%d set output is failed! ========\n",data.gpio);
                return -EBUSY;
            }
            break;
        case RK3288_GET_GPIO://获取gpio口的值
            //printk(KERN_NOTICE "======== get <gpio%d> value operator! ========\n",data.gpio);
            data.state = __gpio_get_value(data.gpio);
            //printk(KERN_NOTICE "======== get <gpio%d> value is %d ========\n",data.gpio,data.state);
            if(copy_to_user(argp, &data, sizeof(struct UserData)))
            {
                printk(KERN_ERR "======== gpio%d copy to user is failed! ========\n",data.gpio);
                return -EINVAL;
            }
            break;
        case RK3288_SET_GPIO://设置gpio口的值
            //printk(KERN_NOTICE "======== set <gpio%d> value operator ========\n",data.gpio);
            __gpio_set_value(data.gpio,data.state);
            break;
        #ifdef SYSTEM_FEED_WATCHDOG
        case RK3288_DISABEL_WATCHDOG://禁用系统喂狗
            feed_watchdog_flag = data.state;//禁用系统喂狗
            if(feed_watchdog_flag)//开启系统看门狗
                schedule_delayed_work(&rk3288_watchdog->work,msecs_to_jiffies(200));
            else
                cancel_delayed_work(&rk3288_watchdog->work);
            break;
        #endif
        default:
			printk(KERN_ERR "======== Erorr Command ========\n");
            return -EINVAL;
    }
    return 0;
}
/* 文件操作结构体，文中已经讲过这个结构*/
static const struct file_operations rk3288_gpio_fops =
{
    .owner = THIS_MODULE,
    //.open = rk3288_gpio_open,
    //.release = rk3288_gpio_close,
    .unlocked_ioctl = rk3288_gpio_ioctl,
};

/*初始化并注册cdev*/
static void rk3288_gpio_setup_cdev(struct rk3288_gpio_dev *dev, int index)
{

    int err, devno = MKDEV(rk3288_major, index);
    printk(KERN_NOTICE "======== rk3288_gpio_setup_cdev 1 ========\n");

    /*初始化一个字符设备，设备所支持的操作在cdevdemo_fops中*/
    cdev_init(&dev->cdev, &rk3288_gpio_fops);
    printk(KERN_NOTICE "======== rk3288_gpio_setup_cdev 2 ========\n");
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &rk3288_gpio_fops;
    printk(KERN_NOTICE "======== rk3288_gpio_setup_cdev 3 ========\n");
    err = cdev_add(&dev->cdev, devno, 1);
    printk(KERN_NOTICE "======== rk3288_gpio_setup_cdev 4 ========\n");
    if(err)
    {
        printk(KERN_NOTICE "Error %d add rk3288_gpio %d", err, index);
    }
}

static int __init rk3288_gpio_init(void)
{

    int ret;
    dev_t devno = MKDEV(rk3288_major, 0);
    printk(KERN_NOTICE "======== rk3288_gpio_init ========\n");
    /*申请设备号，如果申请失败采用动态申请方式*/
    if(rk3288_major)
    {
        printk(KERN_NOTICE "======== rk3288_gpio_init 1 ========\n");
        ret = register_chrdev_region(devno, 1, "rk3288_gpio");
    }else
    {
        printk(KERN_NOTICE "======== rk3288_gpio_init 2 ========\n");
        ret = alloc_chrdev_region(&devno,0,1,"rk3288_gpio");
        rk3288_major = MAJOR(devno);
    }
    if(ret < 0)
    {
        printk(KERN_NOTICE "======== rk3288_gpio_init 3 ========\n");
        goto fail_malloc;
    }
    /*动态申请设备结构体内存*/
    rk3288_gpio_devp = kmalloc(sizeof(struct rk3288_gpio_dev), GFP_KERNEL);
    if(!rk3288_gpio_devp)  /*申请失败*/
    {
        ret = -ENOMEM;
        printk(KERN_NOTICE "Error add rk3288_gpio");
        goto fail_malloc;
    }

    memset(rk3288_gpio_devp,0,sizeof(struct rk3288_gpio_dev));
    printk(KERN_NOTICE "======== rk3288_gpio_init 3 ========\n");
    rk3288_gpio_setup_cdev(rk3288_gpio_devp, 0);

    /*下面两行是创建了一个总线类型，会在/sys/class下生成rk3288_gpio目录
      这里的还有一个主要作用是执行device_create后会在/dev/下自动生成
      rk3288_gpio设备节点。而如果不调用此函数，如果想通过设备节点访问设备
      需要手动mknod来创建设备节点后再访问。*/
    rk3288_gpio_class = class_create(THIS_MODULE, "rk3288_gpio");
    device_create(rk3288_gpio_class, NULL, MKDEV(rk3288_major, 0), NULL, "rk3288_gpio");
    #ifdef SYSTEM_FEED_WATCHDOG
    //喂狗数据初始化
    rk3288_watchdog = kmalloc(sizeof(struct rk3288_gpio_watchdog), GFP_KERNEL);
    if(!rk3288_watchdog)  /*申请失败*/
    {
        ret = -ENOMEM;
        kfree(rk3288_gpio_devp);
        printk(KERN_NOTICE "Error add rk3288 watchdog");
        goto fail_malloc;
    }
    INIT_DELAYED_WORK(&rk3288_watchdog->work,rk3288_feed_watchdog);
    rk3288_watchdog_gpio();//不需要申请该引脚
    schedule_delayed_work(&rk3288_watchdog->work,msecs_to_jiffies(200));
    #endif
    printk(KERN_NOTICE "======== rk3288_gpio_init 4 ========\n");
    return 0;

    fail_malloc:
        unregister_chrdev_region(devno,1);
}

static void __exit rk3288_gpio_exit(void)    /*模块卸载*/
{
    #ifdef SYSTEM_FEED_WATCHDOG
    //释放watchdog gpio
    cancel_delayed_work(&rk3288_watchdog->work);
    gpio_free(WATCHDOG_GPIO);//看门狗不需要释放
    kfree(rk3288_watchdog);
    #endif
    printk(KERN_NOTICE "======== rk3288_gpio_exit 1 ========\n");
    cdev_del(&rk3288_gpio_devp->cdev);  /*注销cdev*/
    kfree(rk3288_gpio_devp);       /*释放设备结构体内存*/
    printk(KERN_NOTICE "======== rk3288_gpio_exit 2 ========\n");
    unregister_chrdev_region(MKDEV(rk3288_major,0),1);    //释放设备号
    printk(KERN_NOTICE "======== rk3288_gpio_exit 3 ========\n");
    device_destroy(rk3288_gpio_class, MKDEV(rk3288_major, 0));
    printk(KERN_NOTICE "======== rk3288_gpio_exit 4 ========\n");
    class_destroy(rk3288_gpio_class);
    printk(KERN_NOTICE "======== rk3288_gpio_exit ========\n");
}

module_param(rk3288_major, int, S_IRUGO);
module_init(rk3288_gpio_init);
module_exit(rk3288_gpio_exit);
/* Module information */
MODULE_DESCRIPTION("ROCKCHIP RK3288-GPIO CONTROL");
MODULE_LICENSE("GPL");
//MODULE_AUTHOR("Aebell Daemon");

