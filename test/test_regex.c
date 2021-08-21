/*   This test code is hereby placed in the public domain. */
#include <stdio.h>
#include "dd_regex.h"

#define DW_DLV_NO_ENTRY -1
#define DW_DLV_OK        0
#define DW_DLV_ERROR     1

static const char *
ename(int v)
{
    switch(v) {
    case DW_DLV_OK:
        return "DW_DLV_OK";
    case DW_DLV_NO_ENTRY:
        return "DW_DLV_NO_ENTRY";
    case DW_DLV_ERROR:
        return "DW_DLV_ERROR";
    }
    return "Unknown return value!!!";
}

static int errcount = 0;
static void
checkres(int res, const char *func,
    const char *re,
    const char *test,int expres,int line)
{
     if (res != expres) {
         printf("FAIL %s re %s input %s got %s exp %s line %d\n",
             func,re,test,
             ename(res), 
             ename(expres), 
             line);
         ++errcount;
     }
     return;
}


static void
test1(void)
{
    int res = 0;
    const char *comp = ".*";
    const char *check = "abc";

    printf("Test1 exp %s str %s,line %d\n",comp,check,__LINE__);
    res = dd_re_comp((char *)comp);
    checkres(res,"dd_re_comp",comp,check,DW_DLV_OK,__LINE__);
    if (res) {
        return;
    }
    res = dd_re_exec((char *)check);
    checkres(res,"dd_re_exec",comp,check,DW_DLV_OK,__LINE__);
}
static void
test2(void)
{
    int res = 0;
    const char *comp = "abc";
    const char *check = "ab";

    printf("Test2 exp %s str %s,line %d\n",comp,check,__LINE__);
    res = dd_re_comp((char *)comp);
    checkres(res,"dd_re_comp",comp,check,DW_DLV_OK,__LINE__);
    if (res) {
        return;
    }
    res = dd_re_exec((char *)check);
    checkres(res,"dd_re_exec",comp,check,DW_DLV_NO_ENTRY,__LINE__);
}

static void
test3(void)
{
    int res = 0;
    const char *comp = "a[xy]j";
    const char *check = "ab";

    printf("Test3 exp %s str %s,line %d\n",comp,check,__LINE__);
    res = dd_re_comp((char *)comp);
    checkres(res,"dd_re_comp",comp,check,DW_DLV_OK,__LINE__);
    if (res) {
        return;
    }
    res = dd_re_exec((char *)check);
    checkres(res,"dd_re_exec",comp,check,DW_DLV_NO_ENTRY,__LINE__);
}
static void
test4(void)
{
    int res = 0;
    /* expect all OK */
    const char *comp = "a[xy]j";
    const char *check = "ayj";

    printf("Test4 exp %s str %s,line %d\n",comp,check,__LINE__);
    res = dd_re_comp((char *)comp);
    checkres(res,"dd_re_comp",comp,check,DW_DLV_OK,__LINE__);
    if (res) { 
        return;
    }
    res = dd_re_exec((char *)check);
    checkres(res,"dd_re_exec",comp,check,DW_DLV_OK,__LINE__);
}

static void
test5(void)
{
    int res = 0;
    /* pattern ok, check string does not match */
    const char *comp = "a[xy]j";
    const char *check = "afj";

    printf("Test5 exp %s str %s,line %d\n",comp,check,__LINE__);
    res = dd_re_comp((char *)comp);
    checkres(res,"dd_re_comp",comp,check,DW_DLV_OK,__LINE__);
    if (res) {
        return;
    }
    res = dd_re_exec((char *)check);
    checkres(res,"dd_re_exec",comp,check,DW_DLV_NO_ENTRY,__LINE__);
}

static void
test6(void)
{
    int res = 0;
    /* incomplete pattern */
    const char *comp = "a[xyj";
    const char *check = "afj";

    printf("Test6 exp %s str %s,line %d\n",comp,check,__LINE__);
    res = dd_re_comp((char *)comp);
    checkres(res,"dd_re_comp",comp,check,DW_DLV_ERROR,__LINE__);
    if (res) {
        return;
    }
    res = dd_re_exec((char *)check);
    checkres(res,"dd_re_exec",comp,check,DW_DLV_NO_ENTRY,__LINE__);
}

/* Simpler testing */
static void
testx(const char *expr,
    int expv1,
    const char*check,
    int intexpv2,
    int line)
{
    int res = 0;

    printf("Test exp %s str %s,line %d\n",expr,check,line);
    res = dd_re_comp((char *)expr);
    checkres(res,"dd_re_comp",expr,check,expv1,line);
    if (res) {
        return;
    }
    res = dd_re_exec((char *)check);
    checkres(res,"dd_re_exec",expr,check,intexpv2,line);
}



int main()
{

    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    /*  A trailing] with no preceeding [ is not
        an error */
    testx("axy]j",DW_DLV_OK,"abc",DW_DLV_NO_ENTRY,__LINE__);
    testx("axy]j",DW_DLV_OK,"axy]j",DW_DLV_OK,__LINE__);
    testx("xy]",DW_DLV_OK,"axy]j",DW_DLV_OK,__LINE__);
    testx("+x",DW_DLV_ERROR,"x",DW_DLV_ERROR,__LINE__);
    testx("x+",DW_DLV_OK,"x",DW_DLV_OK,__LINE__);
    testx("^ax",DW_DLV_OK,"yaxfoo",DW_DLV_NO_ENTRY,__LINE__);
    testx("^ax",DW_DLV_OK,"axfoob",DW_DLV_OK,__LINE__);
    testx(0,DW_DLV_NO_ENTRY,"axfoob",DW_DLV_OK,__LINE__);
    testx("",DW_DLV_NO_ENTRY,"axfoob",DW_DLV_OK,__LINE__);
    testx("+",DW_DLV_ERROR,"axfoob",DW_DLV_OK,__LINE__);
    testx("(+",DW_DLV_OK,"axfoob",DW_DLV_NO_ENTRY,__LINE__);
    testx("(+",DW_DLV_OK,"((",DW_DLV_OK,__LINE__);
    testx("(+)",DW_DLV_OK,"x((()",DW_DLV_OK,__LINE__);
    testx("yy+(",DW_DLV_OK,"yyy(",DW_DLV_OK,__LINE__);
    testx("[fx]+i[0-9]*",DW_DLV_OK,"yxxffi123",DW_DLV_OK,__LINE__);
    testx("a\\(\\(\\(\\(\\(\\(\\(\\(\\(\\(\\(\\(\\(\\(\\(\\(\\(\(\(\()))))))))))))))))))))",DW_DLV_ERROR,"yxxffi123",DW_DLV_NO_ENTRY,__LINE__);
    testx("[fx]+[0-9]*a",DW_DLV_OK,"yxxff123a",DW_DLV_OK,__LINE__);
    testx("[fx]+[0-9]*",DW_DLV_OK,"yxxffi123a",DW_DLV_OK,__LINE__);
    testx("[fx]+[0-9]+",DW_DLV_OK,"yxxffi123a",DW_DLV_NO_ENTRY,__LINE__);
    testx("[fx]+",DW_DLV_OK,"yxxffi123a",DW_DLV_OK,__LINE__);
    testx("[fx]+i[0-9]+",DW_DLV_OK,"yxxffi123",DW_DLV_OK,__LINE__);
    testx("a[fx]+b[cd]",DW_DLV_OK,"afffbdddy",DW_DLV_OK,__LINE__);
    testx("a[fx]+b[cd]",DW_DLV_OK,"afffdddy",DW_DLV_NO_ENTRY,__LINE__);
    if (errcount > 0) {
        printf("FAIL test_regex\n");
        return 1;
    }
    return 0;
}
