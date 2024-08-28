#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>

//pcd : pseudo char driver

#undef pr_fmt
#define pr_fmt(fmt) "%s:" fmt, __func__

#define NO_OF_DEVICES 4

#define MEM_SIZE_MAX_PCDEV1 1024
#define MEM_SIZE_MAX_PCDEV2 512
#define MEM_SIZE_MAX_PCDEV3 1024
#define MEM_SIZE_MAX_PCDEV4 512

//pseudo device's memory
char device_buffer_pcdev1[MEM_SIZE_MAX_PCDEV1];
char device_buffer_pcdev2[MEM_SIZE_MAX_PCDEV2];
char device_buffer_pcdev3[MEM_SIZE_MAX_PCDEV3];
char device_buffer_pcdev4[MEM_SIZE_MAX_PCDEV4];

/* device private data stucture */
struct pcdev_private_data{
        char *buffer;
	unsigned size;
	const char *serial_number;
	int perm; /* permission */
	//cdev variable: which represents char device
	struct cdev cdev;
};

/* driver private data stucture */
struct pcdrv_private_data{
	int total_devices;
	//this holds the device number
	dev_t device_number;
        struct class *class_pcd;
        struct device *device_pcd;
	struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
};

#define RDONLY 0x01
#define WRONLY 0x10
#define RDWR   0x11

struct pcdrv_private_data pcdrv_data = {
	.total_devices = NO_OF_DEVICES,
	.pcdev_data = {
		[0] = {
			.buffer = device_buffer_pcdev1,
			.size = MEM_SIZE_MAX_PCDEV1,
			.serial_number = "PCDEV1XYZ123",
			.perm = RDONLY
		},
                [1] = {
                        .buffer = device_buffer_pcdev2,
                        .size = MEM_SIZE_MAX_PCDEV2,
                        .serial_number = "PCDEV2XYZ123",
                        .perm = WRONLY
                },

                [2] = {
                        .buffer = device_buffer_pcdev3,
                        .size = MEM_SIZE_MAX_PCDEV3,
                        .serial_number = "PCDEV3XYZ123",
                        .perm = RDWR
                },

                [3] = {
                        .buffer = device_buffer_pcdev4,
                        .size = MEM_SIZE_MAX_PCDEV4,
                        .serial_number = "PCDEV4XYZ123",
                        .perm = RDWR
                }
	}
};


//defining file operation methods

//lseek changes the position of the f_pos variable
loff_t pcd_lseek(struct file *filp, loff_t offset, int whence){
        struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
        int max_size = pcdev_data->size;

	pr_info("lseek requested\n");
        pr_info("current file position = %lld\n", filp->f_pos);

	switch(whence){
		case SEEK_SET:
			if((offset > max_size) || (offset < 0))
				return -EINVAL;
			filp->f_pos = offset;
			break;
		case SEEK_CUR: /*seek from current*/
			if((filp->f_pos + offset > max_size) || (filp->f_pos + offset < 0))
                                return -EINVAL;
                        else
	                        filp->f_pos += offset;
                        break;
		case SEEK_END: /*seek from end*/
                        if(offset > 0 || (max_size + offset < 0))
                                return -EINVAL;
                        else
				filp->f_pos = max_size + offset;
                        break;
		default:
			return -EINVAL;
	}

        pr_info("new file position = %lld\n", filp->f_pos);
	return filp->f_pos;
}

ssize_t pcd_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos){
	struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
	int max_size = pcdev_data->size;

	// if f_pos at EOF, then return 0;
	if(*f_pos == max_size)
		return 0;

        pr_info("read requested for %zu bytes \n", count);
	pr_info("current file position = %lld \n", *f_pos);

	//1. check the user requested 'count' value against DEV_MEM_SIZE of the device
	// Adjust the count
	if((*f_pos + count) > max_size)
		count = max_size - *f_pos;

	//2. copy 'count' number of bytes from device memory to user buffer
	//unsigned_long  copy_to_user(void __user *to, const void *from, unsigned long n);
	if(copy_to_user(buff, pcdev_data->buffer + (*f_pos), count))
		return -EFAULT;

	//3. update the current file position(f_pos)
	*f_pos += count;

	pr_info("number of bytes successfully read = %zu \n", count);
	pr_info("updated file position = %lld \n", *f_pos);

	//4. Return number of bytes successfully read
	return count;
}

ssize_t pcd_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){
        struct pcdev_private_data *pcdev_data = (struct pcdev_private_data *)filp->private_data;
        int max_size = pcdev_data->size;

	// if f_pos at EOF, then return 0;
        if(*f_pos == max_size){
		pr_err("No space left on the device\n");
                return -ENOMEM;
	}
        pr_info("write requested for %zu bytes \n", count);
        pr_info("current file position = %lld \n", *f_pos);

	//1. check the user requested 'count' value against DEV_MEM_SIZE of the device
        // Adjust the count
        if((*f_pos + count) > max_size)
                count = max_size - *f_pos;

        //2. copy 'count' number of bytes from user buffer to device memory
	//unsigned_long copy_from_user(void *to, const void __user *from, unsigned long n);
	if(copy_from_user(pcdev_data->buffer + (*f_pos), buff, count))
		return -EFAULT;

	//3. update the current file position(f_pos)
        *f_pos += count;

	pr_info("number of bytes successfully written = %zu \n", count);
        pr_info("updated file position = %lld \n", *f_pos);

        //4. Return number of bytes successfully written
        return count;
}

int check_permission(int dev_perm, int acc_mode){ /* acc_mode : access mode */
	if(dev_perm == RDWR)
		return 0;

	/* ensures readonly access */
        if((dev_perm == RDONLY) && ((acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE)))
                return 0;

	/* ensures writeonly access */
        if((dev_perm == WRONLY) && ((acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ)))
                return 0;

	return -EPERM;
}

int pcd_open(struct inode *inode, struct file *filp){
	int ret;

	int minor_n;

	struct pcdev_private_data *pcdev_data;

	/* Find out which device file open was attempted by the user space*/
	minor_n = MINOR(inode->i_rdev); /* i_rdev : device number */
	pr_info("minor access = %d\n", minor_n);

	/* Get device's private data structure*/
	pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, cdev);

	/* to supply device private data to other methods of the driver */
	filp->private_data = pcdev_data;

	/* check permission*/
	ret = check_permission(pcdev_data->perm, filp->f_mode);

        (!ret)?pr_info("open was successful\n"):pr_info("open was unsuccessful\n");
	return ret;
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

static int __init pcd_driver_init(void){
	int ret; /* return value*/
	int i;

	// dynamically allocate device numbers
	ret = alloc_chrdev_region(&pcdrv_data.device_number, 0, NO_OF_DEVICES, "pcdevs");
	if(ret < 0){
		pr_err("Alloc chrdev failed\n");
		goto out;
	}

	// create device class under /sys/class/
	pcdrv_data.class_pcd = class_create("pcd_class");
	if(IS_ERR(pcdrv_data.class_pcd)){
		pr_err("Class creation failed\n");
		ret = PTR_ERR(pcdrv_data.class_pcd);
		goto unreg_chrdev;
	}
	/* PTR_ERR() converts pointer to error code(int) */
	/* ERR_PTR() converts error code(int) to pointer */

	for(i = 0; i < NO_OF_DEVICES; ++i){
		pr_info("Device number <major>:<minor> = %d:%d\n", MAJOR(pcdrv_data.device_number + i), MINOR(pcdrv_data.device_number + i));

		// Initialize the cdev structure with fops
		cdev_init(&pcdrv_data.pcdev_data[i].cdev, &pcd_fops);

		// Register a device (cdev structure) with VFS
		pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;
		ret = cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.device_number + i, 1);
		if(ret < 0){
			pr_err("cdev add failed\n");
			goto class_delete;
		}

		// device file creation
		// populate the sysfs with device information
		pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.device_number + i, NULL, "pcdev-%d", i + 1);
		if(IS_ERR(pcdrv_data.device_pcd)){
	                pr_err("Device creation failed\n");
	                ret = PTR_ERR(pcdrv_data.device_pcd);
	                goto cdev_delete;
	        }
	}

	pr_info("Module init was successful\n");

	return 0;
cdev_delete:
	for(;i >= 0; --i)
		cdev_del(&pcdrv_data.pcdev_data[i].cdev);
class_delete:
	class_destroy(pcdrv_data.class_pcd);
unreg_chrdev:
	unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);
out:
	pr_err("Module insertion failed\n");
	return ret;

}

static void __exit pcd_driver_cleanup(void){
	//chronologically reverse order
	for(int i = 0;i < NO_OF_DEVICES; ++i){
		device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number + i);
                cdev_del(&pcdrv_data.pcdev_data[i].cdev);
	}
        class_destroy(pcdrv_data.class_pcd);

	unregister_chrdev_region(pcdrv_data.device_number, NO_OF_DEVICES);

	pr_info("module unloaded\n");
}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bariscancoskun");
MODULE_DESCRIPTION("A pseudo character driver which handles n devices");



