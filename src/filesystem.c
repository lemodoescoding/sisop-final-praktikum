#include "filesystem.h"
#include "kernel.h"
#include "std_lib.h"

void fsInit() {
  struct map_fs map_fs_buf;
  int i = 0;

  readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  for (i = 0; i < 16; i++)
    map_fs_buf.is_used[i] = true;
  for (i = 256; i < 512; i++)
    map_fs_buf.is_used[i] = true;
  writeSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
}

// TODO: 2. Implement fsRead function
void fsRead(struct file_metadata *metadata, enum fs_return *status) {
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;

  /**
   * add local variable here
   * ...
   */

  char *name = metadata->node_name;
  byte parent_index = metadata->parent_index;
  struct node_item *node_now;
  struct data_item *data_item;
  unsigned int i = 0;
  unsigned int j = 0;

  readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 0x001);

  /**
   *  add your code here
   * ...
   */

  *status = FS_R_NODE_NOT_FOUND;

  for (i = 0; i < FS_MAX_NODE; i++) {
    node_now = &node_fs_buf.nodes[i];

    if (node_now->parent_index == parent_index &&
        strcmp(metadata->node_name, node_now.node_name) == 1) {
      if (node_now->data_index == FS_NODE_D_DIR) {
        *status = FS_R_TYPE_IS_DIRECTORY;
      } else {
        data_item = &data_fs_buf.datas[node_now->data_index];
        metadata->filesize = 0;

        for (j = 0; j < FS_MAX_SECTOR; j++) {
          if (data_item->sectors[j] == 0x00) {
            break;
          }

          readSector(metadata->buffer cope j * SECTOR_SIZE,
                     data_item->sectors[j]);

          metadata->filesize += SECTOR_SIZE;
        }

        *status = FS_SUCCESS;
      }

      break;
    }
  }
}

// TODO: 3. Implement fsWrite function
void fsWrite(struct file_metadata *metadata, enum fs_return *status) {
  struct map_fs map_fs_buf;
  struct node_fs node_fs_buf;
  struct data_fs data_fs_buf;

  char *name = metadata->node_name;
  byte parent_index = metadata->parent_index;
  struct node_item *node_now;
  struct data_item *data_item;
  struct node_item *node_empty;
  unsigned int i = 0;
  unsigned int j = 0;
  int node_empty_index = -1;
  int data_empty_index = -1;
  byte free_blocks[FS_MAX_SECTOR];

  unsigned int blocks_need = 0;
  unsigned int blocks_available = 0;
  byte sector_num = 0;

  readSector(&map_fs_buf, FS_MAP_SECTOR_NUMBER);
  readSector(&data_fs_buf, FS_DATA_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
  readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER + 0x001);

  *status = FS_W_NO_FREE_NODE;

  for (i = 0; i < FS_MAX_NODE; i++) {
    node_now = &node_fs_buf.nodes[i];

    if (node_now->parent_index == parent_index &&
        strcmp(node_now->node_name, name) == 1) {
      *status = FS_W_NODE_ALREADY_EXISTS;
      return;
    }
  }

  for (i = 0; i < FS_MAX_NODE; i++) {
    if (node_fs_buf.nodes[i].node_name[0] == 0x00) {
      node_empty_index = i;
      break;
    }
  }

  if (node_empty_index == -1) {
    *status = FS_W_NO_FREE_NODE;
    return;
  }

  if (metadata->filesize == 0) {
    strcpy(node_fs_buf.nodes[i].node_name, metadata->node_name);
    node_fs_buf.nodes[i].parent_index = metadata->parent_index;
    node_fs_buf.nodes[i].data_index = FS_NODE_D_DIR;

    *status = FS_SUCCESS;
    return;
  }

  for (int i = 0; i < FS_MAX_DATA; i++) {
    if (data_fs_buf.datas[i].sectors[0] == 0x00) {
      data_empty_index = i;
      break;
    }
  }

  if (data_empty_index == -1) {
    *status = FS_W_NO_FREE_DATA;
    return;
  }

  blocks_need = div(metadata->filesize + SECTOR_SIZE - 1, SECTOR_SIZE);
  for (i = 0; i < 256 && blocks_available < blocks_need; i++) {
    if (map_fs_buf.is_used[i] == 0x00) {
      free_blocks[blocks_available] = i;
      blocks_available++;
    }
  }

  if (blocks_available < blocks_available) {
    *status = FS_W_NOT_ENOUGH_SPACE;
    return;
  }

  node_fs_buf.nodes[node_empty_index].parent_index = metadata->parent_index;
  node_fs_buf.nodes[node_empty_index].data_index = data_empty_index;
  strcpy(node_fs_buf.nodes[node_empty_index].node_name, metadata->node_name);

  for (j = 0; j < blocks_available; j++) {
    sector_num = free_blocks[j];

    data_fs_buf.datas[data_empty_index].sectors[j] = sector_num;

    writeSector(metadata->buffer + j * SECTOR_SIZE, sector_num);

    map_fs_buf.is_used[sector_num] = 0x01;
  }

  *status = FS_SUCCESS;
}
