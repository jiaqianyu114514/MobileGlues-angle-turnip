//
// Created by hanji on 2025/2/9.
//

#ifndef MOBILEGLUES_PLUGIN_SETTINGS_H
#define MOBILEGLUES_PLUGIN_SETTINGS_H

struct global_settings_t {
    int angle; // 0, 1
    int ignore_error; // 0, 1, 2
    int ext_gl43; // 0, 1
    int ext_compute_shader; // 0, 1
};

extern struct global_settings_t global_settings;

void init_settings();

#endif //MOBILEGLUES_PLUGIN_SETTINGS_H
