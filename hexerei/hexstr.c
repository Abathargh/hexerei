#include "hexerei.h"

uint8_t hex_to_char(const char *in);
hexerei_err_e decode_hexstr(const char *in, size_t i_len, char *out);

hexerei_err_e
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

uint8_t hex_to_char(const char *in)
{
	// len == 2, already checked for hex digit
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