#if !defined(STORAGE_H)
#define STORAGE_H

void write_nvs_str(const char* ns, const char* key, const char* data);
char* read_nvs_str(const char* ns, const char* key);
void write_nvs_int(const char* ns, const char* key, const int data);
int read_nvs_int(const char* ns, const char* key);
void init_storage(void);

#endif  // STORAGE_H
