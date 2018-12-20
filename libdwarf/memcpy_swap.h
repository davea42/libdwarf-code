

#ifndef MEMCPY_SWAP_H
#define MEMCPY_SWAP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void _dwarf_memcpy_swap_bytes(void *s1, const void *s2, 
    unsigned long len);
/*  It's inconvenient to use memcpy directly as it
    uses size_t and that requires <stddef.h> */
void _dwarf_memcpy_noswap_bytes(void *s1, const void *s2, 
    unsigned long len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MEMCPY_SWAP_H */

