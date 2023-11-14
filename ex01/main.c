#include <linux/module.h>
#include <linux/printk.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clément Hamada");
MODULE_DESCRIPTION("Hello world module");

int __init hello_world_init(void)
{
	pr_info("Hello world !\n");

	return 0;
}

void __exit hello_world_exit(void)
{
	pr_info("Cleaning up module.\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);
