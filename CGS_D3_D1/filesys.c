/* filesys.c
 * 
 * provides interface to virtual disk
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "filesys.h"


diskblock_t  virtualDisk [MAXBLOCKS] ;           // define our in-memory virtual, with MAXBLOCKS blocks
fatentry_t   FAT         [MAXBLOCKS] ;           // define a file allocation table with MAXBLOCKS 16-bit entries
fatentry_t   rootDirIndex            = 0 ;       // rootDir will be set by format
direntry_t * currentDir              = NULL ;
fatentry_t   currentDirIndex         = 0 ;

/* writedisk : writes virtual disk out to physical disk
 * 
 * in: file name of stored virtual disk
 */

void writedisk ( const char * filename )
{
   printf ( "writedisk> virtualdisk[0] = %s\n", virtualDisk[0].data ) ;
   FILE * dest = fopen( filename, "w" ) ;
   if ( fwrite ( virtualDisk, sizeof(virtualDisk), 1, dest ) < 0 )
      fprintf ( stderr, "write virtual disk to disk failed\n" ) ;
   //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
   fclose(dest) ;
   
}

void readdisk ( const char * filename )
{
   FILE * dest = fopen( filename, "r" ) ;
   if ( fread ( virtualDisk, sizeof(virtualDisk), 1, dest ) < 0 )
      fprintf ( stderr, "write virtual disk to disk failed\n" ) ;
   //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
      fclose(dest) ;
}


/* the basic interface to the virtual disk
 * this moves memory around
 */

void writeblock ( diskblock_t * block, int block_address )
{
   //printf ( "writeblock> block %d = %s\n", block_address, block->data ) ;
   memmove ( virtualDisk[block_address].data, block->data, BLOCKSIZE ) ;
   //printf ( "writeblock> virtualdisk[%d] = %s / %d\n", block_address, virtualDisk[block_address].data, (int)virtualDisk[block_address].data ) ;
}


/* read and write FAT
 * 
 * please note: a FAT entry is a short, this is a 16-bit word, or 2 bytes
 *              our blocksize for the virtual disk is 1024, therefore
 *              we can store 512 FAT entries in one block
 * 
 *              how many disk blocks do we need to store the complete FAT:
 *              - our virtual disk has MAXBLOCKS blocks, which is currently 1024
 *                each block is 1024 bytes long
 *              - our FAT has MAXBLOCKS entries, which is currently 1024
 *                each FAT entry is a fatentry_t, which is currently 2 bytes
 *              - we need (MAXBLOCKS /(BLOCKSIZE / sizeof(fatentry_t))) blocks to store the
 *                FAT
 *              - each block can hold (BLOCKSIZE / sizeof(fatentry_t)) fat entries
 */

/* implement format()
 */
void format ( )
{
   
   /*
   direntry_t  rootDir ;
   int         pos             = 0 ;
   int         fatentry        = 0 ;
   int         fatblocksneeded =  (MAXBLOCKS / FATENTRYCOUNT ) ;
   */
   /* prepare block 0 : fill it with '\0',
    * use strcpy() to copy some text to it for test purposes
	* write block 0 to virtual disk
	*/
   diskblock_t block ; // block 0
   for(int i =0; i < BLOCKSIZE; ++i)
   {
      block.data[i] = '\0';
   }
   strcpy((char*)block.data, "CS3026 Operating Systems Assessment 2023");
   writeblock(&block,0);
	/* prepare FAT table
	 * write FAT blocks to virtual disk
	 */
   // can only store 512 entries in each block
   // we need 2 blocks to hold FAT entries
   diskblock_t block_1;
   diskblock_t block_2;
   // all FAT entries are UNUSED
   for(int i = 0; i < BLOCKSIZE; ++i)
   {
      FAT[i] = UNUSED; // 1024 entries set to UNUSED
   }
   FAT[0] = ENDOFCHAIN; // block 0
   FAT[1] = 2; // fat block 1
   FAT[2] = ENDOFCHAIN; // fat block 2
   FAT[3] = ENDOFCHAIN; // root
   // 4-1023 entries == UNUSED
   for(int i=0;i<FATENTRYCOUNT; ++i)
   {
      block_1.fat[i] = FAT[i]; // fatblock 0 -> 512 stores FAT 0 -> 512 entries
   }
   for (int i = FATENTRYCOUNT; i < BLOCKSIZE; ++i)
   {
      block_2.fat[i-512] = FAT[i]; //fatblock 0 -> 512 stores FAT 512 -> 1023 entries
   }
   // write fat to disk
   writeblock(&block_1,1);
   writeblock(&block_2,2);
	 /* prepare root directory
	  * write root directory block to virtual disk
	  */
   diskblock_t root_Block;
   // fill with \0
   for(int i=0;i<BLOCKSIZE;++i)
   {
      root_Block.data[i] = '\0';
   }
   //is a directory 
   root_Block.dir.isdir = TRUE;
   root_Block.dir.nextEntry = 0; // starts at 0
   rootDirIndex = 3;
   // write root to block
   writeblock(&root_Block, 3);
}

/* use this for testing
 */
void printBlock ( int blockIndex )
{
   printf ( "virtualdisk[%d] = %s\n", blockIndex, virtualDisk[blockIndex].data ) ;
}

