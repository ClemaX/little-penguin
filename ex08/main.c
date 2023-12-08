#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Louis Solofrizzo <louis@ne02ptzero.me>");
MODULE_DESCRIPTION("Reverse Character Device");

static ssize_t reverse_read(struct file *fp, char __user *user, size_t size,
			 loff_t *offs);
static ssize_t reverse_write(struct file *fp, const char __user *user, size_t size,
			  loff_t *offs);

static struct file_operations reverse_fops = {
    .owner = THIS_MODULE,
	.read = &reverse_read,
	.write = &reverse_write,
};

static struct miscdevice reverse_device = {
    .minor = MISC_DYNAMIC_MINOR,
	.name = "reverse",
	.fops = &reverse_fops,
};

char data[PAGE_SIZE];
size_t data_length;

/*
 * Retrieve data in reverse order to how it was previously written.
 *
 * Empty by default.
 */
static ssize_t reverse_read(struct file *fp, char __user *user, size_t size, loff_t *offs)
{
	return simple_read_from_buffer(user, size, offs, data, data_length);
}

/*
 * Store up to one page of data.
 */
static ssize_t reverse_write(struct file *fp, const char __user *user, size_t size,
		   loff_t *offs)
{
	ssize_t res;
	size_t i, rev_i;
	char tmp;

	res = simple_write_to_buffer(data, sizeof(data) - 1, offs, user, size);

	if (res <= 0) {
		data_length = 0;
		goto error;
	}

	data_length = res;

	for (i = 0, rev_i = data_length - 1; i < data_length / 2; i++, rev_i--) {
		tmp = data[i];

		data[i] = data[rev_i];
		data[rev_i] = tmp; 
	}

error:
	data[data_length] = '\0';

	return res;
}

static int __init reverse_init(void)
{
	return misc_register(&reverse_device);
}

static void __exit reverse_cleanup(void) {
	misc_deregister(&reverse_device);
}

module_init(reverse_init);
module_exit(reverse_cleanup);
