

static unsigned count;
/*
2000: Could not allocate Dwarf_Error structure, abort() in libdwarf.
1000: FAIL:bad alloc caughtstd::bad_alloc
500:  FAIL:bad alloc caughtstd::bad_alloc
250:  FAIL:bad alloc caughtstd::bad_alloc
125:  Could not allocate Dwarf_Error structure, abort() in libdwarf.
      Aborted (core dumped)
100:  FAIL:bad alloc caughtstd::bad_alloc
46-60: FAIL:bad alloc caughtstd::bad_alloc  
45- Could not allocate Dwarf_Error structure, abort() in libdwarf.
    Aborted (core dumped)
42-44: FAIL:bad alloc caughtstd::bad_alloc  
30- 41: Could not allocate Dwarf_Error structure, abort() in libdwarf.
    Aborted (core dumped)
28-29  :FAIL:bad alloc caught std::bad_alloc
    Aborted (core dumped)
  
8,9,10,11-27: Could not allocate Dwarf_Error structure, abort() in libdwarf.
    Aborted (core dumped)
7: 6: FAIL:bad alloc caught std::bad_alloc
 
0,1,2,3,4,5: terminate called after throwing an instance of 'std::bad_alloc'
  what():  std::bad_alloc
  Aborted (core dumped)
*/

extern void * __libc_malloc();

void *malloc(unsigned len)
{
  if (count == 499) {
    return 0;
  }
  count++;
  return __libc_malloc(len);
}
