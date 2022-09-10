#include "hexerei/hexerei.h"
#include <stdio.h>


int main(void)
{
	FILE *f = fopen("test.hex", "r");
	hexerei_hex_file_t *hexfile;
	hexerei_err_e err = hexerei_hex_readall(f, &hexfile);
	if(err != NO_ERR) {
		printf("ERROR: %d\n", err);
		return 1;
	}
	
	for(int i = 0; i < hexfile->records->length; i++) {
		printf(
			"record[%d]: %s, length: %ld, type: %d\n",
			i,
			hexfile->records->items[i]->data,
			hexfile->records->items[i]->length,
			hexfile->records->items[i]->type
		);
	}
	hexerei_hex_free(hexfile);
	fclose(f);	
	return 0;
}

