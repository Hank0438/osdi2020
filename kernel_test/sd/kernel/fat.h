int fat_get_mbr();
int fat_get_partition();
int fat_get_root_dir();
void fat_get_fat_table();
int fat_setup_mount();
void fat_read_file();
void fat_write_file();
int fat_lookup(const char* name, const char* ext);
void fat_create(const char* component_name);