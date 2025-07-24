#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <usb.h>
#include <time.h>

/* LCD2USB USB IDs */
#define LCD2USB_VID  0x0403
#define LCD2USB_PID  0xc630

#define LCD_CTRL_0         (1<<3)
#define LCD_CTRL_1         (1<<4)
#define LCD_BOTH           (LCD_CTRL_0 | LCD_CTRL_1)

#define LCD_CMD            (1<<5)
#define LCD_DATA           (2<<5)

usb_dev_handle *handle = NULL;

int lcd_send(int request, int value, int index) {
    return usb_control_msg(handle, USB_TYPE_VENDOR, request, value, index, NULL, 0, 1000);
}

void lcd_enqueue_cmd(int type, unsigned char val) {
    int request = type | 0;  // only 1 byte
    int value = val;
    int index = 0;
    lcd_send(request, value, index);
}

void lcd_command(unsigned char ctrl, unsigned char cmd) {
    lcd_enqueue_cmd(LCD_CMD | ctrl, cmd);
}

void lcd_write(const char *str, int ctrl) {
    while (*str) {
        lcd_enqueue_cmd(LCD_DATA | ctrl, *str++);
    }
}

void lcd_clear() {
    lcd_command(LCD_BOTH, 0x01);  // Clear
    usleep(2000);
}

void lcd_home() {
    lcd_command(LCD_BOTH, 0x02);  // Home
    usleep(2000);
}

void lcd_set_cursor_line2() {
    lcd_command(LCD_BOTH, 0xC0);  // Line 2 on 16x2
}

float get_cpu_usage() {
    static long prev_total = 0, prev_idle = 0;
    long user, nice, system, idle, iowait, irq, softirq, steal;
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return 0;
    fscanf(fp, "cpu  %ld %ld %ld %ld %ld %ld %ld %ld",
           &user, &nice, &system, &idle,
           &iowait, &irq, &softirq, &steal);
    fclose(fp);

    long total = user + nice + system + idle + iowait + irq + softirq + steal;
    long total_diff = total - prev_total;
    long idle_diff = idle - prev_idle;

    prev_total = total;
    prev_idle = idle;

    if (total_diff == 0) return 0;
    return 100.0f * (total_diff - idle_diff) / total_diff;
}

float get_ram_usage() {
    long total, available;
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return 0;
    fscanf(fp, "MemTotal: %ld kB\nMemFree: %*d kB\nMemAvailable: %ld kB", &total, &available);
    fclose(fp);
    return 100.0f * (total - available) / total;
}

int get_cpu_temp() {
    int temp_milli;
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (!fp) return 0;
    fscanf(fp, "%d", &temp_milli);
    fclose(fp);
    return temp_milli / 1000;
}

int get_uptime_minutes() {
    double uptime;
    FILE *fp = fopen("/proc/uptime", "r");
    if (!fp) return 0;
    fscanf(fp, "%lf", &uptime);
    fclose(fp);
    return (int)(uptime / 60);
}

int main() {
    struct usb_bus *bus;
    struct usb_device *dev;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    for (bus = usb_get_busses(); bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if ((dev->descriptor.idVendor == LCD2USB_VID) &&
                (dev->descriptor.idProduct == LCD2USB_PID)) {
                handle = usb_open(dev);
                break;
            }
        }
        if (handle) break;
    }

    if (!handle) {
        fprintf(stderr, "Could not find LCD2USB device.\n");
        return 1;
    }

    while (1) {
        float cpu = get_cpu_usage();
        float ram = get_ram_usage();
        int temp = get_cpu_temp();
        int uptime_min = get_uptime_minutes();

        char line1[17], line2[17];
        snprintf(line1, sizeof(line1), "CPU:%4.1f%% T:%2dC", cpu, temp);
        snprintf(line2, sizeof(line2), "RAM:%4.1f%% U:%dm", ram, uptime_min / 60);

        lcd_clear();
        lcd_home();
        lcd_write(line1, LCD_CTRL_0);
        lcd_set_cursor_line2();
        lcd_write(line2, LCD_CTRL_0);

        sleep(1);
    }

    usb_close(handle);
    return 0;
}
