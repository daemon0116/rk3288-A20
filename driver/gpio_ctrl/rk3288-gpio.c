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
#define RK3288_REQ_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x20,int)//����gpio��
#define RK3288_REL_GPIO			_IOWR(RK3288_GPIO_MAGIC,0x21,int)//�ͷ�gpio��
#define RK3288_INP_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x22,int)//����gpio��Ϊ����
#define RK3288_OUT_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x23,int)//����gpio��Ϊ���
#define RK3288_SET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x24,int)//����gpio�ڵ�ֵ
#define RK3288_GET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x25,int)//��ȡgpio�ڵ�ֵ
#define RK3288_DISABEL_WATCHDOG _IOWR(RK3288_GPIO_MAGIC,0x26,int)//����ϵͳι��
#define RK3288_GPIO_MAXNR       0x26
#define RK3288_GPIO_MINNR       0x20
#define FEED_WATCHDOG_INTERVAL  3000//3000ms
#define WATCHDOG_GPIO           10 //GPIO0_B2 = 0 + 8 + 2
#define SYSTEM_FEED_WATCHDOG    //���ڿ��Ź���������������Ϊ��Ե����,���Բ���Ҫ����

#define RK3288_MAJOR 255  /*Ԥ��rk3288_major�����豸��*/
static int rk3288_major = 0;//RK3288_MAJOR;
struct class *rk3288_gpio_class;
static unsigned char feed_watchdog_flag = 1;//�ײ�������ʱι����־
/*
 * ���Ź��ṹ�嶨��
 */
struct rk3288_gpio_watchdog{
    struct delayed_work work;
};
struct rk3288_gpio_watchdog *rk3288_watchdog;
/*
    �ýṹ����Ҫ����ioctl����gpio�ڲ���
*/
struct UserData{
    unsigned int gpio;  //�û���gpio��
    unsigned int state; //�û���gpio��״̬
};

/*
    �豸�ṹ��,�˽ṹ����Է�װ�豸��ص�һЩ��Ϣ��
    �ź�����Ҳ���Է�װ�ڴ˽ṹ�У��������豸ģ��һ�㶼
    Ӧ�÷�װһ�������Ľṹ�壬���˽ṹ���б������ĳЩ
    ��Ա�������ַ��豸��˵�����Ǳ������struct cdev cdev
*/
struct rk3288_gpio_dev
{
    struct cdev cdev;
};

struct rk3288_gpio_dev *rk3288_gpio_devp; /*�豸�ṹ��ָ��*/
/* rk3288��ʱι������ */
static void rk3288_feed_watchdog(struct work_struct *work)
{
    //ι������
    //printk(KERN_NOTICE "%s:feed watchdog watchdog_state = %d\n",__func__,__gpio_get_value(WATCHDOG_GPIO));
    __gpio_set_value(WATCHDOG_GPIO,__gpio_get_value(WATCHDOG_GPIO)?0:1);

    //��ʱι���ĵ���
    if(feed_watchdog_flag)
    {
        schedule_delayed_work(&rk3288_watchdog->work,msecs_to_jiffies(FEED_WATCHDOG_INTERVAL));
    }
}
/* ��ʼ��rk3288���Ź�IO�� */
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
/*rk3288_gpio�ڵĴ�,�ϲ�Դ��豸����openʱ��ִ��*/
static int rk3288_gpio_open(struct inode *inode, struct file *filp)
{
    printk(KERN_NOTICE "======== rk3288_gpio_open ========\n");
    return 0;
}

/*rk3288_gpio�ڵĹر�,�ϲ�Դ��豸����closeʱ��ִ��*/
static int rk3288_gpio_close(struct inode *inode, struct file *filp)
{
    printk(KERN_NOTICE "======== rk3288_gpio_close ========\n");
    return 0;
}
#endif
/*rk3288_gpio�ڵĿ��Ʋ���,�ϲ�Դ��豸����ioctlʱ��ִ��*/
static long rk3288_gpio_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    int err = 0;
    struct UserData data;
    //char gpio_label[16] = {0};
    /* ����������Ч�� */
    if (_IOC_TYPE(cmd) != RK3288_GPIO_MAGIC)
        return -EINVAL;
    if (_IOC_NR(cmd) > RK3288_GPIO_MAXNR || _IOC_NR(cmd) < RK3288_GPIO_MINNR)
        return -EINVAL;

    /* �����������ͣ��������ռ��Ƿ���Է��� */
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
        case RK3288_REQ_GPIO://����gpio��
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
        case RK3288_REL_GPIO://�ͷ�gpio��
            //printk(KERN_NOTICE "======== release <gpio%d> operator! ========\n",data.gpio);
			gpio_free(data.gpio);
            break;
        case RK3288_INP_GPIO://����gpio��Ϊ����
            //printk(KERN_NOTICE "======== input <gpio%d>  operator! ========\n",data.gpio);
            if(gpio_direction_input(data.gpio))
            {
                printk(KERN_ERR "========   gpio%d set input is failed! ========\n",data.gpio);
                return -EBUSY;
            }
            break;
		case RK3288_OUT_GPIO://����gpio��Ϊ���
            //printk(KERN_NOTICE "======== output <gpio%d> operator ========\n",data.gpio);
            if(gpio_direction_output(data.gpio,data.state))
            {
                printk(KERN_ERR "========   gpio%d set output is failed! ========\n",data.gpio);
                return -EBUSY;
            }
            break;
        case RK3288_GET_GPIO://��ȡgpio�ڵ�ֵ
            //printk(KERN_NOTICE "======== get <gpio%d> value operator! ========\n",data.gpio);
            data.state = __gpio_get_value(data.gpio);
            //printk(KERN_NOTICE "======== get <gpio%d> value is %d ========\n",data.gpio,data.state);
            if(copy_to_user(argp, &data, sizeof(struct UserData)))
            {
                printk(KERN_ERR "======== gpio%d copy to user is failed! ========\n",data.gpio);
                return -EINVAL;
            }
            break;
        case RK3288_SET_GPIO://����gpio�ڵ�ֵ
            //printk(KERN_NOTICE "======== set <gpio%d> value operator ========\n",data.gpio);
            __gpio_set_value(data.gpio,data.state);
            break;
        #ifdef SYSTEM_FEED_WATCHDOG
        case RK3288_DISABEL_WATCHDOG://����ϵͳι��
            feed_watchdog_flag = data.state;//����ϵͳι��
            if(feed_watchdog_flag)//����ϵͳ���Ź�
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
/* �ļ������ṹ�壬�����Ѿ���������ṹ*/
static const struct file_operations rk3288_gpio_fops =
{
    .owner = THIS_MODULE,
    //.open = rk3288_gpio_open,
    //.release = rk3288_gpio_close,
    .unlocked_ioctl = rk3288_gpio_ioctl,
};

/*��ʼ����ע��cdev*/
static void rk3288_gpio_setup_cdev(struct rk3288_gpio_dev *dev, int index)
{

    int err, devno = MKDEV(rk3288_major, index);
    printk(KERN_NOTICE "======== rk3288_gpio_setup_cdev 1 ========\n");

    /*��ʼ��һ���ַ��豸���豸��֧�ֵĲ�����cdevdemo_fops��*/
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
    /*�����豸�ţ��������ʧ�ܲ��ö�̬���뷽ʽ*/
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
    /*��̬�����豸�ṹ���ڴ�*/
    rk3288_gpio_devp = kmalloc(sizeof(struct rk3288_gpio_dev), GFP_KERNEL);
    if(!rk3288_gpio_devp)  /*����ʧ��*/
    {
        ret = -ENOMEM;
        printk(KERN_NOTICE "Error add rk3288_gpio");
        goto fail_malloc;
    }

    memset(rk3288_gpio_devp,0,sizeof(struct rk3288_gpio_dev));
    printk(KERN_NOTICE "======== rk3288_gpio_init 3 ========\n");
    rk3288_gpio_setup_cdev(rk3288_gpio_devp, 0);

    /*���������Ǵ�����һ���������ͣ�����/sys/class������rk3288_gpioĿ¼
      ����Ļ���һ����Ҫ������ִ��device_create�����/dev/���Զ�����
      rk3288_gpio�豸�ڵ㡣����������ô˺����������ͨ���豸�ڵ�����豸
      ��Ҫ�ֶ�mknod�������豸�ڵ���ٷ��ʡ�*/
    rk3288_gpio_class = class_create(THIS_MODULE, "rk3288_gpio");
    device_create(rk3288_gpio_class, NULL, MKDEV(rk3288_major, 0), NULL, "rk3288_gpio");
    #ifdef SYSTEM_FEED_WATCHDOG
    //ι�����ݳ�ʼ��
    rk3288_watchdog = kmalloc(sizeof(struct rk3288_gpio_watchdog), GFP_KERNEL);
    if(!rk3288_watchdog)  /*����ʧ��*/
    {
        ret = -ENOMEM;
        kfree(rk3288_gpio_devp);
        printk(KERN_NOTICE "Error add rk3288 watchdog");
        goto fail_malloc;
    }
    INIT_DELAYED_WORK(&rk3288_watchdog->work,rk3288_feed_watchdog);
    rk3288_watchdog_gpio();//����Ҫ���������
    schedule_delayed_work(&rk3288_watchdog->work,msecs_to_jiffies(200));
    #endif
    printk(KERN_NOTICE "======== rk3288_gpio_init 4 ========\n");
    return 0;

    fail_malloc:
        unregister_chrdev_region(devno,1);
}

static void __exit rk3288_gpio_exit(void)    /*ģ��ж��*/
{
    #ifdef SYSTEM_FEED_WATCHDOG
    //�ͷ�watchdog gpio
    cancel_delayed_work(&rk3288_watchdog->work);
    gpio_free(WATCHDOG_GPIO);//���Ź�����Ҫ�ͷ�
    kfree(rk3288_watchdog);
    #endif
    printk(KERN_NOTICE "======== rk3288_gpio_exit 1 ========\n");
    cdev_del(&rk3288_gpio_devp->cdev);  /*ע��cdev*/
    kfree(rk3288_gpio_devp);       /*�ͷ��豸�ṹ���ڴ�*/
    printk(KERN_NOTICE "======== rk3288_gpio_exit 2 ========\n");
    unregister_chrdev_region(MKDEV(rk3288_major,0),1);    //�ͷ��豸��
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

