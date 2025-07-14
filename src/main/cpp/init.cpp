//
// Created by Swung0x48 on 2024/10/8.
//
#ifdef __cplusplus
extern "C" {
#endif

#include "includes.h"

#ifdef __cplusplus
}
#endif

struct static_block_t {
    static_block_t() {
        proc_init();
    }
};

static static_block_t static_block;