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
#include "hwacomp.h"

#include "irqsystem.h"
#include "trace.h"

#define ACD 0x80
#define ACO 0x20
#define ACI 0x10
#define ACIE 0x08
#define ACIC 0x04
#define ACIS1 0x02
#define ACIS0 0x01


HWAcomp::HWAcomp(AvrDevice *core, HWIrqSystem *irqsys, PinAtPort ain0, PinAtPort ain1, unsigned int _irqVec):
Hardware(core), irqSystem(irqsys), pinAin0(ain0), pinAin1(ain1), irqVec(_irqVec) {
    core->AddToCycleList(this);
//    irqSystem->RegisterIrqPartner(this, _irqVec);
}


void HWAcomp::Reset(){
    acsr=0;
}

void HWAcomp::SetAcsr(unsigned char val) {
    unsigned char old= acsr&0x30; 
    acsr=val&0x9f; //Bits 5 & 6 are read only for 4433 that is not ok TODO XXX
    acsr|= old;
    if (val & ACI) {
        acsr &=~ACI; // reset ACI if ACI in val is set 1
    }

    if ( (acsr & ( ACI | ACIE) ) == ( ACI | ACIE) ) {
        irqSystem->SetIrqFlag(this, irqVec);
    } else {
        irqSystem->ClearIrqFlag(irqVec);
    }

}

unsigned char HWAcomp::GetAcsr() {
    return acsr;
}

unsigned int HWAcomp::CpuCycle() {
    //cout << "HWAComp: acsr: start" << hex << (unsigned int ) acsr << endl;
    bool oldComp=(acsr & ACO);

    //cout << "Compare Pin0: " << pinAin0.GetAnalog() << " with Pin1: " << pinAin1.GetAnalog() << endl;

    if (pinAin0.GetAnalog()>pinAin1.GetAnalog()) { //set comperator 1
        if (oldComp==false) { //not set before
            acsr|=ACO;
            //do the irq TODO
            unsigned char irqMask= acsr & (ACIS1|ACIS0);
            if ( (irqMask==0) || (irqMask==( ACIS1 | ACIS0) ) ) { //toggle or rising
                acsr|=ACI;
                if (acsr&ACIE) irqSystem->SetIrqFlag(this, irqVec);
            }
        }
    } else { //set comperator 0
        if (oldComp==true) { //ACO was set before
            acsr&=~ACO;
            //do the irq
            unsigned char irqMask= acsr & (ACIS1|ACIS0);
            if ( (irqMask==0) || (irqMask==( ACIS1 ) )) { //toggle or falling
                acsr|=ACI;
                if (acsr&ACIE) irqSystem->SetIrqFlag(this, irqVec);
            }
        }
    }

    //cout << "HWAComp: acsr: end" << hex << (unsigned int ) acsr << endl;

    return 0;
}

#if 0

bool HWAcomp::IsIrqFlagSet(unsigned int vec) {
    /* XXX remove that function next time
    if (vec== irqVec) {
        if ( (acsr & ( ACI | ACIE) ) == ( ACI | ACIE) ) { //irq flag and irq enabled?
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
    */

    return 1;
}
#endif


void HWAcomp::ClearIrqFlag(unsigned int vector){
    //cout << "Clear Irq Flag in HWAComp" << endl;
    //cout << "acsr= " << hex << (unsigned int ) acsr << endl;
    if (vector==irqVec) {
        acsr&=~ACI;
        irqSystem->ClearIrqFlag(irqVec);
    }
    //cout << "acsr after clear = " << hex << ( unsigned int) acsr << endl;
}


unsigned char RWAcsr::operator=(unsigned char val) { trioaccess("Acsr",val); acomp->SetAcsr(val);  return val; } 
RWAcsr::operator unsigned char() const { return acomp->GetAcsr(); } 
