#include "hexerei.h"

#include <string.h>
#include <stdio.h>

#define START_CODE ':'
#define START_CODE_LEN 1

#define COUNT_LEN 2
#define COUNT_IDX 1
#define COUNT_END (COUNT_IDX + COUNT_LEN)

#define ADDR_LEN 4
#define ADDR_IDX COUNT_END
#define ADDR_END (ADDR_IDX + ADDR_LEN)

#define TYPE_LEN 2
#define TYPE_IDX ADDR_END
#define TYPE_END (TYPE_IDX + TYPE_LEN)

#define DATA_IDX 9
#define CKSUM_LEN 2

// Minimal Length for the whole word (e.g. EOF record)
#define MIN_REC_LEN  (START_CODE_LEN + COUNT_LEN + \
                      ADDR_LEN + TYPE_LEN + CKSUM_LEN)

#define DECODE_COUNT(d,r)  decode_hexstr(&d->data[COUNT_IDX], COUNT_LEN, r)
#define DECODE_ADDR(d,r)   decode_hexstr(&d->data[ADDR_IDX], ADDR_LEN, r)
#define DECODE_CKSM(d,l,r) decode_hexstr(&d->data[DATA_IDX+l*2], CKSUM_LEN, r)
#define DECODE_TYPE(d,r)   decode_hexstr(&d->data[TYPE_IDX], TYPE_LEN, r)

static hexerei_rtype_e validate_record(hexerei_record_t *);
static uint8_t checksum(char *record, size_t len);
static void checksum_hexstr(char *record, size_t len, char *hcks);

extern uint8_t hex_to_char(const char *);
extern hexerei_err_e decode_hexstr(const char *, size_t, char *);


hexerei_err_e
hexerei_record_parse(FILE *f, hexerei_record_t **rec)
{
  if(f == NULL || rec == NULL) return NULL_INPUT_ERR;

  int curr = getc(f);
  if(curr == EOF) return NO_MORE_RECORDS_ERR;
  if(curr != START_CODE) return MISSING_START_CODE_ERR;

	hexerei_record_t *new_rec = malloc(sizeof(hexerei_record_t));
	if(new_rec == NULL) return OUT_OF_MEM_ERR;

  int idx = 0;
  for(; curr != '\r' && curr != '\n' && idx < 64; idx++) {
    new_rec->data[idx] = (char)curr;
    curr = getc(f);
    if(curr == EOF) {
			free(new_rec);
			return WRONG_RECORD_FMT_ERR;
		}
  }

  if(curr == '\r') {
    curr = getc(f);
    if(curr == EOF || curr != '\n') {
			free(new_rec);
			return WRONG_RECORD_FMT_ERR;
    }
  }

	new_rec->length = idx;
	hexerei_rtype_e type = validate_record(new_rec);
	if(type == INVALID_REC) {
		free(new_rec);
		return WRONG_RECORD_FMT_ERR;
	}

	DECODE_COUNT(new_rec, (char*)&new_rec->count);

	uint8_t addr[2];
	DECODE_ADDR(new_rec, (char*)addr);
	new_rec->address = addr[0] << 8 | addr[1];

	new_rec->data[idx] = 0;
	new_rec->type = type;
	*rec = new_rec;
	return NO_ERR;
}

hexerei_err_e
hexerei_record_write(hexerei_record_t *r, int s, const char *d, size_t dl)
{
	if(r == NULL) return OUT_OF_BOUNDS_ERR;

	uint8_t count;
	hexerei_err_e cerr = DECODE_COUNT(r,(char*)&count);
	if(cerr != NO_ERR || s < 0 || s+dl > 2*count)
		return OUT_OF_BOUNDS_ERR;

	// this loops the write-data two times
	for(size_t i = 0; i < dl; i++) {
		if(!VALID_HEX_DIGIT(d[i])) return INVALID_HEX_DIGIT;
	}

	for(size_t i = 0; i < dl; i++) {
		r->data[DATA_IDX+s+i] = d[i];
	}

	char hex_cks[2];
	checksum_hexstr(r->data, r->length, hex_cks);
	for(int i = 0; i < 2; i++) {
		r->data[DATA_IDX+count*2+i] = hex_cks[i];
	}

	return NO_ERR;
}

hexerei_err_e
hexerei_record_data(hexerei_record_t *r, const char  **datap)
{
	if(r == NULL) return NULL_INPUT_ERR;
	*datap = r->data + DATA_IDX;
	return NO_ERR;
}

hexerei_err_e
hexerei_record_read_hex(hexerei_record_t *r, char *out, size_t l)
{
	if(r == NULL || out == NULL)
		return OUT_OF_BOUNDS_ERR;

	// expect validated record
	uint8_t len;
	DECODE_COUNT(r, (char*)&len);
	memcpy(out, (r->data + DATA_IDX), l < len*2 ? l : len*2);
	return NO_ERR;
}

hexerei_err_e
hexerei_record_read(hexerei_record_t *r, uint8_t *out, size_t l)
{
	if(r == NULL || out == NULL)
		return OUT_OF_BOUNDS_ERR;

	// expect validated record
	uint8_t len;
	DECODE_COUNT(r, (char*)&len);
	hexerei_err_e err = decode_hexstr((r->data + DATA_IDX), l < len*2 ? l : len*2, (char*)out);
	return err;
}


static hexerei_rtype_e
validate_record(hexerei_record_t *rec)
{
  uint8_t len, cks, curr_cks;
	uint16_t addr;

	if((rec->length < MIN_REC_LEN) ||
	 	(DECODE_COUNT(rec, (char*)&len) != NO_ERR) ||
		((int)rec->length != MIN_REC_LEN + 2*len) ||
		(DECODE_CKSM(rec, len, (char*)&curr_cks) != NO_ERR)
	)
    return INVALID_REC;

	cks = checksum(rec->data, rec->length);
	if(cks != curr_cks) return INVALID_REC;

	uint8_t type;
	if((DECODE_TYPE(rec, (char*)&type) != NO_ERR) ||
		(type > INVALID_REC))
		return INVALID_REC;

	switch(type) {
		case EXT_SEG_ADDR_REC:
		case EXT_LIN_ADDR_REC:
			if(len != 2) {
				return INVALID_REC;
			}
			break;
		case START_SEG_ADDR_REC:
		case START_LIN_ADDR_REC:
			if((DECODE_ADDR(rec, (char*)&addr) != NO_ERR) || addr != 0 || len != 4) {
				return INVALID_REC;
			}
			break;
		default:
			break;
	}

	return (hexerei_rtype_e)type;
}

static void
checksum_hexstr(char *record, size_t len, char *hcks)
{
	// ip: len(cks) == 2
	uint8_t cks = checksum(record, len);
	for(int i = 0; i < 2; i++) {
		uint8_t cks_hilo = (cks & (0x0f << (1-i)*4)) >> ((1-i)*4);
		uint8_t p = '0';
		if (cks_hilo >= 10)
			p = 'A' - 10;
		hcks[i] = (char)(cks_hilo + p);
	}
}

static uint8_t
checksum(char *record, size_t len)
{
	int start = 0, end = (int)len - 2;
	if(record[0] == START_CODE) start = 1;

	int plen = end - start;
	uint8_t decoded[plen / 2];
	decode_hexstr(&record[start], plen, (char*)decoded);

	uint8_t cks = 0;
	for(int i = 0; i < plen / 2; i++)
		cks += decoded[i];

	return ~(cks)+1;
}
