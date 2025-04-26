#include "fdc.h"

#define STATUS_REGISTER_A                 0x3F0, // read-only
#define STATUS_REGISTER_B                 0x3F1, // read-only
#define DIGITAL_OUTPUT_REGISTER           0x3F2,
#define TAPE_DRIVE_REGISTER               0x3F3,
#define MAIN_STATUS_REGISTER              0x3F4, // read-only
#define DATARATE_SELECT_REGISTER          0x3F4, // write-only
#define DATA_FIFO                         0x3F5,
#define DIGITAL_INPUT_REGISTER            0x3F7, // read-only
#define CONFIGURATION_CONTROL_REGISTER    0x3F7  // write-only

void fdc_init() {
  
}

