#include "internal/reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct sd_reader_t {
    FILE *handle;
    char err[64];
} sd_reader;

sd_reader* sd_reader_open(const char *file) {
    sd_reader *reader = malloc(sizeof(sd_reader));

    reader->handle = fopen(file, "r");
    if(!reader->handle) {
        free(reader);
        return 0;
    }

    return reader;
}

void sd_reader_close(sd_reader *reader) {
    fclose(reader->handle);
    free(reader);
}

int sd_reader_ok(sd_reader *reader) {
    if(feof(reader->handle)) {
        strcpy(reader->err, "Error: End of file reached.");
        return 0;
    }
    return 1;
}

long sd_reader_pos(sd_reader *reader) {
    return ftell(reader->handle);
}

int sd_read_buf(sd_reader *reader, char *buf, int len) {
    if(fread(buf, 1, len, reader->handle) != len) {
        strcpy(reader->err, "Error: Read less than requested amount of bytes.");
        return 0;
    }
    return 1;
}

int sd_peek_buf(sd_reader *reader, char *buf, int len) {
    if(sd_read_buf(reader, buf, len)) {
        return 0;
    }
    fseek(reader->handle, ftell(reader->handle) - len, SEEK_SET);
    return 1;
}

uint8_t sd_read_ubyte(sd_reader *reader) {
    uint8_t d;
    sd_read_buf(reader, (char*)&d, 1);
    return d;
}

uint16_t sd_read_uword(sd_reader *reader) {
    uint16_t d;
    sd_read_buf(reader, (char*)&d, 2);
    return d;
}

uint32_t sd_read_udword(sd_reader *reader) {
    uint32_t d;
    sd_read_buf(reader, (char*)&d, 4);
    return d;
}

int8_t sd_read_byte(sd_reader *reader) {
    int8_t d;
    sd_read_buf(reader, (char*)&d, 1);
    return d;
}

int16_t sd_read_word(sd_reader *reader) {
    int16_t d;
    sd_read_buf(reader, (char*)&d, 2);
    return d;
}

int32_t sd_read_dword(sd_reader *reader) {
    int32_t d;
    sd_read_buf(reader, (char*)&d, 4);
    return d;
}

uint8_t sd_peek_ubyte(sd_reader *reader) {
    uint8_t d;
    sd_peek_buf(reader, (char*)&d, 1);
    return d;
}

uint16_t sd_peek_uword(sd_reader *reader) {
    uint16_t d;
    sd_peek_buf(reader, (char*)&d, 2);
    return d;
}

uint32_t sd_peek_udword(sd_reader *reader) {
    uint32_t d;
    sd_peek_buf(reader, (char*)&d, 4);
    return d;
}

int8_t sd_peek_byte(sd_reader *reader) {
    int8_t d;
    sd_peek_buf(reader, (char*)&d, 1);
    return d;
}

int16_t sd_peek_word(sd_reader *reader) {
    int16_t d;
    sd_peek_buf(reader, (char*)&d, 2);
    return d;
}

int32_t sd_peek_dword(sd_reader *reader) {
    int32_t d;
    sd_peek_buf(reader, (char*)&d, 4);
    return d;
}

int sd_match(sd_reader *reader, char *buf, unsigned int nbytes) {
    char t[nbytes];
    sd_peek_buf(reader, t, nbytes);
    if(memcmp(t, buf, nbytes) == 0) {
        return 1;
    }
    return 0;
}

void sd_skip(sd_reader *reader, unsigned int nbytes) {
    fseek(reader->handle, nbytes, SEEK_CUR);
}

