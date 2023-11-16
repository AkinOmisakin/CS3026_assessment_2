#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "filesys.h"

int main()
{
    // test format
    format();
    // call print block
    printBlock(0);
    writedisk("virtualdiskD3_D1");
    return 0 ;
}