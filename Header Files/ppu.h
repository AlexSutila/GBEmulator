#ifndef PPU_H_
#define PPU_H_

#include <Windows.h> // Bad practice, I know. It works right now so idc

/*
	Bit 6 - LYC=LY STAT Interrupt source         (1=Enable) (Read/Write)
	Bit 5 - Mode 2 OAM STAT Interrupt source     (1=Enable) (Read/Write)
	Bit 4 - Mode 1 VBlank STAT Interrupt source  (1=Enable) (Read/Write)
	Bit 3 - Mode 0 HBlank STAT Interrupt source  (1=Enable) (Read/Write)
	Bit 2 - LYC=LY Flag                          (0=Different, 1=Equal) (Read Only)
	Bit 1-0 - Mode Flag                          (Mode 0-3, see below) (Read Only)
	          0: HBlank
	          1: VBlank
	          2: Searching OAM
	          3: Transferring Data to LCD Controller
	Mode 2  2_____2_____2_____2_____2_____2___________________2____ 80 dots
	Mode 3  _33____33____33____33____33____33__________________3___ 168 - 291
	Mode 0  ___000___000___000___000___000___000________________000 85 - 208
	Mode 1  ____________________________________11111111111111_____ 
*/

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef signed short int16_t;

#define SCANLINE_LENGTH 456
#define SCANLINE_COUNT 154

// Screen definitions
#define v_pixelSize  4
#define v_VRES 144
#define v_HRES 160
#define v_WIDTH (v_HRES * v_pixelSize)
#define v_HEIGHT (v_VRES * v_pixelSize)

// Tile data definitions
#define n_TILE_WIDTH 8
#define n_TILE_HEIGHT 8
#define n_TILE_COUNT 384

// Definitions/Addresses
#define index_LCDC		0xFF40
#define index_STAT      0xFF41
#define index_SCY       0xFF42
#define index_SCX       0xFF43
#define index_LY        0xFF44
#define index_LYC       0xFF45
#define index_BGP       0xFF47
#define index_OBP0      0xFF48
#define index_OBP1      0xFF49
#define index_WY        0xFF4A
#define index_WX        0xFF4B

// LCDC bits
#define LCDC_ENABLE_MASK	0b10000000 // LCD and PPU enable
#define LCDC_WINMAP_MASK	0b01000000 // Window tile map area
#define LCDC_WINEN_MASK		0b00100000 // Window enable
#define LCDC_WINBG_MASK		0b00010000 // BG and Window tile data area
#define LCDC_BGMAP_MASK		0b00001000 // BG tile map area
#define LCDC_OBJS_MASK		0b00000100 // OBJ size
#define LCDC_OBJEN_MASK		0b00000010 // OBJ enable
#define LCDC_BGWINP_MASK	0b00000001 // BG and Window enable/priority

// Tile Struct
struct tileStruct 
{
    uint8_t bytes[8][2];
};

// Basic PPU functions
void init_ppu(struct PPU* ppu);
void ppu_step(struct GB* gb, int cycles);
void free_ppu(struct PPU* ppu);

// For real time synchronized video output
void draw_to_screen(struct GB* gb, HWND window, HDC hdc);

struct PPU 
{
	// Bitmap stuff
	struct bitmapStruct* bitmapBMI;
	void* bitmap;
	uint8_t* bitmap_PTR;

	// Actual PPU stuff
	int enabled, scanline;
	uint8_t mode, nextMode;

	// Internal scanline counter for window
	uint8_t win_LY;

	// Used to sync to video
	int frameIncomplete;
};

#endif
