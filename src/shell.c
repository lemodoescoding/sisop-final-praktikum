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
  char paths[64][14];
  int path_count = 0;
  int j = 0;

  if (cwd == FS_NODE_P_ROOT) {
    printString("/");
    return;
  }

  readSector(&(node_fs_buf.nodes), FS_NODE_SECTOR_NUMBER);
  readSector(((byte *)&node_fs_buf.nodes) + SECTOR_SIZE,
             FS_NODE_SECTOR_NUMBER + 1);

  while (cwd != FS_NODE_P_ROOT) {
    clear((byte *)paths[path_count], 14);
    strcpy(paths[path_count], node_fs_buf.nodes[cwd].node_name);
    path_count++;

    cwd = node_fs_buf.nodes[cwd].parent_index;
  }

  for (j = path_count - 1; j >= 0; j--) {
    printString("/");
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
  unsigned int i = 0;
  unsigned int foundDir = 0;

  readSector(&(node_fs_buf.nodes), FS_NODE_SECTOR_NUMBER);
  readSector(((byte *)&node_fs_buf.nodes) + SECTOR_SIZE,
             FS_NODE_SECTOR_NUMBER + 1);

  if (strcmp(dirname, "/") == 1) {
    *cwd = FS_NODE_P_ROOT;
    return;
  }

  else if (strcmp(dirname, "..") == 1) {
    if (*cwd != FS_NODE_P_ROOT) {
      *cwd = node_fs_buf.nodes[*cwd].parent_index;
    }
    return;
  } else {
    for (i = 0; i < FS_MAX_NODE; i++) {
      if (node_fs_buf.nodes[i].parent_index == *cwd &&
          node_fs_buf.nodes[i].data_index == 0xFF &&
          strcmp(node_fs_buf.nodes[i].node_name, dirname) == 1) {
        *cwd = i;
        return;
      }
    }
  }

  printString("cd: directory nowhere to be found\n");
}

// TODO: 7. Implement ls function
void ls(byte cwd, char *dirname) {
  struct node_fs node_fs_buf;
  byte cwdSecond;
  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int foundDir = 0;

  readSector(&(node_fs_buf.nodes), FS_NODE_SECTOR_NUMBER);
  readSector(((byte *)&node_fs_buf.nodes) + SECTOR_SIZE,
             FS_NODE_SECTOR_NUMBER + 1);

  if (strcmp(dirname, ".") == 1 || strcmp(dirname, "") == 1) {
    /* printString("CURRECT DIR\n"); */

    for (i = 0; i < FS_MAX_NODE; i++) {
      if (node_fs_buf.nodes[i].parent_index == cwd &&
          node_fs_buf.nodes[i].data_index == 0xFF) {

        printString(node_fs_buf.nodes[i].node_name);
        printString("/ ");
      } else if (node_fs_buf.nodes[i].parent_index == cwd &&
                 node_fs_buf.nodes[i].data_index != 0xFF) {
        printString(node_fs_buf.nodes[i].node_name);
        printString(" ");
      }
    }

    printString("\n");
  } else {

    for (i = 0; i < FS_MAX_NODE; i++) {
      if (node_fs_buf.nodes[i].parent_index == cwd &&
          strcmp(node_fs_buf.nodes[i].node_name, dirname) == 1) {
        cwdSecond = i;
        foundDir = 1;
        break;
      }
    }

    if (foundDir == 0) {
      printString("ls: directory not exist anywhere\n");
      return;
    }

    for (i = 0; i < FS_MAX_NODE; i++) {
      if (node_fs_buf.nodes[i].parent_index == cwdSecond &&
          node_fs_buf.nodes[i].data_index == 0xFF) {

        printString(node_fs_buf.nodes[i].node_name);
        printString("/ ");
      } else if (node_fs_buf.nodes[i].parent_index == cwdSecond &&
                 node_fs_buf.nodes[i].data_index != 0xFF) {
        printString(node_fs_buf.nodes[i].node_name);
        printString(" ");
      }
    }

    printString("\n");
  }
}

// TODO: 8. Implement mv function
void mv(byte cwd, char *src, char *dst) {
  struct node_fs node_fs_buf;
  unsigned int i = 0;
  unsigned int j = 0;

  int slashPos = 0;
  int isThereSlash = -1;
  int dirExist = -1;
  int dirLoc = 0;

  char firstPart[14];
  char secondPart[14];

  unsigned int fileExist = 0;
  unsigned int folderExist = 0;
  unsigned int fileLoc = 0;
  unsigned int folderLoc = 0;

  int dstLen = 0;

  clear((byte *)firstPart, 14);
  clear((byte *)secondPart, 14);

  readSector(&(node_fs_buf.nodes), FS_NODE_SECTOR_NUMBER);
  readSector(((byte *)&node_fs_buf.nodes) + SECTOR_SIZE,
             FS_NODE_SECTOR_NUMBER + 1);

  dstLen = strlen(dst);

  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].parent_index == cwd &&
        strcmp(node_fs_buf.nodes[i].node_name, src) == 1 &&
        node_fs_buf.nodes[i].data_index != 0xFF) {
      fileExist = 1;
      fileLoc = i;
      break;
    }
  }

  if (fileExist == 0) {
    printString("mv: ");
    printString(src);
    printString(" nowhere to be found\n");
    return;
  }

  for (i = 0; dst[i] != '\0' && i < dstLen; i++) {
    if (i == dstLen - 1 && isThereSlash == -1 && dst[i] != '/') {
      isThereSlash = -1;
      break;
    }

    if (dst[i] == '/' && i < dstLen && isThereSlash == -1) {
      isThereSlash = 1;
      slashPos = i;
      break;
    }
  }

  i = 0;
  j = 0;
  if (isThereSlash == -1) {
    if (strcmp(src, dst) == 1) {
      return;
    }

    node_fs_buf.nodes[fileLoc].parent_index = cwd;
    strcpy(secondPart, dst);
  } else if (slashPos == 0 && isThereSlash == 1) {
    strcpy(secondPart, dst + 1);
    node_fs_buf.nodes[fileLoc].parent_index = 0xFF;
  } else if (dst[0] == '.' && dst[1] == '.' && dst[2] == '/' &&
             isThereSlash == 1) {
    strcpy(secondPart, dst + 3);

    node_fs_buf.nodes[fileLoc].parent_index =
        node_fs_buf.nodes[cwd].parent_index;
  } else if (slashPos > 0 && isThereSlash == 1) {
    j = 0;
    dst[slashPos] = '\0';
    strcpy(firstPart, dst);

    j = 0;
    strcpy(secondPart, dst + slashPos + 1);

    i = 0;
    for (i = 0; i < FS_MAX_NODE; i++) {
      if (node_fs_buf.nodes[i].parent_index == cwd &&
          node_fs_buf.nodes[i].data_index == 0xFF &&
          strcmp(node_fs_buf.nodes[i].node_name, firstPart) == 1) {
        dirExist = 1;
        dirLoc = i;
        break;
      }
    }

    if (dirExist == -1) {
      printString("mv: directory doesnt exist\n");
      return;
    }

    node_fs_buf.nodes[fileLoc].parent_index = dirLoc;
  }
  /**/
  strcpy(node_fs_buf.nodes[fileLoc].node_name, secondPart);
  writeSector(&node_fs_buf.nodes[0], FS_NODE_SECTOR_NUMBER);
  writeSector(&node_fs_buf.nodes[32], FS_NODE_SECTOR_NUMBER + 1);
  return;
}

// TODO: 9. Implement cp function
void cp(byte cwd, char *src, char *dst) {
  struct node_fs node_fs_buf;
  struct file_metadata file_data;
  struct file_metadata dir_data;
  char firstPart[14];
  char secondPart[14];
  int i = 0, j = 0;
  int slashPos = -1;
  int isThereSlash = -1;
  byte dst_parent = cwd;
  int dstLen = 0;
  int dirExist = -1;
  int dirLoc = 0;
  int blockCount = 0;
  enum fs_return status;

  char buf[2];
  char *output_name;
  char *dir_name;

  clear((byte *)firstPart, MAX_FILENAME);
  clear((byte *)secondPart, MAX_FILENAME);
  clear((byte *)file_data.node_name, MAX_FILENAME);
  clear((byte *)file_data.buffer, FS_MAX_SECTOR * SECTOR_SIZE);

  file_data.parent_index = cwd;
  strcpy(file_data.node_name, src);

  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 1);

  dstLen = strlen(dst);

  fsRead(&file_data, &status);

  if (status == FS_R_NODE_NOT_FOUND) {
    printString("cp: source file not found\n");
    return;
  } else if (status == FS_R_TYPE_IS_DIRECTORY) {
    printString("cp: source is a directory\n");
    return;
  }

  for (i = 0; dst[i] != '\0' && i < dstLen; i++) {
    if (i == dstLen - 1 && isThereSlash == -1 && dst[i] != '/') {
      isThereSlash = -1;
      break;
    }

    if (dst[i] == '/' && i < dstLen && isThereSlash == -1) {
      isThereSlash = 1;
      slashPos = i;
      break;
    }
  }

  i = 0;
  j = 0;

  if (isThereSlash == -1) {
    if (strcmp(src, dst) == 1) {
      return;
    }

    file_data.parent_index = cwd;
    strcpy(secondPart, dst);
  } else if (slashPos == 0 && isThereSlash == 1) {
    for (i = 1; dst[i] != '\0' && j < MAX_FILENAME - 1; i++) {
      secondPart[j++] = dst[i];
    }

    secondPart[j] = '\0';

    file_data.parent_index = 0xFF;
  } else if (dst[0] == '.' && dst[1] == '.' && dst[2] == '/' &&
             isThereSlash == 1) {
    for (i = 3; dst[i] != '\0' && j < MAX_FILENAME - 1; i++) {
      secondPart[j++] = dst[i];
    }

    secondPart[j] = '\0';

    file_data.parent_index = node_fs_buf.nodes[cwd].parent_index;
  } else if (slashPos > 0 && isThereSlash == 1) {
    j = 0;
    for (i = 0; dst[i] != '\0' && j < MAX_FILENAME - 1; i++) {
      if (dst[i] == '/') {
        break;
      }
      firstPart[j++] = dst[i];
    }

    firstPart[j] = '\0';

    j = 0;
    for (i += 1; dst[i] != '\0' && j < MAX_FILENAME - 1; i++) {
      secondPart[j++] = dst[i];
    }

    secondPart[j] = '\0';
    i = 0;
    for (i = 0; i < FS_MAX_NODE; i++) {
      if (node_fs_buf.nodes[i].parent_index == cwd &&
          node_fs_buf.nodes[i].data_index == 0xFF &&
          strcmp(node_fs_buf.nodes[i].node_name, firstPart) == 1) {
        dirExist = 1;
        dirLoc = i;
        break;
      }
    }

    if (dirExist == -1) {
      printString("cp: directory doesnt exist\n");
      return;
    }

    file_data.parent_index = dirLoc;
  }

  strcpy(file_data.node_name, secondPart);
  fsWrite(&file_data, &status);
  if (status != FS_W_SUCCESS) {
    printString("cp: something going unexpectedly\n");
  }
}

// TODO: 10. Implement cat function
void cat(byte cwd, char *filename) {
  struct file_metadata file_data;
  enum fs_return file_return_status;
  char buf[FS_MAX_SECTOR * SECTOR_SIZE];
  char smallBuf[2];

  unsigned int i = 0;
  unsigned int j = 0;
  unsigned int fileFound = 0;

  file_data.parent_index = cwd;
  clear((byte *)smallBuf, 2);
  clear((byte *)file_data.node_name, 14);
  clear((byte *)file_data.buffer, FS_MAX_SECTOR * SECTOR_SIZE);
  strcpy(file_data.node_name, filename);

  smallBuf[1] = '\0';

  fsRead(&file_data, &file_return_status);

  if (file_return_status == FS_R_SUCCESS) {
    for (i = 0; i < file_data.filesize; i++) {
      smallBuf[0] = file_data.buffer[i];
      printString(smallBuf);
    }

    printString("\n");
    return;
  }

  else if (file_return_status == FS_R_TYPE_IS_DIRECTORY) {
    printString("cat: ");
    printString(filename);
    printString(" is a directory\n");
    return;
  }

  else {
    printString("cat: ");
    printString(filename);
    printString(" is not a file\n");
    return;
  }
}

// TODO: 11. Implement mkdir function
void mkdir(byte cwd, char *dirname) {
  struct node_fs node_fs_buf;
  unsigned int i = 0;

  readSector(&(node_fs_buf.nodes), FS_NODE_SECTOR_NUMBER);
  readSector(((byte *)&node_fs_buf.nodes) + SECTOR_SIZE,
             FS_NODE_SECTOR_NUMBER + 1);

  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].parent_index == cwd &&
        strcmp(node_fs_buf.nodes[i].node_name, dirname) == 1 &&
        node_fs_buf.nodes[i].data_index == 0xFF) {
      printString("mkdir: ");
      printString(dirname);
      printString(" already exists\n");

      return;
    }
  }
  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].node_name[0] == 0x00) {
      node_fs_buf.nodes[i].parent_index = cwd;
      node_fs_buf.nodes[i].data_index = 0xFF;
      strcpy(node_fs_buf.nodes[i].node_name, dirname);

      writeSector(&node_fs_buf.nodes[0], FS_NODE_SECTOR_NUMBER);
      writeSector(&node_fs_buf.nodes[32], FS_NODE_SECTOR_NUMBER + 1);

      return;
    }
  }

  printString("mkdir: not enough space on disk\n");
}
