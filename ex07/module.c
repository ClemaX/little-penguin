#include <linux/fs.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "linux/debugfs.h"
#include "module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clément Hamada");
MODULE_DESCRIPTION("Little Penguin debugfs exercise");

int id_init(struct dentry *parent);

static struct dentry *debugfs_dir;

static int __init fortytwo_init(void)
{
	int error = 0;

	pr_info("Initializing '" FT_DEBUGFS_DIR "' debugfs directory...\n");

	// error = misc_register(&fortytwo_device);

	debugfs_dir = debugfs_create_dir(FT_DEBUGFS_DIR, NULL);

	if (IS_ERR_VALUE(debugfs_dir)) {
		pr_err("Failed to create debugfs directory at '" FT_DEBUGFS_DIR
		       "'!\n");

		error = PTR_ERR(debugfs_dir);
		goto fail_create_dir;
	}

	error = id_init(debugfs_dir);
	if (error)
		goto fail_id_init;

	pr_info("Created '" FT_ID_NAME "' file!\n");

	error = jiffies_init(debugfs_dir);
	if (error)
		goto fail_jiffies_init;

	pr_info("Created '" FT_JIFFIES_NAME "' file!\n");

	error = foo_init(debugfs_dir);
	if (error)
		goto fail_foo_init;

	pr_info("Created '" FT_FOO_NAME "' file!\n");

	goto done;

fail_foo_init:
fail_jiffies_init:
fail_id_init:
	debugfs_remove_recursive(debugfs_dir);

fail_create_dir:

done:
	return error;
}

void __exit fortytwo_exit(void)
{
	pr_info("Tearing down '" FT_DEBUGFS_DIR "'...\n");

	if (!IS_ERR_VALUE(debugfs_dir)) {
		debugfs_remove_recursive(debugfs_dir);
	}
}

module_init(fortytwo_init);
module_exit(fortytwo_exit);
