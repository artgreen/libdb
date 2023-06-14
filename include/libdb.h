#ifndef LIBDB_LIBDB_H
#define LIBDB_LIBDB_H


#include <stdbool.h>

// maximum number of DB's
#define MAX_DB      10
// data sub directory
#define DB_DIR      "data"

#define MAX_FILENAME 14
#define MAX_DIR 50
#define MAX_PATH (MAX_DIR + MAX_FILENAME)
typedef char filename_t[MAX_FILENAME];
typedef char path_t[MAX_PATH];
typedef int id_t;                           // for ID/key values
typedef unsigned char byte_t;

bool dbCreate(byte_t dbNum);
bool dbAdd(byte_t dbNum, byte_t *record, bool assignId);
bool dbGet(byte_t dbNum, id_t id, byte_t *record);

#endif //LIBDB_LIBDB_H
