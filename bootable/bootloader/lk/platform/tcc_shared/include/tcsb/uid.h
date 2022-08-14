#define UID_TAG		"UIDSTORE"
#define UID_ENABLE_FLAG	0x0001
#define UID_EMPTY_TAG		0x0000


struct secure_uid_box {
	char uid_tag[8];
	unsigned int total_size;
	char Reserved0 [4];
	unsigned int uid_flags;
	unsigned int uid_size;
	unsigned int uid_offset;
	unsigned int Reserved1;
	} SECURE_UID_T;
