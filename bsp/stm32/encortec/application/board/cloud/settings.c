#include "settings.h"
#include "rtthread.h"
#include <stdio.h>

#include "logging.h"
// #define DBG_TAG "settings"
// #define DBG_LVL DBG_LOG
// #include <rtdbg.h>

int settings_init(settings_t *settings, const char *settings_file_path, settings_params_t *params) {
    if(settings && settings_file_path) {
        rt_memset(settings, 0, sizeof(settings_t));
        rt_snprintf(settings->settings_file_path, sizeof(settings->settings_file_path), settings_file_path);
        FILE *file = fopen(settings_file_path, "rb+");
        if(!file) {
            file = fopen(settings_file_path, "wb+");
        }

        if(file) {
            if(params) {
                rt_memcpy(&settings->params, params, sizeof(settings_params_t));
                if(fwrite(params, 1, sizeof(settings_params_t), file) != sizeof(settings_params_t)) {
                    fclose(file);
                    return -1;
                } else {
                    fclose(file);
                    return 0;
                }
            } else {
                fclose(file);
                return 0;
            }
        } else {
            return -1;
        }
    } else {
        return -1;
    }
}

int settings_update(settings_t *settings, settings_params_t *params) {
    if(settings && params) {
        rt_memcpy(&settings->params, params, sizeof(settings_params_t));
        FILE *file = fopen(settings->settings_file_path, "wb+");
        if(!file) {
            return -1;
        } else {
            if(fwrite(params, 1, sizeof(settings_params_t), file) != sizeof(settings_params_t)) {
                fclose(file);
                return -1;
            } else {
                fclose(file);
                return 0;
            }
        }
    } else {
        return -1;
    }
}

settings_params_t *settings_read(settings_t *settings) {
    if(settings) {
        FILE *file = fopen(settings->settings_file_path, "rb+");
        if(!file) {
            return NULL;
        } else {
            fseek(file, 0, SEEK_END);
            size_t file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            if(!file_size) {
                fclose(file);
                return &settings->params;
            }

            int len = 0;
            if((len = fread(&settings->params, 1, sizeof(settings_params_t), file)) != sizeof(settings_params_t)) {
                fclose(file);
                return NULL;
            } else {
                fclose(file);
                return &settings->params;
            }
        }
    } else {
        return NULL;
    }
}
