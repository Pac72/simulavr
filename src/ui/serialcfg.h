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
#ifndef SERIALCFG_H_INCLUDED
#define SERIALCFG_H_INCLUDED

#include <stdint.h>
#include <string>

class SerialCfg {
    private:
        std::string _pin;
        std::string _filename;
        uint32_t _baudrate;

    public:
        SerialCfg(std::string pin, std::string filename, uint32_t baudrate) :
            _pin(pin), _filename(filename), _baudrate(baudrate) { }

        inline std::string & pin() { return _pin; }
        inline std::string & filename() { return _filename; }
        inline uint32_t baudrate() { return _baudrate; }
        static SerialCfg * parse(char *cstr);
};

#endif
