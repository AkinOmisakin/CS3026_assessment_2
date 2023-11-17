#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "filesys.h"

int main()
{
    // test format
    format();
    // declare pointer to pointers
    char ** listdirs;
    //test mymkdir
    mymkdir("/firstdir/seconddir");
    MyFILE * ptr_file = myfopen("/firstdir/seconddir/testfile1.txt","w");
    myfclose(ptr_file);
    // test mylistdir
    listdirs = mylistdir("/myfirstdir/myseconddir");
    for (int i = 0; i < DIRENTRYCOUNT; i++) 
    {
        if(strcmp(listdirs[i], "\0") != 0) 
        {
            printf("%s\n", listdirs[i]);
        }
    }
    // change directory here --->
    
    writedisk("virtualdiskA5_A1_a");
    return 0 ;
}