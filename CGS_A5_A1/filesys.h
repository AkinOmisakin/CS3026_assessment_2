/* filesys.h
 * 
 * describes FAT structures
 * http://www.c-jump.com/CIS24/Slides/FAT/lecture.html#F01_0020_fat
 * http://www.tavi.co.uk/phobos/fat.html
 */

#ifndef FILESYS_H
#define FILESYS_H

#include <time.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MAXBLOCKS     1024
#define BLOCKSIZE     1024
#define FATENTRYCOUNT (BLOCKSIZE / sizeof(fatentry_t))
#define DIRENTRYCOUNT ((BLOCKSIZE - (2*sizeof(int)) ) / sizeof(direntry_t))
#define MAXNAME       256
#define MAXPATHLENGTH 1024

#define UNUSED        -1
#define ENDOFCHAIN     0
#define EOF           -1

#define LOCKED        1
#define UNLOCKED      0

typedef unsigned char Byte ;

/* create a type fatentry_t, we set this currently to short (16-bit)
 */
typedef short fatentry_t ;


// a FAT block is a list of 16-bit entries that form a chain of disk addresses

//const int   fatentrycount = (blocksize / sizeof(fatentry_t)) ;

typedef fatentry_t fatblock_t [ FATENTRYCOUNT ] ;


/* Introducing extra struct for block 0 
*/
typedef struct block0 {
   char name [MAXNAME] ;
   int lock;
} block0_t ;

/* create a type direntry_t
 */

typedef struct direntry {
   int         entrylength ;   // records length of this entry (can be used with names of variables length)
   Byte        isdir ;
   Byte        unused ;
   time_t      modtime ;
   int         filelength ;
   fatentry_t  firstblock ;
   char   name [MAXNAME] ;
} direntry_t ;

// a directory block is an array of directory entries

//const int   direntrycount = (blocksize - (2*sizeof(int)) ) / sizeof(direntry_t) ;

typedef struct dirblock {
   int isdir ;
   int nextEntry ;
   direntry_t entrylist [ DIRENTRYCOUNT ] ; // the first two integer are marker and endpos
} dirblock_t ;



// a data block holds the actual data of a filelength, it is an array of 8-bit (byte) elements

typedef Byte datablock_t [ BLOCKSIZE ] ;


// a diskblock can be either a directory block, a FAT block or actual data

typedef union block {
   datablock_t data ;
   dirblock_t  dir  ;
   fatblock_t  fat  ;
} diskblock_t ;

// finally, this is the disk: a list of diskblocks
// the disk is declared as extern, as it is shared in the program
// it has to be defined in the main program filelength

extern diskblock_t virtualDisk [ MAXBLOCKS ] ;


// when a file is opened on this disk, a file handle has to be
// created in the opening program

typedef struct filedescriptor {
   char        mode[3] ;
   fatentry_t  blockno ;           // block no
   int         pos     ;           // byte within a block
   diskblock_t buffer  ;
} MyFILE ;

/*implement & test helper functions
*/ 
//D3 - D1
void format() ; 
void writedisk ( const char * filename ) ;
void printBlock( int blockIndex);
//C3 - C1
MyFILE * myfopen ( const char * filename, const char * mode ) ;
void myfclose ( MyFILE * stream ) ;
int myfgetc ( MyFILE * stream );
void myfputc ( int b, MyFILE * stream ) ;
// B3 - B1
void mymkdir ( const char * path );
void myrmdir ( const char * path );
void mychdir ( const char * path );
void myremove ( const char * path );
char ** mylistdir (const char * path); 
//added functions
int findUNUSEDfatentry () ;
void addfatentry ( int blokno) ;
void addtofatentry (int blokno , int newblokno) ;
int findfilebyname (dirblock_t * current, const char * filename);
void deletefat(int blokno);
void CopyToMyFILE ( FILE * realfile, MyFILE * fakefile) ;
void CopyToRealFILE ( MyFILE * fakefile, FILE * realfile);
void movefile (const char* file1, const char * file2) ;
#endif

/*
#define NUM_TYPES (sizeof types / sizeof types[0])
static* int types[] = { 
    1,
    2, 
    3, 
    4 };
*/