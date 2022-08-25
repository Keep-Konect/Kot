#ifndef _UTILS_H
#define _UTILS_H 1

#define Kot_VendorID 0x107111116

static inline bool IsBeetween(uint64_t lower, uint64_t value, uint64_t upper){
    return (lower <= value) && (upper >= value);
}

#endif