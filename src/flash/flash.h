#ifndef _FLASH_H_
#define _FLASH_H_

#include "hardware/flash.h"

#define FLASH_TARGET_OFFSET (256 * 1024)

extern const uint8_t* flash_target_contents;

//Sector size is 16 times the FLASH_PAGE_SIZE
bool save_flash(uint8_t buffer[FLASH_PAGE_SIZE]);

#endif /* _FLASH_H_ */