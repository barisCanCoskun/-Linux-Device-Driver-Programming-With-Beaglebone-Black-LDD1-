00 - 2 platform devices added:    
![00-2_platform_devices_added](https://github.com/user-attachments/assets/907d7f3b-b407-419b-9ea2-21c41d702419)    

01 - probe messages after loading modules    
![01-probe_messages_after_loading_modules](https://github.com/user-attachments/assets/7632cd62-0978-44c1-bebd-7cc634ed4226)    

02 - kernel mem allocation in probe func for device private data    
![02-kernel_mem_allocation_in_probe_func](https://github.com/user-attachments/assets/4ea5e40b-83d5-43dd-8f0a-1bf1f170de1f)    

03 - kmalloc func declaration
![03-kmalloc_declaration](https://github.com/user-attachments/assets/fb4496b5-d6d2-4c5d-a387-0bc0bb2392ca)

04 - kmalloc explanation    
![04-kmalloc_explanation](https://github.com/user-attachments/assets/89dbe4ca-c5aa-43a9-a4d8-2dad91fd9997)    

05 - kmalloc flags    
![05-kmalloc_flags](https://github.com/user-attachments/assets/8beb3456-4bcb-40d8-b556-fe2a9da22972)    
'GFP' stands for getting free pages. By using these flags, you can change the behavior of the underlying  
memory allocator.  
For example, the most commonly used flag for the process context is GFP_KERNEL.  
It makes the underlying allocator allocate the memory from the normal kernel RAM.  
If the memory is unavailable when your driver requests it, the kernel may put your process to sleep.  
That means, a kernel can put your process into a blocked state.  
That's why, kmalloc() with a flag GFP_KERNEL may sleep, you should remember this.  
And when memory becomes available, your process gets unblocked.  
  
GFP_ATOMIC flag is another important flag when you use kmalloc inside an interrupt handler.   
If the memory is unavailable, the allocator immediately returns.     
The caller will not be put to sleep because interrupt handlers cannot sleep,  
they can't be rescheduled like your normal processes.  
So, most probably you'll be using GFP_KERNEL in your driver when you want to allocate memory dynamically  
on behalf of process context. And use GFP_ATOMIC to request memory from the interrupt handler.    

06 - kmalloc usage  
![06-kmalloc_usage](https://github.com/user-attachments/assets/bd109024-8ab2-4f53-8566-31fba4632bd5)  

07 - kfree  
![07-kfree](https://github.com/user-attachments/assets/1d05cb75-eac1-42dc-a357-e330ddb2c916)    

08 - kmalloc friends  
![08-kmalloc_friends](https://github.com/user-attachments/assets/2cb56661-6576-4129-be5d-5d24f8e9c1cd)  

09 - kzalloc prefered  
![09-kzalloc_prefered](https://github.com/user-attachments/assets/e1e06697-98de-406d-853b-c9a33b3850cb)  

10 - kzalloc over kmalloc  
![10-kzalloc_over_kmalloc](https://github.com/user-attachments/assets/8af30444-affe-4b2f-9799-b536b93d5bcc)  
The "bar_processing_fun" function doesn't check whether "pbar->name" has a valid pointer or not.   
It just tries to dereference. Now, you may face such issues if you use kmalloc() here.  
Why?  
Because kmalloc() doesn't return a zeroed memory.  
That's why the "pbar->name" variable may have some random values and that random value is checked in the if condition.  
If the random value is present, this function assumes that the memory for the 'name' variable is already allocated.  
It just tries to dereference that blindly, and there could be a crash.  
So, one thing you can do here is, before sending the "pbar" variable to this function, you can  
zero out each and every member element of this pointer variable, which could solve the problem.  
Otherwise, just use kzalloc instead of kmalloc. So, it anyway zeroes out the memory locations.  

11 - kzfree  
![11-kzfree](https://github.com/user-attachments/assets/40a97a47-5ad5-4e29-8ea9-69d5e3ed5f15)  





