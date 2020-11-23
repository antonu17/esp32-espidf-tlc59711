#if !defined(NVS_H)
#define NVS_H

void write_nvs_str(const char* namespace, const char* key, const char* data);
char* read_nvs_str(const char* namespace, const char* key);
void write_nvs_int(const char* namespace, const char* key, const int data);
int read_nvs_int(const char* namespace, const char* key);
void init_nvs();

#endif  // NVS_H
