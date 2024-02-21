#ifndef SECPLUS_STORAGE_HEADER
#define SECPLUS_STORAGE_HEADER

#define SECPLUS_APP_FOLDER EXT_PATH("secplus")
#define SECPLUS_APP_FILE EXT_PATH("secplus/key.data")

#include "secplus.h"

bool secplus_init_storage();
bool secplus_read_codes(SecPlusApp* app);
bool secplus_save_codes(SecPlusApp* app);

#endif