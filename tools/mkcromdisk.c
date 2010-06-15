/*-
 * Copyright (c) 2010, David Given
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <zlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

typedef struct
{
	uLongf length;
	int offset;
	Bytef* data;
}
Block;

Block* blocks;
FILE* infp;
FILE* outfp;
int fileBytes;
int fileBlocks;

#define BLOCKSIZE 512

static void writebyte(unsigned char b)
{
	fwrite(&b, 1, 1, outfp);
}

static void writeshort(unsigned short s)
{
	unsigned char hi = (s >> 8) & 0xff;
	unsigned char lo = (s) & 0xff;

	writebyte(lo);
	writebyte(hi);
}

static void writeint(unsigned int s)
{
	unsigned short hi = (s >> 16) & 0xffff;
	unsigned short lo = (s) & 0xffff;

	writeshort(lo);
	writeshort(hi);
}

int doDeflate(Bytef* dest, uLongf* destLen,
		const Bytef* source, uLong sourceLen)
{
    z_stream stream;
    int err;

    stream.next_in = (Bytef*)source;
    stream.avail_in = (uInt)sourceLen;
    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen)
    	return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit2(&stream, 9, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    if (err != Z_OK)
    	return err;

    err = deflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END)
    {
        deflateEnd(&stream);
        return err == Z_OK ? Z_BUF_ERROR : err;
    }
    *destLen = stream.total_out;

    err = deflateEnd(&stream);
    return err;
}

int main(int argc, const char* argv[])
{
	int i;

	if (argc != 3)
	{
		fprintf(stderr, "Syntax error: mkcromdisk <infile> <outfile>\n");
		exit(1);
	}

	infp = fopen(argv[1], "rb");
	if (!infp)
	{
		fprintf(stderr, "Unable to open input file: %s\n",
				strerror(errno));
		exit(1);
	}

	outfp = fopen(argv[2], "wb");
	if (!outfp)
	{
		fprintf(stderr, "Unable to open input file: %s\n",
				strerror(errno));
		exit(1);
	}

	fseek(infp, 0, SEEK_END);
	fileBytes = ftell(infp);
	fseek(infp, 0, SEEK_SET);
	fileBlocks = (fileBytes + (BLOCKSIZE-1)) / BLOCKSIZE;

	fprintf(stderr, "Input file length: %d bytes, %d blocks\n", fileBytes,
			fileBlocks);
	blocks = calloc(fileBlocks, sizeof(Block));

	for (i = 0; i < fileBlocks; i++)
	{
		Bytef buffer[BLOCKSIZE];
		int sourcelen;
		int e;

		memset(buffer, 0, BLOCKSIZE);
		fseek(infp, i*BLOCKSIZE, SEEK_SET);
		sourcelen = fread(buffer, 1, BLOCKSIZE, infp);
		if (sourcelen == -1)
		{
			fprintf(stderr, "I/O error reading file: %s\n",
					strerror(errno));
			exit(1);
		}

		blocks[i].data = calloc(BLOCKSIZE, 1);
		blocks[i].length = BLOCKSIZE;
		e = doDeflate(blocks[i].data, &blocks[i].length,
				buffer, sourcelen);
		if (e != Z_OK)
		{
			fprintf(stderr, "Compression error\n");
			exit(1);
		}
	}

	{
		int offset = fileBlocks * 4 + 2;
		writeshort(fileBlocks);
		for (i = 0; i < fileBlocks; i++)
		{
			unsigned int j;

			blocks[i].offset = offset;

			offset += blocks[i].length;
			j = (blocks[i].length + 1) & ~1;
			writebyte(j >> 1);

			j = blocks[i].offset;
			writebyte((j >> 0) & 0xff);
			writebyte((j >> 8) & 0xff);
			writebyte((j >> 16) & 0xff);
		}

		for (i = 0; i < fileBlocks; i++)
			fwrite(blocks[i].data, 1, blocks[i].length, outfp);

		fprintf(stderr, "Output file length: %ld bytes (compression ratio %.2f)\n",
				ftell(outfp), (double)ftell(outfp) / fileBytes);
	}


	return 0;
}
