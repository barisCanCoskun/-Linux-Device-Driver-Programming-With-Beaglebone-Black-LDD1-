#include<linux/module.h>
#include<linux/platform_device.h>

#include"platform.h"

//1. create 2 platform data

struct pcdev_platform_data pcdev_pdata[2] = {
	[0] = {.size = 512, .perm = RDWR, .serial_number = "PCDEVABC1111"},
	[1] = {.size = 1024, .perm = RDWR, .serial_number = "PCDEVXYZ2222"}
};

//2. create 2 platform devices

struct platform_device platform_pcdev_1 = {
	.name = "pseudo-char-device",
	.id = 0,
	.dev = {
		.platform_data = &pcdev_pdata[0]
	}
};

struct platform_device platform_pcdev_2 = {
        .name = "pseudo-char-device",
        .id = 1,
        .dev = {
                .platform_data = &pcdev_pdata[1]
        }
};

static int __init pcdev_platform_init(void){
	//register platform devices
	platform_device_register(&platform_pcdev_1);
	platform_device_register(&platform_pcdev_2);
	return 0;
}

static void __exit pcdev_platform_exit(void){
	//unregister platform devices
        platform_device_unregister(&platform_pcdev_1);
        platform_device_unregister(&platform_pcdev_2);
}

module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bariscancoskun");
MODULE_DESCRIPTION("Module which registers platform devices");









