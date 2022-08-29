#include "hexerei.h"

#include <stdbool.h>
#include <string.h>

typedef struct record_view {
	uint32_t start;
	int first;
	hexerei_record_list_t *records;
} record_view;

static hexerei_err_e
access_at(hexerei_hex_file_t *hf, uint32_t pos, size_t size, record_view *view);

extern uint8_t hex_to_char(const char *);
extern hexerei_err_e decode_hexstr(const char *, size_t, char *);
extern uint8_t hexerei_record_data(hexerei_record_t *r, const char  **datap);


hexerei_err_e hexerei_hex_readall(FILE *f, hexerei_hex_file_t **hfile) {
	if (f == NULL || hfile == NULL) return NULL_INPUT_ERR;

	hexerei_hex_file_t *hf = malloc(sizeof(hexerei_hex_file_t));
	if (hf == NULL) return OUT_OF_MEM_ERR;

	hexerei_record_list_t *list = hexerei_record_list_init();
	if (list == NULL) { free(hf); return OUT_OF_MEM_ERR; }

	bool eof = false;
	hexerei_err_e err = OUT_OF_MEM_ERR;

	hexerei_record_t *rec;
	hexerei_err_e parse_err = hexerei_record_parse(f, &rec);

	while (parse_err == NO_ERR) {
		if (eof) {
			err = RECORD_AFTER_EOF_ERR;
			free(rec);
			goto cleanup;
		}
		list_err_e lerr = hexerei_record_list_append(list, rec);
		if (lerr != LIST_NO_ERR) { free(rec); goto cleanup; }
		if (rec->type == EOF_REC) eof = true;
		parse_err = hexerei_record_parse(f, &rec);
	}

	if (parse_err != NO_MORE_RECORDS_ERR || rec->type == EOF_REC) {
		hf->records = list;
		*hfile = hf;
		return NO_ERR;
	}
	err = NO_EOF_REC_ERR;

cleanup:
	hexerei_record_list_free_all(list);
	free(hf);
	return err;
}

hexerei_err_e
hexerei_hex_read_at(hexerei_hex_file_t *hf, uint32_t pos, size_t size, uint8_t *read)
{
	record_view view = {0};
	hexerei_err_e err = access_at(hf, pos, size, &view);
	if(err != NO_ERR) return err;

	const size_t hsize = size * 2;
	uint32_t written_len = 0;
	char *read_data = calloc(hsize, sizeof(char));
	if(read_data == NULL) {
		hexerei_record_list_free(view.records);
		return OUT_OF_MEM_ERR;
	}

	for(size_t i = 0; i < view.records->length; i++) {
		hexerei_record_t *curr = view.records->items[i];

		const char *datap = &(const char){0};
		err = hexerei_record_data(curr, &datap);
		if(err != NO_ERR) {
			hexerei_record_list_free(view.records);
			free(read_data);
			return NULL_INPUT_ERR;
		}
		uint8_t len_data = curr->count*2;

		if(i == 0 && view.start != 0) {
			if(view.start + hsize < len_data) {
				memcpy(read_data, datap + view.start, hsize);
				break;
			}
			memcpy(read_data, datap + view.start, len_data - view.start);
			written_len += len_data - view.start;
			continue;
		}

		uint32_t end = (hsize - written_len);
		if(curr->count > hsize - written_len) {
			memcpy(read_data + written_len, datap, end);
			break;
		} else {
			memcpy(read_data + written_len, datap, len_data > end ? end : len_data);
			written_len += len_data;
		}
	}

	err = decode_hexstr(read_data, hsize, (char*)read);
	hexerei_record_list_free(view.records);
	free(read_data);
	return err;
}

void hexerei_hex_free(hexerei_hex_file_t *hf)
{
	hexerei_record_list_free_all(hf->records);
	free(hf);
	hf = NULL;
}

static hexerei_err_e
access_at(hexerei_hex_file_t *hf, uint32_t pos, size_t size, record_view *view)
{
	if(view == NULL) return NULL_INPUT_ERR;
	if(size < 1) return NO_ERR;

	const size_t hsize = size*2;
	uint32_t base = 0;

	hexerei_err_e err;
	uint16_t u16_ext_addr;
	uint8_t ext_addr[2] = {0};

	for(size_t i = 0; i < hf->records->length; i++) {
		hexerei_record_t *rec = hf->records->items[i];
		uint32_t ulen;
		uint32_t recordbase;

		switch (rec->type) {
			case EXT_SEG_ADDR_REC:
				err = hexerei_record_read(rec, ext_addr, rec->count);
				if(err != NO_ERR) return WRONG_RECORD_FMT_ERR;
				u16_ext_addr = ext_addr[0] << 8 | ext_addr[1];
				base = u16_ext_addr * 16;
				break;
			case EXT_LIN_ADDR_REC:
				err = hexerei_record_read(rec, ext_addr, rec->count);
				if(err != NO_ERR) return WRONG_RECORD_FMT_ERR;
				u16_ext_addr = ext_addr[0] << 8 | ext_addr[1];
				base = u16_ext_addr << 16;
				break;
			case DATA_REC:
				ulen = rec->count * 2;
				recordbase = rec->address + base;

				if(pos >= recordbase && pos < recordbase + ulen) {
					uint32_t start = (pos - recordbase)* 2;
					uint32_t end = start + hsize;
					if(end > ulen) end = ulen;

					view->start = (pos - recordbase) * 2;
					view->first = (int)i;
					view->records = hexerei_record_list_init();
					if(view->records == NULL) return OUT_OF_MEM_ERR;
					list_err_e lerr = hexerei_record_list_append(view->records, rec);
					if(lerr != LIST_NO_ERR) {
						hexerei_record_list_free(view->records);
						return OUT_OF_MEM_ERR;
					}

					uint32_t already_accessed_len = end - start;
					while(already_accessed_len < hsize && i != hf->records->length-1) {
						i++;
						hexerei_record_t *curr = hf->records->items[i];
						if(curr->type != DATA_REC) {
							hexerei_record_list_free(view->records);
							return OUT_OF_BOUNDS_ERR;
						}
						lerr = hexerei_record_list_append(view->records, curr);
						if(lerr != LIST_NO_ERR) {
							hexerei_record_list_free(view->records);
							return OUT_OF_MEM_ERR;
						}
						already_accessed_len += curr->count;
					}

					if(already_accessed_len < hsize) {
						hexerei_record_list_free(view->records);
						return OUT_OF_BOUNDS_ERR;
					}
					return NO_ERR;
				}
			default:
				break;
		}
	}
	return OUT_OF_BOUNDS_ERR;
}