/*
 * ImageFile.h
 *
 *  Created on: Nov 19, 2020
 *      Author: julian
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_IMAGEFILE_H_
#define INC_IMAGEFILE_H_

#include "stm32f7xx_hal.h"


#if EURORACK_MODULE
#pragma message "Building eurorack Version"
#endif




typedef struct ImageFileHeaderStruct
{
	char fileHeader[8];		// ("LXRV2IMG") for Desktop and ("LXRERIMG") for Eurorack 8 bytes
	uint32_t datasize;		// size of the data blob, 4 bytes
	char checksum;			// 1 byte checksum
	char unused[3];			// 3 bytes padding to make header 16 byte long

} ImageFileHeader;

#define HEADER_SIZE sizeof(ImageFileHeader)

#endif /* INC_IMAGEFILE_H_ */
