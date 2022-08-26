#ifndef HEXEREI_HEXEREI_H
#define HEXEREI_HEXEREI_H

#include <stdint.h>
#include <stdio.h>

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
	RECORD_AFTER_EOF_ERR,
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
	char data[64];
} hexerei_record_t;


#define LIST_T hexerei_record_t*
#define LIST_NAME hexerei_record_list
#define LIST_POINTER_TYPE
#include "list.h"
typedef struct hexerei_hex_file_t {
	hexerei_record_list_t *records;
} hexerei_hex_file_t;

hexerei_err_e hexerei_record_parse(FILE *f, hexerei_record_t **rec);
hexerei_err_e hexerei_record_write(hexerei_record_t *record, int start, const char *data, size_t length);
hexerei_err_e hexerei_record_read(hexerei_record_t *record, char *output, size_t length);

hexerei_err_e hexerei_hex_readall(FILE *f, hexerei_hex_file_t **hf);
hexerei_err_e hexerei_hex_read_at(hexerei_hex_file_t *hf, uint32_t pos, size_t size);
hexerei_err_e hexerei_hex_write_at(hexerei_hex_file_t *hf, uint32_t pos, size_t size);
void hexerei_hex_free(hexerei_hex_file_t *hf);


#endif
