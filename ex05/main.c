#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/printk.h>

#define FT_DEV_CLASS "ft"
#define FT_DEV_NAME "fortytwo"
#define FT_DEV_CONTENT "chamada"
#define FT_DEV_CONTENT_LEN (sizeof(FT_DEV_CONTENT) - 1)
#define FT_DEV_MINOR (MISC_DYNAMIC_MINOR)

static int dev_open(struct inode *inode, struct file *file);
static int dev_release(struct inode *inode, struct file *file);
static ssize_t dev_read(struct file *file, char *data, size_t size,
			loff_t *offset);
static ssize_t dev_write(struct file *file, const char *data, size_t size,
			 loff_t *offset);
static loff_t dev_llseek(struct file *file, loff_t offset, int whence);

static struct file_operations file_operations = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .llseek = dev_llseek,
};

struct miscdevice fortytwo_device = {
    .minor = FT_DEV_MINOR,
    .name = FT_DEV_NAME,
    .fops = &file_operations,
    .mode = 0666,
};

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
	size_t copy_length;

	if (*offset < FT_DEV_CONTENT_LEN)
		copy_length = min(size, FT_DEV_CONTENT_LEN - (size_t)*offset);
	else
		copy_length = 0;

	if (copy_length == 0)
		goto done;

	error = copy_to_user(data, (const char *)FT_DEV_CONTENT + *offset,
			     copy_length);

	if (error) {
		pr_err("Failed to copy '" FT_DEV_CONTENT "' to the user!\n");
		return -EFAULT;
	}

	pr_info("Copied '" FT_DEV_CONTENT "' to the user!\n");

	*offset += copy_length;

done:

	return copy_length;
}

/*
 * Allows writing only "chamada".
 * Returns EINVAL otherwise.
 */
/*
 * Allows writing only "chamada".
 * Returns EINVAL otherwise.
 */
static ssize_t dev_write(struct file *file, const char *data, size_t size,
			 loff_t *offset)
{
	char buffer[FT_DEV_CONTENT_LEN];
	int valid;
	int error;

	valid = *offset = 0 && size == FT_DEV_CONTENT_LEN;

	if (!valid)
		goto invalid;

	error = copy_from_user(buffer, data, size);

	if (error) {
		error = -EFAULT;
		goto error;
	}

	valid = !memcmp((const char *)FT_DEV_CONTENT, buffer, size);

	if (!valid)
		goto invalid;

	*offset += size;

	return size;

invalid:
	error = -EINVAL;

error:
	return error;
}

static loff_t dev_llseek(struct file *file, loff_t offset, int whence)
{
	loff_t new_position;

	switch (whence) {
	case SEEK_SET:
		new_position = offset;

		break;
	case SEEK_CUR:
		new_position = file->f_pos + offset;

		break;
	case SEEK_END:
		new_position = FT_DEV_CONTENT_LEN + offset;
		break;

	default:
		goto invalid;
	}

	if (new_position < 0)
		goto invalid;

	file->f_pos = new_position;

	return new_position;

invalid:
	return -EINVAL;
}

static int __init fortytwo_init(void)
{
	int error;

	pr_info("Initializing '" FT_DEV_NAME "' device...\n");

	error = misc_register(&fortytwo_device);

	if (error)
		pr_err("Failed to create '" FT_DEV_NAME "' misc device!\n");

	return error;
}

void __exit fortytwo_exit(void)
{
	pr_info("Tearing down '" FT_DEV_NAME "'...\n");
	misc_deregister(&fortytwo_device);
}

module_init(fortytwo_init);
module_exit(fortytwo_exit);

MODULE_LICENSE("GPL");
