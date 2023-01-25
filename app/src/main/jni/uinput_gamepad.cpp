#include <linux/input.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "uinput_gamepad.h"
#include "android/log.h"
static const char *TAG="TEFIX";
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO,  TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, TAG, fmt, ##args)

/* sends a key event to the virtual device */
static void send_key_event(int fd, unsigned int keycode, int keyvalue,
                           unsigned int evtype) {
    struct input_event event;
    gettimeofday(&event.time, NULL);

    event.type = evtype;
    event.code = keycode;
    event.value = keyvalue;

    if (write(fd, &event, sizeof(event)) < 0) {
        printf("[uinput_gamepad] Simulate key error\n");
        LOGE("[uinput_gamepad] Simulate Key Error!\n");
    }

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(fd, &event, sizeof(event));
    if (write(fd, &event, sizeof(event)) < 0) {
        printf("[uinput_gamepad] Simulate key error\n");
        LOGE("[uinput_gamepad] Simulate Key Error!\n");
    }
}


int16_t uinput_mouse_open(UINP_GPAD_DEV* const gpad) {
    int16_t uinp_fd = -1;

    gpad->fd = open("/dev/uinput", O_WRONLY | O_NDELAY);
    if (gpad->fd <= 0) {
        printf("Unable to open /dev/uinput (running as root may help)\n");
        LOGE("Unable to open /dev/uinput!\n");
        return -1;
    }

    struct uinput_user_dev uinp;
    memset(&uinp, 0, sizeof(uinp));
    snprintf(uinp.name, sizeof(uinp.name), "TETRACKBALL");
    uinp.id.version = 4;
    uinp.id.bustype = BUS_USB;
    uinp.id.product = 0x0504;
    uinp.id.vendor = 0x1241;

    ioctl(gpad->fd, UI_SET_EVBIT, EV_KEY);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_RIGHT);

    ioctl(gpad->fd, UI_SET_EVBIT, EV_REL);
    ioctl(gpad->fd, UI_SET_RELBIT, ABS_X);
    ioctl(gpad->fd, UI_SET_RELBIT, ABS_Y);
    ioctl(gpad->fd, UI_SET_RELBIT, REL_WHEEL);
    ioctl(gpad->fd, UI_SET_RELBIT, REL_HWHEEL);
    ioctl(gpad->fd, UI_SET_PROPBIT, INPUT_PROP_POINTER);


    ioctl(gpad->fd, UI_SET_EVBIT, EV_SYN);

    write(gpad->fd, &uinp, sizeof(uinp));
    if (ioctl(gpad->fd, UI_DEV_CREATE)) {
        printf("[uinput_gamepad] Unable to create UINPUT device.");
        LOGE("[uinput-gamepad] Unable to create UINPUT device.\n");
        return -2;
    }

    uinp_fd = gpad->fd;

    return uinp_fd;
}


int16_t uinput_mouse_close(UINP_GPAD_DEV* const gpad) {
    ioctl(gpad->fd, UI_DEV_DESTROY);
    return close(gpad->fd);
}

/* sends a key event to the virtual device */
int16_t uinput_mouse_write(UINP_GPAD_DEV* const gpad, int dx, int dy) {
    struct input_event event;

    gettimeofday(&event.time, NULL);
    event.type = EV_REL;
    event.code = REL_X;
    event.value = dx;


    if (write(gpad->fd, &event, sizeof(event)) < 0) {
        printf("[uinput_mouse] Simulate key error\n");
        LOGE("Unable to open /dev/uinput!\n");
    }
    memset(&event,0x00,sizeof(event));
    gettimeofday(&event.time, NULL);
    event.type = EV_REL;
    event.code = REL_Y;
    event.value = dy;

    if (write(gpad->fd, &event, sizeof(event)) < 0) {
        printf("[uinput_mouse] Simulate key error\n");
        LOGE("Unable to open /dev/uinput!\n");
    }

    memset(&event,0x00,sizeof(event));
    gettimeofday(&event.time, NULL);

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    if (write(gpad->fd, &event, sizeof(event)) < 0) {
        printf("[uinput_mouse] Simulate key error\n");
        LOGE("Unable to open /dev/uinput!\n");
    }
    return 0;
}


/* Setup the uinput device */
int16_t uinput_gpad_open(UINP_GPAD_DEV* const gpad, unsigned char number) {
    int16_t uinp_fd = -1;

    gpad->fd = open("/dev/uinput", O_WRONLY | O_NDELAY);
    if (gpad->fd <= 0) {
        printf("Unable to open /dev/uinput (running as root may help)\n");
        LOGE("Unable to open /dev/uinput!\n");
        return -1;
    }

    struct uinput_user_dev uinp;
    memset(&uinp, 0, sizeof(uinp));
    snprintf(uinp.name, sizeof(uinp.name), "TeamEncoderGP #%i", number);
    uinp.id.version = 4;
    uinp.id.bustype = BUS_USB;
    uinp.id.product = 8449;
    uinp.id.vendor = 11720;

    // Setup the uinput device
    ioctl(gpad->fd, UI_SET_EVBIT, EV_KEY);
    ioctl(gpad->fd, UI_SET_EVBIT, EV_REL);

    // gamepad, buttons
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_A);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_B);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_C);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_X);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_Y);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_Z);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_TL);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_TR);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_BACK);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_GAMEPAD);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_SELECT);
    ioctl(gpad->fd, UI_SET_KEYBIT, BTN_START);
    ioctl(gpad->fd, UI_SET_KEYBIT, KEY_VOLUMEDOWN);
    ioctl(gpad->fd, UI_SET_KEYBIT, KEY_VOLUMEUP);


    uinp.absmin[ABS_HAT0X]  = -1;
    uinp.absmax[ABS_HAT0X]  = 1;
    uinp.absmin[ABS_HAT0Y]  = -1;
    uinp.absmax[ABS_HAT0Y]  = 1;

    ioctl(gpad->fd, UI_SET_EVBIT, EV_ABS);


    if(ioctl(gpad->fd, UI_SET_ABSBIT, ABS_HAT0X) < 0)
    {
        fprintf(stderr, "error on uinput ioctl (UI_SET_ABSBIT)\n");
        LOGE("error on uinput ioctl (UI_SET_ABSBIT)!\n");
    }

    ioctl(gpad->fd, UI_SET_ABSBIT, ABS_HAT0Y);

    /* Create input device into input sub-system */
    write(gpad->fd, &uinp, sizeof(uinp));
    if (ioctl(gpad->fd, UI_DEV_CREATE)) {
        printf("[uinput_gamepad] Unable to create UINPUT device.");
        LOGE("[uinput-gamepad] Unable to create UINPUT device.\n");
        return -2;
    }

    uinp_fd = gpad->fd;

    return uinp_fd;
}

int16_t uinput_gpad_close(UINP_GPAD_DEV* const gpad) {
    ioctl(gpad->fd, UI_DEV_DESTROY);
    return close(gpad->fd);
}

/* sends a key event to the virtual device */
int16_t uinput_gpad_write(UINP_GPAD_DEV* const gpad, uint16_t keycode,
                          int16_t keyvalue, uint16_t evtype) {
    struct input_event event;
    gettimeofday(&event.time, NULL);

    event.type = evtype;
    event.code = keycode;
    event.value = keyvalue;

    if (write(gpad->fd, &event, sizeof(event)) < 0) {
        printf("[uinput_gamepad] Simulate key error\n");
        LOGE("Unable to open /dev/uinput!\n");
    }

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    if (write(gpad->fd, &event, sizeof(event)) < 0) {
        printf("[uinput_gamepad] Simulate key error\n");
        LOGE("Unable to open /dev/uinput!\n");
    }
    return 0;
}

/* sleep between immediate successive gpad writes */
int16_t uinput_gpad_sleep() {
    usleep(50000);
    return 0;
}
