#ifndef PHYTREX_EFUSE_H
#define PHYTREX_EFUSE_H

/* phytrex_encrypto_data	*/
/* initial efuse function	*/
void phytrex_efuse_init();

/* phytrex_encrypto_data														*/
/* input	- input data buffer													*/
/* output	- output data buffer												*/
/* len		- input data length													*/
/* key		- set crypto key buffer												*/
/* keylen	- set crypto key length												*/
/*			  Key lengths in the range 16 <= key_len <= 32 are given in bytes,	*/
/*			  those in the range 128 <= key_len <= 256 are given in bits		*/
void phytrex_encrypto_data(uint8_t *input,
                           uint8_t *output,
                           int len,
                           uint8_t *key,
                           int keylen);

/* phytrex_encrypto_data														*/
/* input	- input data buffer													*/
/* output	- output data buffer												*/
/* len		- input data length													*/
/* key		- set crypto key buffer												*/
/* keylen	- set crypto key length												*/
/*			  Key lengths in the range 16 <= key_len <= 32 are given in bytes,	*/
/*			  those in the range 128 <= key_len <= 256 are given in bits		*/
void phytrex_decrypto_data(uint8_t *input,
                           uint8_t *output,
                           int len,
                           uint8_t *key,
                           int keylen);

/* phytrex_efuse_write									*/
/* iBlock	- efuse block 0/1/2							*/
/* data		- data buffer								*/
/* len		- data buffer length (32 bytes/characters)	*/
int phytrex_efuse_write(int iBlock, uint8_t *data, int len);

/* phytrex_efuse_read									*/
/* iBlock	- efuse block 0/1/2							*/
/* data		- data buffer								*/
/* len		- data buffer length (32 bytes/characters)	*/
void phytrex_efuse_read(int iBlock, uint8_t *data, int len);

#endif