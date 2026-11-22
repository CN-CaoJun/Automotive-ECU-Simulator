#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define LED_PWM_PIN  GET_PIN(B, 0)
#define LED_RUN_PIN  GET_PIN(E, 13)

static void led_write(rt_base_t pin, rt_bool_t on)
{
    rt_pin_write(pin, on ? PIN_LOW : PIN_HIGH);
}

static int led_init(void)
{
    rt_pin_mode(LED_PWM_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED_RUN_PIN, PIN_MODE_OUTPUT);
    led_write(LED_PWM_PIN, RT_FALSE);
    led_write(LED_RUN_PIN, RT_FALSE);
    return RT_EOK;
}

static int led(int argc, char **argv)
{
    if (argc != 2)
    {
        rt_kprintf("usage: led on|off\n");
        return -RT_EINVAL;
    }

    if (rt_strcmp(argv[1], "on") == 0)
    {
        led_write(LED_PWM_PIN, RT_TRUE);
        led_write(LED_RUN_PIN, RT_TRUE);
    }
    else if (rt_strcmp(argv[1], "off") == 0)
    {
        led_write(LED_PWM_PIN, RT_FALSE);
        led_write(LED_RUN_PIN, RT_FALSE);
    }
    else
    {
        rt_kprintf("usage: led on|off\n");
        return -RT_EINVAL;
    }

    return RT_EOK;
}

INIT_DEVICE_EXPORT(led_init);
MSH_CMD_EXPORT(led, control PB0/PE13 LEDs);
