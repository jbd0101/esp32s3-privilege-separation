// Copyright 2020 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//         http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "soc/soc.h"

/* WORLD1 range */
#define SOC_UDROM_LOW    0x3C400000
#define SOC_UDROM_HIGH   0x3C800000

#define SOC_UIROM_LOW    0x42400000
#define SOC_UIROM_HIGH   0x42800000

/* TLS system calls require larger kernel stack size.
 * If TLS not in use, this can be set to 2560
 */
#define KERNEL_STACK_SIZE   3072
#define tskSTACK_FILL_BYTE  0xa5U

#define UIRAM_ATTR __attribute__((section(".uiram")))
#define UDRAM_ATTR __attribute__((section(".udram")))

#ifndef __ASSEMBLER__

extern int _reserve_w1_dram_start, _reserve_w1_dram_end;
extern int _reserve_w1_iram_start, _reserve_w1_iram_end;

typedef struct {
    uint32_t user_app_dram_start;
    uint32_t user_app_heap_start;
} usr_custom_app_desc_t;

static inline int is_valid_uiram_addr(void *ptr)
{
    if (ptr >= (void *)&_reserve_w1_iram_start && ptr < (void *)&_reserve_w1_iram_end) {
        return 1;
    }

    return 0;
}

static inline int is_valid_udram_addr(void *ptr)
{
    if (ptr >= (void *)&_reserve_w1_dram_start && ptr < (void *)&_reserve_w1_dram_end) {
        return 1;
    }

    return 0;
}

static inline int is_valid_user_i_addr(void *ptr)
{
    if (ptr >= (void *)SOC_UIROM_LOW && ptr < (void *)SOC_UIROM_HIGH) {
        return 1;
    }

    return is_valid_uiram_addr(ptr);
}

static inline int is_valid_user_d_addr(void *ptr)
{
    if (ptr >= (void *)SOC_UDROM_LOW && ptr < (void *)SOC_UDROM_HIGH) {
        return 1;
    }

    return is_valid_udram_addr(ptr);
}

static inline int is_valid_kernel_i_addr(void *ptr)
{
    if (is_valid_user_i_addr(ptr)) {
        return 0;
    }

    if (ptr >= (void *)SOC_IROM_LOW && ptr < (void *)SOC_IROM_HIGH) {
        return 1;
    }

    if (ptr >= (void *)SOC_IRAM_LOW && ptr < (void *)SOC_IRAM_HIGH) {
        return 1;
    }

    return 0;
}

static inline int is_valid_kernel_d_addr(void *ptr)
{
    if (is_valid_user_d_addr(ptr)) {
        return 0;
    }

    if (ptr >= (void *)SOC_DROM_LOW && ptr < (void *)SOC_DROM_HIGH) {
        return 1;
    }

    if (ptr >= (void *)SOC_DRAM_LOW && ptr < (void *)SOC_DRAM_HIGH) {
        return 1;
    }

    if (ptr >= (void *)SOC_RTC_DRAM_LOW && ptr < (void *)SOC_RTC_DRAM_HIGH) {
        return 1;
    }

    return 0;
}

#endif