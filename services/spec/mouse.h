// Autogenerated C header file for Mouse
#ifndef _JACDAC_SPEC_MOUSE_H
#define _JACDAC_SPEC_MOUSE_H 1

#define JD_MOUSE_SERVICE_CLASS 0x1885dc1c

// enum Button (uint16_t)
#define JD_MOUSE_BUTTON_RIGHT 0x1
#define JD_MOUSE_BUTTON_MIDDLE 0x4
#define JD_MOUSE_BUTTON_LEFT 0x2

// enum ButtonEvent (uint8_t)
#define JD_MOUSE_BUTTON_EVENT_UP 0x1
#define JD_MOUSE_BUTTON_EVENT_DOWN 0x2
#define JD_MOUSE_BUTTON_EVENT_CLICK 0x3
#define JD_MOUSE_BUTTON_EVENT_DOUBLE_CLICK 0x4

/**
 * Sets the up/down state of one or more buttons.
 * A ``Click`` is the same as ``Down`` followed by ``Up`` after 100ms.
 * A ``DoubleClick`` is two clicks with ``150ms`` gap between them (that is, ``100ms`` first click, ``150ms`` gap, ``100ms`` second click).
 */
#define JD_MOUSE_CMD_SET_BUTTON 0x80
typedef struct jd_mouse_set_button {
    uint16_t buttons;  // Button
    uint8_t event;  // ButtonEvent
} jd_mouse_set_button_t;


/**
 * Moves the mouse by the distance specified.
 * If the time is positive, it specifies how long to make the move.
 */
#define JD_MOUSE_CMD_MOVE 0x81
typedef struct jd_mouse_move {
    int16_t dx; // #
    int16_t dy; // #
    uint16_t time; // ms
} jd_mouse_move_t;


/**
 * Turns the wheel up or down. Positive if scrolling up.
 * If the time is positive, it specifies how long to make the move.
 */
#define JD_MOUSE_CMD_WHEEL 0x82
typedef struct jd_mouse_wheel {
    int16_t dy; // #
    uint16_t time; // ms
} jd_mouse_wheel_t;


#endif
