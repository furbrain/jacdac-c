// Autogenerated C header file for Common registers and commands
#ifndef _JACDAC_SPEC_BASE_H
#define _JACDAC_SPEC_BASE_H 1

#define JD_BASE_SERVICE_CLASS 0x1ffffff1

/**
 * No args. Enumeration data for control service; service-specific advertisement data otherwise.
 * Control broadcasts it automatically every 500ms, but other service have to be queried to provide it.
 */
#define JD_BASE_CMD_ANNOUNCE 0x0
// Report: No args

/**
 * No args. Registers number `N` is fetched by issuing command `0x1000 | N`.
 * The report format is the same as the format of the register.
 */
#define JD_BASE_CMD_GET_REGISTER 0x1000
// Report: No args

/**
 * No args. Registers number `N` is set by issuing command `0x2000 | N`, with the format
 * the same as the format of the register.
 */
#define JD_BASE_CMD_SET_REGISTER 0x2000

/** Event from sensor or a broadcast service. */
#define JD_BASE_CMD_EVENT 0x1
typedef struct jd_base_event_report {
    uint32_t event_id;
    uint32_t event_argument;
} jd_base_event_report_t;


/** No args. Request to calibrate a sensor. The report indicates the calibration is done. */
#define JD_BASE_CMD_CALIBRATE 0x2
// Report: No args

/** No args. Request human-readable description of service. */
#define JD_BASE_CMD_DESCRIPTION 0x3
// Report: Argument: text string (bytes)

/** Read-write uint32_t. This is either binary on/off (0 or non-zero), or can be gradual (eg. brightness of an RGB LED strip). */
#define JD_BASE_REG_INTENSITY 0x1

/** Read-write int32_t. The primary value of actuator (eg. servo pulse length, or motor duty cycle). */
#define JD_BASE_REG_VALUE 0x2

/** Read-write mA uint16_t. Limit the power drawn by the service, in mA. */
#define JD_BASE_REG_MAX_POWER 0x7

/**
 * Read-write uint8_t. Asks device to stream a given number of samples
 * (clients will typically write `255` to this register every second or so, while streaming is required).
 */
#define JD_BASE_REG_STREAM_SAMPLES 0x3

/** Read-write ms uint32_t. Period between packets of data when streaming in milliseconds. */
#define JD_BASE_REG_STREAMING_INTERVAL 0x4

/** Read-only int32_t. Read-only value of the sensor, also reported in streaming. */
#define JD_BASE_REG_READING 0x101

/** Read-write int32_t. Thresholds for event generation for event generation for analog sensors. */
#define JD_BASE_REG_LOW_THRESHOLD 0x5

/** Read-write int32_t. Thresholds for event generation for event generation for analog sensors. */
#define JD_BASE_REG_HIGH_THRESHOLD 0x6

#endif
