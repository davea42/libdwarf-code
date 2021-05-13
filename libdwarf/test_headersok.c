/*   Just to verify no typos preventing inclusion */
/*   This test code is hereby placed in the public domain. */
#include "dwarf_reloc_386.h"
#include "dwarf_reloc_mips.h"
#include "dwarf_reloc_ppc.h"
#include "dwarf_reloc_arm.h"
#include "dwarf_reloc_ppc64.h"
#include "dwarf_reloc_x86_64.h"

/*  The assignments and tests are to avoid compiler warnings
    with -Wall */
int main()
{
    const char *y = reloc_type_names_PPC64[0];
    if (!y) {
        return 1;
    }
    y = reloc_type_names_X86_64[0];
    if (!y) {
        return 1;
    }
    y = reloc_type_names_ARM[0];
    if (!y) {
        return 1;
    }
    y = reloc_type_names_PPC[0];
    if (!y) {
        return 1;
    }
    y = reloc_type_names_MIPS[0];
    if (!y) {
        return 1;
    }
    y = reloc_type_names_386[0];
    if (!y) {
        return 1;
    }
    y = reloc_type_names_386[0];
    if (!y) {
        return 1;
    }
    return 0;
}
