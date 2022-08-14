#define INVALID_PTN               -1

#define PARTITION_TYPE_MBR         0
#define PARTITION_TYPE_GPT         1
#define PARTITION_TYPE_GPT_BACKUP  2

#define GUID_VERSION		0x00010000
#define GUID_MAGIC		"EFI PART"
#define GUID_RESERVED		17

/* GPT Signature should be 0x5452415020494645 */
#define GPT_SIGNATURE_1 0x54524150
#define GPT_SIGNATURE_2 0x20494645

#define MMC_MBR_SIGNATURE_BYTE_0  0x55
#define MMC_MBR_SIGNATURE_BYTE_1  0xAA

/* GPT Offsets */
#define PROTECTIVE_MBR_SIZE       BLOCK_SIZE
#define HEADER_SIZE_OFFSET        12
#define HEADER_CRC_OFFSET         16
#define PRIMARY_HEADER_OFFSET     24
#define BACKUP_HEADER_OFFSET      32
#define FIRST_USABLE_LBA_OFFSET   40
#define LAST_USABLE_LBA_OFFSET    48
#define PARTITION_ENTRIES_OFFSET  72
#define PARTITION_COUNT_OFFSET    80
#define PENTRY_SIZE_OFFSET        84
#define PARTITION_CRC_OFFSET      88

#define MIN_PARTITION_ARRAY_SIZE  0x4000

#define PARTITION_ENTRY_LAST_LBA  40

#define ENTRY_SIZE              0x080
#define UNIQUE_GUID_OFFSET        16
#define FIRST_LBA_OFFSET          32
#define LAST_LBA_OFFSET           40
#define ATTRIBUTE_FLAG_OFFSET     48
#define PARTITION_NAME_OFFSET     56

#define MAX_GPT_NAME_SIZE          72
#define PARTITION_TYPE_GUID_SIZE   16
#define UNIQUE_PARTITION_GUID_SIZE 16
#define NUM_PARTITIONS             32

/* Some useful define used to access the MBR/EBR table */
#define BLOCK_SIZE                0x200
#define TABLE_ENTRY_0             0x1BE
#define TABLE_ENTRY_1             0x1CE
#define TABLE_ENTRY_2             0x1DE
#define TABLE_ENTRY_3             0x1EE
#define TABLE_SIGNATURE           0x1FE
#define TABLE_ENTRY_SIZE          0x010

#define OFFSET_STATUS             0x00
#define OFFSET_TYPE               0x04
#define OFFSET_FIRST_SEC          0x08
#define OFFSET_SIZE               0x0C
#define COPYBUFF_SIZE             (1024 * 16)
#define BINARY_IN_TABLE_SIZE      (16 * 512)
#define MAX_FILE_ENTRIES          20

#define MBR_EBR_TYPE              0x05
#define MBR_MODEM_TYPE            0x06
#define MBR_MODEM_TYPE2           0x0C
#define MBR_SBL1_TYPE             0x4D
#define MBR_SBL2_TYPE             0x51
#define MBR_SBL3_TYPE             0x45
#define MBR_RPM_TYPE              0x47
#define MBR_TZ_TYPE               0x46
#define MBR_MODEM_ST1_TYPE        0x4A
#define MBR_MODEM_ST2_TYPE        0x4B
#define MBR_EFS2_TYPE             0x4E


#define BLOCK_SIZE                0x200
#define TABLE_ENTRY_0             0x1BE
#define TABLE_ENTRY_1             0x1CE
#define TABLE_ENTRY_2             0x1DE
#define TABLE_ENTRY_3             0x1EE
#define TABLE_SIGNATURE           0x1FE
#define TABLE_ENTRY_SIZE          0x010

#define MAX_PARTITIONS 64

#define NATIVE_LINUX_PARTITION 		0x83
#define VFAT_PARTITION 			0xb
#define VFAT16_PARTITION 		0x6
#define WIN_EXTENDED_PARTITION 		0xf
#define LINUX_EXTENDED_PARTITION 	0x5
#define RAW_EMPTY_PARTITION 		0x0
#define MBR_PROTECTED_TYPE        	0xEE

#define GET_LWORD_FROM_BYTE(x)    ((unsigned)*(x) | \
        ((unsigned)*(x+1) << 8) | \
        ((unsigned)*(x+2) << 16) | \
        ((unsigned)*(x+3) << 24))

#define GET_LLWORD_FROM_BYTE(x)    ((unsigned long long)*(x) | \
        ((unsigned long long)*(x+1) << 8) | \
        ((unsigned long long)*(x+2) << 16) | \
        ((unsigned long long)*(x+3) << 24) | \
        ((unsigned long long)*(x+4) << 32) | \
        ((unsigned long long)*(x+5) << 40) | \
        ((unsigned long long)*(x+6) << 48) | \
        ((unsigned long long)*(x+7) << 56))

#define GET_LONG(x)    ((uint32_t)*(x) | \
            ((uint32_t)*(x+1) << 8) | \
            ((uint32_t)*(x+2) << 16) | \
            ((uint32_t)*(x+3) << 24))

#define PUT_LONG(x, y)   *(x) = y & 0xff;     \
    *(x+1) = (y >> 8) & 0xff;     \
    *(x+2) = (y >> 16) & 0xff;    \
    *(x+3) = (y >> 24) & 0xff;

#define PUT_LONG_LONG(x,y)    *(x) =(y) & 0xff; \
     *((x)+1) = (((y) >> 8) & 0xff);    \
     *((x)+2) = (((y) >> 16) & 0xff);   \
     *((x)+3) = (((y) >> 24) & 0xff);   \
     *((x)+4) = (((y) >> 32) & 0xff);   \
     *((x)+5) = (((y) >> 40) & 0xff);   \
     *((x)+6) = (((y) >> 48) & 0xff);   \
     *((x)+7) = (((y) >> 56) & 0xff);


/* UEFI Partition Info Structure -- START */
struct partition_entry {
	unsigned char type_guid[PARTITION_TYPE_GUID_SIZE];
	unsigned dtype;
	unsigned char unique_partition_guid[UNIQUE_PARTITION_GUID_SIZE];
	unsigned long long first_lba;
	unsigned long long last_lba;
	unsigned long long size;
	unsigned long long attribute_flag;
	unsigned char name[MAX_GPT_NAME_SIZE];
};

struct gpt_partition_entry {
	unsigned char type_guid[PARTITION_TYPE_GUID_SIZE];
	unsigned char unique_partition_guid[UNIQUE_PARTITION_GUID_SIZE];
	unsigned long long first_lba;
	unsigned long long last_lba;
	unsigned long long attribute_flag;
	unsigned char name[MAX_GPT_NAME_SIZE];
};

struct uefi_header{
	uint8_t		magic[8];

	uint32_t 	version;
	uint32_t 	header_size;

	uint32_t 	crc32;
	uint8_t		rev[4];

	uint64_t	header_lba;
	uint64_t	backup_lba;
	uint64_t	first_lba;
	uint64_t	last_lba;

	uint8_t		guid[16];

	uint64_t	efi_entries_lba;
	uint32_t	efi_entries_count;
	uint32_t	efi_entries_size;
	uint32_t	efi_entries_crc32;
}__attribute__((packed));

struct guid_partition_tbl {
	uint8_t mbr[512];
	union{
		struct uefi_header guid_header;
		uint8_t alloc[512];
	};
	struct gpt_partition_entry guid_entry[ENTRY_SIZE];
};

struct guid_partition {
	const char* 	name;
	const char* 	type;
	unsigned int	size_kb;
};

static const uint8_t guid_partition_type[5][16] = {
	{ 0xa2, 0xa0, 0xd0, 0xeb, 0xe5, 0xb9, 0x33, 0x44,
	  0x87,	0xc0, 0x68, 0xb6, 0xb7, 0x26, 0x99, 0xc7,}, // Basic Data Partition.
	{ 0xaf, 0x3d, 0xc6, 0x0f, 0x83, 0x84, 0x72, 0x47,
	  0x8e, 0x79, 0x3d, 0x69, 0xd8, 0x47, 0x7d, 0xe4,}, // Linux Filesystem Data.
	{ 0x5d, 0x2a, 0x3a, 0xfe, 0x32, 0x4f, 0xa7, 0x41,
	  0xb7, 0x25, 0xac, 0xcc, 0x32, 0x85, 0xa3, 0x09,}, // Chrome OS Kernel.
	{ 0x02, 0xe2, 0xb8, 0x3c, 0x7e, 0x3b, 0xdd, 0x47,
	  0x8a, 0x3c, 0x7f, 0xf2, 0xa1, 0x3c, 0xfc, 0xec,}, // Chrome OS Boot.
	{ 0x3d, 0x75, 0x0a, 0x2e, 0x48, 0x9e, 0xb0, 0x43,
	  0x83, 0x37, 0xb1, 0x51, 0x92, 0xcb, 0x1b, 0x5e,}, // Chrome OS Reserved
};

static const uint8_t volume_guid[16] = {
	0x54, 0x45, 0x4c, 0x45, 0x43, 0x48, 0x49, 0x50, // TELECHIPSANDROID
	0x53, 0x41, 0x4e, 0x44, 0x53, 0x4f, 0x49, 0x44,
};

/* UEFI Partition Info Structure -- END*/

unsigned int read_partition_tlb();
void mbr_set_partition_name(struct partition_entry* mbr, unsigned int type);

int partition_get_index(const char* name);
unsigned long long partition_get_offset(int index);
unsigned long long partition_get_size(int index);
void partition_dump();
int get_partition_info(char *partition_name, unsigned long *addr, unsigned long *size);

int guid_format(const char* arg);



