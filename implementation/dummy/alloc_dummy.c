#include "interfaces/core/jd_alloc.h"
#include "jd_util.h"
#include "interfaces/core/jd_hw.h"

__attribute__((weak)) void jd_alloc_collision_check(void) {
}

__attribute__((weak)) void jd_alloc_init(void) {
}

__attribute__((weak)) void *jd_alloc(uint32_t size) {
    return NULL;
}

__attribute__((weak)) void jd_free(void* ptr) {
}

__attribute__((weak)) void *jd_alloc_emergency_area(uint32_t size) {
    return NULL;
}