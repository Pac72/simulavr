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

bool StringToInt64(const char *s, int64_t *n, char **endptr, int base) {
#if __WORDSIZE == 64
    return StringToLong(s, n, endptr, base);
#else
    return StringToLongLong(s, n, endptr, base);
#endif
}

static void assertOrDie(bool cond, const char *msg, const char *cstr) {
    if (!cond) {
        std::cerr << "Invalid SerialCfg string \"" << cstr << "\": " << msg << std::endl;
        std::cerr << "Format is pin_name,filename,baudrate" << std::endl;
        exit(1);
    }
}

SerialCfg * SerialCfg::parse(char *cstr, bool allowDelay) {
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

    // optional 
    pp = strtok_r(NULL, ",", &saveptr);
    int64_t delayNanos = 0;
    if (pp != NULL) {
        if (allowDelay) {
            if (!StringToInt64(pp, &delayNanos, &end, 10)) {
                std::cerr << "delayNanos " << pp << " is not a number" << std::endl;
                exit(1);
            }
        } else {
            std::cerr << "delayNanos not allowed here, remove '," << pp << "'" << std::endl;
            exit(1);
        }
    }

    free(buf);

    return new SerialCfg(pinname, filename, (uint32_t)ulBaudrate, (int64_t)delayNanos);
}

SerialCfg * SerialCfg::parseRx(char *cstr) {
    return parse(cstr, false);
}

SerialCfg * SerialCfg::parseTx(char *cstr) {
    return parse(cstr, true);
}
