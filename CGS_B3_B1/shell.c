#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "filesys.h"

int main()
{
    // test format
    format();
    char ** listdirs;
    //test mymkdir
    mymkdir("/myfirstdir/myseconddir/mythirddir");
    // test mylistdir
    listdirs = mylistdir("/myfirstdir/myseconddir");
    for (int i = 0; i < DIRENTRYCOUNT; i++) {
        if(strcmp(listdirs[i], "\0") != 0) {
            printf("%s\n", listdirs[i]);
        }
    }
    
    writedisk("virtualdiskB3_B1_a");
    // test mymkdir
    mymkdir("/myfirstdir/myseconddir/testfile.txt");
    
    listdirs = mylistdir("/myfirstdir/myseconddir");
    for (int i = 0; i < DIRENTRYCOUNT; i++) {
        if(strcmp(listdirs[i], "\0") != 0) {
            printf("%s\n", listdirs[i]);
        }
    }
    
    writedisk("virtualdiskB3_B1_b");
    return 0 ;
}