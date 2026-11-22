#include <stdint.h>
#include "assert.h"
#include "isotp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <rtthread.h> 

/* user implemented, print debug message */
void isotp_user_debug(const char* message, ...)
{   
    rt_kprintf("isotp_user_debug: %s\n", message);
}

/* user implemented, send can message. should return ISOTP_RET_OK when success.
*/
int  isotp_user_send_can(const uint32_t arbitration_id,
                         const uint8_t* data, const uint8_t DLC)
{
    rt_err_t res;

    res = rt_can_send(arbitration_id, data, DLC);
    if (res != RT_EOK)
    {
        return ISOTP_RET_ERROR;
    }
    return ISOTP_RET_OK;
}

/* user implemented, get millisecond */
uint32_t isotp_user_get_ms(void)
{   
    return rt_tick_get_millisecond();
}