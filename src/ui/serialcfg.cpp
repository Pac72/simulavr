/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 */

#include "serialcfg.h"
#include "string2.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>

static void assertOrDie(bool cond, const char *msg, const char *cstr) {
    if (!cond) {
        std::cerr << "Invalid SerialCfg string \"" << cstr << "\": " << msg << std::endl;
        std::cerr << "Format is pin_name,filename,baudrate" << std::endl;
        exit(1);
    }
}

SerialCfg * SerialCfg::parse(char *cstr) {
    char *saveptr;
    char *buf = strdup(cstr);
    char *pp, *end;

    pp = strtok_r(buf, ",", &saveptr);
    assertOrDie(pp != NULL, "missing pin_name", cstr);
    std::string pinname = pp;

    pp = strtok_r(NULL, ",", &saveptr);
    assertOrDie(pp != NULL, "missing filename", cstr);
    std::string filename = pp;

    pp = strtok_r(NULL, ",", &saveptr);
    assertOrDie(pp != NULL, "missing baudrate", cstr);

    unsigned long ulBaudrate;
    if(!StringToUnsignedLong(pp, &ulBaudrate, &end, 10)) {
        std::cerr << "baudrate " << pp << " is not a number" << std::endl;
        exit(1);
    }

    free(buf);

    return new SerialCfg(pinname, filename, (uint32_t)ulBaudrate);
}
