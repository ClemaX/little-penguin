#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>

#define MT_PROCFS_NAME "mountpoints"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clément Hamada");
MODULE_DESCRIPTION("Mountpoints listing module");

static ssize_t mountpoints_read(struct file *file_pointer, char __user *buffer,
				size_t length, loff_t *offset);

#ifdef HAVE_PROC_OPS

static const struct proc_ops proc_file_fops = {
    .proc_read = mountpoints_read,
};

#else

static const struct file_operations proc_file_fops = {
    .read = mountpoints_read,
};

#endif

static struct proc_dir_entry *mountpoints_entry;

static ssize_t mountpoints_read(struct file *file_pointer, char __user *buffer,
				size_t length, loff_t *offset)
{
	const char s[13] = "HelloWorld!\n";

	const int len = sizeof(s) - 1;

	ssize_t ret;

	if (*offset >= len || copy_to_user(buffer, s, len)) {
		pr_info("copy_to_user failed\n");

		ret = 0;
	} else {
		pr_info("procfile read %s\n",
			file_pointer->f_path.dentry->d_name.name);

		*offset += len;

		ret = len;
	}

	return ret;
}

static int __init mountpoints_init(void)
{
	pr_info("Creating '" MT_PROCFS_NAME "' proc entry...\n");

	mountpoints_entry =
	    proc_create(MT_PROCFS_NAME, 0644, NULL, &proc_file_fops);

	if (mountpoints_entry == NULL) {
		pr_alert("Could not create '" MT_PROCFS_NAME "' proc entry!\n");
		return -ENOMEM;
	}

	pr_info("Successfully created '" MT_PROCFS_NAME "' proc entry!\n");

	return 0;
}

static void __exit mountpoints_exit(void)
{
	pr_info("Destroying '" MT_PROCFS_NAME "' proc entry...\n");

	proc_remove(mountpoints_entry);
}

module_init(mountpoints_init);
module_exit(mountpoints_exit);
