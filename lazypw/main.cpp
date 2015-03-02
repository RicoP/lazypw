//sha512

#include <string.h>

#if defined(_MSC_VER) || defined(__WATCOMC__)
#define UL64(x) x##ui64
typedef unsigned __int64 uint64_t;
#else
#include <inttypes.h>
#define UL64(x) x##ULL
#endif

#define POLARSSL_ERR_SHA512_FILE_IO_ERROR              -0x007A  /**< Read/write error in file. */


	/**
	* \brief          SHA-512 context structure
	*/
	typedef struct
	{
		uint64_t total[2];          /*!< number of bytes processed  */
		uint64_t state[8];          /*!< intermediate digest state  */
		unsigned char buffer[128];  /*!< data block being processed */

		unsigned char ipad[128];    /*!< HMAC: inner padding        */
		unsigned char opad[128];    /*!< HMAC: outer padding        */
		int is384;                  /*!< 0 => SHA-512, else SHA-384 */
	}
	sha512_context;




/* Implementation that should never be optimized out by the compiler */
static void polarssl_zeroize(void *v, size_t n) {
	volatile unsigned char *p = (unsigned char*)v; while (n--) *p++ = 0;
}

/*
* 64-bit integer manipulation macros (big endian)
*/
#ifndef GET_UINT64_BE
#define GET_UINT64_BE(n,b,i)                            \
{                                                       \
    (n) = ( (uint64_t) (b)[(i)    ] << 56 )       \
        | ( (uint64_t) (b)[(i) + 1] << 48 )       \
        | ( (uint64_t) (b)[(i) + 2] << 40 )       \
        | ( (uint64_t) (b)[(i) + 3] << 32 )       \
        | ( (uint64_t) (b)[(i) + 4] << 24 )       \
        | ( (uint64_t) (b)[(i) + 5] << 16 )       \
        | ( (uint64_t) (b)[(i) + 6] <<  8 )       \
        | ( (uint64_t) (b)[(i) + 7]       );      \
}
#endif /* GET_UINT64_BE */

#ifndef PUT_UINT64_BE
#define PUT_UINT64_BE(n,b,i)                            \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 56 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 48 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >> 40 );       \
    (b)[(i) + 3] = (unsigned char) ( (n) >> 32 );       \
    (b)[(i) + 4] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 5] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 6] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 7] = (unsigned char) ( (n)       );       \
}
#endif /* PUT_UINT64_BE */

/*
* Round constants
*/
static const uint64_t K[80] =
{
	UL64(0x428A2F98D728AE22), UL64(0x7137449123EF65CD),
	UL64(0xB5C0FBCFEC4D3B2F), UL64(0xE9B5DBA58189DBBC),
	UL64(0x3956C25BF348B538), UL64(0x59F111F1B605D019),
	UL64(0x923F82A4AF194F9B), UL64(0xAB1C5ED5DA6D8118),
	UL64(0xD807AA98A3030242), UL64(0x12835B0145706FBE),
	UL64(0x243185BE4EE4B28C), UL64(0x550C7DC3D5FFB4E2),
	UL64(0x72BE5D74F27B896F), UL64(0x80DEB1FE3B1696B1),
	UL64(0x9BDC06A725C71235), UL64(0xC19BF174CF692694),
	UL64(0xE49B69C19EF14AD2), UL64(0xEFBE4786384F25E3),
	UL64(0x0FC19DC68B8CD5B5), UL64(0x240CA1CC77AC9C65),
	UL64(0x2DE92C6F592B0275), UL64(0x4A7484AA6EA6E483),
	UL64(0x5CB0A9DCBD41FBD4), UL64(0x76F988DA831153B5),
	UL64(0x983E5152EE66DFAB), UL64(0xA831C66D2DB43210),
	UL64(0xB00327C898FB213F), UL64(0xBF597FC7BEEF0EE4),
	UL64(0xC6E00BF33DA88FC2), UL64(0xD5A79147930AA725),
	UL64(0x06CA6351E003826F), UL64(0x142929670A0E6E70),
	UL64(0x27B70A8546D22FFC), UL64(0x2E1B21385C26C926),
	UL64(0x4D2C6DFC5AC42AED), UL64(0x53380D139D95B3DF),
	UL64(0x650A73548BAF63DE), UL64(0x766A0ABB3C77B2A8),
	UL64(0x81C2C92E47EDAEE6), UL64(0x92722C851482353B),
	UL64(0xA2BFE8A14CF10364), UL64(0xA81A664BBC423001),
	UL64(0xC24B8B70D0F89791), UL64(0xC76C51A30654BE30),
	UL64(0xD192E819D6EF5218), UL64(0xD69906245565A910),
	UL64(0xF40E35855771202A), UL64(0x106AA07032BBD1B8),
	UL64(0x19A4C116B8D2D0C8), UL64(0x1E376C085141AB53),
	UL64(0x2748774CDF8EEB99), UL64(0x34B0BCB5E19B48A8),
	UL64(0x391C0CB3C5C95A63), UL64(0x4ED8AA4AE3418ACB),
	UL64(0x5B9CCA4F7763E373), UL64(0x682E6FF3D6B2B8A3),
	UL64(0x748F82EE5DEFB2FC), UL64(0x78A5636F43172F60),
	UL64(0x84C87814A1F0AB72), UL64(0x8CC702081A6439EC),
	UL64(0x90BEFFFA23631E28), UL64(0xA4506CEBDE82BDE9),
	UL64(0xBEF9A3F7B2C67915), UL64(0xC67178F2E372532B),
	UL64(0xCA273ECEEA26619C), UL64(0xD186B8C721C0C207),
	UL64(0xEADA7DD6CDE0EB1E), UL64(0xF57D4F7FEE6ED178),
	UL64(0x06F067AA72176FBA), UL64(0x0A637DC5A2C898A6),
	UL64(0x113F9804BEF90DAE), UL64(0x1B710B35131C471B),
	UL64(0x28DB77F523047D84), UL64(0x32CAAB7B40C72493),
	UL64(0x3C9EBE0A15C9BEBC), UL64(0x431D67C49C100D4C),
	UL64(0x4CC5D4BECB3E42B6), UL64(0x597F299CFC657E2A),
	UL64(0x5FCB6FAB3AD6FAEC), UL64(0x6C44198C4A475817)
};

/*
* SHA-512 HMAC context setup
*//*
void sha512_hmac_starts(sha512_context *ctx, const unsigned char *key,
	size_t keylen, int is384)
{
	size_t i;
	unsigned char sum[64];

	if (keylen > 128)
	{
		sha512(key, keylen, sum, is384);
		keylen = (is384) ? 48 : 64;
		key = sum;
	}

	memset(ctx->ipad, 0x36, 128);
	memset(ctx->opad, 0x5C, 128);

	for (i = 0; i < keylen; i++)
	{
		ctx->ipad[i] = (unsigned char)(ctx->ipad[i] ^ key[i]);
		ctx->opad[i] = (unsigned char)(ctx->opad[i] ^ key[i]);
	}

	sha512_starts(ctx, is384);
	sha512_update(ctx, ctx->ipad, 128);

	polarssl_zeroize(sum, sizeof(sum));
}
*/
/*
* SHA-512 HMAC process buffer
*//*
void sha512_hmac_update(sha512_context  *ctx,
	const unsigned char *input, size_t ilen)
{
	sha512_update(ctx, input, ilen);
}
*/
/*
* SHA-512 HMAC final digest
*//*
void sha512_hmac_finish(sha512_context *ctx, unsigned char output[64])
{
	int is384, hlen;
	unsigned char tmpbuf[64];

	is384 = ctx->is384;
	hlen = (is384 == 0) ? 64 : 48;

	sha512_finish(ctx, tmpbuf);
	sha512_starts(ctx, is384);
	sha512_update(ctx, ctx->opad, 128);
	sha512_update(ctx, tmpbuf, hlen);
	sha512_finish(ctx, output);

	polarssl_zeroize(tmpbuf, sizeof(tmpbuf));
}*/

/*
* SHA-512 HMAC context reset
*//*
void sha512_hmac_reset(sha512_context *ctx)
{
	sha512_starts(ctx, ctx->is384);
	sha512_update(ctx, ctx->ipad, 128);
}*/

/*
* output = HMAC-SHA-512( hmac key, input buffer )
*//*
void sha512_hmac(const unsigned char *key, size_t keylen,
	const unsigned char *input, size_t ilen,
	unsigned char output[64], int is384)
{
	sha512_context ctx;

	sha512_init(&ctx);
	sha512_hmac_starts(&ctx, key, keylen, is384);
	sha512_hmac_update(&ctx, input, ilen);
	sha512_hmac_finish(&ctx, output);
	sha512_free(&ctx);
}

*/



void sha512_init(sha512_context *ctx)
{
	memset(ctx, 0, sizeof(sha512_context));
}

void sha512_free(sha512_context *ctx)
{
	if (ctx == NULL)
		return;

	polarssl_zeroize(ctx, sizeof(sha512_context));
}

/*
* SHA-512 context setup
*/
void sha512_starts(sha512_context *ctx, int is384)
{
	ctx->total[0] = 0;
	ctx->total[1] = 0;

	if (is384 == 0)
	{
		/* SHA-512 */
		ctx->state[0] = UL64(0x6A09E667F3BCC908);
		ctx->state[1] = UL64(0xBB67AE8584CAA73B);
		ctx->state[2] = UL64(0x3C6EF372FE94F82B);
		ctx->state[3] = UL64(0xA54FF53A5F1D36F1);
		ctx->state[4] = UL64(0x510E527FADE682D1);
		ctx->state[5] = UL64(0x9B05688C2B3E6C1F);
		ctx->state[6] = UL64(0x1F83D9ABFB41BD6B);
		ctx->state[7] = UL64(0x5BE0CD19137E2179);
	}
	else
	{
		/* SHA-384 */
		ctx->state[0] = UL64(0xCBBB9D5DC1059ED8);
		ctx->state[1] = UL64(0x629A292A367CD507);
		ctx->state[2] = UL64(0x9159015A3070DD17);
		ctx->state[3] = UL64(0x152FECD8F70E5939);
		ctx->state[4] = UL64(0x67332667FFC00B31);
		ctx->state[5] = UL64(0x8EB44A8768581511);
		ctx->state[6] = UL64(0xDB0C2E0D64F98FA7);
		ctx->state[7] = UL64(0x47B5481DBEFA4FA4);
	}

	ctx->is384 = is384;
}

void sha512_process(sha512_context *ctx, const unsigned char data[128])
{
	int i;
	uint64_t temp1, temp2, W[80];
	uint64_t A, B, C, D, E, F, G, H;

#define  SHR(x,n) (x >> n)
#define ROTR(x,n) (SHR(x,n) | (x << (64 - n)))

#define S0(x) (ROTR(x, 1) ^ ROTR(x, 8) ^  SHR(x, 7))
#define S1(x) (ROTR(x,19) ^ ROTR(x,61) ^  SHR(x, 6))

#define S2(x) (ROTR(x,28) ^ ROTR(x,34) ^ ROTR(x,39))
#define S3(x) (ROTR(x,14) ^ ROTR(x,18) ^ ROTR(x,41))

#define F0(x,y,z) ((x & y) | (z & (x | y)))
#define F1(x,y,z) (z ^ (x & (y ^ z)))

#define P(a,b,c,d,e,f,g,h,x,K)                  \
		{                                               \
    temp1 = h + S3(e) + F1(e,f,g) + K + x;      \
    temp2 = S2(a) + F0(a,b,c);                  \
    d += temp1; h = temp1 + temp2;              \
		}

	for (i = 0; i < 16; i++)
	{
		GET_UINT64_BE(W[i], data, i << 3);
	}

	for (; i < 80; i++)
	{
		W[i] = S1(W[i - 2]) + W[i - 7] +
			S0(W[i - 15]) + W[i - 16];
	}

	A = ctx->state[0];
	B = ctx->state[1];
	C = ctx->state[2];
	D = ctx->state[3];
	E = ctx->state[4];
	F = ctx->state[5];
	G = ctx->state[6];
	H = ctx->state[7];
	i = 0;

	do
	{
		P(A, B, C, D, E, F, G, H, W[i], K[i]); i++;
		P(H, A, B, C, D, E, F, G, W[i], K[i]); i++;
		P(G, H, A, B, C, D, E, F, W[i], K[i]); i++;
		P(F, G, H, A, B, C, D, E, W[i], K[i]); i++;
		P(E, F, G, H, A, B, C, D, W[i], K[i]); i++;
		P(D, E, F, G, H, A, B, C, W[i], K[i]); i++;
		P(C, D, E, F, G, H, A, B, W[i], K[i]); i++;
		P(B, C, D, E, F, G, H, A, W[i], K[i]); i++;
	} while (i < 80);

	ctx->state[0] += A;
	ctx->state[1] += B;
	ctx->state[2] += C;
	ctx->state[3] += D;
	ctx->state[4] += E;
	ctx->state[5] += F;
	ctx->state[6] += G;
	ctx->state[7] += H;
}

/*
* SHA-512 process buffer
*/
void sha512_update(sha512_context *ctx, const unsigned char *input,
	size_t ilen)
{
	size_t fill;
	unsigned int left;

	if (ilen == 0)
		return;

	left = (unsigned int)(ctx->total[0] & 0x7F);
	fill = 128 - left;

	ctx->total[0] += (uint64_t)ilen;

	if (ctx->total[0] < (uint64_t)ilen)
		ctx->total[1]++;

	if (left && ilen >= fill)
	{
		memcpy((void *)(ctx->buffer + left), input, fill);
		sha512_process(ctx, ctx->buffer);
		input += fill;
		ilen -= fill;
		left = 0;
	}

	while (ilen >= 128)
	{
		sha512_process(ctx, input);
		input += 128;
		ilen -= 128;
	}

	if (ilen > 0)
		memcpy((void *)(ctx->buffer + left), input, ilen);
}

static const unsigned char sha512_padding[128] =
{
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
* SHA-512 final digest
*/
void sha512_finish(sha512_context *ctx, unsigned char output[64])
{
	size_t last, padn;
	uint64_t high, low;
	unsigned char msglen[16];

	high = (ctx->total[0] >> 61)
		| (ctx->total[1] << 3);
	low = (ctx->total[0] << 3);

	PUT_UINT64_BE(high, msglen, 0);
	PUT_UINT64_BE(low, msglen, 8);

	last = (size_t)(ctx->total[0] & 0x7F);
	padn = (last < 112) ? (112 - last) : (240 - last);

	sha512_update(ctx, sha512_padding, padn);
	sha512_update(ctx, msglen, 16);

	PUT_UINT64_BE(ctx->state[0], output, 0);
	PUT_UINT64_BE(ctx->state[1], output, 8);
	PUT_UINT64_BE(ctx->state[2], output, 16);
	PUT_UINT64_BE(ctx->state[3], output, 24);
	PUT_UINT64_BE(ctx->state[4], output, 32);
	PUT_UINT64_BE(ctx->state[5], output, 40);

	if (ctx->is384 == 0)
	{
		PUT_UINT64_BE(ctx->state[6], output, 48);
		PUT_UINT64_BE(ctx->state[7], output, 56);
	}
}


/*
* output = SHA-512( input buffer )
*/
void sha512(const unsigned char *input, size_t ilen,
	unsigned char output[64], int is384)
{
	sha512_context ctx;

	sha512_init(&ctx);
	sha512_starts(&ctx, is384);
	sha512_update(&ctx, input, ilen);
	sha512_finish(&ctx, output);
	sha512_free(&ctx);
}

//base64

#include <inttypes.h>
#include <string.h>

int base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize)
{
	const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_";
	const uint8_t *data = (const uint8_t *)data_buf;
	size_t resultIndex = 0;
	size_t x;
	uint32_t n = 0;
	int padCount = dataLength % 3;
	uint8_t n0, n1, n2, n3;

	/* increment over the length of the string, three characters at a time */
	for (x = 0; x < dataLength; x += 3)
	{
		/* these three 8-bit (ASCII) characters become one 24-bit number */
		n = ((uint32_t)data[x]) << 16; //parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0

		if ((x + 1) < dataLength)
			n += ((uint32_t)data[x + 1]) << 8;//parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0

		if ((x + 2) < dataLength)
			n += data[x + 2];

		/* this 24-bit number gets separated into four 6-bit numbers */
		n0 = (uint8_t)(n >> 18) & 63;
		n1 = (uint8_t)(n >> 12) & 63;
		n2 = (uint8_t)(n >> 6) & 63;
		n3 = (uint8_t)n & 63;

		/*
		* if we have one byte available, then its encoding is spread
		* out over two characters
		*/
		if (resultIndex >= resultSize) return 0;   /* indicate failure: buffer too small */
		result[resultIndex++] = base64chars[n0];
		if (resultIndex >= resultSize) return 0;   /* indicate failure: buffer too small */
		result[resultIndex++] = base64chars[n1];

		/*
		* if we have only two bytes available, then their encoding is
		* spread out over three chars
		*/
		if ((x + 1) < dataLength)
		{
			if (resultIndex >= resultSize) return 0;   /* indicate failure: buffer too small */
			result[resultIndex++] = base64chars[n2];
		}

		/*
		* if we have all three bytes available, then their encoding is spread
		* out over four characters
		*/
		if ((x + 2) < dataLength)
		{
			if (resultIndex >= resultSize) return 0;   /* indicate failure: buffer too small */
			result[resultIndex++] = base64chars[n3];
		}
	}

	if (resultIndex >= resultSize) return 0;   /* indicate failure: buffer too small */
	result[resultIndex] = 0;
	return resultIndex;
}

//MAIN 

#include <stdio.h>
#include "utils.h"

STATIC_ASSERT(sizeof(uint64_t) == 8);

void encodeAndPrint(unsigned char * password) {
	union {
		uint64_t passwordSha512Pack[8];
		unsigned char passwordSha512[64];
	};

	STATIC_ASSERT(sizeof(passwordSha512Pack) == sizeof(passwordSha512));

	sha512(password, strlen((char*)password), passwordSha512, 0);

	char base64out[2][16];

	uint64_t halfs[2] = {
		passwordSha512Pack[0] ^ passwordSha512Pack[1] ^ passwordSha512Pack[2] ^ passwordSha512Pack[3],
		passwordSha512Pack[4] ^ passwordSha512Pack[5] ^ passwordSha512Pack[6] ^ passwordSha512Pack[7]
	};

	int totalLength = 0;

	for (int i = 0; i < 2; ++i) {
		int length = base64encode(&halfs[i], 8, base64out[i], 16);
		totalLength += length;
		ASSERT(length < 16);
	}

	char finalOutput[32];

	SPRINTF(finalOutput, "%s%s", base64out[0], base64out[1]);

	//insert # to satisfy security rules of many websites to include special characters into PW
	finalOutput[passwordSha512Pack[0] % totalLength] = '#';

	printf("%s\n", finalOutput);
}

extern "C" int main(int argc, char** argv) {
	for (int i = 0; i < argc; i++) {
		unsigned char * password = (unsigned char * ) argv[i];
		printf("%s ", argv[i]);
		encodeAndPrint(password);
	}
	return 0;
}