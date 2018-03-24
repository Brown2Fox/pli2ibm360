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


typedef struct ESD_T  /*структ.буфера карты ESD */
{
    uint8 PADDING1; /*место для кода 0x02     */
    uint8 CARD_TYPE[3]; /*поле типа об'ектн.карты */
    uint8 PADDING2[10]; /*пробелы               */
    uint8 ID_NUM[2]; /*внутр.ид-р имени прогр. */
    uint8 SYM_NAME[8]; /*имя программы           */
    uint8 SYM_TYPE; /*код типа ESD-имени      */
    uint8 R_ADDR[3]; /*относит.адрес программы */
    uint8 PADDING3; /*пробелы                 */
    uint8 PR_LENGTH[3]; /*длина программы         */
    uint8 PADDING4[40]; /*пробелы                 */
    uint8 ID_FIELD[8]; /*идентификационное поле  */
} ESD_;


typedef struct TXT_T  /*структ.буфера карты txt_map */
{
    uint8 PADDING1; /*место для кода 0x02     */
    uint8 CARD_TYPE[3]; /*поле типа об'ектн.карты */
    uint8 PADDING2; /*пробел                  */
    uint8 OP_ADDR[3]; /*относит.адрес опреации  */
    uint8 PADDING3[2]; /*пробелы                 */
    uint8 OP_LENGTH[2]; /*длина операции          */
    uint8 PADDING4[2]; /*пробелы                 */
    uint8 ID_NUM[2]; /*внутренний идент.прогр. */
    uint8 OP_BODY[56]; /*тело операции           */
    uint8 ID_FIELD[8]; /*идентификационное поле  */
} TXT_;

typedef struct RLD_T
{
    uint8 PADDING1; /*место для кода 0x02        */
    uint8 CARD_TYPE[3]; /*поле типа об'ектн.карты     */
    uint8 PADDING2; /*пробел                     */
    uint8 ID_NUM[2]; /*внутр.ид-р имени прогр.    */
    uint8 PADDING3[3]; /*пробелы                 */
    uint8 SIGN[2]; /*знак операции               */
    uint8 PADDING4[4]; /*пробелы                 */
    uint8 OFFSET_ADDR[3]; /*адрес                   */
    uint8 PADDING5[53];
    uint8 ID_FIELD[8]; /*идентификационное поле */
} RLD_;

typedef struct END_T /*структ.буфера карты end_map */
{
    uint8 PADDING1; /*место для кода 0x02     */
    uint8 CARD_TYPE[3]; /*поле типа об'ектн.карты */
    uint8 PADDING2[68]; /*пробелы                 */
    uint8 ID_FIELD[8]; /*идентификационное поле  */
} END_;

typedef struct TSYM_T /*структ.строки табл.симв.*/
{
    uint8  sym_name[8]; /*имя символа             */
    uint32 sym_addr; /*значение символа        */
    uint32 sym_length; /*длина символа           */
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

#endif // _IBM_360_TYPES_