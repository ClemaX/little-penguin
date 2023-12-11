#include "module.h"

static int foo_open(struct inode *inode, struct file *file);
static int foo_release(struct inode *inode, struct file *file);
static ssize_t foo_read(struct file *file, char *data, size_t size,
			loff_t *offset);
static ssize_t foo_write(struct file *file, const char *data, size_t size,
			 loff_t *offset);
static loff_t foo_llseek(struct file *file, loff_t offset, int whence);

static struct file_operations fortytwo_fops = {
    .owner = THIS_MODULE,
    .open = foo_open,
    .release = foo_release,
    .read = foo_read,
    .write = foo_write,
    .llseek = foo_llseek,
};

DEFINE_MUTEX(foo_mutex);

static char foo_content[FT_FOO_CONTENT_LEN];
static size_t foo_length;
static struct dentry *foo_file;

static int foo_open(struct inode *inode, struct file *file)
{
	pr_info("'" FT_FOO_NAME "' is being opened, flags: %X...\n",
		file->f_flags);

	if ((file->f_flags & O_APPEND) == O_APPEND) {
		mutex_lock(&foo_mutex);
		pr_info("'" FT_FOO_NAME "' Appending: length %zu...\n",
			foo_length);

		file->f_pos = foo_length;
		mutex_unlock(&foo_mutex);
	}

	if (file->f_mode == FMODE_WRITE) {
	}

	return 0;
}

static int foo_release(struct inode *inode, struct file *file)
{
	pr_info("'" FT_FOO_NAME "' is being released...\n");

	return 0;
}

/*
 * Copies written content back to the user.
 */
static ssize_t foo_read(struct file *file, char *data, size_t size,
			loff_t *offset)
{
	(void)offset;
	int error;
	ssize_t copy_length;
	//loff_t position;

	//file->f_pos = file->f_pos + *offset;

	mutex_lock(&foo_mutex);

	if (*offset < foo_length)
		copy_length = min(size, foo_length - (size_t)*offset);
	else
		copy_length = 0;

	if (copy_length == 0)
		goto done;

	error = copy_to_user(data, foo_content + *offset, copy_length);

	if (error) {
		pr_err("Failed to copy foo content to the user!\n");
		copy_length = -EFAULT;
		goto done;
	}

	pr_info("Copied foo content at offset %lld (%zu bytes) to the user!\n",
		*offset, copy_length);

	*offset += copy_length;

done:
	mutex_unlock(&foo_mutex);

	return copy_length;
}

/*
 * Allows writing at most PAGE_SIZE information.
 *
 * Returns EINVAL for invalid size or offset.
 */
static ssize_t foo_write(struct file *file, const char *data, size_t size,
			 loff_t *offset)
{
	int valid;
	int error;
	//loff_t position;
	ssize_t copy_length;

	//position = file->f_pos + *offset;

	mutex_lock(&foo_mutex);

	valid = *offset <= foo_length && *offset + size <= FT_FOO_CONTENT_LEN;

	if (!valid) {
		pr_info("foo: Attempted to write data of invalid size %zu at "
			"offset %lld (max size %zu)\n",
			size, *offset, FT_FOO_CONTENT_LEN);

		copy_length = -EINVAL;
		goto done;
	}

	pr_info("foo: Copying %zu bytes from user at offset %lld...\n", size,
		*offset);

	error = copy_from_user(foo_content + *offset, data, size);

	if (error) {
		copy_length = -EFAULT;
		foo_length = 0;
		goto done;
	}

	foo_length = *offset + size;

	*offset += size;
	copy_length = size;

done:
	mutex_unlock(&foo_mutex);

	return copy_length;
}

static loff_t foo_llseek(struct file *file, loff_t offset, int whence)
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
		new_position = foo_length + offset;
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

int __init foo_init(struct dentry *parent)
{
	int error = 0;

	foo_file = debugfs_create_file(FT_FOO_NAME, FT_FOO_MODE, parent, NULL,
				       &fortytwo_fops);

	if (IS_ERR_VALUE(foo_file)) {
		pr_err("Failed to create '" FT_FOO_NAME "' misc device!\n");

		error = PTR_ERR(foo_file);
	}

	return error;
}
