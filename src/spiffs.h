#if !defined(SPIFFS_H)
#define SPIFFS_H

static const char BASE_PATH[] = "/spiffs";

void init_spiffs(void);
void list_dir(const char *name);


#endif  // SPIFFS_H
