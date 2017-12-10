#include "fs.h"
#include <iostream>
#include <fstream>

using namespace std;

myFileSystem::myFileSystem(char diskName[16]) {
  // open the file with the above name
  // this file will act as the "disk" for your file system
  this->disk.open(diskName, ios::in | ios::out);
}

int myFileSystem::create_file(char name[8], int size) {
  //create a file with this name and this size

  // high level pseudo code for creating a new file
  if(size > 8 || size < 1)
    return -1;
  if(strlen(name) > 8 || strlen(name) < 1)
    return -1;
  // Step 1: Check to see if we have sufficient free space on disk by
  //   reading in the free block list. To do this:
  // Move the file pointer to the start of the disk file.
  // Read the first 128 bytes (the free/in-use block information)
  // Scan the list to make sure you have sufficient free blocks to
  //   allocate a new file of this size
  this->disk.seekg(0, disk.beg);
  char * buffer = new char [128];
  this->disk.read(buffer, 128);
  int counter = 0;
  for(int i = 0; i < 128; i++){
    if(buffer[i] == 0){
      counter += 1;
      if(counter >= size){
        break;
      }
    }
  }
  if(counter < size || counter == 0){
    return -1;
  }
  // Step 2: we look for a free inode on disk
  // Read in an inode
  // Check the "used" field to see if it is free
  // If not, repeat the above two steps until you find a free inode
  // Set the "used" field to 1
  // Copy the filename to the "name" field
  // Copy the file size (in units of blocks) to the "size" field
  idxNode found;
  int position = -1;
  for(int i = 0; i < 16; i++){
    char * buff = new char[48];
    this->disk.read(buff, 48);
    memcpy(&found, buff, sizeof(found));
    delete buff;
    if(found.used == 0){
      position = 1;
      break;
    }
  }
  if(position == -1){
    return -1;
  }
  found.used = 1;
  strncpy(found.name, name, 8);
  found.size = size;
  // Step 3: Allocate data blocks to the file
  // for(i=0;i<size;i++)
  // Scan the block list that you read in Step 1 for a free block
  // Once you find a free block, mark it as in-use (Set it to 1)
  // Set the blockPointer[i] field in the inode to this block number.
  // end for
  for(int i = 0; i < size; i++){
    found.blockPointers[i] = buffer[i];
    buffer[i] = 1;
  }
  // Step 4: Write out the inode and free block list to disk
  // Move the file pointer to the start of the file
  // Write out the 128 byte free block list
  // Move the file pointer to the position on disk where this inode was stored
  // Write out the inode
  this->disk.seekp(0, disk.beg);
  this->disk.write(buffer, 128);
  this->disk.seekp(48 * position, disk.cur);
  char * buff = new char[48];
  memcpy(buff, &found, sizeof(found));
  this->disk.write(buff, 48);
  delete buffer;
  delete buff;
  return 1;
} // End Create

int myFileSystem::delete_file(char name[8]){
  // Delete the file with this name

  // Step 1: Locate the inode for this file
  // Move the file pointer to the 1st inode (129th byte)
  // Read in an inode
  // If the inode is free, repeat above step.
  // If the inode is in use, check if the "name" field in the
  //   inode matches the file we want to delete. If not, read the next
  //   inode and repeat
  this->disk.seekg(128, disk.beg);
  idxNode found;
  int position = -1;
  for(int i = 0; i < 16; i++){
    char * buff = new char[48];
    this->disk.read(buff, 48);
    memcpy(&found, buff, sizeof(found));
    if(found.used == 1 && strncmp(found.name, name, 8) == 0){
      position = i;
      delete buff;
      break;
    }
    delete buff;
  }
  if(position == -1){
    return -1;
  }
  // Step 2: free blocks of the file being deleted
  // Read in the 128 byte free block list (move file pointer to start
  //   of the disk and read in 128 bytes)
  // Free each block listed in the blockPointer fields as follows:
  // for(i=0;i< inode.size; i++)
  // freeblockList[ inode.blockPointer[i] ] = 0;
  this->disk.seekg(0, disk.beg);
  char * buffer = new char[128];
  this->disk.read(buffer, 128);
  for (int i = 0; i < found.size; i++){
    buffer[found.blockPointers[i]] = 0;
  }
  // Step 3: mark inode as free
  // Set the "used" field to 0.
  found.used = 0;
  // Step 4: Write out the inode and free block list to disk
  // Move the file pointer to the start of the file
  // Write out the 128 byte free block list
  // Move the file pointer to the position on disk where this inode was stored
  // Write out the inode
  this->disk.seekp(0, disk.beg);
  this->disk.write(buffer, 128);
  this->disk.seekp(48 * position, disk.cur);
  char * buff = new char[48];
  memcpy(buff, &found, sizeof(found));
  this->disk.write(buff, 48);
  delete buff;
  return 1;
} // End Delete


int myFileSystem::ls(){
  // List names of all files on disk

  // Step 1: read in each inode and print
  // Move file pointer to the position of the 1st inode (129th byte)
  // for(i=0;i<16;i++)
  // Read in an inode
  // If the inode is in-use
  // print the "name" and "size" fields from the inode
  // end for
  this->disk.seekg(128, disk.beg);
  for(int i = 0; i < 16; i++){
    idxNode found;
    char * buff = new char[48];
    this->disk.read(buff, 48);
    memcpy(&found, buff, sizeof(found));
    if (found.used == 1){
      cout << "idxNode: " << found.name << " size: " << found.size << endl;
    }
    delete buff;
  }
  return 1;
}; // End ls

int myFileSystem::read(char name[8], int blockNum, char buf[1024]){
  // read this block from this file

  // Step 1: locate the inode for this file
  // Move file pointer to the position of the 1st inode (129th byte)
  // Read in an inode
  // If the inode is in use, compare the "name" field with the above file
  // If the file names don't match, repeat
  this->disk.seekg(128, disk.beg);
  idxNode found;
  int position = -1;
  for(int i = 0; i < 16; i++){
    char * buff = new char[48];
    this->disk.read(buff, 48);
    memcpy(&found, buff, sizeof(found));
    if (found.used == 1 && strncmp(found.name, name, 8) == 0){
      position = i;
      delete buff;
      break;
    } 
    delete buff;
  }
  if(position = -1){
    return -1;
  }
  // Step 2: Read in the specified block
  // Check that blockNum < inode.size, else flag an error
  // Get the disk address of the specified block
  // That is, addr = inode.blockPointer[blockNum]
  // Move the file pointer to the block location (i.e., to byte #
  //   addr*1024 in the file)
  if (blockNum >= found.size){
    return -1;
  }
  int addr = found.blockPointers[blockNum] * block_size;
  // Read in the block => Read in 1024 bytes from this location
  //   into the buffer "buf"
  this->disk.seekg(addr, disk.beg);
  this->disk.read(buf, block_size);
  return 1;
} // End read


int myFileSystem::write(char name[8], int blockNum, char buf[1024]){
  // write this block to this file

  // Step 1: locate the inode for this file
  // Move file pointer to the position of the 1st inode (129th byte)
  // Read in a inode
  // If the inode is in use, compare the "name" field with the above file
  // If the file names don't match, repeat
  this->disk.seekg(128, disk.beg);
  idxNode found;
  int position = -1;
  for(int i = 0; i < 16; i++){
    char * buff = new char[48];
    this->disk.read(buff, 48);
    memcpy(&found, buff, sizeof(found));
    if (found.used == 1 && strncmp(found.name, name, 8) == 0){
      position = i;
      delete buff;
      break;
    }
    delete buff;
  }
  if(position = -1){
    return -1;
  }
  // Step 2: Write to the specified block
  // Check that blockNum < inode.size, else flag an error
  // Get the disk address of the specified block
  // That is, addr = inode.blockPointer[blockNum]
  // Move the file pointer to the block location (i.e., byte # addr*1024)
  if (blockNum >= found.size){
    return -1;
  }
  int addr = found.blockPointers[blockNum] * block_size;
  // Write the block! => Write 1024 bytes from the buffer "buff" to
  //   this location
  this->disk.seekp(addr, disk.beg);
  this->disk.write(buf, block_size);
  return 1;
}

int myFileSystem::close_disk(){
    this->disk.close();
    return 1;
}
