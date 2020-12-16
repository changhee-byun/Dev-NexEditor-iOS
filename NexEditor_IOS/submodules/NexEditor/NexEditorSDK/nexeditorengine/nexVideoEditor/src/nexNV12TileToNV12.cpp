#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexNV12TileToNV12.h"

// #define USING_NEON

#define NV12TILE_BLOCK_WIDTH		64
#define NV12TILE_BLOCK_HEIGHT		32
#define NV12TILE_BLOCK_SIZE		(NV12TILE_BLOCK_WIDTH * NV12TILE_BLOCK_HEIGHT)
#define NV12TILE_BLOCK_GROUP_SIZE	(NV12TILE_BLOCK_SIZE*4)

int nv12TileGetTiledMemBlockNum(int bx, int by, int nbx, int nby)
{
    int base; // Number of memory block of the first block in row by
    int offs; // Offset from base

    if((by & 1)==0) {
        base = by * nbx;
        if((nby & 1) && (by == (nby - 1))) {
            // Last row when nby is odd
            offs = bx;
        }
        else {
            offs = bx + ((bx + 2) & ~3);
        }
    }
    else {
        base = (by & (~1))*nbx + 2;
        offs = bx + (bx & ~3);
    }

    return base + offs;
}

void nv12TileToNV12(unsigned char **dstPtr, const unsigned char *blockUV, const unsigned char *blockY, int blockWidth, int dstSkip, int ySize) 
{
	unsigned char *dest_ptr = *dstPtr;
	int uvOffset = 0;
	int i = 0;
	for(i = 0; i < blockWidth; i++) 
	{
		signed y= (signed)blockY[i];
		signed u = (signed)blockUV[i & ~1] ;
		signed v = (signed)blockUV[(i & ~1)+1];

		dest_ptr[i] =  y;
		dest_ptr[ySize+uvOffset] = u;
		dest_ptr[ySize+uvOffset+1] =v;
		//LOGI("%d, %d", ySize, uvOffset);
		uvOffset+= 2;
	}

	dest_ptr += dstSkip;
	*dstPtr = dest_ptr;
}

void nv12TileTraverseBlock(unsigned char **dstPtr, const unsigned char *blockY, const unsigned char *blockUV, int blockWidth, int blockHeight, int dstSkip, int ySize) 
{
	int row = 0;
	const unsigned char *block_UV = 0;
	for(row = 0; row < blockHeight; row++) 
	{
		if(row & 1) 
		{
			// Only Luma, the converter can use the previous values if needed
			nv12TileToNV12(dstPtr, block_UV, blockY, blockWidth, dstSkip, ySize);
			blockUV += NV12TILE_BLOCK_WIDTH;
		}
		else 
		{
			block_UV = blockUV;
			nv12TileToNV12(dstPtr, block_UV, blockY, blockWidth, dstSkip, ySize);
		}
		blockY += NV12TILE_BLOCK_WIDTH;
	}
}

void convertNV12TiledToNV12( int width, int height, int wpitch, int hpitch, void *srcBits, void *dstBits) 
{
	int bx, by, ix;
	unsigned char *src_y;
	unsigned char *src_uv;

	unsigned char *base_ptr = (unsigned char *)dstBits;
	unsigned char *dst_ptr = 0;

	// wpitch = wpitch == 896 ? 832 : wpitch;

	int framesize = width*height;

	// Absolute number of columns of blocks in the Luma and Chroma spaces
	int abx = (wpitch - 1) / NV12TILE_BLOCK_WIDTH + 1;

	// Number of columns of blocks in the Luma and Chroma spaces rounded to
	// the next multiple of 2
	int nbx = (abx + 1) & ~1;

	// Number of rows of blocks in the Luma space
	int nby_y = (hpitch - 1) / NV12TILE_BLOCK_HEIGHT + 1;

	// Number of rows of blocks in the Chroma space
	int nby_uv = (hpitch / 2 - 1) / NV12TILE_BLOCK_HEIGHT + 1;

	// Calculate the size of the Luma section
	int size_y = nbx * nby_y * NV12TILE_BLOCK_SIZE;

	int width_offset = NV12TILE_BLOCK_WIDTH - (wpitch - width); // -32
	int height_offset = NV12TILE_BLOCK_HEIGHT - (hpitch - height);
	
	// int width_offset = wpitch - width; // -32
	// int height_offset = hpitch - height;

	int abx_min = abx - 1;
	int nby_min = nby_y - 1;

	if((size_y % NV12TILE_BLOCK_GROUP_SIZE) != 0) 
	{
		size_y = (((size_y-1) / NV12TILE_BLOCK_GROUP_SIZE)+1) * NV12TILE_BLOCK_GROUP_SIZE;
	}
	// Pointers to the start of the Luma and Chroma spaces
	src_y   = (unsigned char*)srcBits;
	src_uv = src_y + size_y;

	// Iterate
	for(by = 0; by < nby_y; by++) 
	{
		for(bx = 0; bx < abx; bx++) 
		{
			unsigned char *block_uv = 0;
			// Address of Luma data for this block
			int block_address = (nv12TileGetTiledMemBlockNum(bx, by, nbx, nby_y) * NV12TILE_BLOCK_SIZE);
			unsigned char *block_y = src_y + block_address;

			// Address of Chroma data for this block
			// since we have half the data for Chroma the same row number is used
			// for two consecutive Luma rows, but we have to offset the base pointer
			// by half a block for odd rows

			block_address = (nv12TileGetTiledMemBlockNum(bx, by/2, nbx, nby_uv) * NV12TILE_BLOCK_SIZE );

			block_uv = src_uv + block_address + ((by & 1) ? NV12TILE_BLOCK_SIZE/2 : 0);
			
			dst_ptr = base_ptr + (bx*NV12TILE_BLOCK_WIDTH) + (by*NV12TILE_BLOCK_HEIGHT*width);

			for(ix = 0; ix<NV12TILE_BLOCK_HEIGHT;ix++)
			{
				if( by == nby_min )
				{
					if( ix >= height_offset )
						break;
				}

				if( bx == abx_min )
				{
					memcpy(dst_ptr, block_y, width_offset);
					dst_ptr += width;
					block_y += NV12TILE_BLOCK_WIDTH;
				}
				else
				{
#ifdef USING_NEON				
					uint8x16_t val1, val2, val3, val4;

					val1 = vld1q_u8(block_y);
					val2 = vld1q_u8(block_y + 16);
					val3 = vld1q_u8(block_y + 32);
					val4 = vld1q_u8(block_y + 48);

					vst1q_u8(dst_ptr, val1);
					vst1q_u8(dst_ptr + 16, val2);
					vst1q_u8(dst_ptr + 32, val3);
					vst1q_u8(dst_ptr + 48, val4);
#else
					memcpy(dst_ptr, block_y, NV12TILE_BLOCK_WIDTH);
#endif
					dst_ptr += width;
					block_y += NV12TILE_BLOCK_WIDTH;
				}
			}

			dst_ptr = base_ptr+framesize + (bx*NV12TILE_BLOCK_WIDTH) + (by*NV12TILE_BLOCK_HEIGHT*width) / 2;
			
			for(ix = 0; ix<NV12TILE_BLOCK_HEIGHT/2;ix++)
			{
				if( by == nby_min )
				{
					if( ix >= height_offset / 2)
						break;
				}

				if( bx == abx_min )
				{
					memcpy(dst_ptr, block_uv, width_offset);
					dst_ptr += width;
					block_uv += NV12TILE_BLOCK_WIDTH;
				}
				else
				{
#ifdef USING_NEON				
					uint8x16_t val1, val2, val3, val4;

					val1 = vld1q_u8(block_uv);
					val2 = vld1q_u8(block_uv + 16);
					val3 = vld1q_u8(block_uv + 32);
					val4 = vld1q_u8(block_uv + 48);

					vst1q_u8(dst_ptr, val1);
					vst1q_u8(dst_ptr + 16, val2);
					vst1q_u8(dst_ptr + 32, val3);
					vst1q_u8(dst_ptr + 48, val4);
#else
					memcpy(dst_ptr, block_uv, NV12TILE_BLOCK_WIDTH);
#endif
					dst_ptr += width;
					block_uv += NV12TILE_BLOCK_WIDTH;
				}
			}
		}
	}
}	

