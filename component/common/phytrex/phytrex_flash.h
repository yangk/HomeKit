#ifndef PHYTREX_FLASH_H
#define PHYTREX_FLASH_H

/* phytrex_FlashDataRead				*/
/* read data							*/
/* uint8 *buf : data					*/
/* int len : data length in byte		*/
void phytrex_FlashDataRead(uint8_t *buf, int len);

/* phytrex_FlashDataWrite				*/
/* write data							*/
/* uint8 *buf : data					*/
/* int len : data length in byte		*/
void phytrex_FlashDataWrite(uint8_t *buf, int len);

/* phytrex_dump_flash				*/
/* dump flash data to screen		*/
/* page - page count				*/
void phytrex_dump_flash(int page);

void phytrex_FlashLogRead(uint32_t offset, void *data, uint32_t len);
void phytrex_FlashLogWrite(uint32_t offset, void *data, uint32_t len);
void phytrex_FlashUserRead(void *data, int len);
void phytrex_FlashUserWrite(void *data, int len);

#endif