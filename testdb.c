
#pragma optimize    9
#pragma lint       -1
#pragma debug       0
#pragma path        "include"

#include <stdio.h>
#include <errno.h>
#include "include/libdb.h"

#define ASSERT(condition, message) do { if (!(condition)) { fprintf(stderr, "Assertion failed: %s\n", message); return -1; } } while (0)

#define MAX_RECORDS  100

typedef struct test_record {
    id_t        id;
    id_t        i;
    char        s[10];
} record_t;

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

int main(void) {
    bool retVal;

    setUp();
    // DB creation
    ASSERT(dbCreate(0) == true, "dbCreate(): couldn't create DB");
    ASSERT(dbCreate(-1) == false, "dbCreate(): dbNum can't be negative");
    ASSERT(errno == EINVAL, "dbOpen(): errno isn't correct - 1"); errno = 0;
    ASSERT(dbCreate(100) == false, "dbCreate(): dbNum exceeds limit");
    ASSERT(errno == EINVAL, "dbOpen(): errno isn't correct - 2"); errno = 0;

    // Adding records
    record_t r = { .id = 1, .i = 0, .s = "0123456789"};
    for(int i = 1; i <= MAX_RECORDS; i++){
        sprintf(r.s, "R%d", i);
        r.i = i*2;
//        printf("Writing i=%d id=%d s='%s' r.i=%d\n", i, r.id, r.s, r.i);
        ASSERT(dbAdd(0, (byte_t *) &r, true) == true, "dbAdd(): Failed to add record");
        ASSERT(errno == 0, "dbAdd(): errno isn't zero");
        ASSERT(r.id == i, "dbAdd(): r.id != i");
    }
    ASSERT(dbAdd(100, (byte_t *) &r, true) == false, "dbNum exceeds limit");
    ASSERT(errno == EINVAL, "dbAdd(): errno isn't correct - 2"); errno = 0;
    ASSERT(dbAdd(-2, (byte_t *) &r, true) == false, "dbNum can't be negative");
    ASSERT(errno == EINVAL, "dbAdd(): errno isn't correct - 3"); errno = 0;

    // Getting records
    for(int i = 1; i <= MAX_RECORDS; i++){
        retVal = dbGet(0, i, (byte_t *) &r);
        ASSERT(errno == 0, "dbGet(): errno isn't zero");
//        printf("Reading %d %d %s %d\n", i, r.id, r.s, r.i);
        ASSERT(retVal == true, "dbGet(): failed to get record");
        ASSERT(r.id == i, "dbGet(): r.id != i");
        ASSERT(r.i == i*2, "dbGet(): r.i != i*2");
    }

    ASSERT(dbGet(100, 0, (byte_t *) &r) == false, "dbGet(): dbNum exceeds limit");
    ASSERT(errno == EINVAL, "dbGet(): errno isn't correct - 1"); errno = 0;
    ASSERT(dbGet(0, 9999, (byte_t *) &r) == false, "dbGet(): id too large");
    ASSERT(errno == EIO, "dbGet(): errno isn't correct - 2"); errno = 0;
    ASSERT(dbGet(0, -100, (byte_t *) &r) == false, "dbGet(): id is negative");
    ASSERT(errno == EINVAL, "dbGet(): errno isn't correct - 3"); errno = 0;

    // Test disabling auto ID and record update
    r.id = 1; r.i = 123; sprintf(r.s, "test%d", r.i);
//    printf("before: %d %s %d\n", r.id, r.s, r.i);
    ASSERT(dbAdd(0, (byte_t *) &r, false) == true, "dbAdd(): failed to update record");
    ASSERT(dbGet(0, 1, (byte_t *) &r), "dbGet(): couldn't read record 90");
//    printf("after: %d %s %d\n", r.id, r.s, r.i);
    ASSERT(r.i == 123, "dbGet(): r.i was not correct");

    retVal = dbGet(0, MAX_RECORDS+1, (byte_t *) &r);
    ASSERT(retVal == false, "dbGet(): was able to read past MAX_RECORDS");
    ASSERT(errno == EIO, "dbGet(): errno isn't correct - 4"); errno = 0;

    tearDown();
    printf("*** Tests passed\n");
    return 0;
}
