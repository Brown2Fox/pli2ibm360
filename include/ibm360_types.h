#ifndef _IBM_360_TYPES_
#define _IBM_360_TYPES_

//typedef struct ESD_T
//{
//    unsigned char PADDING1; /*место для кода 0x02       */
//    unsigned char TYPE[3]; /*поле типа об'ектн.карты    */
//    unsigned char PADDING2[10]; /*пробелы               */
//    unsigned char IDNUM[2]; /*внутр.ид-р имени прогр.   */
//    unsigned char SYMBOL[8]; /*имя программы            */
//    unsigned char SYMTYP; /*код типа ESD-имени          */
//    unsigned char OADR[3]; /*относит.адрес программы    */
//    unsigned char PADDING3; /*пробелы                   */
//    unsigned char DLINA[3]; /*длина программы           */
//    unsigned char PADDING4[48]; /*пробелы               */
//} ESD;

typedef struct STR_BUF_ESD /*структ.буфера карты ESD */
{
    unsigned char PADDING1; /*место для кода 0x02     */
    unsigned char MAP_TYPE[3]; /*поле типа об'ектн.карты */
    unsigned char PADDING2[10]; /*пробелы               */
    unsigned char ID_NUM[2]; /*внутр.ид-р имени прогр. */
    unsigned char SYM_NAME[8]; /*имя программы           */
    unsigned char SYM_TYPE; /*код типа ESD-имени      */
    unsigned char R_ADDR[3]; /*относит.адрес программы */
    unsigned char PADDING3; /*пробелы                 */
    unsigned char LENGTH[3]; /*длина программы         */
    unsigned char PADDING4[40]; /*пробелы                 */
    unsigned char ID_FIELD[8]; /*идентификационное поле  */
} ESD;

//typedef struct TXT_T
//{
//    unsigned char PADDING1; /*место для кода 0x02       */
//    unsigned char TYPE[3]; /*поле типа об'ектн.карты    */
//    unsigned char PADDING2; /*пробел                    */
//    unsigned char ADOP[3]; /*относит.адрес опреации     */
//    unsigned char PADDING3[2]; /*пробелы                */
//    unsigned char DLNOP[2]; /*длина операции            */
//    unsigned char PADDING4[4]; /*пробелы                */
//    unsigned char OPERND[56]; /*тело операции           */
//    unsigned char PADDING5[8]; /*идентификационное поле */
//} TXT;

typedef struct STR_BUF_TXT /*структ.буфера карты txt_map */
{
    unsigned char PADDING1; /*место для кода 0x02     */
    unsigned char MAP_TYPE[3]; /*поле типа об'ектн.карты */
    unsigned char PADDING2; /*пробел                  */
    unsigned char OP_ADDR[3]; /*относит.адрес опреации  */
    unsigned char PADDING3[2]; /*пробелы                 */
    unsigned char OP_LENGTH[2]; /*длина операции          */
    unsigned char PADDING4[2]; /*пробелы                 */
    unsigned char ID_NUM[2]; /*внутренний идент.прогр. */
    unsigned char OP_BODY[56]; /*тело операции           */
    unsigned char ID_FIELD[8]; /*идентификационное поле  */
} TXT;

typedef struct RLD_T
{
    unsigned char PADDING1; /*место для кода 0x02        */
    unsigned char MAP_TYPE[3]; /*поле типа об'ектн.карты     */
    unsigned char PADDING2; /*пробел                     */
    unsigned char ID_NUM[2]; /*внутр.ид-р имени прогр.    */
    unsigned char PADDING3[3]; /*пробелы                 */
    unsigned char SIGN[2]; /*знак операции               */
    unsigned char PADDING4[4]; /*пробелы                 */
    unsigned char OFFSET_ADDR[3]; /*адрес                   */
    unsigned char PADDING5[53];
    unsigned char ID_FIELD[8]; /*идентификационное поле */
} RLD;

typedef struct STR_BUF_END /*структ.буфера карты end_map */
{
    unsigned char PADDING1; /*место для кода 0x02     */
    unsigned char MAP_TYPE[3]; /*поле типа об'ектн.карты */
    unsigned char PADDING2[68]; /*пробелы                 */
    unsigned char ID_FIELD[8]; /*идентификационное поле  */
} END;

#endif // _IBM_360_TYPES_