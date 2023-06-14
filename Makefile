
EXE_DIR := build/
LOAD_DIR := loader/
DATA_DIR := data/
LOADER_DATA_DIR := $(LOAD_DIR)/$(DATA_DIR)
TEST_DIR := tests/
TEST_DATA_DIR := $(TEST_DIR)/$(DATA_DIR)

names := testdb libdb
SRCS := $(names:%=*.c)
OBJS := $(foreach wrd, $(names),$(wrd).a)
ADDS := $(foreach wrd, $(names),+$(wrd).a)
CFLAGS := -I -P -D +O

.PHONY: realclean clean all run

all: db.lib testdb
run: testdb
	cd $(TEST_DIR) && iix testdb

realclean: clean
	rm -rf -- $(TEST_DIR)/*
clean:
	rm -f -- *.sym *.a *.b *.d *.e *.root *.lib testdb

testdb: testdb.a db.lib | $(TEST_DATA_DIR)
	iix link testdb.a db.lib KEEP=$@
	mv $@ $(TEST_DIR)

db.lib: libdb.a
	rm -f db.lib
	iix MakeLib db.lib +libdb.a
	iix chtyp -t lib db.lib

testdb.a: testdb.c include/libdb.h
libdb.a: libdb.c include/libdb.h
$(TEST_DATA_DIR):
	mkdir -p $@
%.a:
	iix compile $(CFLAGS) $<
