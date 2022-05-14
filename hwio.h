#ifndef IO_WR_H_
#define IO_WR_H_

#include "types.h"

// Reads
uint8_t JOYP_RB(struct GB* gb, uint8_t cycles);                  // $FF00
uint8_t DIV_RB(struct GB* gb, uint8_t cycles);                   // $FF04
uint8_t TIMA_RB(struct GB* gb, uint8_t cycles);                  // $FF05
uint8_t TMA_RB(struct GB* gb, uint8_t cycles);                   // $FF06
uint8_t TAC_RB(struct GB* gb, uint8_t cycles);                   // $FF07
uint8_t IF_RB(struct GB* gb, uint8_t cycles);                    // $FF0F
uint8_t LCDC_RB(struct GB* gb, uint8_t cycles);                  // $FF40
uint8_t STAT_RB(struct GB* gb, uint8_t cycles);                  // $FF41
uint8_t SCY_RB(struct GB* gb, uint8_t cycles);                   // $FF42
uint8_t SCX_RB(struct GB* gb, uint8_t cycles);                   // $FF43
uint8_t LY_RB(struct GB* gb, uint8_t cycles);                    // $FF44
uint8_t LYC_RB(struct GB* gb, uint8_t cycles);                   // $FF45
uint8_t DMA_RB(struct GB* gb, uint8_t cycles);                   // $FF46
uint8_t BGP_RB(struct GB* gb, uint8_t cycles);                   // $FF47
uint8_t OBP0_RB(struct GB* gb, uint8_t cycles);                  // $FF48
uint8_t OBP1_RB(struct GB* gb, uint8_t cycles);                  // $FF49
uint8_t WY_RB(struct GB* gb, uint8_t cycles);                    // $FF4A
uint8_t WX_RB(struct GB* gb, uint8_t cycles);                    // $FF4B

// Writes
void JOYP_WB(struct GB* gb, uint8_t val, uint8_t cycles);        // $FF00
void SC_WB(struct GB* gb, uint8_t val, uint8_t cycles);          // $FF02
void DIV_WB(struct GB* gb, uint8_t val, uint8_t cycles);         // $FF04
void TIMA_WB(struct GB* gb, uint8_t val, uint8_t cycles);        // $FF05
void TMA_WB(struct GB* gb, uint8_t val, uint8_t cycles);         // $FF06
void TAC_WB(struct GB* gb, uint8_t val, uint8_t cycles);         // $FF07
void LCDC_WB(struct GB* gb, uint8_t val, uint8_t cycles);        // $FF40
void STAT_WB(struct GB* gb, uint8_t val, uint8_t cycles);        // $FF41
void SCY_WB(struct GB* gb, uint8_t val, uint8_t cycles);         // $FF42
void SCX_WB(struct GB* gb, uint8_t val, uint8_t cycles);         // $FF43
void LY_WB(struct GB* gb, uint8_t val, uint8_t cycles);          // $FF44
void LYC_WB(struct GB* gb, uint8_t val, uint8_t cycles);         // $FF45
void DMA_WB(struct GB* gb, uint8_t val, uint8_t cycles);         // $FF46
void BGP_WB(struct GB* gb, uint8_t val, uint8_t cycles);         // $FF47
void OBP0_WB(struct GB* gb, uint8_t val, uint8_t cycles);        // $FF48
void OBP1_WB(struct GB* gb, uint8_t val, uint8_t cycles);        // $FF49
void WY_WB(struct GB* gb, uint8_t val, uint8_t cycles);          // $FF4A
void WX_WB(struct GB* gb, uint8_t val, uint8_t cycles);          // $FF4B

#endif