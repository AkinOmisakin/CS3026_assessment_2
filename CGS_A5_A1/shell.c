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
    char text1 [32] = "This is the text for tesfile 1";
    for (int i=0;i<sizeof(text1);++i)
    {
        myfputc(text1[i], ptr_file);
    }
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
    char text2 [32] = "This is the text for tesfile 2";
    for (int i=0;i<sizeof(text2);++i)
    {
        myfputc(text2[i], ptr_file2);
    }
    //close file
    myfclose(ptr_file2);
    //test
    printf("Path contents ... \n");
    listdirs = mylistdir(".");
    for (int i = 0; i < DIRENTRYCOUNT; i++) 
    {
        if(strcmp(listdirs[i], "\0") != 0) 
        {
            printf("%s\n", listdirs[i]);
        }
    }

    //create third directory
    mymkdir("/thirddir");
    //create file in thirddir
    MyFILE * ptr_file3 = myfopen("thirddir/testfile3.txt", "w");
    char text3 [32] = "This is the text for tesfile 3";
    for (int i=0;i<sizeof(text3);++i)
    {
        myfputc(text3[i], ptr_file3);
    }
    //close file
    myfclose(ptr_file3);
    printf("Path contents ... \n");
    listdirs = mylistdir(".");
    for (int i = 0; i < DIRENTRYCOUNT; i++) 
    {
        if(strcmp(listdirs[i], "\0") != 0) 
        {
            printf("%s\n", listdirs[i]);
        }
    } 
    // write
    writedisk("virtualdiskA5_A1_a");
   
    //change dir
    mychdir("/firstdir/seconddir");
    //test
    printf("Path contents ... \n");
    listdirs = mylistdir(".");
    for (int i = 0; i < DIRENTRYCOUNT; i++) 
    {
        if(strcmp(listdirs[i], "\0") != 0) 
        {
            printf("%s\n", listdirs[i]);
        }
    }
    // remove files
    myremove("testfile1.txt");
    myremove("testfile2.txt");
    //test
    printf("Path contents ... \n");
    listdirs = mylistdir(".");
    for (int i = 0; i < DIRENTRYCOUNT; i++) 
    {
        if(strcmp(listdirs[i], "\0") != 0) 
        {
            printf("%s\n", listdirs[i]);
        }
    }
    writedisk("virtualdiskA5_A1_b");
    // change dir
    mychdir("thirddir");
    // remove files
    myremove("testfile3.txt");
    writedisk("virtualdiskA5_A1_c");
    // change dir
    mychdir("/");
    //delete dir
    myrmdir("thirddir");
    //test
    printf("Path contents ... \n");
    listdirs = mylistdir(".");
    for (int i = 0; i < DIRENTRYCOUNT; i++) 
    {
        if(strcmp(listdirs[i], "\0") != 0) 
        {
            printf("%s\n", listdirs[i]);
        }
    }
    // change dir
    mychdir("/firstdir");
    // delete dir
    myrmdir("seconddir");
    //test
    printf("Path contents ... \n");
    listdirs = mylistdir(".");
    for (int i = 0; i < DIRENTRYCOUNT; i++) 
    {
        if(strcmp(listdirs[i], "\0") != 0) 
        {
            printf("%s\n", listdirs[i]);
        }
    }
    //change dir
    mychdir(".."); // or mychdir("..")
    // delete dir
    myrmdir("firstdir");
    //test
    printf("Path contents ... \n");
    listdirs = mylistdir(".");
    for (int i = 0; i < DIRENTRYCOUNT; i++) 
    {
        if(strcmp(listdirs[i], "\0") != 0) 
        {
            printf("%s\n", listdirs[i]);
        }
    }
    //write
    writedisk("virtualdiskA5_A1_d");

    /* A3 Copy functions test
    */
    //COPY from real file in hard disk to virtual disk
    /* IF THERE IS ALREADY TEXT IN testfilecopy2.txt DELETE THE CONTENTS
        THEN RUN THE CODE BELOW */
    mychdir("/");
    FILE * realfile = fopen("testfilecopy1.txt", "r");
    MyFILE * fakefile = myfopen("testfile5.txt", "w");
    CopyToMyFILE(realfile, fakefile);
    fclose(realfile);
    myfclose(fakefile);
    // Copy Vice versa function
    mychdir("/");
    FILE * realfile2 = fopen("testfilecopy2.txt", "w");
    fakefile = myfopen("testfile5.txt", "r");
    CopyToRealFILE(fakefile, realfile2);
    fclose(realfile);
    myfclose(fakefile);  
    writedisk("virtualdiskA3");
    // deleting files
    mychdir("/");
    myremove("testfile5.txt");
  
    /* A2 Copy and move function
        */
    mychdir("/");
    MyFILE * newfile1 = myfopen("testfile6.txt", "w");
    char text6 [32] = "This is the text for tesfile 6";
    for (int i=0;i<sizeof(text6);++i)
    {
        myfputc(text6[i], newfile1);
    }
    myfclose(newfile1);
    newfile1 = myfopen("testfile6.txt", "r");
    MyFILE * newfile2 = myfopen("testfile7.txt", "w");
    movefile("testfile6.txt", "testfile7.txt");
    writedisk("virtualdiskA2");
    
    return 0 ;
}