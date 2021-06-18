#ifndef CONSTANTS_H
#define CONSTANTS_H

//#define DISABLE_GRAPHICS

#define GB_SCREEN_WIDTH  160
#define GB_SCREEN_HEIGHT 144

// Jump conditions
#define PARAM_CC_NZ 100
#define PARAM_CC_Z  101
#define PARAM_CC_NC 102
#define PARAM_CC_C  103

// Special register addresses
#define REG_JOYP 0xFF00
#define REG_DIV  0xFF04
#define REG_TIMA 0xFF05
#define REG_TMA  0xFF06
#define REG_TAC  0xFF07
#define REG_NR10 0xFF10
#define REG_NR11 0xFF11
#define REG_NR12 0xFF12
#define REG_NR14 0xFF14
#define REG_NR21 0xFF16
#define REG_NR22 0xFF17
#define REG_NR24 0xFF19
#define REG_NR30 0xFF1A
#define REG_NR31 0xFF1B
#define REG_NR32 0xFF1C
#define REG_NR34 0xFF1E
#define REG_NR41 0xFF20
#define REG_NR42 0xFF21
#define REG_NR43 0xFF22
#define REG_NR44 0xFF23
#define REG_NR50 0xFF24
#define REG_NR51 0xFF25
#define REG_NR52 0xFF26
#define REG_LCDC 0xFF40
#define REG_STAT 0xFF41
#define REG_SCY  0xFF42
#define REG_SCX  0xFF43
#define REG_LY   0xFF44
#define REG_LYC  0xFF45
#define REG_DMA  0xFF46
#define REG_BGP  0xFF47
#define REG_OBP0 0xFF48
#define REG_OBP1 0xFF49
#define REG_WY   0xFF4A
#define REG_WX   0xFF4B
#define REG_IF   0xFF0F
#define REG_IE   0xFFFF

// Memory offsets
#define OFFSET_ROMBANK0 0x0000
#define OFFSET_ROMBANKN 0x4000
#define OFFSET_VIDEORAM 0x8000
#define OFFSET_EXTRAM   0xA000
#define OFFSET_WORKRAMBANK0 0xC000
#define OFFSET_WORKRAMBANK1 0xD000
#define OFFSET_ECHORAM  0xE000
#define OFFSET_SPRITEATTRIBUTETABLE 0xFE00
#define OFFSET_UNUSABLE 0xFEA0
#define OFFSET_IOREGISTERS 0xFF00
#define OFFSET_HIGHRAM  0xFF80

#endif