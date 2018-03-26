#ifndef _IBM_360_TYPES_
#define _IBM_360_TYPES_

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef signed int int32;

typedef struct OP_RX_T /*структ.буф.опер.форм.RX */
{
    uint8 OP_CODE; /*код операции            */
    uint8 R1_X2; /*R1 - первый операнд     */
//    short B2_D2; /*X2 - второй операнд     */
    int32 B2_D2;
    /*X2 - второй операнд     */
    /*B2 - баз.рег.2-го оп-да */
    /*D2 - смещен.относит.базы*/
} OP_RX;

typedef struct OP_RR_T /*структ.буф.опер.форм.RR */
{
    uint8 OP_CODE; /*код операции            */
    uint8 R1_R2; /*R1 - первый операнд     */
    /*R2 - второй операнд     */
} OP_RR;





typedef struct TSYM_T /*структ.строки табл.симв.*/
{
    uint8  name[8]; /*имя символа             */
    uint32 val; /*значение символа        */
    uint32 length; /*длина символа           */
    uint8 transfer_flag; /*признак перемещения     */
} TSYM;

typedef struct TMOP_T /*структ.стр.табл.маш.опер*/
{
    uint8 op_type;
    uint8 op_name[5]; /*мнемокод операции       */
    uint8 op_code; /*машинный код операции   */
    uint8 op_len; /*длина операции в байтах */
    int (*callback)(struct TMOP_T*); /*указатель на подпр.обраб*/
} TMOP;

typedef struct TBASR_T
{
    uint32 base_addr; /*                        */
    char activity_flag; /*                        */
} TBASR;

typedef struct ASM_MAPPING_T
{
    uint8 label[8]; /*поле метки              */
    uint8 PADDING1[1]; /*пробел-разделитель      */
    uint8 op_name[5]; /*поле операции           */
    uint8 PADDING2[1]; /*пробел-разделитель      */
    uint8 operand[12]; /*поле операнда           */
    uint8 PADDING3[1]; /*пробел разделитель      */
    uint8 COMMENT[52]; /*поле комментария        */
} ASM_MAPPING;

typedef union
{
    uint8 buffer[80]; /*буфер карты.исх.текста  */
    ASM_MAPPING structure; /*наложение шабл.на буфер */
} asm_mapping_u;

//void GEN_ESD_CARD(uint32 pr_length, uint32 r_addr, char* sym_name);
//void GEN_TXT_CARD(uint8 op_length, uint32 op_addr, uint8* op_body);
//void GEN_RLD_CARD(uint32 offset_addr);
//void GEN_END_CARD();


#endif // _IBM_360_TYPES_