#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>

using namespace std;

//block size = 1KB
#define block_size 1024

//structure for the inode
struct idxNode{
  char name[8]; //file name
  int size;     // file size (in number of blocks)
  int blockPointers[8]; // direct block pointers
  int used;             // 0 => inode is free; 1 => in use
};

class myFileSystem
{
private:
  fstream disk;
public:
  myFileSystem(char diskName[16]);

  int create_file(char name[8], int size);
  int delete_file(char name[8]);
  int ls();
  int read(char name[8], int blockNum, char buf[1024]);
  int write(char name[8], int blockNum, char buf[1024]);
  int close_disk();
};
