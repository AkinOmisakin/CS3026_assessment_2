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
    printf("Path contents ... \n");
    listdirs = mylistdir("/firstdir/seconddir");
    for (int i = 0; i < DIRENTRYCOUNT; i++) 
    {
        if(strcmp(listdirs[i], "\0") != 0) 
        {
            printf("%s\n", listdirs[i]);
        }
    }
    // change directory here --->
    mychdir("/firstdir/seconddir");
    // print the contents of path with . reference
    printf("Path contents ... \n");
    listdirs = mylistdir(".");
    for (int i = 0; i < DIRENTRYCOUNT; i++) 
    {
        if(strcmp(listdirs[i], "\0") != 0) 
        {
            printf("%s\n", listdirs[i]);
        }
    }
    // create new file
    MyFILE * ptr_file2 = myfopen("testfile2.txt", "w");
    // text to input to file above
    char text2 [32] = "This is the text fot tesfile 2";
    for (int i=0;i<sizeof(text2);++i)
    {
        myfputc(text2[i], ptr_file2);
    }
    //close
    myfclose(ptr_file2);
    //create third directory
    mymkdir("/thirddir");
    
    writedisk("virtualdiskA5_A1_a");
    return 0 ;
}