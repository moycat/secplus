#include <stdint.h>
#include <storage/storage.h>
#include <furi_hal_random.h>
#include "secplus.h"
#include "storage.h"

bool secplus_init_storage() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(!storage_simply_mkdir(storage, SECPLUS_APP_FOLDER)) {
        FURI_LOG_E(TAG, "Could not create folder %s", SECPLUS_APP_FOLDER);
        furi_record_close(RECORD_STORAGE);
        return false;
    }
    if(storage_file_exists(storage, SECPLUS_APP_FILE)) {
        File* file = storage_file_alloc(storage);
        if(storage_file_open(file, SECPLUS_APP_FILE, FSAM_READ, FSOM_OPEN_EXISTING)) {
            if(storage_file_size(file) == 8) {
                storage_file_close(file);
                storage_file_free(file);
                furi_record_close(RECORD_STORAGE);
                return true;
            }
            storage_file_close(file);
        }
        storage_file_free(file);
    }
    FURI_LOG_I(TAG, "Initializing codes...");
    uint32_t codes[2];
    codes[0] = furi_hal_random_get() >> 2;
    codes[1] = furi_hal_random_get() >> 2;
    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, SECPLUS_APP_FILE, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_E(TAG, "Could not create file %s", SECPLUS_APP_FILE);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }
    storage_file_write(file, codes, 8);
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return true;
}

bool secplus_read_codes(SecPlusApp* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, SECPLUS_APP_FILE, FSAM_READ, FSOM_OPEN_EXISTING) ||
       storage_file_size(file) != 8) {
        FURI_LOG_E(TAG, "Could not open file %s", SECPLUS_APP_FILE);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }
    size_t read = 0;
    while(read < 8) {
        size_t to_read = 8 - read;
        uint16_t now_read = storage_file_read(file, (void*)(app->codes) + read, (uint16_t)to_read);
        read += now_read;
    }
    FURI_LOG_I(TAG, "Read codes, fixed=%lu, rolling=%lu", app->codes[0], app->codes[1]);
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return true;
}

bool secplus_save_codes(SecPlusApp* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    if(!storage_file_open(file, SECPLUS_APP_FILE, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_E(TAG, "Could not open file %s", SECPLUS_APP_FILE);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }
    storage_file_write(file, app->codes, 8);
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    FURI_LOG_I(TAG, "Saved codes, fixed=%lu, rolling=%lu", app->codes[0], app->codes[1]);
    return true;
}
