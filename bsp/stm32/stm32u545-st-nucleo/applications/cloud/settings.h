#ifndef __ENCORTEC_SETTINGS_H
#define __ENCORTEC_SETTINGS_H

typedef struct {
    int collect_interval;
} settings_params_t;

typedef struct {
    char settings_file_path[64];
    settings_params_t params;
} settings_t;

extern int settings_init(settings_t *settings, const char *settings_file_path, settings_params_t *params);
extern int settings_update(settings_t *settings, settings_params_t *params);
extern settings_params_t *settings_read(settings_t *settings);

#endif
