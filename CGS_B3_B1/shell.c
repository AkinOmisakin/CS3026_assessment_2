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
    /* commented out for C3_C1 writedisk("virtualdiskD3_D1");*/
    // create testfile.txt in virtualDisk in read mode
    MyFILE * ptr_file = myfopen("testfile.txt", "w");
    if (ptr_file == NULL)
    {
      printf("FILE NOT OPENED");
      return 0;
    }
    // test myfputc
    /*char arraytext[4*BLOCKSIZE]; // text size set: 4 * BLOCKSIZE = 4096
    const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // alphabet text
    // fill array with chars randomly from alphabet
    for (int i=0; i < sizeof(arraytext); ++i)
    {
      arraytext[i] = alphabet[rand() % 26];
    }*/
    // when full
    // fill file with text contents
    /*for (int i=0; i < sizeof(arraytext);++i)
    {
        myfputc(arraytext[i], ptr_file);
    }
    //test myfclose
    myfclose(ptr_file);
    //test myfgetc
    // reopen file in read mode
    ptr_file = myfopen("testfile.txt", "r");
    // check file opened
    if (ptr_file == NULL)
    {
      printf("file not opened\n");
      return 0;
    }*/
    //myfgetc(ptr_file);
    //myfclose(ptr_file);
    /* commented out of part B
    writedisk("virtualdiskC3_C1"); 
    */

    //test mymkdir
    writedisk("virtualdiskB3_B1_a");
    // test mylistdir
    writedisk("virtualdiskB3_B1_b");
    return 0 ;
}