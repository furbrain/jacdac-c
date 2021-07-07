// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "jd_protocol.h"

// Enabling logging can cause delays and dropped packets!
// #define LOG JD_LOG
#define LOG JD_NOLOG

#define JD_STATUS_RX_ACTIVE 0x01
#define JD_STATUS_TX_ACTIVE 0x02
#define JD_STATUS_TX_QUEUED 0x04

static jd_frame_t _rxBuffer[2];
static jd_frame_t *rxFrame = &_rxBuffer[0];
static void set_tick_timer(uint8_t statusClear);
static volatile uint8_t status;

static jd_frame_t *txFrame;
static uint64_t nextAnnounce;
static uint8_t txPending;

static jd_diagnostics_t jd_diagnostics;

jd_diagnostics_t *jd_get_diagnostics(void) {
    jd_diagnostics.bus_state = 0; // TODO?
    return &jd_diagnostics;
}

int jd_is_running(void) {
    return nextAnnounce != 0;
}

int jd_is_busy(void) {
    return status != 0;
}

static void tx_done(void) {
#ifdef JD_DEBUG_MODE
    jd_debug_signal_write(0);
#endif
    set_tick_timer(JD_STATUS_TX_ACTIVE);
}

void jd_tx_completed(int errCode) {
    LOG("tx done: %d", errCode);
    jd_tx_frame_sent(txFrame);
    txFrame = NULL;
    tx_done();
}

static void tick(void) {
    set_tick_timer(0);
}

static void flush_tx_queue(void) {
    LOG("flush %d", status);
    target_disable_irq();
    if (status & (JD_STATUS_RX_ACTIVE | JD_STATUS_TX_ACTIVE)) {
        target_enable_irq();
        return;
    }
    status |= JD_STATUS_TX_ACTIVE;
    target_enable_irq();

    txPending = 0;
    if (!txFrame) {
        txFrame = jd_tx_get_frame();
        if (!txFrame) {
            tx_done();
            return;
        }
    }

#ifdef JD_DEBUG_MODE
    jd_debug_signal_write(1);
#endif

    if (uart_start_tx(txFrame, JD_FRAME_SIZE(txFrame)) < 0) {
        // ERROR("race on TX");
        jd_diagnostics.bus_lo_error++;
        tx_done();
        txPending = 1;
        return;
    }

    set_tick_timer(0);
}

static void set_tick_timer(uint8_t statusClear) {
    target_disable_irq();
    if (statusClear) {
        // LOG("st %d @%d", statusClear, status);
        status &= ~statusClear;
    }
    if ((status & JD_STATUS_RX_ACTIVE) == 0) {
        if (txPending && !(status & JD_STATUS_TX_ACTIVE)) {
            // the JD_WR_OVERHEAD value should be such, that the time from pulse1() above
            // to beginning of low-pulse generated by the current device is exactly 150us
            // (when the line below is uncommented)
            // tim_set_timer(150 - JD_WR_OVERHEAD, flush_tx_queue);
            status |= JD_STATUS_TX_QUEUED;
            tim_set_timer(jd_random_around(150) - JD_WR_OVERHEAD, flush_tx_queue);
        } else {
            status &= ~JD_STATUS_TX_QUEUED;
            tim_set_timer(10000, tick);
        }
    }
    target_enable_irq();
}

static void rx_timeout(void) {
    target_disable_irq();
    jd_diagnostics.bus_timeout_error++;
    ERROR("RX t/o");
    uart_disable();
#ifdef JD_DEBUG_MODE
    jd_debug_signal_read(0);
#endif
    set_tick_timer(JD_STATUS_RX_ACTIVE);
    target_enable_irq();
#ifdef JD_DEBUG_MODE
    jd_debug_signal_error(1);
    jd_debug_signal_error(0);
#endif
}

static void setup_rx_timeout(void) {
    // It's possible this only gets executed after the entire reception process has finished.
    // In that case, we don't want to set any additional timers.
    if (status & JD_STATUS_RX_ACTIVE) {
        uart_flush_rx();
        uint32_t *p = (uint32_t *)rxFrame;
        if (p[0] == 0 && p[1] == 0) {
            rx_timeout(); // didn't get any data after lo-pulse
        } else {
            // got the size - set timeout for whole packet
            tim_set_timer(JD_FRAME_SIZE(rxFrame) * 12 + 60, rx_timeout);
        }
    }
}

void jd_line_falling() {
    LOG("line fall");

#ifdef JD_DEBUG_MODE
    jd_debug_signal_read(1);
#endif

    // target_disable_irq();
    // no need to disable IRQ - we're at the highest IRQ level
    if (status & JD_STATUS_RX_ACTIVE)
        jd_panic();
    status |= JD_STATUS_RX_ACTIVE;

    // 1us faster than memset() on SAMD21
    uint32_t *p = (uint32_t *)rxFrame;
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    p[3] = 0;

    // otherwise we can enable RX in the middle of LO pulse
    if (uart_wait_high() < 0) {
        // line didn't get high in 1ms or so - bail out
        rx_timeout();
        return;
    }
    // pulse1();
    // target_wait_us(2);

    uart_start_rx(rxFrame, sizeof(*rxFrame));
    // log_pin_set(1, 0);

    // 200us max delay according to spec, +50us to get the first 4 bytes of data
    tim_set_timer(250, setup_rx_timeout);

    // target_enable_irq();
}

void jd_rx_completed(int dataLeft) {
    LOG("rx cmpl");
    jd_frame_t *frame = rxFrame;

#ifdef JD_DEBUG_MODE
    jd_debug_signal_read(0);
#endif

    set_tick_timer(JD_STATUS_RX_ACTIVE);

    if (dataLeft < 0) {
        ERROR("rx err: %d", dataLeft);
        jd_diagnostics.bus_uart_error++;
        return;
    }

    uint32_t txSize = sizeof(*frame) - dataLeft;
    uint32_t declaredSize = JD_FRAME_SIZE(frame);
    if (txSize < declaredSize) {
        ERROR("short frm");
        jd_diagnostics.bus_uart_error++;
        return;
    }
    uint16_t crc = jd_crc16((uint8_t *)frame + 2, declaredSize - 2);
    if (crc != frame->crc) {
        ERROR("crc err");
        jd_diagnostics.bus_uart_error++;
        return;
    }

    if (frame->flags & JD_FRAME_FLAG_VNEXT) {
        jd_diagnostics.packets_dropped++;
        return;
    }

    jd_diagnostics.packets_received++;

    // only swap frames just before we're about to process
    // no reason to swap frames that are dropped above
    if (rxFrame == &_rxBuffer[0])
        rxFrame = &_rxBuffer[1];
    else
        rxFrame = &_rxBuffer[0];

    // pulse1();
    int err = jd_rx_frame_received(frame);

    if (err)
        jd_diagnostics.packets_dropped++;
}

void jd_packet_ready(void) {
    target_disable_irq();
    txPending = 1;
    if (status == 0)
        set_tick_timer(0);
    target_enable_irq();
}

void _jd_phys_start(void) {
    set_tick_timer(0);
}