#include "Machine_Code.h"

//x86
#define MOV_TYP 0x89
#define PTR_MOV 0x48
#define MEM_MOV 0x8B
#define FNC_RET 0xC3
#define FAR_CAL 0xFF
#define MEM_LOD 0x89
#define BYT_ADR 0x45
#define WRD_OFF 0x40
#define PTR_ADR 0x4D
#define WID_ADR 0x6D
#define ESP_BYTE 0xC6
#define ESP_WORD 0xC7
#define PSH_INT_B 0x6A
#define PSH_INT_W 0x68

//Stack
#define STK_POP 0x83
#define STK_PP4 0x81
#define SUB_ESP 0xEC
#define ADD_ESP 0xC4

//Math
#define MEM_ADD 0x01
#define REG_ADD 0x03
#define REG_MUL_L 0x0F
#define REG_MUL_H 0xAF
#define MEM_SUB 0x29
#define REG_SUB 0x2B
#define REG_IMUL 0xF7

//Register ops
#define REG_OR 0x08
#define REG_AND 0x21
#define LEA_EAX 0x24
#define REG_XOR 0x33
#define REG_INC 0x40
#define REG_LEA 0x44
#define REG_DEC 0x48
#define REG_PSH 0x50
#define PSH_EDX 0x52
#define PSH_EBP 0x55
#define POP_EDX 0x5A
#define POP_EBP 0x5D
#define REG_XCH 0x87
#define REG_ADR 0x8D
#define EAX_XCH 0x90
#define OPR_EAX 0xC0
#define REG_LOD 0xB8
#define FAR_ECX 0xD1
#define ESP_EBP 0xE5
#define EBP_ESP 0xEC

//Jump Codes
#define CMP_BYTE 0x83
#define CMP_WORD 0x81
#define CMP_MEM 0x7D
#define CODE_JMP 0xEB
#define CODE_JE 0x74
#define CODE_JNE 0x75
#define CODE_JLE 0x7E
#define CODE_JL 0x7C
#define CODE_JG 0x7F

//x87
#define FPU_LOAD_ADDR 0x05
#define FPU_ADR 0x1C
#define FPU_LOAD_VAR 0x45
#define FPU_STORE 0x55
#define FPU_PUSH 0x5D
#define FPU_ADD 0xC0
#define FPU_MUL 0xC8
#define FPU_SQRT 0xC8
#define FPU_XCHNG 0xC9
#define FPU_FLOAT_OP 0xD9
#define FPU_INT_OP 0xDB
#define FPU_80P_OP 0xDC
#define FPU_DOUBLE_OP 0xDD
#define FPU_MATH 0xDE
#define FPU_INVERT 0xE0
#define FPU_ABS 0xE1
#define FPU_SUB 0xE8
#define FPU_PUSH_ONE 0xE8
#define FPU_PUSH_PI 0xEB
#define FPU_PUSH_ZERO 0xEE
#define FPU_DIV 0xF8
#define FPU_ROOT 0xFA
#define FPU_ROUND 0xFC
#define FPU_SIN 0xFE
#define FPU_COS 0xFF