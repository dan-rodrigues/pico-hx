// Mostly coped from the stdio_usb example

#include "usb_uart.h"

#include "tusb.h"

#include "pico/stdio/driver.h"
#include "pico/binary_info.h"
#include "pico/mutex.h"
#include "pico/time.h"

#define PICO_STDIO_USB_STDOUT_TIMEOUT_US 500000
#define PICO_STDIO_USB_DEFAULT_CRLF PICO_STDIO_DEFAULT_CRLF

static mutex_t stdio_usb_mutex;

static void stdio_usb_out_chars(const char *buf, int length);
static int stdio_usb_in_chars(char *buf, int length);

stdio_driver_t stdio_usb = {
    .out_chars = stdio_usb_out_chars,
    .in_chars = stdio_usb_in_chars,
#if PICO_STDIO_ENABLE_CRLF_SUPPORT
    .crlf_enabled = PICO_STDIO_USB_DEFAULT_CRLF
#endif
};

void stdio_usb_init() {
     mutex_init(&stdio_usb_mutex);
     stdio_set_driver_enabled(&stdio_usb, true);
}

static void stdio_usb_out_chars(const char *buf, int length) {
    static uint64_t last_avail_time;
    uint32_t owner;
    if (!mutex_try_enter(&stdio_usb_mutex, &owner)) {
        if (owner == get_core_num()) return; // would deadlock otherwise
        mutex_enter_blocking(&stdio_usb_mutex);
    }
    if (tud_cdc_connected()) {
        for (int i = 0; i < length;) {
            int n = length - i;
            int avail = tud_cdc_write_available();
            if (n > avail) n = avail;
            if (n) {
                int n2 = tud_cdc_write(buf + i, n);
                tud_task();
                tud_cdc_write_flush();
                i += n2;
                last_avail_time = time_us_64();
            } else {
                tud_task();
                tud_cdc_write_flush();
                if (!tud_cdc_connected() ||
                    (!tud_cdc_write_available() && time_us_64() > last_avail_time + PICO_STDIO_USB_STDOUT_TIMEOUT_US)) {
                    break;
                }
            }
        }
    } else {
        // reset our timeout
        last_avail_time = 0;
    }
    mutex_exit(&stdio_usb_mutex);
}

static int stdio_usb_in_chars(char *buf, int length) {
    // (Unsupported for now)
    return 0;
}
