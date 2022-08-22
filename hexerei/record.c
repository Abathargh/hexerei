#include "hexerei.h"
#include <stdint.h>

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

#define IS_DIGIT(d)        (d >= '0' && d <= '9')
#define IS_HEXCHAR_LO(d)   (d >= 'a' && d <= 'f')
#define IS_HEXCHAR_HI(d)   (d >= 'A' && d <= 'F')
#define VALID_HEX_DIGIT(d) (IS_DIGIT(d) || IS_HEXCHAR_LO(d) || IS_HEXCHAR_HI(d))

#define DECODE_COUNT(d,r)  decode_hexstr(&d->data[COUNT_IDX], COUNT_LEN, r)
#define DECODE_ADDR(d,r)   decode_hexstr(&d->data[ADDR_IDX], ADDR_LEN, r)
#define DECODE_CKSM(d,l,r) decode_hexstr(&d->data[DATA_IDX+l*2], CKSUM_LEN, r)
#define DECODE_TYPE(d,r)   decode_hexstr(&d->data[TYPE_IDX], TYPE_LEN, r)

static hex_record_type_e validate_record(hex_record_t *);
static uint8_t hex_to_char(const char *);
static hexerei_err_e decode_hexstr(const char *, size_t, char *);
static uint8_t checksum(char *record, size_t len);

hexerei_err_e
hexerei_parse_record(FILE *f, hex_record_t *rec)
{
  if(rec == NULL) {
    return NULL_INPUT_ERR;
  }

  int curr = getc(f);
  if(curr == EOF) {
    return NO_MORE_RECORDS_ERR;
  }

  if(curr != START_CODE) {
    return MISSING_START_CODE_ERR;
  }

  int idx = 0;
  for(;curr != '\r' && curr != '\n'; idx++) {
    rec->data[idx] = (char)curr;
    curr = getc(f);
    if(curr == EOF) {
      return WRONG_RECORD_FMT_ERR;
    }
  }

  if(curr == '\r') {
    curr = getc(f);
    if(curr == EOF || curr != '\n') {
      return WRONG_RECORD_FMT_ERR;
    }
  }

  rec->length = idx;
	hex_record_type_e type = validate_record(rec);
	if(type == INVALID_REC)
	{
		return WRONG_RECORD_FMT_ERR;
	}
	rec->type = type;
	return NO_ERR;
}

static hex_record_type_e
validate_record(hex_record_t *rec) {
  uint8_t len, cks, curr_cks;
	uint16_t addr;

	if((rec->length < MIN_REC_LEN) ||
	 	(DECODE_COUNT(rec, (char*)&len) != NO_ERR) ||
		(rec->length != MIN_REC_LEN + 2*len) ||
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
		case EXTENDED_SEGMENT_ADDRESS_REC:
		case EXTENDED_LINEAR_ADDRESS_REC:
			if(len != 2) {
				return INVALID_REC;
			}
			break;
		case START_SEGMENT_ADDRESS_REC:
		case START_LINEAR_ADDRESS_REC:
			if((DECODE_ADDR(rec, (char*)&addr) != NO_ERR) || addr != 0 || len != 4) {
				return INVALID_REC;
			}
			break;
		default:
			break;
	}

	return (hex_record_type_e)type;
}


static hexerei_err_e
decode_hexstr(const char *in, size_t i_len, char *out)
{
	// len(out) == len(in) / 2
  char buf[2] = {};
	int out_idx = 0;
  for(size_t i = 0; i < i_len; i++) {
    if(!VALID_HEX_DIGIT(in[i]))
			return INVALID_HEX_DIGIT;
		buf[i%2] = in[i];
    if(i%2 != 0) out[out_idx++] = (char)hex_to_char(buf);
  }

  return NO_ERR;
}

static uint8_t hex_to_char(const char *in)
{
	// len == 2
	uint8_t r = 0;
	uint8_t p = 0;
	for(int i = 0; i < 2; i++) {
		if (IS_DIGIT(in[i]))
			p = '0';
		else if (IS_HEXCHAR_LO(in[i]))
			p = 'a' - 10;
		else if (IS_HEXCHAR_HI(in[i]))
			p =  'A' - 10;
		r |= (in[i] - p) << ((1-i)*4);
	}
	return r;
}

static hexerei_err_e
hex_to_u8(char *in, uint8_t *n)
{
	return NO_ERR;
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