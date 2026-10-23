/*  This test code is hereby placed in the PUBLIC DOMAIN
    for anyone to use for any purpose. */

#include <config.h>

#include <ctype.h> /* tolower() */
#include <stdio.h> /* printf() */

#include "dwarf.h"
#include "libdwarf.h"
#include "dd_defined_types.h"
#include "dd_checkutil.h"
#include "dd_glflags.h"
#include "dd_globals.h"

static void
test(const  char *t1, const char *t2,int resexp)
{
    Dwarf_Bool  res = is_strstrnocase(t1,t2);
    if (res == resexp) {
        return;
    }
    printf("Error,mismatch %s and %s.  Expected %d got %d\n",
        t1,t2,resexp,res);
}

int main(void)
{
    test("aaaaa","a",1);
    test("aab_varaaa","ab_var",1);
    /*  Matching even if container cannot match all of
        test string by being short but partially matching. */
    test("aab_va","ab_var",1);

    /* no match */
    test("iaibic","i3",0);
    /*  run off end of main string with partial
        match of test string.  */
    test("iaibii","i3",1);

    test("aaaaa","b",0);
    test("abaaba","ba",1);
    test("abaaxa","x",1);
    test("abaabA","Ba",1);
    test("a","ab",0);
    test("b","c",0);
    test("b","",0);
    test("","c",0);
    test("","",0);
    test("aaaaa","aaaaaaaa",0);
}

