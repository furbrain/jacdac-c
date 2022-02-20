// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 * A reception queue for JD packets.
 */

#ifndef JD_RX_H
#define JD_RX_H

#include "jd_service_framework.h"

void jd_rx_init(void);
int jd_rx_frame_received(jd_frame_t *frame);
jd_frame_t* jd_rx_get_frame(void);

#endif
