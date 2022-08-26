#include "hexerei.h"

#include <stdbool.h>

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
	hexerei_record_list_free(list);
	free(hf);
	return err;
}

void hexerei_hex_free(hexerei_hex_file_t *hf)
{
	if(hf->records) {
		for(int i = 0; i < hf->records->length; i++)
			free(hf->records->items[i]);
	}
	free(hf->records);
	free(hf);
	hf = NULL;
}