#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

/*
 * Mock task that takes some time to complete.
 */
int __exit do_work(int *my_int, int retval)
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

int __init my_init(void)
{
	int x = 10;

	x = do_work(&x, x);

	return x;
}

void my_exit(void) {}

module_init(my_init);
module_exit(my_exit);
