#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>

//pcd : pseudo char driver

#undef pr_fmt
#define pr_fmt(fmt) "%s:" fmt, __func__

#define DEV_MEM_SIZE 512

//pseudo device's memory
char device_buffer[DEV_MEM_SIZE];

//this holds the device number
dev_t device_number;

//cdev variable: which represents char device
struct cdev pcd_cdev;

//defining file operation methods

//lseek changes the position of the f_pos variable
loff_t pcd_lseek(struct file *filp, loff_t offset, int whence){
	pr_info("lseek requested\n");
        pr_info("current file position = %lld\n", filp->f_pos);

	switch(whence){
		case SEEK_SET:
			if((offset > DEV_MEM_SIZE) || (offset < 0))
				return -EINVAL;
			filp->f_pos = offset;
			break;
		case SEEK_CUR: /*seek from current*/
			if((filp->f_pos + offset > DEV_MEM_SIZE) || (filp->f_pos + offset < 0))
                                return -EINVAL;
                        else
	                        filp->f_pos += offset;
                        break;
		case SEEK_END: /*seek from end*/
                        if(offset > 0 || (DEV_MEM_SIZE + offset < 0))
                                return -EINVAL;
                        else
				filp->f_pos = DEV_MEM_SIZE + offset;
                        break;
		default:
			return -EINVAL;
	}

        pr_info("new file position = %lld\n", filp->f_pos);
	return filp->f_pos;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos){
	//5. if f_pos at EOF, then return 0;
	if(*f_pos == DEV_MEM_SIZE)
		return 0;

        pr_info("read requested for %zu bytes \n", count);
	pr_info("current file position = %lld \n", *f_pos);

	//1. check the user requested 'count' value against DEV_MEM_SIZE of the device
	// Adjust the count
	if((*f_pos + count) > DEV_MEM_SIZE)
		count = DEV_MEM_SIZE - *f_pos;

	//2. copy 'count' number of bytes from device memory to user buffer
	//unsigned_long  copy_to_user(void __user *to, const void *from, unsigned long n);
	if(copy_to_user(buff, &device_buffer[*f_pos], count))
		return -EFAULT;

	//3. update the current file position(f_pos)
	*f_pos += count;

	pr_info("number of bytes successfully read = %zu \n", count);
	pr_info("updated file position = %lld \n", *f_pos);

	//4. Return number of bytes successfully read
	return count;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){
	//5. if f_pos at EOF, then return 0;
        if(*f_pos == DEV_MEM_SIZE){
		pr_err("No space left on the device\n");
                return -ENOMEM;
	}
        pr_info("write requested for %zu bytes \n", count);
        pr_info("current file position = %lld \n", *f_pos);

	//1. check the user requested 'count' value against DEV_MEM_SIZE of the device
        // Adjust the count
        if((*f_pos + count) > DEV_MEM_SIZE)
                count = DEV_MEM_SIZE - *f_pos;

        //2. copy 'count' number of bytes from user buffer to device memory
	//unsigned_long copy_from_user(void *to, const void __user *from, unsigned long n);
	if(copy_from_user(&device_buffer[*f_pos], buff, count))
		return -EFAULT;

	//3. update the current file position(f_pos)
        *f_pos += count;

	pr_info("number of bytes successfully written = %zu \n", count);
        pr_info("updated file position = %lld \n", *f_pos);

        //4. Return number of bytes successfully written
        return count;
}

int pcd_open(struct inode *inode, struct file *filp){
        pr_info("open was successful\n");
	return 0;
}

int pcd_release(struct inode *inode, struct file *filp){
        pr_info("release was successful\n");
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

struct class *class_pcd;

struct device *device_pcd;

static int __init pcd_driver_init(void){
	int ret; /* return value*/

	//1. dynamically allocate a device number
	ret = alloc_chrdev_region(&device_number, 0, 1, "pcd_devices");
	if(ret < 0){
		pr_err("Alloc chrdev failed\n");
		goto out;
	}

	pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(device_number), MINOR(device_number));

	//2. Initialize the cdev structure with fops
	pcd_cdev.owner = THIS_MODULE;
	cdev_init(&pcd_cdev, &pcd_fops);

	//3. Register a device (cdev structure) with VFS 
	ret = cdev_add(&pcd_cdev, device_number, 1);
	if(ret < 0){
		pr_err("cdev add failed\n");
		goto unreg_chrdev;
	}

	//4. create device class under /sys/class/
	class_pcd = class_create("pcd_class");
	if(IS_ERR(class_pcd)){
		pr_err("Class creation failed\n");
		ret = PTR_ERR(class_pcd);
		goto cdev_delete;
	}
	/* PTR_ERR() converts pointer to error code(int) */
	/* ERR_PTR() converts error code(int) to pointer */

	//5. device file creation
	// populate the sysfs with device information
	device_pcd = device_create(class_pcd, NULL, device_number, NULL, "pcd");
	if(IS_ERR(device_pcd)){
                pr_err("Device creation failed\n");
                ret = PTR_ERR(device_pcd);
                goto class_delete;
        }

	pr_info("Module init was successful\n");

	return 0;
class_delete:
	class_destroy(class_pcd);
cdev_delete:
	cdev_del(&pcd_cdev);
unreg_chrdev:
	unregister_chrdev_region(device_number, 1);
out:
	pr_err("Module insertion failed\n");
	return ret;
}

static void __exit pcd_driver_cleanup(void){
	//chronologically reverse order
	device_destroy(class_pcd, device_number);

	class_destroy(class_pcd);

	cdev_del(&pcd_cdev);

	unregister_chrdev_region(device_number, 1);

	pr_info("module unloaded\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bariscancoskun");
MODULE_DESCRIPTION("A pseudo character driver");



