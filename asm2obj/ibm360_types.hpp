#ifndef _IBM_360_TYPES_
#define _IBM_360_TYPES_



typedef struct
{
    uint8_t  name[8];
    uint32_t val;
    uint32_t length;
    uint8_t transfer_flag;
} TSYM;

typedef struct
{
    uint32_t base_addr;
    char activity_flag;
} TBASR;

typedef struct
{
    uint8_t label[8];
    uint8_t padding1[1];
    uint8_t op_name[5];
    uint8_t padding2[1];
    uint8_t operand[12];
    uint8_t padding3[1];
    uint8_t comment[52];
} ASM_MAPPING;

typedef union
{
    uint8_t buffer[80];
    ASM_MAPPING structure;
} asm_mapping_u;


#endif // _IBM_360_TYPES_