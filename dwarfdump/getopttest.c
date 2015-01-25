
/* This is for testing the local getopt. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for strcmp */
#include "dwgetopt.h" /* for dwgetopt */
char *argv1[20];

static void
chkval(int ct,int c,int expchar,char *extra,char *expextra,int optnum,int
   expnum,const char *testid)
{
    int err = 0;
    if (c != expchar) {
        err++;
        printf("Mismatch %d %s: got char %c 0x%x exp char %c 0x%x\n",
            ct,testid,c,c,expchar,expchar);
    }
    if (extra != expextra) {
        /* pointers non-match */
        if (extra && expextra && (!strcmp(extra,expextra))) {
            /* strings match. */
        } else {
            err++;
            printf("Mismatch %d %s: got extra %s 0x%x exp extra %s 0x%x\n",
                ct,testid,
                extra?extra:"",(unsigned)extra,
                expextra?expextra:"", (unsigned)expextra);
        }
    }
    if (optnum != expnum) {
        err++;
        printf("Mismatch %d %s: got optnum %d 0x%x exp optnum %d 0x%x\n",
            ct,testid,optnum,optnum,expnum,expnum);
    }
    if (err > 0) {
        printf("FAIL getopttest\n");
        exit(1);
    }
}

static void
test1(void)
{
    int ct = 1;
    int c = 0;
    int argct = 10;
    argv1[0]="a.out";
    argv1[1]="-a";
    argv1[2]="-#";
    argv1[3]="6";
    argv1[4]="-ka";
    argv1[5]="-l";
    argv1[6]="-lv";
    argv1[7]="-x";
    argv1[8]="path=./foo";
    argv1[9]="progtoread";
    argv1[10]=0;
    for ( ;(c = dwgetopt(argct, argv1,
        "#:abc::CdDeE::fFgGhH:iIk:l::mMnNo::O:pPqQrRsS:t:u:UvVwW::x:yz"))
        != EOF; ct++) {
    switch(ct) {
    case 1:
        chkval(ct,c,'a',optarg,0,optind,2,"test1");
        break;
    case 2:
        chkval(ct,c,'#',optarg,"6",optind,4,"test2");
        break;
    case 3:
        chkval(ct,c,'k',optarg,"a",optind,5,"test3");
        break;
    case 4:
        chkval(ct,c,'l',optarg,0,optind,6,"test4");
        break;
    case 5:
        chkval(ct,c,'l',optarg,"v",optind,7,"test5");
        break;
    case 6:
        chkval(ct,c,'x',optarg,"path=./foo",optind,9,"test6");
        break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    default:
        printf("FAIL unexpected ct %d\n",ct);
        exit(1);
    }
    }
#if 0
    printf(" final check: ct %d optind %d\n",ct,optind);
#endif
    if (strcmp(argv1[optind],"progtoread")) {
        printf("FAIL on non-dash arg\n");
    }
    printf("PASS getopt\n");
    exit(0);
}

int main(int argc, const char **argv)
{
    int ct = 0;
#if 0
    printf("argc: %d \n",argc);
    for( ct = 0; ct < argc ; ++ct) {
        printf("argv[%d] = %s\n",ct,argv[ct]);
    }
#endif
    test1();
    return 0;
}
