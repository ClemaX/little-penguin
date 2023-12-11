#pragma once

#include <linux/debugfs.h>

#define FT_DEBUGFS_DIR "fortytwo"

#define FT_ID_NAME "id"
#define FT_ID_MODE 0666
#define FT_ID_CONTENT "chamada"
#define FT_ID_CONTENT_LEN (sizeof(FT_ID_CONTENT) - 1)

#define FT_JIFFIES_NAME "jiffies"
#define FT_JIFFIES_MODE 0444

#define FT_FOO_NAME "foo"
#define FT_FOO_MODE 0444
#define FT_FOO_CONTENT_LEN PAGE_SIZE

int id_init(struct dentry *parent);
int jiffies_init(struct dentry *parent);
int foo_init(struct dentry *parent);
