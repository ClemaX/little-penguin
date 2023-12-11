#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

/*
 * Mock task that takes some time to complete.
 */
static int __init do_work(int *my_int)
{
	int x;
	int y = *my_int;
	int z;

	for (x = 0; x < (uintptr_t)my_int; ++x)
		udelay(10);

	if (y < 10)
		pr_info("We slept a long time!");

	z = x * y;

	return z;
}

static int __init my_init(void)
{
	int x = 10;

	x = do_work(&x);

	return x;
}

static void __exit my_exit(void) {}

module_init(my_init);
module_exit(my_exit);
