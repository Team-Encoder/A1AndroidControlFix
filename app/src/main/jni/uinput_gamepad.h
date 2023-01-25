#ifndef UINPUT_GAMEPAD_H_
#define UINPUT_GAMEPAD_H_

#include <stdint.h>

typedef struct {
    int fd;
    int16_t state;
} UINP_GPAD_DEV;

int16_t uinput_gpad_open(UINP_GPAD_DEV* const gpad, unsigned char number);
int16_t uinput_gpad_close(UINP_GPAD_DEV* const gpad);
int16_t uinput_gpad_write(UINP_GPAD_DEV* const gpad, uint16_t keycode,
                          int16_t keyvalue, uint16_t evtype);
int16_t uinput_gpad_sleep();
int16_t uinput_mouse_write(UINP_GPAD_DEV* const gpad, int dx, int dy);
int16_t uinput_mouse_open(UINP_GPAD_DEV* const gpad);
int16_t uinput_mouse_close(UINP_GPAD_DEV* const gpad);


#endif /* UINPUT_GAMEPAD_H_ */
