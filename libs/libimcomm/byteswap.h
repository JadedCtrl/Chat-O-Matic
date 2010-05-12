#define BYTE_SWAP_16(x) (((x & 0xFF00) >> 8) | \
			 ((x & 0x00FF) << 8))

#define BYTE_SWAP_32(x) (((x & 0xFF000000) >> 24) | \
			 ((x & 0x00FF0000) >> 8) | \
			 ((x & 0x0000FF00) << 8) | \
			 ((x & 0x000000FF) << 24))
