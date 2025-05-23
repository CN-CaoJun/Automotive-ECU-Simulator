
// #include <rtthread.h>
// #include "rtdevice.h"

// #define CAN_DEV_NAME "fdcan1" /* CAN 设备名称 */

// static struct rt_semaphore rx_sem; /* 用于接收消息的信号量 */
// static rt_device_t can_dev;        /* CAN 设备句柄 */

// static uint8_t isfdcan1open = 0;
// static uint8_t isfdcan2open = 0;
// static rt_uint8_t dlc_to_length(uint8_t dlc)
// {
//     static const rt_uint8_t dlc_to_len_table[16] = {
//         0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

//     if (dlc > 15)
//     {
//         return 0;
//     }

//     return dlc_to_len_table[dlc];
// }
// static rt_err_t can_rx_call(rt_device_t dev, rt_size_t size)
// {

//     rt_sem_release(&rx_sem);

//     return RT_EOK;
// }

// static void can_rx_thread(void *parameter)
// {
//     int i;
//     rt_err_t res;
//     struct rt_can_msg rxmsg = {0};

//     rt_device_set_rx_indicate(can_dev, can_rx_call);

// #ifdef RT_CAN_USING_HDR
//     struct rt_can_filter_item items[5] =
//         {
//             RT_CAN_FILTER_ITEM_INIT(0x100, 0, 0, 0, 0x700, RT_NULL, RT_NULL), /* std,match ID:0x100~0x1ff，hdr 为 - 1，设置默认过滤表 */
//             RT_CAN_FILTER_ITEM_INIT(0x300, 0, 0, 0, 0x700, RT_NULL, RT_NULL), /* std,match ID:0x300~0x3ff，hdr 为 - 1 */
//             RT_CAN_FILTER_ITEM_INIT(0x211, 0, 0, 0, 0x7ff, RT_NULL, RT_NULL), /* std,match ID:0x211，hdr 为 - 1 */
//             RT_CAN_FILTER_STD_INIT(0x486, RT_NULL, RT_NULL),                  /* std,match ID:0x486，hdr 为 - 1 */
//             {
//                 0x555,
//                 0,
//                 0,
//                 0,
//                 0x7ff,
//                 7,
//             } /* std,match ID:0x555，hdr 为 7，指定设置 7 号过滤表 */
//         };
//     struct rt_can_filter_config cfg = {5, 1, items};

//     res = rt_device_control(can_dev, RT_CAN_CMD_SET_FILTER, &cfg);
//     RT_ASSERT(res == RT_EOK);
// #endif

//     while (1)
//     {
//         rxmsg.hdr_index = -1;

//         rt_sem_take(&rx_sem, RT_WAITING_FOREVER);

//         rt_device_read(can_dev, 0, &rxmsg, sizeof(rxmsg));

//         rt_kprintf("ID-> %X ", rxmsg.id);
//         rt_kprintf("Payload ->");
//         for (i = 0; i < dlc_to_length(rxmsg.len); i++)
//         {
//             rt_kprintf("%02x", rxmsg.data[i]);
//         }

//         rt_kprintf("isfd-%d, isbrs-%d\n", rxmsg.fd_frame, rxmsg.brs);
//     }
// }

// int can_sample(int argc, char *argv[])
// {
//     struct rt_can_msg msg = {0};
//     rt_err_t res;
//     rt_size_t size;
//     rt_thread_t thread;
//     char can_name[RT_NAME_MAX];

//     if (argc == 2)
//     {
//         rt_strncpy(can_name, argv[1], RT_NAME_MAX);
//     }
//     else
//     {
//         rt_strncpy(can_name, CAN_DEV_NAME, RT_NAME_MAX);
//     }
//     can_dev = rt_device_find(can_name);
//     if (!can_dev)
//     {
//         rt_kprintf("find %s failed!\n", can_name);
//         return RT_ERROR;
//     }
//     else
//     {
//         rt_kprintf("find %s success!\n", can_name);
//     }

//     rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);

//     res = rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
//     RT_ASSERT(res == RT_EOK);
//     if (res == RT_EOK)
//     {
//         isfdcan1open = 1;
//     }
    
//     thread = rt_thread_create("can_rx", can_rx_thread, RT_NULL, 1024, 25, 10);
//     if (thread != RT_NULL)
//     {
//         rt_thread_startup(thread);
//     }
//     else
//     {
//         rt_kprintf("create can_rx thread failed!\n");
//     }

//     return res;
// }

// int can_test(int argc, char *argv[])
// {
//     struct rt_can_msg msg = {0};
//     rt_err_t res;
//     rt_size_t size;
//     rt_thread_t thread;
//     char can_name[RT_NAME_MAX];

//     if (argc == 2)
//     {
//         rt_strncpy(can_name, argv[1], RT_NAME_MAX);
//     }
//     else
//     {
//         rt_strncpy(can_name, CAN_DEV_NAME, RT_NAME_MAX);
//     }

//     can_dev = rt_device_find(can_name);
//     if (!can_dev)
//     {
//         rt_kprintf("find %s failed!\n", can_name);
//         return RT_ERROR;
//     }
//     else
//     {
//         rt_kprintf("find %s success!\n", can_name);
//     }

//     rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);

//     res = rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
//     RT_ASSERT(res == RT_EOK);

//     // thread = rt_thread_create("can_rx", can_rx_thread, RT_NULL, 1024, 25, 10);
//     // if (thread != RT_NULL)
//     // {
//     //     rt_thread_startup(thread);
//     // }
//     // else
//     // {
//     //     rt_kprintf("create can_rx thread failed!\n");
//     // }

//     msg.id = 0x78;
//     msg.ide = RT_CAN_STDID;
//     msg.rtr = RT_CAN_DTR;
//     msg.len = 8;

//     msg.data[0] = 0x00;
//     msg.data[1] = 0x11;
//     msg.data[2] = 0x22;
//     msg.data[3] = 0x33;
//     msg.data[4] = 0x44;
//     msg.data[5] = 0x55;
//     msg.data[6] = 0x66;
//     msg.data[7] = 0x77;

//     for (rt_uint8_t send_ind = 0; send_ind < 10; send_ind++)
//     {
//         rt_thread_mdelay(50);
//         msg.id = 0x78 + send_ind + 1;
//         msg.data[0] = msg.data[0] + 0x01;
//         msg.data[1] = msg.data[1] + 0x01;
//         msg.data[2] = msg.data[2] + 0x01;
//         msg.data[3] = msg.data[3] + 0x01;
//         msg.data[4] = msg.data[4] + 0x01;
//         msg.data[5] = msg.data[5] + 0x01;
//         msg.data[6] = msg.data[6] + 0x01;
//         msg.data[7] = msg.data[7] + 0x01;

//         size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
//         if (size == 0)
//         {
//             rt_kprintf("can dev write data failed!\n");
//         }
//     }

//     return res;
// }

// int can_send_one(int argc, char *argv[])
// {
//     struct rt_can_msg msg = {0};
//     rt_err_t res;
//     rt_size_t size;
//     rt_thread_t thread;
//     char can_name[RT_NAME_MAX];

//     if (argc == 2)
//     {
//         rt_strncpy(can_name, argv[1], RT_NAME_MAX);
//     }
//     else
//     {
//         rt_strncpy(can_name, CAN_DEV_NAME, RT_NAME_MAX);
//     }

//     can_dev = rt_device_find(can_name);
//     if (!can_dev)
//     {
//         rt_kprintf("find %s failed!\n", can_name);
//         return RT_ERROR;
//     }
//     else
//     {
//         rt_kprintf("find %s success!\n", can_name);
//     }

//     res = rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
//     RT_ASSERT(res == RT_EOK);

//     for (rt_uint8_t send_ind = 0; send_ind < 1; send_ind++)
//     {
//         msg.id = 0x222 + send_ind;
//         msg.ide = RT_CAN_STDID;
//         msg.rtr = RT_CAN_DTR;
//         msg.len = 8;
//         msg.data[0] = msg.data[0] + 0x01;
//         msg.data[1] = msg.data[1] + 0x01;
//         msg.data[2] = msg.data[2] + 0x01;
//         msg.data[3] = msg.data[3] + 0x01;
//         msg.data[4] = msg.data[4] + 0x01;
//         msg.data[5] = msg.data[5] + 0x01;
//         msg.data[6] = msg.data[6] + 0x01;
//         msg.data[7] = msg.data[7] + 0x01;

//         size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
//         if (size == 0)
//         {
//             rt_kprintf("can dev write data failed!\n");
//         }
//     }

//     res = rt_device_close(can_dev);
//     RT_ASSERT(res == RT_EOK);

//     return res;
// }

// int canfd_send_one(int argc, char *argv[])
// {
//     struct rt_can_msg msg = {0};
//     rt_err_t res;
//     rt_size_t size;
//     rt_thread_t thread;
//     char can_name[RT_NAME_MAX];

//     if (argc == 2)
//     {
//         rt_strncpy(can_name, argv[1], RT_NAME_MAX);
//     }
//     else
//     {
//         rt_strncpy(can_name, CAN_DEV_NAME, RT_NAME_MAX);
//     }

//     can_dev = rt_device_find(can_name);
//     if (!can_dev)
//     {
//         rt_kprintf("find %s failed!\n", can_name);
//         return RT_ERROR;
//     }
//     else
//     {
//         rt_kprintf("find %s success!\n", can_name);
//     }

//     if (isfdcan1open == 0)
//     {
//         res = rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
//         RT_ASSERT(res == RT_EOK);
//     }

//     msg.id = 0x222;
//     msg.ide = RT_CAN_STDID;
//     msg.rtr = RT_CAN_DTR;
//     msg.fd_frame = 1;
//     msg.len = 15;
//     msg.data[0] = msg.data[0] + 11;
//     msg.data[1] = msg.data[1] + 0x01;
//     msg.data[2] = msg.data[2] + 0x01;
//     msg.data[3] = msg.data[3] + 0x01;
//     msg.data[4] = msg.data[4] + 0x01;
//     msg.data[5] = msg.data[5] + 0x01;
//     msg.data[6] = msg.data[6] + 0x01;
//     msg.data[7] = msg.data[7] + 11;
//     msg.data[8] = msg.data[8] + 0x01;
//     msg.data[9] = msg.data[9] + 0x01;

//     size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
//     if (size == 0)
//     {
//         rt_kprintf("can dev write data failed!\n");
//     }
//     else
//     {
//         rt_kprintf("can dev write data success!\n");
//     }

//     res = rt_device_close(can_dev);
//     RT_ASSERT(res == RT_EOK);

//     return res;
// }

// static rt_uint8_t buf_gen[64] = {0};
// void generate_array(int length)
// {
//     if (length <= 0)
//     {
//         rt_kprintf("Length must be a positive integer.\n");
//         return;
//     }

//     if (length > 64)
//     {
//         length = 64;
//     }

//     for (size_t i = 0; i < length; i++)
//     {
//         buf_gen[i] = i;
//     }
// }

// int canfd_args_send(int argc, char *argv[])
// {
//     struct rt_can_msg msg = {0};
//     rt_err_t res;
//     rt_size_t size;
//     rt_thread_t thread;
//     char can_name[RT_NAME_MAX];
//     char *endptr;
//     long CANID;

//     if (argc == 5)
//     {
//         rt_strncpy(can_name, argv[1], RT_NAME_MAX);
//     }
//     else
//     {
//         rt_strncpy(can_name, CAN_DEV_NAME, RT_NAME_MAX);
//     }

//     can_dev = rt_device_find(can_name);
//     if (!can_dev)
//     {
//         rt_kprintf("find %s failed!\n", can_name);
//         return RT_ERROR;
//     }
//     else
//     {
//         rt_kprintf("find %s success!\n", can_name);
//     }

//     // detect canid
//     errno = 0;
//     CANID = strtol(argv[2], &endptr, 0);

//     int isfd = atoi(argv[3]);
//     int len = atoi(argv[4]);

//     res = rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);

//     RT_ASSERT(res == RT_EOK);

//     msg.id = CANID;
//     msg.ide = RT_CAN_STDID;
//     msg.rtr = RT_CAN_DTR;
//     msg.fd_frame = isfd;
//     msg.len = len;

//     generate_array(len);
//     rt_memcpy(msg.data, buf_gen, len);

//     size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
//     if (size == 0)
//     {
//         rt_kprintf("can dev write data failed!\n");
//     }
//     else
//     {
//         rt_kprintf("can dev write data success!\n");
//     }
//     rt_memset(buf_gen, 0x00, len);

//     res = rt_device_close(can_dev);
//     RT_ASSERT(res == RT_EOK);

//     return res;
// }

// MSH_CMD_EXPORT(can_test, can device sample);
// MSH_CMD_EXPORT(can_send_one, can device sample);
// MSH_CMD_EXPORT(canfd_send_one, can device sample);
// MSH_CMD_EXPORT(can_sample, can device sample);
// MSH_CMD_EXPORT(canfd_args_send, can device sample);
