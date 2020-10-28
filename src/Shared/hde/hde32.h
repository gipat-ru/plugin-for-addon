/*
 * Hacker Disassembler Engine 32
 * Copyright (c) 2006-2009, Vyacheslav Patkov.
 * All rights reserved.
 *
 * hde32.h: C/C++ header file
 *
 */

#pragma once

#include <stdint.h>

#define HDE_F_MODRM 0x00000001
#define HDE_F_SIB 0x00000002
#define HDE_F_IMM8 0x00000004
#define HDE_F_IMM16 0x00000008
#define HDE_F_IMM32 0x00000010
#define HDE_F_DISP8 0x00000020
#define HDE_F_DISP16 0x00000040
#define HDE_F_DISP32 0x00000080
#define HDE_F_RELATIVE 0x00000100
#define HDE_F_2IMM16 0x00000800
#define HDE_F_ERROR 0x00001000
#define HDE_F_ERROR_OPCODE 0x00002000
#define HDE_F_ERROR_LENGTH 0x00004000
#define HDE_F_ERROR_LOCK 0x00008000
#define HDE_F_ERROR_OPERAND 0x00010000
#define HDE_F_PREFIX_REPNZ 0x01000000
#define HDE_F_PREFIX_REPX 0x02000000
#define HDE_F_PREFIX_REP 0x03000000
#define HDE_F_PREFIX_66 0x04000000
#define HDE_F_PREFIX_67 0x08000000
#define HDE_F_PREFIX_LOCK 0x10000000
#define HDE_F_PREFIX_SEG 0x20000000
#define HDE_F_PREFIX_ANY 0x3f000000

#define HDE_PREFIX_SEGMENT_CS 0x2e
#define HDE_PREFIX_SEGMENT_SS 0x36
#define HDE_PREFIX_SEGMENT_DS 0x3e
#define HDE_PREFIX_SEGMENT_ES 0x26
#define HDE_PREFIX_SEGMENT_FS 0x64
#define HDE_PREFIX_SEGMENT_GS 0x65
#define HDE_PREFIX_LOCK 0xf0
#define HDE_PREFIX_REPNZ 0xf2
#define HDE_PREFIX_REPX 0xf3
#define HDE_PREFIX_OPERAND_SIZE 0x66
#define HDE_PREFIX_ADDRESS_SIZE 0x67

#pragma pack(push, 1)

typedef struct
{
    uint8_t len;
    uint8_t p_rep;
    uint8_t p_lock;
    uint8_t p_seg;
    uint8_t p_66;
    uint8_t p_67;
    uint8_t opcode;
    uint8_t opcode2;
    uint8_t modrm;
    uint8_t modrm_mod;
    uint8_t modrm_reg;
    uint8_t modrm_rm;
    uint8_t sib;
    uint8_t sib_scale;
    uint8_t sib_index;
    uint8_t sib_base;
    union
    {
        uint8_t imm8;
        uint16_t imm16;
        uint32_t imm32;
    } imm;
    union
    {
        uint8_t disp8;
        uint16_t disp16;
        uint32_t disp32;
    } disp;
    uint32_t flags;
} hde32s;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

unsigned int hde32_disasm(const void *code, hde32s *hs);

#ifdef __cplusplus
}
#endif
