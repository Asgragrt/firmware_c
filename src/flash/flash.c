#include "flash.h"
#include "hardware/flash.h"
#include "pico/multicore.h"

const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);

//Sector size is 16 times the FLASH_PAGE_SIZE

bool save_flash(uint8_t buffer[FLASH_PAGE_SIZE]){
    multicore_lockout_start_blocking();
    uint32_t interrupts = save_and_disable_interrupts();

    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);

    restore_interrupts(interrupts);
    multicore_lockout_end_blocking();

    bool mismatch = false;
    
    for(uint16_t i = 0; i < FLASH_PAGE_SIZE; i++){
        if ( buffer[i] != flash_target_contents[i]){
            mismatch = true;
            break;
        }
    }
    

    return mismatch;
}