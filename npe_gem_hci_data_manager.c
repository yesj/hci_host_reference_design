#include <pthread.h> 
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "npe_gem_hci_data_manager.h"
#include "npe_error_code.h"

static data_timeout_func_t m_process_tick_cb;

static void* npe_transmit_thread(void *vargp)
{
    while(1)
    {
        sleep(1);
        

        if(m_process_tick_cb)
            m_process_tick_cb();
    }
    return(NULL);

}


uint32_t npe_gem_hci_data_manager_init(data_timeout_func_t process_tick_cb)
{
    uint32_t err_code = NPE_GEM_RESPONSE_OK;
    pthread_t thread_id; 
    if(process_tick_cb)
    {
        m_process_tick_cb = process_tick_cb;
    } 


    int res = pthread_create(&thread_id, NULL, npe_transmit_thread, NULL);
    assert(res == 0);
        

    return(err_code);
}
