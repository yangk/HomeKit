#ifndef PHYTREX_USER_H
#define PHYTREX_USER_H

void phytrex_FlashUserRead(uint32_t offset, void *data, uint32_t len);
void phytrex_FlashUserWrite(uint32_t offset, void *data, uint32_t len);

#endif