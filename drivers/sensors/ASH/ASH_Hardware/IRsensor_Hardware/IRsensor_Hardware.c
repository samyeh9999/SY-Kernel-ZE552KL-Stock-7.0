/* 
 * Copyright (C) 2015 ASUSTek Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/********************************/
/* IR Sensor Hardware Module */
/******************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include "IRsensor_Hardware.h"
#include <linux/input/ASH.h>

/**************************/
/* Debug and Log System */
/************************/
#define MODULE_NAME			"ASH_HW"
#define SENSOR_TYPE_NAME		"IRsensor"

#undef dbg
#ifdef ASH_HW_DEBUG
	#define dbg(fmt, args...) printk(KERN_DEBUG "[%s][%s]"fmt,MODULE_NAME,SENSOR_TYPE_NAME,##args)
#else
	#define dbg(fmt, args...)
#endif
#define log(fmt, args...) printk(KERN_INFO "[%s][%s]"fmt,MODULE_NAME,SENSOR_TYPE_NAME,##args)
#define err(fmt, args...) printk(KERN_ERR "[%s][%s]"fmt,MODULE_NAME,SENSOR_TYPE_NAME,##args)

/********************/
/* Global Variables */
/******************/
static IRsensor_I2C* g_IRsensor_I2C;
static struct i2c_client * g_i2c_client = NULL;

int mIRsensor_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	if(g_IRsensor_I2C->IRsensor_probe == NULL){
		err("IRsensor_probe NOT implement. \n");
		return -EINVAL;
	}
	
	g_IRsensor_I2C->IRsensor_probe(client);

	/* We use Probe function to get i2c client */
	g_i2c_client = client;
	
	return 0;
}

int mIRsensor_remove(struct i2c_client *client)
{
	if(g_IRsensor_I2C->IRsensor_remove == NULL){
		err("IRsensor_remove NOT implement. \n");
		return -EINVAL;
	}
	
	g_IRsensor_I2C->IRsensor_remove();
	return 0;
}

void mIRsensor_shutdown(struct i2c_client *client)
{
	if(g_IRsensor_I2C->IRsensor_shutdown == NULL){
		err("IRsensor_shutdown NOT implement. \n");
	}
	
	g_IRsensor_I2C->IRsensor_shutdown();
}

int mIRsensor_suspend(struct i2c_client *client, pm_message_t mesg)
{
	if(g_IRsensor_I2C->IRsensor_suspend == NULL){
		err("IRsensor_suspend NOT implement. \n");
		return -EINVAL;
	}
	
	g_IRsensor_I2C->IRsensor_suspend();
	return 0;
}

int mIRsensor_resume(struct i2c_client *client)
{
	if(g_IRsensor_I2C->IRsensor_resume == NULL){
		err("IRsensor_resume NOT implement. \n");
		return -EINVAL;
	}
	
	g_IRsensor_I2C->IRsensor_resume();
	return 0;
}

static struct i2c_driver IRsensor_i2c_driver_client = {
	.probe = mIRsensor_probe,
	.remove = mIRsensor_remove,
	.shutdown = mIRsensor_shutdown,
	.suspend = mIRsensor_suspend,
	.resume = mIRsensor_resume,
};

int IRsensor_i2c_register(IRsensor_I2C *ir_i2c)
{
	if(ir_i2c == NULL){
		err("%s : IRsensor_I2C is NULL pointer. \n", __FUNCTION__);
		return -EINVAL;
	}
	
	g_IRsensor_I2C = ir_i2c;
	return 0;
}
EXPORT_SYMBOL(IRsensor_i2c_register);

int IRsensor_i2c_unregister(void)
{
	i2c_del_driver(&IRsensor_i2c_driver_client);

	return 0;
}
EXPORT_SYMBOL(IRsensor_i2c_unregister);

/***********************/
/*CM36686 I2c Driver*/
/**********************/
static const struct i2c_device_id cm36686_i2c_id[] = {
	{"cm36686", 0},
	{}
};

static struct of_device_id cm36686_match_table[] = {
	{ .compatible = "qcom,cm36686",},
	{},
};

/***********************/
/*AP3045 I2c Driver*/
/**********************/
static const struct i2c_device_id ap3045_i2c_id[] = {
	{"ap3045", 0},
	{}
};

static struct of_device_id ap3045_match_table[] = {
	{ .compatible = "qcom,ap3045",},
	{},
};

static int IRsensor_hw_setI2cDriver(struct i2c_driver* i2c_driver_client, 
	int hardware_source)
{
	switch(hardware_source) {
		case IRsensor_hw_source_cm36686:
			dbg("set i2c client : cm36686 \n");
			i2c_driver_client->driver.name = "cm36686";
			i2c_driver_client->driver.owner = THIS_MODULE;
			i2c_driver_client->driver.of_match_table = cm36686_match_table;
			i2c_driver_client->id_table = cm36686_i2c_id;
			break;

		case IRsensor_hw_source_ap3045:
			dbg("set i2c client : ap3045 \n");	
			i2c_driver_client->driver.name = "ap3045";
			i2c_driver_client->driver.owner = THIS_MODULE;
			i2c_driver_client->driver.of_match_table = ap3045_match_table;
			i2c_driver_client->id_table = ap3045_i2c_id;
			break;

		default:
			err("get hardware client ERROR : hardware enum=%d\n", hardware_source);
			return -EINVAL;
	}
	
	return 0;
}

IRsensor_hw* IRsensor_hw_getHardwareClient(int hardware_source)
{
	IRsensor_hw* IRsensor_hw_client = NULL;
		
	switch(hardware_source) {
		case IRsensor_hw_source_cm36686:
			dbg("get hardware client : cm36686 \n");
			IRsensor_hw_client = IRsensor_hw_cm36686_getHardware();
			break;

		case IRsensor_hw_source_ap3045:
			dbg("get hardware client : ap3045 \n");		
			IRsensor_hw_client = IRsensor_hw_ap3045_getHardware();
			break;

		default:
			err("get hardware client ERROR : hardware enum=%d\n", hardware_source);
	}

	return IRsensor_hw_client;
}

IRsensor_hw* IRsensor_hw_getHardware(void)
{	
	IRsensor_hw* IRsensor_hw_client = NULL;
	int ir_sensor_source;
	int ret = 0;
	
	/*check i2c function pointer*/
	if(g_IRsensor_I2C == NULL) {
		err("g_IRsensor_I2C is NULL. Please use 'IRsensor_i2c_register' first. \n");
		return NULL;
	}		
	
	/* i2c Registration */	
	for (ir_sensor_source = 0;ir_sensor_source < IRsensor_hw_source_max; 
			ir_sensor_source++) {
				
		/* i2c Registration and g_client will get i2c client */
		IRsensor_hw_setI2cDriver(&IRsensor_i2c_driver_client, ir_sensor_source);
		ret = i2c_add_driver(&IRsensor_i2c_driver_client);
		if ( ret != 0 ) {
			err("%s: i2c_add_driver ERROR(%d). \n", __FUNCTION__, ret);	
			return NULL;
		}
		if(g_i2c_client == NULL){
			err("%s: g_i2c_client is NULL pointer. \n", __FUNCTION__);	
			return NULL;
		}

		/* get hardware client and check the i2c status */
		IRsensor_hw_client = IRsensor_hw_getHardwareClient(ir_sensor_source);
		if(IRsensor_hw_client == NULL){
			err("IRsensor_hw_client is NULL pointer. \n");
			return NULL;
		}
			
		if(IRsensor_hw_client->IRsensor_hw_init == NULL){
			err("IRsensor_hw_init is NULL pointer. \n");
			return NULL;
		}

		ret = IRsensor_hw_client->IRsensor_hw_init(g_i2c_client);
		if (ret < 0) {
			i2c_del_driver(&IRsensor_i2c_driver_client);
			log("%s Probe Fail. \n", IRsensor_i2c_driver_client.driver.name);
			continue;
		}else{
			log("%s Probe Success. \n", IRsensor_i2c_driver_client.driver.name);
			break;
		}
	
	}
			
	if(ir_sensor_source == IRsensor_hw_source_max) {
		err("There is NO source can Probe.\n");
		return NULL;
	}

	return 	IRsensor_hw_client;
}
EXPORT_SYMBOL(IRsensor_hw_getHardware);


