
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path        "include"
#pragma noroot

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "include/libdb.h"

// Symbolic macros for DB index numbers
#define DB_ITEM             0
//#define DB_ADDRESS        1
//#define DB_CUST           1

// format for path names
#define PATH_FMT "%s:%s"

typedef struct {
    id_t id;
    char name[25];
    char sku[25];
} item_t;

/**
 * Database table catalog
 */
static const struct {
    byte_t dbNum;
    filename_t fileName;        /**< File name for the table */
    int recordSize;             /**< Record size for records in this table */
} databases[MAX_DB] = {
    DB_ITEM, "items.bin", sizeof(item_t),
};

/**
 * Open database file
 *
 * @param dbNum Database to open.
 * @param mode File mode to use when opening.
 * @return false on error or pointer to open file. Sets errno.
 */
static FILE *dbOpen(byte_t dbNum, const char *mode) {
    FILE* dbFile = NULL;

    if(dbNum < 0 || dbNum > MAX_DB || databases[dbNum].recordSize <= 0) {
        errno = EINVAL;
    } else {
        //TODO: since we know what DB_DIR is, can we eliminate the call to sprintf()?
        path_t path;
        sprintf(path, PATH_FMT, DB_DIR, databases[dbNum].fileName);
        dbFile = fopen(path, mode);
    }
    return dbFile;
}
/**
 * Creates a database file
 *
 * @param dbNum Database number to create.
 * @return true on success, false on failure. Sets errno.
 */
bool dbCreate(byte_t dbNum) {
    bool retVal = true;
    id_t id = 0;

    // create the file
    FILE *dbFile = dbOpen(dbNum, "wb");
    if(dbFile == (FILE *) NULL) {
        // carry errno from previous function
        retVal = false;
    } else {
        // write a zero for the first ID. Idea is to maybe add meta data to this DB later.
        // yes, i know, this will currently write garbage to the file
        if(fwrite(&id, 1, databases[dbNum].recordSize, dbFile) != databases[dbNum].recordSize) {
            errno = EIO;
            retVal = false;
        }
        // clean up and leave
        fclose(dbFile);
    }
    return retVal;
}
/**
 * Get a record from the database file
 *
 * @param dbNum Database number to read.
 * @param id ID number of the record to read.
 * @param record Record to fill from disk.
 * @return false on failure or true on success. Sets errno.
 */
bool dbGet(byte_t dbNum, id_t id, byte_t *record) {
    bool retVal = true;

    // make sure ID is sane
    if(id <= 0 || id > UINT_MAX) {
        errno = EINVAL;
        retVal = false;
    } else {
        FILE *dbFile = dbOpen(dbNum, "rb");
        if (dbFile == (FILE *) NULL) {
            // carry errno from previous function
            retVal = false;
        } else {
            // Calculate the offset of the record in the file
            long offset = (long) id * databases[dbNum].recordSize;
            // Seek to the offset in the file
            long r = fseek(dbFile, offset, SEEK_SET);
            if (r != -1) {
                if(fread(record, 1, databases[dbNum].recordSize, dbFile) != databases[dbNum].recordSize) {
                    errno = EIO;
                    retVal = false;
                }
            } else {
                // we tried to seek beyond the EOF
                retVal = false;
                // carry errno from previous function
            }
            // Close the file
            fclose(dbFile);
        }
    }
    return retVal;
}
/**
 * Add record to database, or update a record in the database file
 *
 * It is assumed that the record being written starts with a field of id_t. If assignID is true,
 * the record is added to the bottom of the database and the id field of the record will be updated.
 * If assignID is false, the record ID to write will be read from the beginning of record.
 *
 * @param dbNum Database number to add or update.
 * @param record Record to write to the table.
 * @param assignId True will append record to database. False is essentially telling the function to update instead of add.
 * @return false on failure and true on success
 */
bool dbAdd(byte_t dbNum, byte_t *record, bool assignId) {
    FILE *dbFile;
    bool retVal = true;
#ifdef DEBUG
    logLine("dbAdd(): starting record\r\f");
    logHex(record, databases[dbNum].recordSize);
#endif
    if(assignId) {
        // append record to the file
        dbFile = dbOpen(dbNum, "ab");
        if( dbFile == (FILE *) NULL ) {
            retVal = false;
            goto bail; // carry errno from previous function
        } else {
            // Determine the current position in the file
            long size = ftell(dbFile);
            // Create the ID for the record
            id_t id = (id_t) size / databases[dbNum].recordSize;
            // and set the record's ID field with it
            *record = id;
            // go ahead and write the record because we're at the EOF
        }
    } else {
        // update a record in place
        dbFile = dbOpen(dbNum, "r+b");
        if( dbFile == (FILE *) NULL ) {
            retVal = false;
            goto bail; // carry errno from previous function
        } else {
            // use the ID as the record offset in the file
            id_t *idPtr = (id_t *) record;
            // make sure it's sane
            if(*idPtr <= 0 || *idPtr > UINT_MAX) {
                errno = EINVAL;
                retVal = false;
            } else {
                // move to the target offset
                fseek(dbFile, *idPtr * databases[dbNum].recordSize, SEEK_SET);
                // carry errno from previous function
            }
        }
    }
    // Write the item to the file
    if(retVal == true) {
        if (fwrite(record, 1, databases[dbNum].recordSize, dbFile) != databases[dbNum].recordSize) {
            errno = EIO;
            retVal = false;
        }
    }
    // Close the file
    fclose(dbFile);
bail: // bypass the fclose() above to preserve errno
#ifdef DEBUG
    logLine("dbAdd(): ending record\r\f");
    logHex(record, databases[dbNum].recordSize);
#endif
    return retVal;
}
