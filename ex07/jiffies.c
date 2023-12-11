#include <linux/jiffies.h>

#include "module.h"

static int jiffies_open(struct inode *inode, struct file *file);
static int jiffies_release(struct inode *inode, struct file *file);
static ssize_t jiffies_read(struct file *file, char *data, size_t size,
			    loff_t *offset);

static struct file_operations fortytwo_fops = {
    .owner = THIS_MODULE,
    .open = jiffies_open,
    .release = jiffies_release,
    .read = jiffies_read,
};

static struct dentry *jiffies_file;

static int jiffies_open(struct inode *inode, struct file *file)
{
	pr_info("'" FT_JIFFIES_NAME "' is being opened...\n");

	return 0;
}

static int jiffies_release(struct inode *inode, struct file *file)
{
	pr_info("'" FT_JIFFIES_NAME "' is being released...\n");

	return 0;
}

/*
 * Copies the current jiffies timer in decimal ASCII-string representation to
 * the user provided address.
 */
static ssize_t jiffies_read(struct file *file, char *data, size_t size,
			    loff_t *offset)
{
	char buffer[22];
	int error;
	size_t copy_length;

	if (*offset != 0)
	{
		copy_length = 0;
		goto done;
	}

	copy_length = size;

	if (copy_length == 0)
		goto done;

	copy_length = snprintf(buffer, sizeof(buffer), "%lu\n", jiffies);

	copy_length = min(copy_length, size);

	if (copy_length == 0)
		goto done;


	error = copy_to_user(data, (const char *)buffer, copy_length);

	if (error) {
		pr_err("Failed to copy '%s' to the user!\n", buffer);
		return -EFAULT;
	}

	pr_info("Copied '%s' to the user!\n", buffer);

	*offset += copy_length;

done:

	return copy_length;
}

int __init jiffies_init(struct dentry *parent)
{
	int error = 0;

	jiffies_file = debugfs_create_file(FT_JIFFIES_NAME, FT_JIFFIES_MODE,
					   parent, NULL, &fortytwo_fops);

	if (IS_ERR_VALUE(jiffies_file)) {
		pr_err("Failed to create '" FT_JIFFIES_NAME "' misc device!\n");

		error = PTR_ERR(jiffies_file);
	}

	return error;
}
