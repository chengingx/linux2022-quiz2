#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <x86intrin.h>
#include "benchmark.h"
#include "bitmap_tables.h"

size_t bitmap_count(uint64_t *bitmap, size_t bitmapsize) {
    uint64_t count = 0;

    for (size_t i = 0; i < bitmapsize; ++i) {
        count += __builtin_popcountll(bitmap[i]);
    }
    
    return count;
}

size_t bitmap_decode_naive(uint64_t *bitmap, size_t bitmapsize, uint32_t *out)
{
    size_t pos = 0;
    for (size_t k = 0; k < bitmapsize; ++k) {
        uint64_t bitset = bitmap[k];
        size_t p = k * 64;
        for (int i = 0; i < 64; i++) {
            if ((bitset >> i) & 0x1)
                out[pos++] = p + i;
        }
    }
    return pos;
}

size_t bitmap_decode_ctz(uint64_t *bitmap, size_t bitmapsize, uint32_t *out)
{
    size_t pos = 0;
    uint64_t bitset;
    for (size_t k = 0; k < bitmapsize; ++k) {
        bitset = bitmap[k];
        while (bitset != 0) {
            uint64_t t = bitset & -bitset;
            int r = __builtin_ctzll(bitset);
            out[pos++] = k * 64 + r;
            bitset ^= t;    
        }
    }   
    return pos;
}

int bitmap_decode_avx2(uint64_t * array, size_t sizeinwords, uint32_t *out) {
	uint32_t *initout = out;
	__m256i baseVec = _mm256_set1_epi32(-1);
	__m256i incVec = _mm256_set1_epi32(64);
	__m256i add8 = _mm256_set1_epi32(8);

	for (int i = 0; i < sizeinwords; ++i) {
		uint64_t w = array[i];
		if (w == 0) {
			baseVec = _mm256_add_epi32(baseVec, incVec);
		} else {
			for (int k = 0; k < 4; ++k) {
				uint8_t byteA = (uint8_t) w;
				uint8_t byteB = (uint8_t)(w >> 8);
				w >>= 16;
				__m256i vecA = _mm256_load_si256((const __m256i *) vecDecodeTable[byteA]);
				__m256i vecB = _mm256_load_si256((const __m256i *) vecDecodeTable[byteB]);
				uint8_t advanceA = lengthTable[byteA];
				uint8_t advanceB = lengthTable[byteB];
				vecA = _mm256_add_epi32(baseVec, vecA);
				baseVec = _mm256_add_epi32(baseVec, add8);
				vecB = _mm256_add_epi32(baseVec, vecB);
				baseVec = _mm256_add_epi32(baseVec, add8);
				_mm256_storeu_si256((__m256i *) out, vecA);
				out += advanceA;
				_mm256_storeu_si256((__m256i *) out, vecB);
				out += advanceB;
			}
		}
	}
	return out - initout;
}

int main(int argc, char *argv[]){
    
    srand(time(NULL));
    int repeat = 50;
    size_t bitmapsize = 10000;
    uint64_t *bitmap = malloc(sizeof(uint64_t) * bitmapsize);
    uint32_t *out = malloc(sizeof(uint32_t) * 64 * bitmapsize);
    double ratios[] = {0.03125, 0.125, 0.25, 0.5, 0.9};
    
    for (int i = 0; i < 5; i++){
		double ratio = ratios[i];
		for (size_t i = 0; i < bitmapsize; i++){
		    bitmap[i] = 0;
		}

		size_t appbitcount = (size_t)ceil(ratio * bitmapsize * 64);
		size_t ccount = 0;

		    while (ccount < appbitcount) {
		        int bit = rand() % (bitmapsize * 64);
		        uint64_t bef = bitmap[bit / 64];
		        uint64_t aft = bef | (UINT64_C(1) << (bit % 64));
		        if (bef != aft)
		            ccount++;
		        bitmap[bit / 64] = aft;
		    }

		size_t bitcount = bitmap_count(bitmap, bitmapsize);

		printf("bitmap density %3.2f  \n", bitcount / (bitmapsize * sizeof(uint64_t) * 8.0));

		BEST_TIME(bitmap_decode_naive(bitmap, bitmapsize, out), bitcount, , repeat,
		          bitcount, 1);
		BEST_TIME(bitmap_decode_ctz(bitmap, bitmapsize, out), bitcount, , repeat,
		          bitcount, 1);
        	BEST_TIME(bitmap_decode_avx2(bitmap, bitmapsize, out), bitcount, , repeat,
                         bitcount, 1);
    }
    
    free(bitmap);
    free(out);
    
    return 0;
}
