#ifndef __NPE_GEM_HCI_DATA_MANAGER__
#define __NPE_GEM_HCI_DATA_MANAGER__

#include "stdint.h"

typedef void (*data_timeout_func_t)(void);  

uint32_t npe_gem_hci_data_manager_init(data_timeout_func_t process_tick_cb);


#endif