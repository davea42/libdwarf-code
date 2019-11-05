/*  This is the test file used to compile
    testobjLE32PE.exe under MinGW on Windows 8.1 
    These leading comments mean a recompile would
    not exactly match line numbers in the DWARF. */
#include <stdio.h>

struct something {
int a;
unsigned b;
};

int 
buffle(struct something *v )
{
     return v->a + 42;

}

int main(int argc, char **argv)
{

    int x = 12;
     int y = 24;
    struct something so;

    so.a = x;
    x  = buffle(&so);
    return x +y + 4 +argc;
}
