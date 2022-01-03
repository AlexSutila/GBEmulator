#ifndef IO_WR_H_
#define IO_WR_H_

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef signed short int16_t;

// Reads
uint8_t DIV_RB(struct GB* gb, uint8_t cycles);					// $FF04
uint8_t TIMA_RB(struct GB* gb, uint8_t cycles);					// $FF05
uint8_t STAT_RB(struct GB* gb, uint8_t cycles);					// $FF41
uint8_t LY_RB(struct GB* gb, uint8_t cycles);					// $FF44

// Writes
void JOYP_WB(struct GB* gb, uint8_t val, uint8_t cycles);			// $FF00
void SC_WB(struct GB* gb, uint8_t val, uint8_t cycles);             // $FF02
void DIV_WB(struct GB* gb, uint8_t val, uint8_t cycles);			// $FF04
void TIMA_WB(struct GB* gb, uint8_t val, uint8_t cycles);			// $FF05
void TMA_WB(struct GB* gb, uint8_t val, uint8_t cycles);			// $FF06
void TAC_WB(struct GB* gb, uint8_t val, uint8_t cycles);			// $FF07
void LCDC_WB(struct GB* gb, uint8_t val, uint8_t cycles);			// $FF40
void STAT_WB(struct GB* gb, uint8_t val, uint8_t cycles);			// $FF41
void backgroundY_WB(struct GB* gb, uint8_t val, uint8_t cycles);	// $FF42
void backgroundX_WB(struct GB* gb, uint8_t val, uint8_t cycles);	// $FF43
void LY_WB(struct GB* gb, uint8_t val, uint8_t cycles);				// &FF44
void LYC_WB(struct GB* gb, uint8_t val, uint8_t cycles);			// $FF45
void DMA_WB(struct GB* gb, uint8_t val, uint8_t cycles);			// %FF46
void windowY_WB(struct GB* gb, uint8_t val, uint8_t cycles);		// $FF4A
void windowX_WB(struct GB* gb, uint8_t val, uint8_t cycles);		// $FF4B

#endif