#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "utils/base64.h"
#include <stdint.h>
#include <errno.h>

int base64encode(const void* src, size_t bytes, char* dst, size_t bufsize){
	static const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	const uint8_t *data = (const uint8_t *)src;
	size_t i = 0;
	uint8_t n0, n1, n2, n3;

	for ( size_t x = 0; x < bytes; x += 3) {
		/* these three 8-bit (ASCII) characters become one 24-bit number */
		size_t n = ((uint32_t)data[x]) << 16;

		if ( (x+1) < bytes ){
			n += ((uint32_t)data[x+1]) << 8;
		}

		if ( (x+2) < bytes ){
			n += data[x+2];
		}

		/* this 24-bit number gets separated into four 6-bit numbers */
		n0 = (uint8_t)(n >> 18) & 63;
		n1 = (uint8_t)(n >> 12) & 63;
		n2 = (uint8_t)(n >> 6) & 63;
		n3 = (uint8_t)n & 63;

		/* if we have one byte available, then its encoding is spread
		 * out over two characters */
		if ( i >= bufsize ) return EMSGSIZE;
		dst[i++] = base64chars[n0];
		if ( i >= bufsize ) return EMSGSIZE;
		dst[i++] = base64chars[n1];

		/* if we have only two bytes available, then their encoding is
		 * spread out over three chars */
		if ( (x+1) < bytes ){
			if(i >= bufsize) return EMSGSIZE;
			dst[i++] = base64chars[n2];
		}

		/*
		 * if we have all three bytes available, then their encoding is spread
		 * out over four characters
		 */
		if ( (x+2) < bytes ){
			if ( i >= bufsize ) return EMSGSIZE;
			dst[i++] = base64chars[n3];
		}
	}

	/* pad as needed */
	int padCount = 3 - bytes % 3;
	while ( padCount --> 0){
		if(i >= bufsize) return EMSGSIZE;
		dst[i++] = '=';
	}

	/* null terminator */
	if(i >= bufsize) return EMSGSIZE;
	dst[i] = 0;

	return 0;
}
