#include "asm-generic/errno-base.h"
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/printk.h>

#define FT_DEV_CLASS "ft"
#define FT_DEV_NAME "fortytwo"
#define FT_DEV_CONTENT "chamada"
#define FT_DEV_MINOR 0

static int dev_open(struct inode *inode, struct file *file);
static int dev_release(struct inode *inode, struct file *file);
static ssize_t dev_read(struct file *file, char *data, size_t size,
			loff_t *offset);
static ssize_t dev_write(struct file *file, const char *data, size_t size,
			 loff_t *offset);

static struct file_operations file_operations = {
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
};

static struct class *device_class;
static struct device *device;
static int device_major;

static int dev_open(struct inode *inode, struct file *file)
{
	pr_info("'" FT_DEV_NAME "' is being opened...\n");

	return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
	pr_info("'" FT_DEV_NAME "' is being released...\n");

	return 0;
}

/*
 * Copies "chamada" to the user provided address.
 */
static ssize_t dev_read(struct file *file, char *data, size_t size,
			loff_t *offset)
{
	int error;
	size_t copy_length = min(size, sizeof(FT_DEV_CONTENT) - 1);

	error = copy_to_user(data, FT_DEV_CONTENT, copy_length);

	if (error) {
		pr_err("Failed to copy '" FT_DEV_CONTENT "' to the user!\n");
		return -EFAULT;
	}

	pr_info("Copied '" FT_DEV_CONTENT "' to the user!\n");

	return copy_length;
}

/*
 * Allows writing only "chamada".
 * Returns EINVAL otherwise.
 */
static ssize_t dev_write(struct file *file, const char *data, size_t size,
			 loff_t *offset)
{
	char buffer[sizeof(FT_DEV_CONTENT) - 1];
	int valid;
	int error;

	valid = size == sizeof(FT_DEV_CONTENT) - 1;

	if (!valid)
		goto invalid;

	error = copy_from_user(buffer, data, size);

	if (error) {
		error = -EFAULT;
		goto error;
	}

	valid = !memcmp(FT_DEV_CONTENT, buffer, size);

	if (!valid)
		goto invalid;

	return size;

invalid:
	error = -EINVAL;

error:
	return error;
}

static int __init fortytwo_init(void)
{
	int error = 0;

	pr_info("Initializing '" FT_DEV_NAME "' device...\n");

	device_major = register_chrdev(0, FT_DEV_NAME, &file_operations);

	if (device_major < 0) {
		pr_err("Failed to register major for '" FT_DEV_NAME "'!\n");
		return device_major;
	}

	pr_info("Registered '" FT_DEV_NAME "' device with major '%d'!\n",
		device_major);

	device_class = class_create(FT_DEV_CLASS);

	if (IS_ERR(device_class)) {
		pr_err("Failed to create device class '" FT_DEV_CLASS "'!\n");

		error = PTR_ERR(device_class);
		goto out_cleanup_major;
	}

	pr_info("Created device class '" FT_DEV_CLASS "'!\n");

	device =
	    device_create(device_class, NULL, MKDEV(device_major, FT_DEV_MINOR),
			  NULL, FT_DEV_NAME);

	if (IS_ERR(device)) {
		pr_err("Failed to create '" FT_DEV_NAME "' device!\n");

		error = PTR_ERR(device);
		goto out_cleanup_class;
	}

	return 0;

out_cleanup_class:
	class_unregister(device_class);
	class_destroy(device_class);

out_cleanup_major:
	unregister_chrdev(device_major, FT_DEV_NAME);

	return error;
}

void __exit fortytwo_exit(void)
{
	pr_info("Tearing down '" FT_DEV_NAME "'...\n");
	device_destroy(device_class, MKDEV(device_major, FT_DEV_MINOR));

	class_unregister(device_class);
	class_destroy(device_class);

	unregister_chrdev(device_major, FT_DEV_NAME);
}

module_init(fortytwo_init);
module_exit(fortytwo_exit);

MODULE_LICENSE("GPL");
