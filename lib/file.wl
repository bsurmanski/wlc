use "importc"

import(C) "stdio.h"

class File {
    FILE^ file
    this(char^ filenm) {
        .file = fopen(filenm, "r")
    }

    ~this() {
        fclose(.file)
    }

    int flush() {
        return fflush(.file)
    }

    int seek(long sval) {
        return fseek(.file, SEEK_CUR, sval)
    }

    int set(long sval) {
        return fseek(.file, SEEK_SET, sval)
    }

    int rset(long sval) {
        return fseek(.file, SEEK_END, sval)
    }

    long tell() {
        return ftell(.file)
    }

    int get() {
        return getc(.file)
    }

    int peek() {
        int ret = .get()
        ungetc(ret, .file)
        return ret
    }

    long read(void^ buf, long sz, long nmem) {
        return fread(buf, sz, nmem, .file)
    }

    bool eof() {
        return feof(.file)
    }
}
