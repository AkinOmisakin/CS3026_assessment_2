/* filesys.c
 * 
 * provides interface to virtual disk
 * 
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
// #include <pthread.h>
#include "filesys.h"


diskblock_t  virtualDisk [MAXBLOCKS] ;           // define our in-memory virtual, with MAXBLOCKS blocks
fatentry_t   FAT         [MAXBLOCKS] ;           // define a file allocation table with MAXBLOCKS 16-bit entries
fatentry_t   rootDirIndex            = 0 ;       // rootDir will be set by format
direntry_t * currentDir              = NULL ;
fatentry_t   currentDirIndex         = 0 ;
// pthread_mutex_t fatLock;
// block0_t *block0 = block0 = (block0_t*)&virtualDisk[0];

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
      block_1.fat[i] = FAT[i]; // fatblock 0 -> 511 stores FAT 0 -> 511 entries
   }
   for (int i = FATENTRYCOUNT; i < BLOCKSIZE; ++i)
   {
      block_2.fat[i-512] = FAT[i]; //fatblock 0 -> 511 stores FAT 512 -> 1023 entries
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
MyFILE * myfopen(const char *filename, const char *mode)
{
   // check mode is in write or read
   if (strcmp("w", mode) != 0 && strcmp("r", mode) != 0)
   {
      printf("File not opened in the appropriate mode\n");
      return NULL;
   }

   /* separate the directory from the file name */
   char *lastSlash = strrchr(filename, '/');
   if (lastSlash != NULL)
   {
      size_t newlength = lastSlash - filename; // the size of dir path

      char newpath[newlength]; // create string

      strncpy(newpath, filename, newlength); // copy dir path into string

      newpath[newlength] = '\0'; // add null terminator

      mymkdir(newpath); // call mymkdir to make the directories and get the currentDirIndex

      filename = lastSlash + 1; // this will be the name of the file only
   }

   // get block of current dir
   diskblock_t *currentParent = &virtualDisk[currentDirIndex];

   // allocate memory to MyFILE pointer
   MyFILE *file_ptr = (MyFILE *)malloc(sizeof(MyFILE));

   if (file_ptr == NULL)
   {
      printf("Memory allocation failed.\n");
      return NULL;
   }

   // initialise the MyFILE file data block
   // set mode value first
   strncpy(file_ptr->mode, mode, 3);

   // if mode set to write
   if (strcmp("w", mode) == 0)
   {
      /*  Search for the existing file in the current dir */
      int dirIndex = findfilebyname(&currentParent->dir, filename);

      // check if file can be found in disk
      if (dirIndex == EOF)
      {
         // if entry name not found, start creating the file
         printf("Creating File...\n");

         // set the blockNo
         int UNUSED_fatentry = findUNUSEDfatentry(); // find a block number set to UNUSED

         // set blockno to found index
         file_ptr->blockno = UNUSED_fatentry; // is still FAT[file_ptr->blockno] = UNUSED
         // set position
         file_ptr->pos = 0;

         // get unused directory in current dir
         dirIndex = findUNUSEDdirentry(&currentParent->dir);

         if (dirIndex == EOF)
         {
            printf("All entries used up in directory");
            free(file_ptr);
            return NULL;
         }

         // add unused dir entry to current
         currentParent->dir.entrylist[dirIndex].entrylength = 0; // entry length is 0 currently
         currentParent->dir.entrylist[dirIndex].filelength = 0; // nothing in file so length 0
         currentParent->dir.entrylist[dirIndex].isdir = FALSE; // is a file
         currentParent->dir.entrylist[dirIndex].unused = FALSE; // set to used
         currentParent->dir.entrylist[dirIndex].firstblock = file_ptr->blockno; // set firstblock
         strncpy(currentParent->dir.entrylist[dirIndex].name, filename, MAXNAME); // set name

         currentParent->dir.nextEntry++;

         writeblock(currentParent, currentDirIndex);

         addfatentry(file_ptr->blockno); // add to fat block and FAT table
      }
      else
      {
         // get blockno and buffer of existing file
         file_ptr->blockno = currentParent->dir.entrylist[dirIndex].firstblock;
         file_ptr->buffer = virtualDisk[file_ptr->blockno];
      }
   }

   // if opened in read mode
   if (strcmp(mode, "r") == 0)
   {
      // get the dir entry of the file
      int dirIndex = findfilebyname(&currentParent->dir, filename);
      // check if file name can be found in disk
      if (dirIndex == EOF)
      {
         printf("File not found error!\n");
         free(file_ptr);
         return NULL;
      }
      // get blockno and buffer of existing file
      file_ptr->blockno = currentParent->dir.entrylist[dirIndex].firstblock;
      file_ptr->buffer = virtualDisk[file_ptr->blockno];
      file_ptr->pos = 0;
   }
   // messages
   printf("File opened in '%c' mode\n", *file_ptr->mode);
   printf("File opened in Directory index = %d \n", currentDirIndex);
   // return file pointer that was created in write mode
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
   int character;
   stream->buffer = virtualDisk[stream->blockno]; // get buffer of current block
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
   character = stream->buffer.data[stream->pos]; // get each character of the buffer
   stream->pos++; // pos++
   return character;
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
   if (stream->pos >= BLOCKSIZE - 1 ) 
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
   diskblock_t *currentParent = &virtualDisk[rootDirIndex]; // root original parent directory
   currentDirIndex = rootDirIndex; // get root block index
   token = strtok_r(pathCopy, "/", &rest); // tokenize path

   // search each directory in path, if they don't exist create the directory in the path
   while (token != NULL)
   {
      // find directory index in current directory
      int dirIndex = findfilebyname(&currentParent->dir, token);

      // if found
      if (dirIndex != EOF) // return an index
      {
         // update the current parent to next dir
         currentDirIndex = currentParent->dir.entrylist[dirIndex].firstblock; // get fat index 
         currentParent = &virtualDisk[currentDirIndex]; // update current parent
      }
      else // the index was not found
      {
         // then we have to create that directory inside currentParent
         dirIndex = findUNUSEDdirentry(&currentParent->dir); // find unused dir in current parent

         if (dirIndex == EOF)
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
               if (path[0] == '/')
               {
                  printf("Creating directory...\n");

                  // initialise the next level directory
                  currentParent->dir.entrylist[dirIndex].firstblock = fatIndex; // set firstblock to found entry
                  currentParent->dir.entrylist[dirIndex].isdir = TRUE; // is dir
                  currentParent->dir.entrylist[dirIndex].unused = FALSE; // used
                  strncpy(currentParent->dir.entrylist[dirIndex].name, token, MAXNAME); // set name
                  writeblock(currentParent, currentDirIndex); // write the current parent block

                  addfatentry(fatIndex); // add entry to fat table
                  currentDirIndex = fatIndex; // update current Index
                  currentParent = &virtualDisk[currentDirIndex]; // give it a block
                  currentParent->dir.isdir = TRUE; // new block is dir
                  currentParent->dir.nextEntry = 0; // set to 0

                  // set all entries in current to unused
                  for (int i = 0; i < DIRENTRYCOUNT; ++i)
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
   // check if path is root
   char *token, *rest; // tokenize path and save pointer
   char *pathCopy = strdup(path); // copy path
   diskblock_t *currentParent; // root original parent directory

   if (path[0] == '/')
   {
      currentParent = &virtualDisk[rootDirIndex];
      currentDirIndex = rootDirIndex;
   }

   char **CharList; // declare ** char pointer which is a list of pointers

   CharList = (char**) malloc(sizeof(char *) * (DIRENTRYCOUNT+1)); // allocate memory to it

   // ------------- Code Changed ----------------- ||
   // first check that path is self-referenced
   if (path[0] == '.')
   {
      // the current block using currentDirIndex
      currentParent = &virtualDisk[currentDirIndex];
      for (int i = 0; i < DIRENTRYCOUNT; i++)
      {
         // allocate memory to each pointer in the list
          CharList[i] = (char*) malloc(sizeof(char) * MAXNAME);
         // set the name of each entry (i) in the current parent (entrylist) into the list at index i
         strncpy(CharList[i], currentParent->dir.entrylist[i].name, MAXNAME);
      }
      free(pathCopy);
      return CharList; // return List
   }
   // -------------------------------------------- ||

   token = strtok_r(pathCopy, "/", &rest); // tokenize path
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
         currentParent = &virtualDisk[currentDirIndex]; // update current parent
      }
      else
      {
         printf("Path not Found !\n"); // if path not found
         free(pathCopy);
      }
      token = strtok_r(NULL, "/", &rest); // return NULL if there are no more tokens
   }

   for (int i = 0; i < DIRENTRYCOUNT; i++)
   {
      // allocate memory to each pointer in the list
      CharList[i] = (char*) malloc(sizeof(char) * MAXNAME);
      // set the name of each entry (i) in the current parent (entrylist) into the list at index i
      strcpy(CharList[i], currentParent->dir.entrylist[i].name);
   }
   printf("Current Directory index is now =  %d \n", currentDirIndex);
   free(pathCopy);
   return CharList; // return list
}


/*  myrmdir function
 */
void myrmdir ( const char * path )
{
   char *token, *rest; // tokenize path and save pointer
   char *pathCopy = strdup(path); // copy path
   diskblock_t *currentParent = &virtualDisk[currentDirIndex];
   int prevDirIndex = 0;
   int dirIndex;
   token = strtok_r(pathCopy, "/", &rest);
   while (token != NULL)
   {
      dirIndex = findfilebyname(&currentParent->dir, token); // find directory by name
      //currentDir = &currentParent->dir.entrylist[dirIndex]; // change into path directory !!
      if (dirIndex != EOF)
      {
         prevDirIndex = currentDirIndex; // update previous parent
         currentDirIndex = currentParent->dir.entrylist[dirIndex].firstblock;
         currentParent = &virtualDisk[currentDirIndex];
      }
      else
      {
         printf("Not found path \n");
      }
      token = strtok_r(NULL, "/",&rest);
   }
   diskblock_t * prevParent = &virtualDisk[prevDirIndex]; // get previous parent
   direntry_t p; // initialise new directory
   for (int i = 0; i < MAXNAME; ++i)
   {
      p.name[i] = '\0'; // set name to null
   }
   // set all to unused
   p.unused = TRUE;
   p.firstblock = NULL;
   p.filelength = 0;
   p.entrylength = 0;
   p.isdir= FALSE;
   if (dirIndex != EOF)
   {
      prevParent->dir.entrylist[dirIndex] = p; // set previous parent entry to new directory
      deletefat(currentDirIndex); // delete the fat entry
      printf("deleted\n");
   }
}


void deletefat (int blokno) 
{
   
   //check if blokno size fits in block 1 or block 2
   if (blokno > 1024)
   {
      printf("block no is outside range of fat table\n"); // blokno outside FAT table range
   }
   // from 4 to 511 remove from  block 1
   if ( blokno < 512)
   {  
      FAT[blokno] = UNUSED;
      virtualDisk[1].fat[blokno] = UNUSED;
      
   }
   else 
   {
      // or from 512 - 1024 remove from block 2
      FAT[blokno] = UNUSED;
      virtualDisk[2].fat[blokno] = UNUSED; 
   }
}
/*  myremove function
 */
void myremove( const char * path) 
{
   // This function removes a file; the path can be absolute or relative
   char *token, *rest; // tokenize path and save pointer
   char *pathCopy = strdup(path); // copy path
   diskblock_t *currentParent = &virtualDisk[rootDirIndex]; // root original parent direcotry
   //currentDirIndex = rootDirIndex;
   // seperate filename from dir path
   char * lastSlash = strrchr(path, '/');

   if (lastSlash != NULL)
   {
      size_t newlength = lastSlash - path + 1; // the size of dir path

      char newpath[newlength]; // create string

      strncpy (newpath, path, newlength); // copy dir path into string
      
      newpath[newlength] = '\0'; // add null terminator

      pathCopy = strdup(newpath); // dir path copied

      path = lastSlash + 1;  // this will be the name of the file only

      mychdir(pathCopy); // change to directory

      currentParent = &virtualDisk[currentDirIndex];// update current parent

      int dirIndex = findfilebyname(&currentParent->dir, path); //get dir index of file by name

      if (dirIndex != EOF)
      {
         if (currentParent->dir.entrylist[dirIndex].isdir == FALSE) // check if file is a directory
         {
            // un-initialise the file dir
            currentParent->dir.entrylist[dirIndex].unused = TRUE; // set unused
            currentParent->dir.entrylist[dirIndex].filelength = 0; // set filelength
            currentParent->dir.entrylist[dirIndex].entrylength = 0; // set entrylength

            for (int i=0;i<MAXNAME;++i) 
            {
               currentParent->dir.entrylist[dirIndex].name[i] = '\0'; //This can be used also
            }

            // remove the file buffer(s) from the FAT table
            int nextblock = currentParent->dir.entrylist[dirIndex].firstblock;

            while (nextblock != ENDOFCHAIN)
            {
               // pointer instead of re writing to block
               diskblock_t * buffer = &virtualDisk[nextblock];
               for (int i = 0;i<BLOCKSIZE;++i)
               {
                  if (buffer->data[i] != '\0') buffer->data[i] = '\0'; // set data to null
               }
               // save the number ---> go to next block ---> delete previous block number
               int saveblkno = nextblock; nextblock = FAT[nextblock]; deletefat(saveblkno);
            }

            currentParent->dir.entrylist[dirIndex].firstblock = NULL; // set firstblock to null
            printf("File is now deleted!\n");
            
         }

         else
         {
            printf("Can not delete a directory\n");
         }

      }

      else
      {
         printf("FileNotFoundError!\n");
      }
   }
   // if just the testfile name
   currentParent = &virtualDisk[currentDirIndex];

   int dirIndex = findfilebyname(&currentParent->dir, path);

   if (dirIndex != EOF)
   {
         if (currentParent->dir.entrylist[dirIndex].isdir == FALSE)
         {
            // un-initialise the file dir
            currentParent->dir.entrylist[dirIndex].unused = TRUE;
            currentParent->dir.entrylist[dirIndex].filelength = 0;
            currentParent->dir.entrylist[dirIndex].entrylength = 0;

            for (int i=0;i<MAXNAME;++i)
            {
               currentParent->dir.entrylist[dirIndex].name[i] = '\0';
            } 

            currentParent->dir.nextEntry = 0;

            // remove the file buffer(s) from the FAT table
            int nextblock = currentParent->dir.entrylist[dirIndex].firstblock;
            while (nextblock != ENDOFCHAIN)
            {
               // pointer instead of re writing to block
               diskblock_t* buffer = &virtualDisk[nextblock];
               for (int i = 0;i<BLOCKSIZE;++i)
               {
                  if (buffer->data[i] != '\0') buffer->data[i] = '\0';
               }
               // save the number ---> go to next block ---> delete previous block number
               int saveblkno = nextblock; nextblock = FAT[nextblock]; deletefat(saveblkno);
            }
            currentParent->dir.entrylist[dirIndex].firstblock = NULL; // set firstblock to null
            printf("File is now deleted!\n");
         }
         else
         {
            printf("Can not delete a directory\n");
         }
   }
      else
      {
         printf("FileNotFoundError!\n");
      }
}

/*  mychdir function || mychdir("..") --> goes to previous dir || mychdir("/") --> goes to root
 */
void mychdir( const char * path) 
{
   //currentDir = strdup(path); 
   char *token, *rest; // tokenize path and save pointer
   char *pathCopy = strdup(path); // copy path

   diskblock_t *currentParent = &virtualDisk[rootDirIndex]; // root original parent direcotry
   currentDirIndex = rootDirIndex; // get root block index
   int parentDirIndex = rootDirIndex; // parenntDirIndex = prev level Dir index 

   if (path == '/')
   {
      currentDirIndex = rootDirIndex; // update current to root
      currentParent = &virtualDisk[currentDirIndex]; // update currentparent
   }
   
   token = strtok_r(pathCopy, "/", &rest); // tokenize path

   while (token != NULL)
   {
      int dirIndex = findfilebyname(&currentParent->dir, token); // find directory by name
      //currentDir = &currentParent->dir.entrylist[dirIndex]; // change into path directory !!
      if (dirIndex != EOF)
      {
         parentDirIndex = currentDirIndex; // update parentDirIndex
         currentDirIndex = currentParent->dir.entrylist[dirIndex].firstblock; // get fat index
         currentParent = &virtualDisk[currentDirIndex]; // update currentparent
      }
      token = strtok_r(NULL, "/",&rest);
   }
   // if mychdir("..") --> cd .. 
   if (path == "..")
   {
      // check if current dir is root
      if (currentDirIndex == rootDirIndex)
      {
         printf("Already in root directory\n");
      }
      else
      {
         currentDirIndex = parentDirIndex;// update current to parent
      }
   }

   printf("Current Directory index is now =  %d \n", currentDirIndex);
}

/* use this for testing
 */
void printBlock ( int blockIndex )
{
   printf ( "virtualdisk[%d] = %s\n", blockIndex, virtualDisk[blockIndex].data ) ;
}

/* A3 Copy function
   */
void CopyToMyFILE ( FILE * realfile, MyFILE * fakefile)
{
   // read real file to string
   fseek(realfile, 0, SEEK_END); // seek to end of file
    long int file_size = ftell(realfile); // get current file pointer
    fseek(realfile, 0, SEEK_SET); // seek back to beginning of file
     char *Text = (char *)malloc(file_size + 1); // allocate memory for file
   fread(Text, 1, file_size, realfile); // read all of file
   Text[file_size] = '\0'; // add null terminator
   // write string to fake file
   for (int i=0; i < file_size;++i)
   {
      myfputc(Text[i], fakefile); // write each character to fake file
   }
   printf("Real file copied to disk\n");
}

void CopyToRealFILE ( MyFILE * fakefile, FILE * realfile)
{
   // copy fake file into real file
   int character = myfgetc(fakefile);
   // while character isnt EOF or null
   while(character != EOF && character != '\0')
   {
      // write to file each character
      fprintf(realfile, "%c", character);
      // get new character
      character = myfgetc(fakefile);
   }
}

/* A2 Copy Move function
   */
void movefile (const char* file1, const char * file2) 
{
   // get contents of file1
   char *token, *rest; // tokenize path and save pointer
   char *pathCopy = strdup(file1); // copy path of file you want to copy from
   diskblock_t *buffer= &virtualDisk[rootDirIndex];
   token = strtok_r(pathCopy, "/", &rest); 
   while (token != NULL)
   {
      int dirIndex = findfilebyname(&buffer->dir, token); // find directory by name
      //currentDir = &currentParent->dir.entrylist[dirIndex]; // change into path directory !!
      if (dirIndex != EOF)
      {
         // update buffer and index
         currentDirIndex = buffer->dir.entrylist[dirIndex].firstblock;
         buffer = &virtualDisk[currentDirIndex];
      }
      else
      {
         printf("PLZ imput correct path!\n");
      }
      token = strtok_r(NULL, "/",&rest);
   }
   char * content[BLOCKSIZE]; // char array that hold content of buffer
   // copy contents of buffer data to array
   for (int i =0;i<BLOCKSIZE;++i)
   {
      content[i] = buffer->data[i];
   }
   // create file2 and copy contents to file2
   char * rest2;
   pathCopy = strdup(file2); // copy path of the file you want to copy to
   buffer= &virtualDisk[rootDirIndex]; // reset buffer
   token = strtok_r(pathCopy, "/", &rest2);
   while (token != NULL)
   {
      int dirIndex = findfilebyname(&buffer->dir, token); // find directory by name
      //currentDir = &currentParent->dir.entrylist[dirIndex]; // change into path directory !!
      if (dirIndex != EOF)
      {
         //update buffer and index
         currentDirIndex = buffer->dir.entrylist[dirIndex].firstblock;
         buffer = &virtualDisk[currentDirIndex];
      }
      else
      {
         printf("PLZ imput correct path!\n");
      }
      token = strtok_r(NULL, "/",&rest2);
   }
   // write to buffer of file2 the content that was copied 
   for (int i =0;i<BLOCKSIZE;++i)
   {
      buffer->data[i] = content[i];
   }
}
