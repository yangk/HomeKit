#ifndef PHYTREX_EXTENSION_H
#define PHYTREX_EXTENSION_H

/* phytrex_FlashDataRead		*/
/* read data			*/
/* uint8 *buf : data				*/
/* int len : data length in byte		*/
void phytrex_FlashDataRead(uint8_t *buf, int len);

/* phytrex_FlashDataWrite		*/
/* write data			*/
/* uint8 *buf : data				*/
/* int len : data length in byte		*/
void phytrex_FlashDataWrite(uint8_t *buf, int len);

/* phytrex_dump_flash				*/
/* dump flash data to screen		*/
/* dumpSize - 0/1/2					*/
/*		      0 : 1 MB				*/
/*		      1 : 2 MB				*/
/*		      2 : 4 MB				*/
void phytrex_dump_flash(int dumpSize);

#endif