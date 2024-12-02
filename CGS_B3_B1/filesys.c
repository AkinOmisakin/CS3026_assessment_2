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
   // set all to unused
   for (int i=0;i<DIRENTRYCOUNT;++i)
   {
      root_Block.dir.entrylist[i].unused = TRUE;
   }
   //is a directory 
   root_Block.dir.isdir = TRUE;
   root_Block.dir.nextEntry = 0; // starts at 0
   rootDirIndex = 3;
   currentDirIndex = 3;
   // write root to block
   writeblock(&root_Block, 3);
}

/*  myfopen function
 */
MyFILE * myfopen ( const char * filename, const char * mode )
{
   // check mode is in write or read
   if (strcmp("w", mode ) !=0  &&  strcmp("r", mode) != 0)
   {
      printf("file not opened in the appropriate mode\n");
      return NULL; // return nothing
   }
   
   /* seperate the directory from the file name*/
   char * lastSlash = strrchr(filename, '/');
   if (lastSlash != NULL)
   {
      size_t newlength = lastSlash - filename; // the size of dir path

      char newpath[newlength]; // create string

      strncpy (newpath, filename, newlength); // copy dir path into string
      
      newpath[newlength] = '\0';

      mymkdir(newpath); // call mymkdir to make the directories or get the currentDirIndex

      filename = lastSlash + 1;  // this will be the name of the file only
   }
   
   // get block of current dir 
   diskblock_t * currentParent = &virtualDisk[currentDirIndex];

   // allocate memory to myFILE pointeer
   MyFILE * file_ptr = (MyFILE*) malloc(sizeof(MyFILE));

   if (file_ptr == NULL) {
      printf("Memory allocation failed.\n");
      return NULL;
   }

   // initialise the MyFILE file data block
   // set mode value first
   strncpy(file_ptr->mode, mode, 3);

   // if mode set to wtite
   if (strcmp("w", mode) == 0)
   {
      // get dir entry from current dir
      int dirIndex = findfilebyname(&currentParent->dir, filename);

      //check file can be found in disk
      if (dirIndex == EOF)
      {
         // if entry name not found create start creating the file
         printf("Creating File...\n");
      }

      else // get exsiting file if name is found
      {
         //get blockno and buffer of existing file
         file_ptr->blockno = currentParent->dir.entrylist[dirIndex].firstblock;
         file_ptr->buffer = virtualDisk[file_ptr->blockno];
         
         return file_ptr;
      }
      
      // set the blockNo
      int UNUSED_fatentry = findUNUSEDfatentry(); // find a block number set to UNUSED

      //set blokno to founs index
      file_ptr->blockno = UNUSED_fatentry; // is still FAT[file_ptr->blockno] = UNUSED
      // set position
      file_ptr->pos = 0;
      
      //get unused directory in current dir
      dirIndex = findUNUSEDdirentry(&currentParent->dir);

      if (dirIndex == EOF)
      {
         printf("All entry used up in directory");
         return NULL;
      }
      // add unused dir entry to current
      currentParent->dir.entrylist[dirIndex].entrylength = 0;// entry length is 0 currently
      currentParent->dir.entrylist[dirIndex].filelength = 0;// nothing in file so length 0
      currentParent->dir.entrylist[dirIndex].isdir = FALSE ;// is a file
      currentParent->dir.entrylist[dirIndex].unused = FALSE;// set to used
      currentParent->dir.entrylist[dirIndex].firstblock = file_ptr->blockno;// set firstblock
      strncpy(currentParent->dir.entrylist[dirIndex].name, filename, MAXNAME); // set name 

      //currentParent->dir.nextEntry++;

      writeblock(currentParent, currentDirIndex);

      addfatentry(file_ptr->blockno);// add to fat block and FAT table
   }
   
   // if opened in read mode
   if (strcmp(mode, "r") == 0)
   {
      // get the dir entry of the file 
      int dirIndex = findfilebyname(&currentParent->dir,filename);
      //check file name can be found in disk
      if (dirIndex == EOF)
      {
         printf("FileNotFoundError!\n");
         return NULL;
      }
      //get blockno and buffer of existing file 
      file_ptr->blockno = currentParent->dir.entrylist[dirIndex].firstblock;
      file_ptr->buffer = virtualDisk[file_ptr->blockno];

      // mode message
      printf("File opened in '%c' mode\n",*file_ptr->mode);
      return file_ptr;
   }

   // mode message
   printf("File opened in '%c' mode\n",*file_ptr->mode);

   //return file pointer that what created in write mode
   return file_ptr;
}

/* find the file in root block using the filename
*/
int findfilebyname (dirblock_t * current ,const char* filename)
{
   for (int i=0; i < DIRENTRYCOUNT; ++i)
   {  
      //checks if filename exists then returns the index of the file
      if (strcmp(current->entrylist[i].name, filename) == 0)
      {
         //printf("file found at index %d\n", i);
         return i;
      }
   }
   // could not find name
   return EOF;
}

/* find an unused in the specific dir
   */
int findUNUSEDdirentry (dirblock_t *dir)
{
   for (int i=0;i<DIRENTRYCOUNT;++i)
   {
      if (dir->entrylist[i].unused == TRUE)
      {
         //printf("file found at index %d\n", i);
         return i;
      }
   }
   return EOF;
}

/*add fat entry to block_1 or block_2 of fat table
   */ 
void addfatentry (int blokno) 
{
   
   //check if blokno size fits in block 1 or block 2
   if (blokno > 1024)
   {
      printf("block no is outside range of fat table\n"); // blokno outside FAT table range
   }
   // from 4 to 511 add to block 1
   if ( blokno < 512)
   {  
      FAT[blokno] = ENDOFCHAIN;
      virtualDisk[1].fat[blokno] = ENDOFCHAIN; 
      
   }
   else 
   {
      // or from 512 - 1024 add to block 2
      FAT[blokno] = ENDOFCHAIN;
      virtualDisk[2].fat[blokno] = ENDOFCHAIN; 
   }
}

/*adds a fat entry to existing FAT chain
   */ 
void addtofatentry (int blokno, int newblokno)
{
   
   // checks within range
   if (blokno > 1024)
   {
      printf("block no is outside range of fat table\n"); // blokno outside FAT table range
   }
   //checks less than 512 add block 1
   if (blokno < 512)
   {
      FAT[blokno] = newblokno;
      virtualDisk[1].fat[blokno] = newblokno;
   } 
   else
   {
      // if greater than 512 add to block 2
      FAT[blokno] = newblokno;
      virtualDisk[2].fat[blokno] = newblokno;
   }
}

// find an UNUSED fat entry in FAT
int findUNUSEDfatentry ()
{
   // 4 - 1023
   for (int i=4;i<BLOCKSIZE;++i)
   {
      // finds fat entry set to UNUSED
      if (FAT[i] == UNUSED){
         return i; //return index of fat block
      }
   }
   //return EOC if fat not found
   return ENDOFCHAIN;
}

/*  myfclose function
 */
void myfclose ( MyFILE * stream )
{
   writeblock(&stream->buffer, stream->blockno);
   free(stream);
   printf("file is now closed\n");
}

/*  myfgetc function
 */
int myfgetc ( MyFILE * stream )
{
   // Check if file mode is set to read mode
   if (strcmp(stream->mode, "r") != 0) {
      // Output error if not in "r" mode
      printf("MyFILE mode not set to 'r' mode\n");
      return EOF;
   }
   else
   {

   
   int character;
   //stream->buffer = virtualDisk[stream->blockno]; // get buffer of current block
   character = stream->buffer.data[stream->pos]; // get each character of the buffer
   stream->pos++; // pos++
   if (stream->pos == BLOCKSIZE - 1)
   {
      //print the current block to terminal
      printBlock(stream->blockno);
      // if eoc is reached then return eof
      if (FAT[stream->blockno] == ENDOFCHAIN)
      {
         return EOF;
      }
      // traverse block chain
      stream->blockno = FAT[stream->blockno];
      stream->pos = 0; // reset pos
   }
   return character;
   }
}

/*  myfputc function
 */
void myfputc ( int b, MyFILE * stream )
{
   // check if file mode is set to write mode
   if ( strcmp(stream-> mode, "w") != 0)
   {
      //output error if not in "w" mode
      printf("MyFILE mode not set to 'w' mode \n");
   }
   else
   {

   
   // creat new block to write on 
   diskblock_t *current = &stream->buffer;

   // checks if the pos is >= to 1023 
   if (stream->pos == BLOCKSIZE - 1 ) 
   {
      printf("buffer is full\n");
      // write buffer to if buffer is full to current block number location
      writeblock(current, stream->blockno);

      // get UNUSED fat entry
      int newfatentry = findUNUSEDfatentry();

      // checks fat entry  does not equal EOC
      if (newfatentry == ENDOFCHAIN)
      {
         // if returned EOC ouput error 
         printf("There are no more fat entries left\n");
      }

      //add new entry to the fat block
      addfatentry(newfatentry);// new fat entry = EOC

      // add new entry to the end of firstblock
      addtofatentry(stream->blockno, newfatentry);

      stream->blockno = newfatentry;// set new blokno to new fat entry found
      stream->pos = 0; //reset position to 0
      memset(stream->buffer.data, 0, BLOCKSIZE); // reset memory location to 0
   }
   // add the data b to the buffer data at the current pos of stream(file)
   current->data[stream->pos] = (Byte) b;
   stream->pos++;//increase pos
   }
}

/*  mymkdir function
 */
void mymkdir ( const char * path )
{
   char *token, *rest; // tokenize path and save pointer
   char *pathCopy = strdup(path); // copy path
   diskblock_t *currentParent = &virtualDisk[rootDirIndex]; // root original parent direcotry
   currentDirIndex = rootDirIndex;//get root block index
   token = strtok_r(pathCopy, "/", &rest); // tokenize path
   //search each directory in path, if they don't exist create the directory in the path
   while (token != NULL)
   {
      // find directory index in current directory
      int dirIndex = findfilebyname(&currentParent->dir, token);
      //printf("current directory is %s\n", token);
      // if found
      if (dirIndex != EOF) // return an index
      {
         // update the current parent to next dir
         currentDirIndex = currentParent->dir.entrylist[dirIndex].firstblock; // get fat index 
         currentParent = &virtualDisk[currentDirIndex]; // update currentparent
      }
      else // the index was not found
      {
         // then we have to create that directory inside currentParent
         dirIndex = findUNUSEDdirentry(&currentParent->dir); // find unused dir in current parent
         if (dirIndex  == EOF)
         {
            printf("All entries used up! \n");
         }
         else 
         {
            int fatIndex = findUNUSEDfatentry(); // find an unused fat entry
            if (fatIndex == ENDOFCHAIN) // end not found
            {
               printf("FAT table is full!\n");
            }
            else
            {
               if ( path[0] == '/')
               {
                  printf("Creating directory...\n");
                  // initialise the next level directory
                  currentParent->dir.entrylist[dirIndex].firstblock = fatIndex; // set firstblock to found entry
                  currentParent->dir.entrylist[dirIndex].isdir = TRUE; // is dir
                  currentParent->dir.entrylist[dirIndex].unused = FALSE;// used
                  strncpy(currentParent->dir.entrylist[dirIndex].name, token, MAXNAME); // set name
                  writeblock(currentParent, currentDirIndex); // write the current parent block

                  addfatentry(fatIndex); // add entry to fat table
                  currentDirIndex = fatIndex; // update current Index
                  currentParent = &virtualDisk[currentDirIndex]; // give it a block
                  currentParent->dir.isdir = TRUE; // new block is dir
                  currentParent->dir.nextEntry = 0; // set to 0

                  // set all entry in current to unused
                  for (int i = 0; i< DIRENTRYCOUNT;++i)
                  {
                     currentParent->dir.entrylist[i].unused = TRUE;
                  }
                  writeblock(currentParent, currentDirIndex);
               }
               else
               {
                  printf("Directory was relative, could not create directory");
               }
            }
         }
      }
      token = strtok_r(NULL, "/", &rest);
   }
}

/*  mylistdir function
 */
char ** mylistdir (const char * path)
{
   char *token, *rest; // tokenize path and save pointer
   char *pathCopy = strdup(path); // copy path
   diskblock_t *currentParent = &virtualDisk[rootDirIndex]; // root original parent direcotry
   currentDirIndex = rootDirIndex;//get root block index

   char ** CharList; // declare ** char pointer which is a list of pointers

   CharList = malloc(sizeof(char *) * DIRENTRYCOUNT) ; // allocate memory to it
   // exception
   if (CharList == NULL)
   {
      printf("memory not allocated\n");
      return NULL;
   }

   token = strtok_r(pathCopy, "/", &rest);// tokenize path
   // if token found
   while (token != NULL)
   {
      // find directory index in current directory
      int dirIndex = findfilebyname(&currentParent->dir, token);
      // if found
      if (dirIndex != EOF)
      {
         // update the current parent to next dir
         currentDirIndex = currentParent->dir.entrylist[dirIndex].firstblock; // get fat index 
         currentParent = &virtualDisk[currentDirIndex]; // update currentparent
         for (int i = 0; i < DIRENTRYCOUNT; i++) 
         {  
            // allocate memory to each pointer in the list
            CharList[i] = malloc(sizeof(char)*MAXNAME);
            // set the name of each entry (i) in the current parent (entrylist) into the list at index i
            strcpy(CharList[i], currentParent->dir.entrylist[i].name);
         }
      }
      else
      {
         printf("Path not Found !\n"); // if path not found
      }
      token = strtok_r(NULL, "/", &rest); // return NULL if there are no more tokens
   }
   return CharList; // return list
}


/*  myfputc function
 */
void myrmdir ( const char * path );

/* use this for testing
 */
void printBlock ( int blockIndex )
{
   printf ( "virtualdisk[%d] = %s\n", blockIndex, virtualDisk[blockIndex].data ) ;
}

