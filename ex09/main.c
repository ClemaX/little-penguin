#include "linux/limits.h"
#include <linux/module.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/ns_common.h>
#include <linux/nsproxy.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clément Hamada");
MODULE_DESCRIPTION("Mountpoints listing module");

#define MT_PROCFS_NAME "mountpoints"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

struct mnt_namespace {
	struct ns_common ns;
	struct mount *root;
	/*
	 * Traversal and modification of .list is protected by either
	 * - taking namespace_sem for write, OR
	 * - taking namespace_sem for read AND taking .ns_lock.
	 */
	struct list_head list;
	spinlock_t ns_lock;
	struct user_namespace *user_ns;
	struct ucounts *ucounts;
	u64 seq; /* Sequence number to prevent loops */
	wait_queue_head_t poll;
	u64 event;
	unsigned int mounts; /* # of mounts in the namespace */
	unsigned int pending_mounts;
};

struct mount {
	struct hlist_node mnt_hash;
	struct mount *mnt_parent;
	struct dentry *mnt_mountpoint;
	struct vfsmount mnt;
	union {
		struct rcu_head mnt_rcu;
		struct llist_node mnt_llist;
	};
#ifdef CONFIG_SMP
	struct mnt_pcp __percpu *mnt_pcp;
#else
	int mnt_count;
	int mnt_writers;
#endif
	struct list_head mnt_mounts;   /* list of children, anchored here */
	struct list_head mnt_child;    /* and going through their mnt_child */
	struct list_head mnt_instance; /* mount instance on sb->s_mounts */
	const char *mnt_devname;       /* Name of device e.g. /dev/dsk/hda1 */
	struct list_head mnt_list;
	struct list_head mnt_expire;	 /* link in fs-specific expiry list */
	struct list_head mnt_share;	 /* circular list of shared mounts */
	struct list_head mnt_slave_list; /* list of slave mounts */
	struct list_head mnt_slave;	 /* slave list entry */
	struct mount *mnt_master;     /* slave is on master->mnt_slave_list */
	struct mnt_namespace *mnt_ns; /* containing namespace */
	struct mountpoint *mnt_mp;    /* where is it mounted */
	union {
		struct hlist_node
		    mnt_mp_list; /* list mounts with the same mountpoint */
		struct hlist_node mnt_umount;
	};
	struct list_head mnt_umounting; /* list entry for umount propagation */
#ifdef CONFIG_FSNOTIFY
	struct fsnotify_mark_connector __rcu *mnt_fsnotify_marks;
	__u32 mnt_fsnotify_mask;
#endif
	int mnt_id;	     /* mount identifier */
	int mnt_group_id;    /* peer group identifier */
	int mnt_expiry_mark; /* true if marked for expiry */
	struct hlist_head mnt_pins;
	struct hlist_head mnt_stuck_children;
};

static int mountpoints_open(struct inode *inode, struct file *file);
static int mountpoints_release(struct inode *inode, struct file *file);

static void *mountpoints_seq_start(struct seq_file *s, loff_t *pos);
static void *mountpoints_seq_next(struct seq_file *s, void *v, loff_t *pos);
static void mountpoints_seq_stop(struct seq_file *s, void *v);
static int mountpoints_seq_show(struct seq_file *s, void *v);

#ifdef HAVE_PROC_OPS

static const struct proc_ops proc_file_operations = {
    .proc_open = mountpoints_open,
    .proc_release = mountpoints_release,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
};

#else

static const struct file_operations proc_file_fops = {
    .open = mountpoints_open,
    .release = mountpoints_release,
    .read = seq_read,
    .lseek = seq_lseek,
};

#endif

static const struct seq_operations seq_file_operations = {
    .start = mountpoints_seq_start,
    .next = mountpoints_seq_next,
    .stop = mountpoints_seq_stop,
    .show = mountpoints_seq_show,
};

static struct proc_dir_entry *mountpoints_entry;

static char *mount_path(const struct mount *mnt, char *buffer, size_t size)
{
	char *filepath = buffer;
	const struct mount *mnt_it = mnt;

	memset(filepath, 0, size);

	filepath = dentry_path_raw(mnt_it->mnt_mountpoint, buffer, size);

	mnt_it = mnt_it->mnt_parent;

	while (mnt_it != NULL &&
	       !(mnt_it->mnt_mountpoint->d_name.name[0] == '/' &&
		 mnt_it->mnt_mountpoint->d_name.name[1] == '\0')) {

		filepath = dentry_path_raw(mnt_it->mnt_mountpoint, buffer,
					   (filepath - buffer) + 1);

		if (IS_ERR(filepath))
			break;

		filepath[strlen(filepath)] = '/';

		mnt_it = mnt_it->mnt_parent;
	}

	return filepath;
}

static int mountpoints_open(struct inode *inode, struct file *file)
{
	int ret;

	pr_info("mountpoints: Opening sequence...\n");

	ret = seq_open_private(file, &seq_file_operations, PATH_MAX);

	return ret;
}

static int mountpoints_release(struct inode *inode, struct file *file)
{
	pr_info("mountpoints: Releasing sequence...\n");

	return seq_release_private(inode, file);
}

static void *mountpoints_seq_start(struct seq_file *s, loff_t *pos)
{
	pr_info("mountpoints: Starting sequence...\n");

	return seq_list_start(&current->nsproxy->mnt_ns->list, *pos);
}

static void *mountpoints_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	return seq_list_next(v, &current->nsproxy->mnt_ns->list, pos);
}

/*
	list_for_each_entry(mnt, &real_mount(root)->mnt_list, mnt_list) {
		res = f(&mnt->mnt, arg);
		if (res)
			return res;
	}
*/

static void mountpoints_seq_stop(struct seq_file *s, void *v)
{
	pr_info("mountpoints: Stopped sequence!\n");
}

static int mountpoints_seq_show(struct seq_file *s, void *v)
{
	const struct mount *const mount = list_entry(v, struct mount, mnt_list);
	char *path;

	path = mount_path(mount, (char*)s->private, PATH_MAX);

	if (IS_ERR(path)) {
		goto skip;
	}

	seq_printf(s, "%s %s\n", mount->mnt_devname, path);

skip:
	return 0;
}

static int __init mountpoints_init(void)
{
	pr_info("Creating '" MT_PROCFS_NAME "' proc entry...\n");

	mountpoints_entry =
	    proc_create(MT_PROCFS_NAME, 0444, NULL, &proc_file_operations);

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
