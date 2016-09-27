/*
 * File: drivers/input/keyboard/sn7326_keys.c
 * Description:  keypad driver for sn7326
 *		 I2C QWERTY Keypad and IO Expander
 * Bugs: Enter bugs at http://blackfin.uclinux.org/
 *
 * Copyright (C) 2008-2010 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/errno.h>
#include <linux/pm.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <mach/sys_config.h>
#include <linux/init-input.h>

#include <linux/i2c/sn7326.h>

struct sn7326_kpad {
    spinlock_t irq_lock;
	struct i2c_client *client;
	struct input_dev *input;
	struct work_struct work;
    int    twi_id;
    int    gpio_int;        //中断引脚号
    int    gpio_irqnum;  //中断编号
    int    gpio_rst;        //复位引脚号
};
static struct workqueue_struct *sn7326_keypad_wq;
static int sn7326_twi_id = 1;//0:I2C0 1:I2C1 2:I2C2
static int SN7326_4_3_CODE[] = {// 4*3的键盘
    KEY_1,KEY_2,KEY_3,\
    KEY_4,KEY_5,KEY_6,\
    KEY_7,KEY_8,KEY_9,\
    KEY_ENTER,KEY_0,KEY_ESC
};

/**
 * keypad_fetch_sysconfig_para - get config info from sysconfig.fex file.
 * return value:
 *                    = 0; success;
 *                    < 0; err
 */
static int keypad_fetch_sysconfig_para(struct sn7326_kpad* kpad)
{
	int ret = -1;
	int device_used = -1;
	script_item_u val;
	script_item_value_type_e type;

	type = script_get_item("keypad_sn7326_para", "keypad_used", &val);

	if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
		printk("%s: type err  device_used = %d. \n", __func__, val.val);
		goto script_get_err;
	}
	device_used = val.val;

	if (1 == device_used) {
		type = script_get_item("keypad_sn7326_para", "keypad_twi_id", &val);
		if(SCIRPT_ITEM_VALUE_TYPE_INT != type){
			printk("%s: type err keypad_twi_id = %d. \n", __func__, val.val);
			goto script_get_err;
		}
		kpad->twi_id= val.val;

        type = script_get_item("keypad_sn7326_para", "keypad_int", &val);
        if(SCIRPT_ITEM_VALUE_TYPE_PIO != type){
            printk("%s: type err keypad_int = %d\n",__func__,val.gpio.gpio);
            goto script_get_err;
        }
        kpad->gpio_int= val.gpio.gpio;
        type = script_get_item("keypad_sn7326_para", "keypad_rst", &val);
        if(SCIRPT_ITEM_VALUE_TYPE_PIO != type){
            printk("%s: type err keypad_rst = %d\n",__func__,val.gpio.gpio);
            goto script_get_err;
        }
        kpad->gpio_rst = val.gpio.gpio;
		ret = 0;
        printk("sn7326_twi_id = %d,sn7326_int = %d,sn7326_rst = %d\n",kpad->twi_id,kpad->gpio_int,kpad->gpio_rst);

	} else {
		printk("%s: keypad_unused. \n",  __func__);
		ret = -1;
	}

	return ret;
script_get_err:
    printk("%s script_get_err\n",__func__);
	return ret;
}

static int sn7326_read(struct i2c_client *client, u8 reg)
{
	int ret = i2c_smbus_read_byte_data(client, reg);
	if (ret < 0)
		dev_err(&client->dev, "Read Error\n");
	return ret;
}

static int sn7326_write(struct i2c_client *client, u8 reg, u8 val)
{
	return i2c_smbus_write_byte_data(client, reg, val);
}
static int keyValue2Code(int keyValue)
{
    int ret = 0;
    switch(keyValue & SN7326_KEY_MASK){
        case NUMBER_ONE:
            ret = KEY_1;
            break;
        case NUMBER_TWO:
            ret = KEY_2;
            break;
        case NUMBER_THREE:
            ret = KEY_3;
            break;
        case NUMBER_FOUR:
            ret = KEY_4;
            break;
        case NUMBER_FIVE:
            ret = KEY_5;
            break;
        case NUMBER_SIX:
            ret = KEY_6;
            break;
        case NUMBER_SEVEN:
            ret = KEY_7;
            break;
        case NUMBER_EIGHT:
            ret = KEY_8;
            break;
        case NUMBER_NINE:
            ret = KEY_9;
            break;
        case NUMBER_SURE:
            ret = KEY_ENTER;
            break;
        case NUMBER_ZERO:
            ret = KEY_0;
            break;
        case NUMBER_CANCEL:
            ret = KEY_ESC;
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

static void sn7326_report_events(struct sn7326_kpad* kpad,struct i2c_client *client)
{
    int ret = 0,code = 0,value = 0;
    ret = sn7326_read(client,SN7326_STATE_REG);
    //printk("sn7326_read:status = %02x\n",ret);
    /*判断DN位的状态 DN=1 MUL KEY;DN=0 SINGLE KEY*/
    if(ret & (1<<SN7326_DN_BIT))//多键
    {
        code = keyValue2Code(ret);
        value = ret & (1<<SN7326_KS_BIT) ? 1:0;
        if(code > 0)
        {
            input_event(kpad->input, EV_KEY,code , value);
            input_sync(kpad->input);
            //printk("mul:code = %d,value = %d\n",code,value);
        }
        sn7326_report_events(kpad,client);
    }else{//一个按键
        code = keyValue2Code(ret);
        value = ret & (1<<SN7326_KS_BIT) ? 1:0;
        if(code > 0){
            input_event(kpad->input, EV_KEY, code, value);
            input_sync(kpad->input);
            //printk("simple:code = %d,value = %d\n",code,value);
        }
    }
}

static void sn7326_keypad_work_func(struct work_struct *work)
{
	struct sn7326_kpad *kpad = container_of(work,
						struct sn7326_kpad, work);
	struct i2c_client *client = kpad->client;

    sn7326_report_events(kpad,client);
    //printk("%s is execute!\n",__func__);
}

static s8 sn7326_request_input_dev(struct sn7326_kpad *kpad)
{
    s8 ret = -1,count = 0;
    kpad->input = input_allocate_device();
    if (kpad->input == NULL) {
        printk("Failed to allocate input device.");
        return -ENOMEM;
    }
    //kpad->input_dev->evbit[0] = BIT_MASK(EV_SYN) | BIT_MASK(EV_KEY);
    kpad->input->name = SN7326_DRV_NAME;
    kpad->input->phys = "sn7326_keypad/input0";
    kpad->input->id.bustype = BUS_I2C;
    kpad->input->id.vendor = 0x0001;
    kpad->input->id.product = 0x0001;
    kpad->input->id.version = 0x0001;
    ret = input_register_device(kpad->input);
    __set_bit(EV_SYN, kpad->input->evbit);
    __set_bit(EV_KEY, kpad->input->evbit);
    for(count = 0;count<sizeof(SN7326_4_3_CODE)/sizeof(SN7326_4_3_CODE[0]);count++)
	    input_set_capability(kpad->input, EV_KEY,SN7326_4_3_CODE[count]);
    if (ret) {
        printk("Register %s input device failed", kpad->input->name);
        input_free_device(kpad->input);
        return -ENODEV;
    }
    return 0;
}

static u32 sn7326_keypad_irq_handler(struct sn7326_kpad *kpad)
{
    //printk("%s interrupt is coming\n",__func__);
    queue_work(sn7326_keypad_wq, &kpad->work);
	return 0;
}
static s8 sn7326_keypad_gpio_setup(struct sn7326_kpad *kpad)
{
    if(!gpio_is_valid(kpad->gpio_rst)){
        printk("%s gpio_rst is invalid\n",__func__);
        return -1;
    }
    if(gpio_request(kpad->gpio_rst,NULL)){
        printk("%s gpio_rst is request is failed!\n",__func__);
        return -1;
    }
    if(gpio_direction_output(kpad->gpio_rst,1)){
        printk("%s gpio_rst set true is failed!\n",__func__);
        return -1;
    }
    return 0;
}
static s8 sn7326_keypad_request_irq(struct sn7326_kpad *kpad)
{
    kpad->gpio_irqnum= sw_gpio_irq_request(kpad->gpio_int,TRIG_EDGE_NEGATIVE,(peint_handle)sn7326_keypad_irq_handler,kpad);
    if (!kpad->gpio_irqnum) {
        printk( "i2c_driver_sn7326_probe: request irq failed\n");
        goto exit_irq_request_failed;
    }
    //ctp_set_int_port_rate(kpad->gpio_int, 1);
    //ctp_set_int_port_deb(kpad->gpio_int, 0x07);
    return 0;

exit_irq_request_failed:
    sw_gpio_irq_free(kpad->gpio_irqnum);
    return -1;
}


static int __devinit i2c_driver_sn7326_probe(struct i2c_client *client,
					const struct i2c_device_id *id)
{
    s32 ret = -1;
	struct sn7326_kpad *kpad;
    printk("%s is execute start!\n", __func__);
    printk("SN7326 Driver Version:%s\n", SN7326_DRIVER_VERSION);
    printk("SN7326 Driver build@%s,%s\n", __TIME__,__DATE__);
    printk("SN7326 I2C Address:0x%02x\n", client->addr);

    /* 测试i2c 是否正常 */
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        printk("I2C check functionality failed.");
        return -ENODEV;
    }
    /* 分配内存 */
    kpad = kzalloc(sizeof(*kpad), GFP_KERNEL);
    if (kpad == NULL) {
        printk("Alloc SN7326_KERNEL memory failed.");
        return -ENOMEM;
    }
    memset(kpad, 0, sizeof(*kpad));
    /* 获取参数信息 */
    if(keypad_fetch_sysconfig_para(kpad)){
        printk("%s:keypad_fetch_sysconfig_para  err.\n", __func__);
        kfree(kpad);
		return -1;
    }
    INIT_WORK(&kpad->work, sn7326_keypad_work_func);
    kpad->client = client;
    i2c_set_clientdata(client, kpad);

    sn7326_keypad_wq = create_singlethread_workqueue("sn7326_keypad_wq");
    if (!sn7326_keypad_wq) {
        printk(KERN_ALERT "Creat sn7326_keypad_wq workqueue failed.\n");
        goto exit_device_detect;
    }
    /* 输入子系统的申请 */
    ret = sn7326_request_input_dev(kpad);
    if (ret < 0) {
        printk("sn7326 request input dev failed");
        goto exit_device_detect;
    }
    /* gpio_rst 口的申请设置 */
    ret = sn7326_keypad_gpio_setup(kpad);
    if(ret < 0){
        printk("Setup gpio_rst is failed\n");
    }
    /* gpio_irq中断引脚的申请和设置 */
    ret = sn7326_keypad_request_irq(kpad);
    if (ret < 0) {
        printk("Request irq fail!\n");
        input_unregister_device(kpad->input);
        input_free_device(kpad->input);
        goto exit_device_detect;
    }

    //spin_lock_init(&kpad->irq_lock);
    printk("SN7326_CONFIG_REG = %02x\n",sn7326_read(client,SN7326_CONFIG_REG));
    printk("SN7326_STATE_REG = %02x\n",sn7326_read(client,SN7326_STATE_REG));
    printk("%s is execute end!\n", __func__);
    return 0;
exit_device_detect:
    i2c_set_clientdata(client, NULL);
    kfree(kpad);
    return ret;
}

static int __devexit i2c_driver_sn7326_remove(struct i2c_client *client)
{
    struct sn7326_kpad *kpad = i2c_get_clientdata(client);

	printk("%s is execute start!\n", __func__);
    sw_gpio_irq_free(kpad->gpio_irqnum);
    gpio_free(kpad->gpio_rst);
	flush_workqueue(sn7326_keypad_wq);
	destroy_workqueue(sn7326_keypad_wq);
  	i2c_set_clientdata(kpad->client, NULL);
	input_unregister_device(kpad->input);
	input_free_device(kpad->input);
	kfree(kpad);
    printk("%s is execute end!\n", __func__);
	return 0;
}

static int i2c_driver_sn7326_detect(struct i2c_client *client,struct i2c_board_info *info)
{
    struct i2c_adapter *adapter = client->adapter;
    int ret = 0;
    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)){
    	printk("i2c_check_functionality is failed!\n");
        return -ENODEV;
    }

    if(sn7326_twi_id == adapter->nr){
        printk("%s: addr = %x\n", __func__, client->addr);
        #if 1
        ret = strlcpy(info->type, SN7326_DRV_NAME, I2C_NAME_SIZE);
		return 0;
        #else
        ret = i2c_test(client);
        if(!ret){
    		printk("%s:I2C connection might be something wrong \n", __func__);
    		return -ENODEV;
    	}else{
	        strlcpy(info->type, SN7326_DRV_NAME, I2C_NAME_SIZE);
		    return 0;
        }
        #endif
	}else{
        return -ENODEV;
	}
    /* obtain device information */
    printk("%s is end!\n",__func__);
    return 0;
}

static const struct i2c_device_id i2c_driver_sn7326_id[] = {
	{ SN7326_DRV_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, i2c_driver_sn7326_id);
static const unsigned short sn7326_i2c[] = {SN7326_ADDR,I2C_CLIENT_END};
static struct i2c_driver i2c_driver_sn7326 = {
    .class          = I2C_CLASS_HWMON,
	.probe		= i2c_driver_sn7326_probe,
	.remove		= __devexit_p(i2c_driver_sn7326_remove),
	.id_table   = i2c_driver_sn7326_id,
	.driver		= {
		.name	= SN7326_DRV_NAME,
		.owner	= THIS_MODULE,
	},
	.detect     = i2c_driver_sn7326_detect,
	.address_list = sn7326_i2c,
};

static int  i2c_driver_sn7326_init(void)
{
    int ret = 0;
    printk("%s is execute!\n",__func__);
    ret = i2c_add_driver(&i2c_driver_sn7326);
    return ret;
}

static void  i2c_driver_sn7326_exit(void)
{
    printk("%s is execute!\n",__func__);
    i2c_del_driver(&i2c_driver_sn7326);
}
module_init(i2c_driver_sn7326_init);
module_exit(i2c_driver_sn7326_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("daemon <601097566@qq.com>");
MODULE_DESCRIPTION("sn7326 Keypad driver");
