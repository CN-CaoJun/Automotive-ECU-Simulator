#include "doip_utils.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

int doip_printf_hex(unsigned char *data, int len)
{
	int offset = 0;
	char buffer[4096];

	for (int i = 0; i < len; ++i) {
		if (i % 64 == 0) {
			offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");
		}
		offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%02x ", data[i]);
	}

	logd("%s\n", buffer);

	return offset;
}

void doip_assert(uint8_t expr, const char *comment)
{
	if (!expr) {
		logd("%s\n", comment);
		assert(expr);
	}
}