#include "shell.h"
#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"

void shell() {
  char buf[64];
  char cmd[64];
  char arg[2][64];

  byte cwd = FS_NODE_P_ROOT;

  while (true) {
    printString("MengOS:");
    printCWD(cwd);
    printString("$ ");
    readString(buf);
    parseCommand(buf, cmd, arg);

    if (strcmp(cmd, "cd"))
      cd(&cwd, arg[0]);
    else if (strcmp(cmd, "ls"))
      ls(cwd, arg[0]);
    else if (strcmp(cmd, "mv"))
      mv(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cp"))
      cp(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cat"))
      cat(cwd, arg[0]);
    else if (strcmp(cmd, "mkdir"))
      mkdir(cwd, arg[0]);
    else if (strcmp(cmd, "clear"))
      clearScreen();
    else
      printString("Invalid command\n");
  }
}

// TODO: 4. Implement printCWD function
void printCWD(byte cwd) {
  struct node_fs node_fs_buf;
  char paths[64][16];
  int path_count = 0;
  unsigned int j = 0;

  if (cwd == FS_NODE_P_ROOT) {
    printString("/");
    return;
  }

  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 0x001);

  while (cwd != FS_NODE_P_ROOT) {
    strcpy(paths[path_count], node_fs_buf.nodes[cwd].node_name);
    path_count++;

    cwd = node_fs_buf.nodes[path_count].parent_index;
  }

  for (j = path_count; j >= 0; j--) {
    if (j >= 0) {
      printString("/");
    }

    printString(paths[j]);
  }
}

// TODO: 5. Implement parseCommand function
void parseCommand(char *buf, char *cmd, char arg[2][64]) {
  unsigned int bufLen = 0;
  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int k = 0;
  unsigned int m = 0;

  clear((byte *)cmd, 64);
  clear((byte *)arg[0], 64);
  clear((byte *)arg[1], 64);

  bufLen = strlen(buf);

  for (i = 0; buf[i] != ' ' && buf[i] != '\0'; i++) {
    cmd[i] = buf[i];
  }

  cmd[i] = '\0';

  if (buf[i] == '\0') {
    return;
  }

  while (buf[i] == ' ')
    i++;

  for (j = i; buf[j] != ' ' && buf[j] != '\0'; j++) {
    arg[0][k] = buf[j];
    k++;
  }

  arg[0][k] = '\0';
  if (buf[j] == '\0') {
    return;
  }

  while (buf[j] == ' ')
    j++;

  k = 0;

  for (m = j; buf[m] != ' ' && buf[m] != '\0'; m++) {
    arg[1][k] = buf[m];
    k++;
  }

  arg[1][k] = '\0';
  return;
}

// TODO: 6. Implement cd function
void cd(byte *cwd, char *dirname) {
  struct node_fs node_fs_buf;
  unsigned int i;
  
  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 0x001);

  if (strcmp(dirname, "/")==0){
    *cwd = FS_NODE_P_ROOT;
    return;
  }

  if(strcmp(dirname, "..")==0){
    if(*cwd!=FS_NODE_P_ROOT){
      *cwd=node_fs_buf.nodes[*cwd].parent_index;
    }
    return;
  }

  for(i=0; i<FS_MAX_NODE; i++){
    if(strcmp(node_fs_buf.nodes[i].node_name, dirname)==0 && node_fs_buf.nodes[i].parent_index==*cwd){
      if(node_fs_buf.nodes[i].data_index != FS_NODE_D_DIR){
        printString("cd: ");
        printString(dirname);
        printString(": Not a directory\n");
        return;
      } else {
        *cwd = i;
        return;
      }
    }
  }

  printString("cd: ");
  printString(dirname);
  printString(": No such file or directory\n");
}

// TODO: 7. Implement ls function
void ls(byte cwd, char *dirname) {
  struct node_fs node_fs_buf;
  unsigned int i;
  unsigned int j;
  
  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 0x001);

  if(strcmp(dirname, ".")==0 || dirname[0]=='\0'){
    for (i=0; i<FS_MAX_NODE; i++){
      if(node_fs_buf.nodes[i].parent_index == cwd && node_fs_buf.nodes[i].node_name[0] != '\0'){
        printString(node_fs_buf.nodes[i].node_name);
        printString("\n");
      }
    }
    return;
  }

  for(i=0; i<FS_MAX_NODE; i++){
    if(strcmp(node_fs_buf.nodes[i].node_name, dirname)==0 && node_fs_buf.nodes[i].parent_index==cwd){
      if(node_fs_buf.nodes[i].data_index==FS_NODE_D_DIR){
        for (j=0; j<FS_MAX_NODE; j++){
          if (node_fs_buf.nodes[j].parent_index == i && node_fs_buf.nodes[j].node_name[0] != '\0'){
          printString(node_fs_buf.nodes[j].node_name);
          printString("\n");
          }
        }
      } else {
        printString(node_fs_buf.nodes[i].node_name);
        printString("\n");
      }
      return;
    }
  }

  printString("ls: cannot access '");
  printString(dirname);
  printString("'");
  printString(": No such file or directory\n");
  return;
}

// TODO: 8. Implement mv function
void mv(byte cwd, char *src, char *dst) {
  struct node_fs node_fs_buf;
  
  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 0x001);



}

// TODO: 9. Implement cp function
void cp(byte cwd, char *src, char *dst) {}

// TODO: 10. Implement cat function
void cat(byte cwd, char *filename) {}

// TODO: 11. Implement mkdir function
void mkdir(byte cwd, char *dirname) {}
