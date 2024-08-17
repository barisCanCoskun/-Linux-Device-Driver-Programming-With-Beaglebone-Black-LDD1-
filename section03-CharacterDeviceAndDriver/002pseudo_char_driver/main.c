#include<linux/module.h>
#include<linux/fs.h>
//pcd : pseudo char driver

#define DEV_MEM_SIZE 512

//pseudo device's memory
char device_buffer[DEV_MEM_SIZE];

//this holds the device number
dev_t device_number;

static int __init pcd_driver_init(void){
	//dynamically allocate a device number
	alloc_chrdev_region(&device_number, 0, 1, "pcd");

	return 0;
}

static void __exit pcd_driver_cleanup(void){


}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bariscancoskun");
MODULE_DESCRIPTION("A pseudo character driver");



