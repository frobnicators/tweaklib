#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "utils/sha1.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define HASH_LENGTH 20
#define BLOCK_LENGTH 64

struct sha1 {
	uint32_t buffer[BLOCK_LENGTH/4];
	uint32_t state[HASH_LENGTH/4];
	uint32_t byteCount;
	uint8_t bufferOffset;
	uint8_t keyBuffer[BLOCK_LENGTH];
	uint8_t innerHash[HASH_LENGTH];
};

#define SHA1_K0  0x5a827999
#define SHA1_K20 0x6ed9eba1
#define SHA1_K40 0x8f1bbcdc
#define SHA1_K60 0xca62c1d6

sha1_t sha1_new() {
	sha1_t sha = malloc(sizeof(struct sha1));
	sha1_reset(sha);
	return sha;
}

void sha1_reset(sha1_t sha){
	sha->state[0] = 0x67452301;
	sha->state[1] = 0xefcdab89;
	sha->state[2] = 0x98badcfe;
	sha->state[3] = 0x10325476;
	sha->state[4] = 0xc3d2e1f0;
	sha->byteCount = 0;
	sha->bufferOffset = 0;
}

void sha1_free(sha1_t sha){
	free(sha);
}

static uint32_t sha1_rol32(uint32_t number, uint8_t bits){
	return ((number << bits) | (number >> (32-bits)));
}

static void sha1_hashBlock(sha1_t s){
	uint32_t a,b,c,d,e,t;

	a=s->state[0];
	b=s->state[1];
	c=s->state[2];
	d=s->state[3];
	e=s->state[4];

	for ( uint8_t i=0; i<80; i++ ){
		if ( i >= 16 ){
			t = s->buffer[(i+13)&15] ^ s->buffer[(i+8)&15] ^ s->buffer[(i+2)&15] ^ s->buffer[i&15];
			s->buffer[i&15] = sha1_rol32(t,1);
		}

		if ( i < 20 ){
			t = (d ^ (b & (c ^ d))) + SHA1_K0;
		} else if (i<40) {
			t = (b ^ c ^ d) + SHA1_K20;
		} else if (i<60) {
			t = ((b & c) | (d & (b | c))) + SHA1_K40;
		} else {
			t = (b ^ c ^ d) + SHA1_K60;
		}

		t+=sha1_rol32(a,5) + e + s->buffer[i&15];
		e=d;
		d=c;
		c=sha1_rol32(b,30);
		b=a;
		a=t;
	}

	s->state[0] += a;
	s->state[1] += b;
	s->state[2] += c;
	s->state[3] += d;
	s->state[4] += e;
}

static void sha1_addUncounted(sha1_t s, uint8_t data) {
	uint8_t * const b = (uint8_t*) s->buffer;
#ifdef WORDS_BIGENDIAN
	b[s->bufferOffset] = data;
#else
	b[s->bufferOffset ^ 3] = data;
#endif
	s->bufferOffset++;
	if (s->bufferOffset == BLOCK_LENGTH) {
		sha1_hashBlock(s);
		s->bufferOffset = 0;
	}
}

void sha1_update(sha1_t s, const void* ptr, size_t n){
	while ( n --> 0 ){
		s->byteCount++;
		sha1_addUncounted(s, *(const char*)ptr++);
	}
}

static void sha1_pad(sha1_t s) {
	// Pad with 0x80 followed by 0x00 until the end of the block
	sha1_addUncounted(s, 0x80);
	while (s->bufferOffset != 56) sha1_addUncounted(s, 0x00);

	// Append length in the last 8 bytes
	sha1_addUncounted(s, 0); // We're only using 32 bit lengths
	sha1_addUncounted(s, 0); // But SHA-1 supports 64 bit lengths
	sha1_addUncounted(s, 0); // So zero pad the top bits
	sha1_addUncounted(s, s->byteCount >> 29); // Shifting to multiply by 8
	sha1_addUncounted(s, s->byteCount >> 21); // as SHA-1 supports bitstreams as well as
	sha1_addUncounted(s, s->byteCount >> 13); // byte.
	sha1_addUncounted(s, s->byteCount >> 5);
	sha1_addUncounted(s, s->byteCount << 3);
}

uint8_t* sha1_hash_bytes(sha1_t s) {
	sha1_pad(s);

#ifndef WORDS_BIGENDIAN
	// Swap byte order back
	int i;
	for (i=0; i<5; i++) {
		s->state[i]=
			  (((s->state[i])<<24)& 0xff000000)
			| (((s->state[i])<<8) & 0x00ff0000)
			| (((s->state[i])>>8) & 0x0000ff00)
			| (((s->state[i])>>24)& 0x000000ff);
	}
#endif

	return (uint8_t*) s->state;
}

const char* sha1_hash_hex(sha1_t sha, sha1_hash buffer){
	memset(buffer, 0, sizeof(sha1_hash));

	char* dst = buffer;
	uint8_t* hash = sha1_hash_bytes(sha);
	for (int i=0; i<HASH_LENGTH; i++) {
		dst += sprintf(dst, "%02x", hash[i]);
	}

	return buffer;
}
