#ifndef _OBJS_H_
#define _OBJS_H_

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef signed short int16_t;

// Definitions/Addresses
#define index_DMA 0x46

// Bit masks for sprite attributes
#define ATTRIBS_OVER_MASK	0b10000000
#define ATTRIBS_YFLIP_MASK	0b01000000
#define ATTRIBS_XFLIP_MASK	0b00100000
#define ATTRIBS_PALNUM_MASK	0b00010000

struct sprite 
{
	uint8_t yPos, xPos, tileIndex, attribs;
};

void draw_Sprite(struct GB* gb, uint8_t* bitmapPtr, struct sprite* curSprite);
int search_OAM(struct GB* gb, struct sprite* selectedSprites, uint8_t curr_LY);

#endif // _OBJS_H_