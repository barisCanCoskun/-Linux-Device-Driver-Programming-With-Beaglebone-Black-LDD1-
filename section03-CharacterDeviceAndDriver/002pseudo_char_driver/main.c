#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
//pcd : pseudo char driver

#define DEV_MEM_SIZE 512

//pseudo device's memory
char device_buffer[DEV_MEM_SIZE];

//this holds the device number
dev_t device_number;

//cdev variable: which represents char device
struct cdev pcd_cdev;

//defining file operation methods
loff_t pcd_lseek(struct file *filp, loff_t off, int whence){
	return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos){
	return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){
	return 0;
}

int pcd_open(struct inode *inode, struct file *filp){
	return 0;
}

int pcd_release(struct inode *inode, struct file *filp){
	return 0;
}

//file operations of the driver
struct file_operations pcd_fops ={
	.open = pcd_open,
	.release = pcd_release,
	.write = pcd_write,
	.read = pcd_read,
	.llseek = pcd_lseek,
	.owner = THIS_MODULE
};

static int __init pcd_driver_init(void){
	//1. dynamically allocate a device number
	alloc_chrdev_region(&device_number, 0, 1, "pcd");

	//2. Initialize the cdev structure with fops
	//pcd_cdev.owner = THIS_MODULE;
	cdev_init(&pcd_cdev, &pcd_fops);

	//3. Register a device (cdev structure) with VFS 
	cdev_add(&pcd_cdev, device_number, 1);

	return 0;
}

static void __exit pcd_driver_cleanup(void){


}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bariscancoskun");
MODULE_DESCRIPTION("A pseudo character driver");



