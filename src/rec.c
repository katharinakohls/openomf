#include <stdlib.h>
#include <string.h>

#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/error.h"
#include "shadowdive/rec.h"

int sd_rec_create(sd_rec_file *rec) {
    if(rec == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(rec, 0, sizeof(sd_rec_file));
    return SD_SUCCESS;
}

void sd_rec_free(sd_rec_file *rec) {
    if(rec == NULL) return;
    for(int i = 0; i < 2; i++) {
        if(rec->pilots[i]) {
            sd_pilot_free(rec->pilots[i]);
            free(rec->pilots[i]);
        }
    }
    if(rec->moves) {
        free(rec->moves);
    }
}

int sd_rec_load(sd_rec_file *rec, const char *file) {
    if(rec == NULL || file == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(file);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Make sure we have at least this much data
    if(sd_reader_filesize(r) < 1224) {
        goto error_0;
    }

    // Read pilot data
    for(int i = 0; i < 2; i++) {
        // MWAHAHAHA
        size_t ppos = sd_reader_pos(r);
        sd_read_buf(r, rec->hack_time[i], 428);
        sd_reader_set(r, ppos);

        // Read pilot data
        rec->pilots[i] = malloc(sizeof(sd_pilot));
        sd_pilot_create(rec->pilots[i]);
        sd_pilot_load(r, rec->pilots[i]);
        sd_skip(r, 168); // This contains empty palette and psrite etc. Just skip.
    }

    // Scores
    for(int i = 0; i < 2; i++)
        rec->scores[i] = sd_read_udword(r);

    // Other flags
    rec->unknown_a = sd_read_byte(r);
    rec->unknown_b = sd_read_byte(r);
    rec->unknown_c = sd_read_byte(r);
    rec->unknown_d = sd_read_word(r);
    rec->unknown_e = sd_read_word(r);
    rec->unknown_f = sd_read_word(r);
    rec->unknown_g = sd_read_word(r);
    rec->unknown_h = sd_read_word(r);
    rec->unknown_i = sd_read_word(r);
    rec->unknown_j = sd_read_word(r);
    rec->unknown_k = sd_read_word(r);
    rec->unknown_l = sd_read_dword(r);
    rec->unknown_m = sd_read_byte(r);

    // Allocate enough space for the record blocks
    // This will be reduced later when we know the ACTUAL count
    size_t rsize = sd_reader_filesize(r) - sd_reader_pos(r);
    rec->move_count = rsize / 7;
    rec->moves = malloc(rec->move_count * sizeof(sd_rec_move));

    // Read blocks
    for(int i = 0; i < rec->move_count; i++) {
        rec->moves[i].tick = sd_read_udword(r);
        rec->moves[i].extra = sd_read_ubyte(r);
        rec->moves[i].player_id = sd_read_ubyte(r);
        uint8_t action = sd_read_ubyte(r);
        rec->moves[i].raw_action = action;

        rec->moves[i].action = SD_REC_NONE;
        if(action & SD_REC_PUNCH) {
            rec->moves[i].action |= SD_REC_PUNCH;
        }
        if(action & SD_REC_KICK) {
            rec->moves[i].action |= SD_REC_KICK;
        }
        switch(action & 0xF0) {
            case 16: rec->moves[i].action |= SD_REC_UP; break;
            case 32: rec->moves[i].action |= (SD_REC_UP|SD_REC_RIGHT); break;
            case 48: rec->moves[i].action |= SD_REC_RIGHT; break;
            case 64: rec->moves[i].action |= (SD_REC_DOWN|SD_REC_RIGHT); break;
            case 80: rec->moves[i].action |= SD_REC_DOWN; break;
            case 96: rec->moves[i].action |= (SD_REC_DOWN|SD_REC_LEFT); break;
            case 112: rec->moves[i].action |= SD_REC_LEFT; break;
            case 128: rec->moves[i].action |= (SD_REC_UP|SD_REC_LEFT); break;
        }
        if(rec->moves[i].extra > 2) {
            sd_read_buf(r, rec->moves[i].extra_data, 7);
            rec->move_count--;
        }
    }

    // Okay, not reduce the allocated memory to match what we actually need
    // Realloc should keep our old data intact
    rec->moves = realloc(rec->moves, rec->move_count * sizeof(sd_rec_move));

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;

error_0:
    sd_reader_close(r);
    return SD_FILE_PARSE_ERROR;
}

int sd_rec_save(sd_rec_file *rec, const char *file) {
    sd_writer *w;

    if(rec == NULL || file == NULL) {
        return SD_INVALID_INPUT;
    }

    if(!(w = sd_writer_open(file))) {
        return SD_FILE_OPEN_ERROR;
    }

    // Write pilots, palettes, etc.
    for(int i = 0; i < 2; i++) {
        // MWAHAHAHA
        sd_write_buf(w, rec->hack_time[i], 428);
        sd_write_fill(w, 0, 168);
    }

    // Scores
    for(int i = 0; i < 2; i++)
        sd_write_udword(w, rec->scores[i]);

    // Other header data
    sd_write_byte(w, rec->unknown_a);
    sd_write_byte(w, rec->unknown_b);
    sd_write_byte(w, rec->unknown_c);
    sd_write_word(w, rec->unknown_d);
    sd_write_word(w, rec->unknown_e);
    sd_write_word(w, rec->unknown_f);
    sd_write_word(w, rec->unknown_g);
    sd_write_word(w, rec->unknown_h);
    sd_write_word(w, rec->unknown_i);
    sd_write_word(w, rec->unknown_j);
    sd_write_word(w, rec->unknown_k);
    sd_write_dword(w, rec->unknown_l);
    sd_write_byte(w, rec->unknown_m);

    // Move records
    for(int i = 0; i < rec->move_count; i++) {
        sd_write_udword(w, rec->moves[i].tick);
        sd_write_ubyte(w, rec->moves[i].extra);
        sd_write_ubyte(w, rec->moves[i].player_id);
        if(rec->moves[i].extra > 2) {
            sd_write_ubyte(w, rec->moves[i].raw_action);
            sd_write_buf(w, rec->moves[i].extra_data, 7);
            continue;
        }

        uint8_t raw_action = 0;
        switch(rec->moves[i].action & SD_MOVE_MASK) {
            case (SD_REC_UP): raw_action = 16; break;
            case (SD_REC_UP|SD_REC_RIGHT): raw_action = 32; break;
            case (SD_REC_RIGHT): raw_action = 48; break;
            case (SD_REC_DOWN|SD_REC_RIGHT): raw_action = 64; break;
            case (SD_REC_DOWN): raw_action = 80; break;
            case (SD_REC_DOWN|SD_REC_LEFT): raw_action = 96; break;
            case (SD_REC_LEFT): raw_action = 112; break;
            case (SD_REC_UP|SD_REC_LEFT): raw_action = 128; break;
        }
        if(rec->moves[i].action & SD_REC_PUNCH)
            raw_action |= 1;
        if(rec->moves[i].action & SD_REC_KICK)
            raw_action |= 2;
        sd_write_ubyte(w, raw_action);

   }

    sd_writer_close(w);
    return SD_SUCCESS;
}

int sd_rec_delete_action(sd_rec_file *rec, unsigned int number) {
    if(number >= rec->move_count || rec == NULL) {
        return SD_INVALID_INPUT;
    }
    size_t single = sizeof(sd_rec_move);
    size_t size = (rec->move_count * single) - single;

    memmove(
        rec->moves + number,
        rec->moves + number + 1,
        size);

    rec->move_count--;
    rec->moves = realloc(rec->moves, rec->move_count * single);

    if(rec->moves == NULL) {
        return SD_OUT_OF_MEMORY;
    }
    return SD_SUCCESS;
}

int sd_rec_insert_action(sd_rec_file *rec, unsigned int number, const sd_rec_move *move) {
    if(rec == NULL) {
        return SD_INVALID_INPUT;
    }
    if(number >= rec->move_count) {
        number = rec->move_count;
    }
    size_t single = sizeof(sd_rec_move);
    size_t start = number * single;
    size_t size = rec->move_count * single;

    rec->move_count++;
    rec->moves = realloc(rec->moves, rec->move_count * single);
    if(rec->moves == NULL) {
        return SD_OUT_OF_MEMORY;
    }

    // Only move if we are inserting.
    if(number < rec->move_count) {
        memmove(
            rec->moves + start + single,
            rec->moves + start, 
            (size - start));
    }
    memcpy(
        rec->moves + start, move, single);

    return SD_SUCCESS;
}
