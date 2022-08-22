#include <string.h>

#include "unity.h"
#include "hexerei.h"
#include "record.h"

void setUp(void) {
}

void tearDown(void) {
}

void test_parse_record(void) {
	struct record_test {
		const char *input;
		hexerei_err_e err;
		hex_record_t *rec;
	}	test_cases[] = {
		{"", NO_MORE_RECORDS_ERR, NULL},
		{"\r\n", MISSING_START_CODE_ERR, NULL},
		{":\r\n", WRONG_RECORD_FMT_ERR, NULL},
		{":\r", WRONG_RECORD_FMT_ERR, NULL},
		{":\n", WRONG_RECORD_FMT_ERR, NULL},
		{":0001\r\n", WRONG_RECORD_FMT_ERR, NULL},
		{":00\r\n", WRONG_RECORD_FMT_ERR, NULL},
		{":020000021000EC\r\n", NO_ERR, &(hex_record_t){
			15,
			EXTENDED_SEGMENT_ADDRESS_REC,
			{':', '0', '2', '0', '0', '0', '0', '0', '2', '1', '0', '0', '0', 'E', 'C'},
			}},
			{":0400000300000000F9\r\n", NO_ERR, &(hex_record_t){
			19,
			START_SEGMENT_ADDRESS_REC,
			{':', '0', '4', '0', '0', '0', '0', '0', '3', '0', '0', '0', '0', '0', '0', '0', '0', 'F', '9'},
			}},
		{":06058000000A000000006B\r\n", NO_ERR, &(hex_record_t){
			23,
			DATA_REC,
			{':', '0', '6', '0', '5', '8', '0', '0', '0', '0', '0', '0', 'A', '0', '0', '0', '0', '0', '0', '0', '0', '6', 'B'}
		}},
		{":00000001FF\r\n", NO_ERR, &(hex_record_t){
			11,
			EOF_REC,
			{':', '0', '0', '0', '0', '0', '0', '0', '1', 'F', 'F'}
		}}
	};

	for(int i = 0; i < sizeof(test_cases)/sizeof(struct record_test); i++) {
		struct record_test test_case = test_cases[i];
		FILE *record_str = fmemopen((void*)test_case.input, strlen(test_case.input), "r");
		hex_record_t record = {0};
		hexerei_err_e err = hexerei_parse_record(record_str, &record);
		if(err != NO_ERR) {
			TEST_ASSERT_EQUAL_MESSAGE(test_cases[i].err, err, test_case.input);
			continue;
		}
		TEST_ASSERT_EQUAL_INT(test_case.rec->length, record.length);
		TEST_ASSERT_EQUAL_CHAR_ARRAY(test_case.rec->data, record.data, test_case.rec->length);
	}
}

int main (void) {
	UNITY_BEGIN();
	RUN_TEST(test_parse_record);
	return UNITY_END();
}