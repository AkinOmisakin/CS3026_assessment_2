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
    mymkdir("/myfirstdir/myseconddir/mythirddir");
    // test mylistdir
    listdirs = mylistdir("/myfirstdir/myseconddir");
    for (int i = 0; i < DIRENTRYCOUNT; i++) 
    {
        if(strcmp(listdirs[i], "\0") != 0) 
        {
            printf("%s\n", listdirs[i]);
        }
    }
    
    writedisk("virtualdiskB3_B1_a");

    // test myfopen in path
    MyFILE * ptr_file = myfopen("/myfirstdir/myseconddir/testfile1.txt", "w");
    //test mylistdir
    listdirs = mylistdir("/myfirstdir/myseconddir");
    /* IN THE OUTPUT THIS LISTS THE mythirddir and testfile.txt */
    for (int i = 0; i < DIRENTRYCOUNT; i++) 
    {
        if(strcmp(listdirs[i], "\0") != 0) 
        {
            printf("%s\n", listdirs[i]);
        }
    }
    

    myfclose(ptr_file);
    writedisk("virtualdiskB3_B1_b");
    return 0 ;
}