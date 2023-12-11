#include "module.h"

static int id_open(struct inode *inode, struct file *file);
static int id_release(struct inode *inode, struct file *file);
static ssize_t id_read(struct file *file, char *data, size_t size,
		       loff_t *offset);
static ssize_t id_write(struct file *file, const char *data, size_t size,
			loff_t *offset);
static loff_t id_llseek(struct file *file, loff_t offset, int whence);

static struct file_operations fortytwo_fops = {
    .owner = THIS_MODULE,
    .open = id_open,
    .release = id_release,
    .read = id_read,
    .write = id_write,
    .llseek = id_llseek,
};

static struct dentry *id_file;

static int id_open(struct inode *inode, struct file *file)
{
	pr_info("'" FT_ID_NAME "' is being opened...\n");

	return 0;
}

static int id_release(struct inode *inode, struct file *file)
{
	pr_info("'" FT_ID_NAME "' is being released...\n");

	return 0;
}

/*
 * Copies "chamada" to the user provided address.
 */
static ssize_t id_read(struct file *file, char *data, size_t size,
		       loff_t *offset)
{
	int error;
	size_t copy_length;

	if (*offset < FT_ID_CONTENT_LEN)
		copy_length = min(size, FT_ID_CONTENT_LEN - (size_t)*offset);
	else
		copy_length = 0;

	if (copy_length == 0)
		goto done;

	error = copy_to_user(data, (const char *)FT_ID_CONTENT + *offset,
			     copy_length);

	if (error) {
		pr_err("Failed to copy '" FT_ID_CONTENT "' to the user!\n");
		return -EFAULT;
	}

	pr_info("Copied '" FT_ID_CONTENT "' to the user!\n");

	*offset += copy_length;

done:

	return copy_length;
}

/*
 * Allows writing only "chamada".
 * Returns EINVAL otherwise.
 */
static ssize_t id_write(struct file *file, const char *data, size_t size,
			loff_t *offset)
{
	char buffer[FT_ID_CONTENT_LEN];
	int valid;
	int error;

	valid = *offset == 0 && size == FT_ID_CONTENT_LEN;

	if (!valid) {
		pr_info("fortytwo: Attempted to write data of invalid size: "
			"'%zu' at offset '%lld'\n",
			size, *offset);

		goto invalid;
	}

	error = copy_from_user(buffer, data, size);

	if (error) {
		error = -EFAULT;
		goto error;
	}

	valid = !memcmp((const char *)FT_ID_CONTENT, buffer, size);

	if (!valid) {
		pr_info(
		    "fortytwo: Attempted to write invalid content: '%.*s'!\n",
		    (int)size, buffer);
		goto invalid;
	}

	*offset += size;

	return size;

invalid:
	error = -EINVAL;

error:
	return error;
}

static loff_t id_llseek(struct file *file, loff_t offset, int whence)
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
		new_position = FT_ID_CONTENT_LEN + offset;
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

int __init id_init(struct dentry *parent) {
	int	error = 0;

	id_file = debugfs_create_file(
	    FT_ID_NAME, FT_ID_MODE, parent, NULL, &fortytwo_fops);

	if (IS_ERR_VALUE(id_file)) {
		pr_err("Failed to create '" FT_ID_NAME "' misc device!\n");

		error = PTR_ERR(id_file);
	}

	return error;
}
