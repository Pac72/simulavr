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
#ifndef LCD
#define LCD


#include <iostream>
#include <string>

#include "simulationmember.h"
//#include "hardware.h"
#include "ui.h"
#include "pin.h"


class Lcd : public SimulationMember {
    protected:
        UserInterface *ui;
        string name;
        unsigned char myPortValue;
        map<string, Pin*> allPins;
        Pin d0;
        Pin d1;
        Pin d2;
        Pin d3;

        Pin enable;
        Pin readWrite;
        Pin commandData;



        int merke_x;
        int merke_y;

        void LcdWriteData(unsigned char data);
        void LcdWriteCommand(unsigned char command);

        //ofstream debugOut;
        void SendCursorPosition();

    public:
        virtual int Step(bool trueHwStep, unsigned long long *timeToNextStepIn_ns=0);
        Lcd(UserInterface *ui, const string &name, const string &baseWindow);
        virtual ~Lcd();
        Pin *GetPin(const char *name); 

};

#endif
