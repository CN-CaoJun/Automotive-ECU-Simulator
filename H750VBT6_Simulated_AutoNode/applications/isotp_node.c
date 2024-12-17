#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "stdint.h"
#include "time.h"

#include "isotp.h"

#include <rtthread.h>
#include "rtdevice.h"

#define LOG_TAG              "isotp"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

#define RET_OK 0
#define RET_NOK 1
#define RET_ERR 2
#define RET_NO_DATA 3

#define FRAMESDUMAX 4095
/* Alloc IsoTpLink statically in RAM */
static IsoTpLink g_link;

/* Alloc send and receive buffer statically in RAM */
static uint8_t g_isotpRecvBuf[FRAMESDUMAX];
static uint8_t g_isotpSendBuf[FRAMESDUMAX];

#define CAN_DEV_NAME "fdcan1"

static struct rt_semaphore tprx_sem;
static rt_device_t can_dev;

static uint8_t isfdcan1open = 0;
static uint8_t isfdcan2open = 0;

static uint8_t sendbuf[100] = {0x00};

#define DEVICEOPEN 1
#define DEVICECLOSE 0
static rt_uint8_t dlc_to_length(uint8_t dlc)
{
    static const rt_uint8_t dlc_to_len_table[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

    if (dlc > 15)
    {
        return 0;
    }

    return dlc_to_len_table[dlc];
}
uint8_t ret_isfdcan1open()
{
    return isfdcan1open;
}
static rt_err_t can_rx_call(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&tprx_sem);
    return RT_EOK;
}

uint8_t payload[4095] = {0};
uint16_t payload_size = 0;
uint16_t out_size = 0;
static void cantp_rx_thread(void *parameter)
{
    int i;
    int ret;
    rt_err_t res;
    struct rt_can_msg rxmsg = {0};

    rt_device_set_rx_indicate(can_dev, can_rx_call);

    while (1)
    {
        rxmsg.hdr_index = -1;

        rt_sem_take(&tprx_sem, RT_WAITING_NO);

        if (isfdcan1open == DEVICEOPEN)
        {
            size_t read_size = rt_device_read(can_dev, 0, &rxmsg, sizeof(rxmsg));
            if (read_size == sizeof(rxmsg))
            {
                LOG_D("RX ID-> %X ", rxmsg.id);
                for (i = 0; i < dlc_to_length(rxmsg.len); i++)
                {
                    LOG_D("%02x", rxmsg.data[i]);
                }
                if (0x731 == rxmsg.id)
                {
                    isotp_on_can_message(&g_link, rxmsg.data, dlc_to_length(rxmsg.len));
                }
            }

            ret = isotp_receive(&g_link, payload, payload_size, &out_size);
            if (ISOTP_RET_OK == ret)
            {
                LOG_D("Received %d bytes !!! ",out_size);
                uint16_t send_size = ((payload[0] << 8) | payload[1]);
                LOG_D("Will Send  %d bytes !!! ",send_size);
                ret = isotp_send(&g_link, sendbuf, send_size);
            }
            else
            {
                // LOG_D("Received not complete!!!");
            }
        
            isotp_poll(&g_link);

        }
    }
}

int rt_can_send(const uint32_t arbitration_id,
                const uint8_t *data, const uint8_t DLC)
{
    struct rt_can_msg msg = {0};
    rt_err_t res;
    rt_size_t size;

    if (isfdcan1open == DEVICEOPEN)
    {
        msg.id = arbitration_id;
        msg.ide = RT_CAN_STDID;
        msg.rtr = RT_CAN_DTR;
        msg.len = 8;

        memcpy(msg.data, data, DLC);

        size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
        if (size == 0)
        {
            LOG_D("can dev write data failed!");
            res = RT_ERROR;
        }
        else
        {
            LOG_D("TX ID-> %X ", msg.id);
            // LOG_D("Payload ->");
            for (int i = 0; i < dlc_to_length(msg.len); i++)
            {
                LOG_D("%02x", msg.data[i]);
            }
            res = RT_EOK;
        }
    }

    return res;
}

int cantp_tx_thread(void)
{

    while (1)
    {
        if (isfdcan1open == DEVICEOPEN)
        {
            // isotp_poll(&g_link);
            // rt_thread_mdelay(5);
        }
    }

    return 0;
}

int rtcan_node_start()
{
    struct rt_can_msg msg = {0};
    rt_err_t res;
    rt_size_t size;
    rt_thread_t thread;
    char can_name[RT_NAME_MAX];

    isotp_init_link(&g_link, 0x739,
                    g_isotpSendBuf, sizeof(g_isotpSendBuf),
                    g_isotpRecvBuf, sizeof(g_isotpRecvBuf));

    for (int i = 0; i < 100; i++)
    {
        sendbuf[i] = i + 1;
    }

    rt_strncpy(can_name, CAN_DEV_NAME, RT_NAME_MAX);

    can_dev = rt_device_find(can_name);

    if (!can_dev)
    {
        LOG_D("find %s failed!", can_name);
        return RT_ERROR;
    }
    else
    {
        LOG_D("find %s success!", can_name);
    }

    rt_sem_init(&tprx_sem, "tprx_sem", 0, RT_IPC_FLAG_FIFO);
    res = rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    RT_ASSERT(res == RT_EOK);
    if (res == RT_EOK)
    {
        isfdcan1open = DEVICEOPEN;
    }

    thread = rt_thread_create("cantp_rx", cantp_rx_thread, RT_NULL, 1024, 25, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
        LOG_D("create cantp_rx thread success!");
    }
    else
    {
        LOG_D("create cantp_rx thread failed!");
    }

    return res;
}

MSH_CMD_EXPORT(rtcan_node_start, can device sample);