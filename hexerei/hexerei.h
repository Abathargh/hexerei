#ifndef HEXEREI_HEXEREI_H
#define HEXEREI_HEXEREI_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef enum hexerei_err_e {
  NO_ERR,
	OUT_OF_MEM_ERR, 

	NO_MORE_RECORDS_ERR,
  MISSING_START_CODE_ERR,
  WRONG_RECORD_FMT_ERR,
  INVALID_HEX_DIGIT,

	OUT_OF_BOUNDS_ERR,
	NULL_INPUT_ERR,	
	NO_EOF_REC_ERR,
	MULTIPLE_EOF_ERR,	
} hexerei_err_e ;

typedef enum hexerei_rtype_e{
	DATA_REC,
	EOF_REC,
	EXT_SEG_ADDR_REC,
	START_SEG_ADDR_REC,
	EXT_LIN_ADDR_REC,
	START_LIN_ADDR_REC,
	INVALID_REC,
} hexerei_rtype_e;

typedef struct hexerei_record_t {
	size_t length;
	hexerei_rtype_e type;
	char data[32];
} hexerei_record_t;

typedef struct hexerei_hex_file_t {
	void *records;
} hexerei_hex_file_t;

hexerei_err_e
hexerei_record_parse(FILE *f, hexerei_record_t *rec);

hexerei_err_e
hexerei_record_write(hexerei_record_t *record, int start, const char *data, size_t length);

hexerei_err_e
hexerei_record_read(hexerei_record_t *record, char *output, size_t length);

hexerei_err_e hexerei_hex_readall(FILE *f, hexerei_hex_file_t **hf);
hexerei_err_e hexerei_hex_read_at(hexerei_hex_file_t *hf, uint32_t pos, size_t size);
hexerei_err_e hexerei_hex_write_at(hexerei_hex_file_t *hf, uint32_t pos, size_t size);

#endif
