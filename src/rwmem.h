/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003   Klaus Rudolph		
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 */
#ifndef RWMEM
#define RWMEM
/*
 * All here defined types are used to simulate the 
 * read write address space. This means also registers
 * io-data space, internal and external sram
 */

class RWMemoryMembers{
    public:
        virtual unsigned char operator=(unsigned char val) =0;
        virtual operator unsigned char() const =0 ;
        void operator=(const RWMemoryMembers &mm);
};

/* the following class have one byte own memory and can be used for
 * registers sram and registers 
 */
class RWMemoryWithOwnMemory: public RWMemoryMembers {
    protected: 
        unsigned char value;

    public:
        RWMemoryWithOwnMemory() {
            value=0;
        }

        unsigned char operator=(unsigned char val);
        operator unsigned char() const;
};

class AvrDevice;

class CPURegister: public RWMemoryWithOwnMemory {
    unsigned int myNumber;
    AvrDevice *core;

    public:
    CPURegister(AvrDevice *c, unsigned int number);

    unsigned char operator=(unsigned char val);
    operator unsigned char() const;
};


class IRam: public RWMemoryWithOwnMemory {
    unsigned int myAddress;
    AvrDevice *core;
    public:
    IRam(AvrDevice *c, unsigned int number) {
        core=c;
        myAddress=number;
    }
    unsigned char operator=(unsigned char val); 
    operator unsigned char() const;
};

//TODO this Ram must be connected to the special io register for controlling ext ram!
class ERam: public RWMemoryWithOwnMemory {
    unsigned int myAddress;
    public:
    ERam(unsigned int number) {
        myAddress=number;
    }
    unsigned char operator=(unsigned char val);
};

class NotAvailableIo: public RWMemoryMembers {
    unsigned int myAddress;
    public:
    NotAvailableIo(unsigned int number) {
        myAddress=number;
    }

    unsigned char operator=(unsigned char val); 
    operator unsigned char() const;
};

class RWReserved: public RWMemoryMembers {
    public:
        RWReserved() { }
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};



class MemoryOffsets {
    protected:
        unsigned int myOffset;
        RWMemoryMembers **rwHandler;

    public:
        MemoryOffsets(unsigned int offset, RWMemoryMembers **rw):rwHandler(rw){
            myOffset=offset;
        }

        RWMemoryMembers &operator[](unsigned int externOffset) const;


};


//;-------------------------------------------------------
#include <fstream>
using namespace std;
class RWWriteToPipe: public RWMemoryMembers {
    protected:
        string pipeName;
        ofstream os;

    public:
        RWWriteToPipe(const string &name): os(name.c_str()) { 
            pipeName=name;
        }
        virtual ~RWWriteToPipe() {}
        virtual unsigned char operator=(unsigned char);
        virtual operator unsigned char() const;
};




#endif
