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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 */

#include "types.h"
#include "decoder.h"
#include "avrdevice.h"

#include "hwsreg.h"
#include "helper.h"
#include "flash.h"
#include "rwmem.h"
#include "ioregs.h"
#include "avrerror.h"

#include <iomanip>
#include <string>
#include <sstream>

using namespace std;

#ifdef __GNUC__
/*
 * Calculate index from mask so that (1<<index)==mask.
 * does not crash if mask has more than 1 bit set
 * but is faster and more compact than the nested
 * ternary operations below.
 */
#define INDEX_FROM_BITMASK(mask) __builtin_ctz(mask)

#else /* __GNUC__ */

/// Calculate index from mask so that (1<<index)==mask. Crash on incorrect values.
#define INDEX_FROM_BITMASK(mask)  \
    ( (mask) == 0x01 ? 0          \
    : (mask) == 0x02 ? 1          \
    : (mask) == 0x04 ? 2          \
    : (mask) == 0x08 ? 3          \
    : (mask) == 0x10 ? 4          \
    : (mask) == 0x20 ? 5          \
    : (mask) == 0x40 ? 6          \
    : (mask) == 0x80 ? 7          \
	: abort_in_expression() )

#endif /* __GNUC__ */

inline void DecodedInstruction::TraceSREG(void) {
    const streamsize width = traceOut.width();
    traceOut << std::setw(0) << (string)(*(core->status));
    traceOut.width(width);
}

int abort_in_expression()
{
    abort();
    return 0;
}

int avr_op_ADC::Trace()  {
    std::stringstream sstr;
    sstr << "ADC R" << (int)R1 << ", R" << (int)R2 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_ADD::Trace() {
    std::stringstream sstr;
    sstr << "ADD R" << (int)R1 << ", R" << (int)R2 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_ADIW::Trace() {
    std::stringstream sstr;
    sstr << "ADIW R" << (int)Rl << ", " << (int)K << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_AND::Trace() {
    std::stringstream sstr;
    sstr << "AND R" << (int)R1 << ", R" << (int)R2 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret=this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_ANDI::Trace() {
    std::stringstream sstr;
    sstr << "ANDI R" << (int)R1 << ", " << HexChar(K) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret=this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_ASR::Trace() {
    std::stringstream sstr;
    sstr << "ASR R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

const char *opcodes_bclr[8]= {
    "CLC",
    "CLZ",
    "CLN",
    "CLV",
    "CLS",
    "CLH",
    "CLT",
    "CLI"
};

int avr_op_BCLR::Trace() {
    std::stringstream sstr;
    sstr << opcodes_bclr[Kbit] << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_BLD::Trace() {
    std::stringstream sstr;
    sstr << "BLD R" << (int)R1 << ", " << (int)Kbit << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

const char *branch_opcodes_clear[8] = {
    "BRCC",
    "BRNE",
    "BRPL",
    "BRVC",
    "BRGE",
    "BRHC",
    "BRTC",
    "BRID"
};

int avr_op_BRBC::Trace() {
    std::stringstream sstr;
    sstr << branch_opcodes_clear[INDEX_FROM_BITMASK(bitmask)]
        << " " << HexShort(offset * 2) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    string sym(core->Flash->GetSymbolAtAddress(core->PC+1+offset));
    int ret = this->operator()();
    
    traceOut << sym << " ";
    for(int len = sym.length(); len < 30; len++)
        traceOut << " ";

    return ret;
}

const char *branch_opcodes_set[8] = {
    "BRCS",
    "BREQ",
    "BRMI",
    "BRVS",
    "BRLT",
    "BRHS",
    "BRTS",
    "BRIE"
};

int avr_op_BRBS::Trace() {
    std::stringstream sstr;
    sstr << branch_opcodes_set[INDEX_FROM_BITMASK(bitmask)]
        << " " << HexShort(offset * 2) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    string sym(core->Flash->GetSymbolAtAddress(core->PC+1+offset));
    int ret = this->operator()();

    traceOut << sym << " ";
    for(int len = sym.length(); len < 30; len++)
        traceOut << " ";

    return ret;
}

const char *opcodes_bset[8]= {
    "SEC",
    "SEZ",
    "SEN",
    "SEV",
    "SES",
    "SEH",
    "SET",
    "SEI"
};

int avr_op_BSET::Trace() {
    std::stringstream sstr;
    sstr << opcodes_bset[Kbit] << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_BST::Trace() {
    std::stringstream sstr;
    sstr << "BST R" << (int)R1 << ", " << (int)Kbit << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_CALL::Trace() {
    word K_lsb = core->Flash->ReadMemWord((core->PC + 1) * 2);
    int k = (KH << 16) | K_lsb;
    std::stringstream sstr;
    sstr << "CALL " << HexShort(k * 2) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_CBI::Trace() {
    std::stringstream sstr;
    sstr << "CBI " << HexChar(ioreg) << ", " << (int)Kbit << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_COM::Trace() {
    std::stringstream sstr;
    sstr << "COM R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_CP::Trace() {
    std::stringstream sstr;
    sstr << "CP R" << (int)R1 << ", R" << (int)R2 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_CPC::Trace() {
    std::stringstream sstr;
    sstr << "CPC R" << (int)R1 << ", R" << (int)R2 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_CPI::Trace() {
    std::stringstream sstr;
    sstr << "CPI R" << (int)R1 << ", " << HexChar(K) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_CPSE::Trace() {
    std::stringstream sstr;
    sstr << "CPSE R" << (int)R1 << ", R" << (int)R2 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_DEC::Trace() {
    std::stringstream sstr;
    sstr << "DEC R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_EICALL::Trace() {
    std::stringstream sstr;
    sstr << "EICALL ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_EIJMP::Trace() {
    std::stringstream sstr;
    sstr << "EIJMP ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_ELPM_Z::Trace() {
    std::stringstream sstr;
    sstr << "ELPM R" << (int)R1 << ", Z " ;
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();

    unsigned char rampz = 0;
    if(core->rampz != NULL)
        rampz = core->rampz->GetRegVal();
    unsigned int Z = (rampz << 16) + core->GetRegZ();

    traceOut << " Flash[0x" << hex << Z << dec << "] ";

    return ret;
}

int avr_op_ELPM_Z_incr::Trace() {
    std::stringstream sstr;
    sstr << "ELPM R" << (int)R1 << ", Z+ ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    unsigned char rampz = 0;
    if(core->rampz != NULL)
        rampz = core->rampz->GetRegVal();
    unsigned int Z = (rampz << 16) + core->GetRegZ();
    int ret = this->operator()();

    traceOut << " Flash[0x" << hex << Z << dec << "] ";

    return ret;
}

int avr_op_ELPM::Trace() {
    std::stringstream sstr;
    sstr << "ELPM ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();

    unsigned char rampz = 0;
    if(core->rampz != NULL)
        rampz = core->rampz->GetRegVal();
    unsigned int Z = (rampz << 16) + core->GetRegZ();

    traceOut << " Flash[0x" << hex << Z << dec << "] ";

    return ret;
}

int avr_op_EOR::Trace() {
    std::stringstream sstr;
    sstr << "EOR R" << (int)R1 << ", R" << (int)R2 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_ESPM::Trace() {
    std::stringstream sstr;
    sstr << "SPM Z+ ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_FMUL::Trace() {
    std::stringstream sstr;
    sstr << "FMUL R" << (int)Rd << ", R" << (int)Rr << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_FMULS::Trace() {
    std::stringstream sstr;
    sstr << "FMULS R" << (int)Rd << ", R" << (int)Rr << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_FMULSU::Trace() {
    std::stringstream sstr;
    sstr << "FMULSU R" << (int)Rd << ", R" << (int)Rr << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_ICALL::Trace() {
    std::stringstream sstr;
    sstr << "ICALL Z " ;
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_IJMP::Trace() {
    std::stringstream sstr;
    sstr << "IJMP Z " ;
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_IN::Trace() {
    std::stringstream sstr;
    sstr << "IN R" << (int)R1 << ", " << HexChar(ioreg) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_INC::Trace() {
    std::stringstream sstr;
    sstr << "INC R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_JMP::Trace() {
    std::stringstream sstr;
    word offset = core->Flash->ReadMemWord((core->PC + 1) * 2);  //this is k!
    sstr << "JMP " << HexShort(2 * offset) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();

    string sym(core->Flash->GetSymbolAtAddress(offset));
    traceOut << sym << " ";
    for(int len = sym.length(); len < 30; len++)
        traceOut << " " ;

    return ret;
}

int avr_op_LDD_Y::Trace() {
    std::stringstream sstr;
    sstr << "LDD R" << (int)Rd << ", Y+" << (int)K << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_LDD_Z::Trace() {
    std::stringstream sstr;
    sstr << "LDD R" << (int)Rd << ", Z+" << (int)K << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_LDI::Trace() {
    std::stringstream sstr;
    sstr << "LDI R" << (int)R1 << ", " << HexChar(K) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_LDS::Trace() {
    word offset = core->Flash->ReadMemWord((core->PC + 1) * 2);  //this is k!
    std::stringstream sstr;
    sstr << "LDS R" << (int)R1 << ", " << HexShort(offset) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_X::Trace() {
    std::stringstream sstr;
    sstr << "LD R" << (int)Rd << ", X ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_X_decr::Trace() {
    std::stringstream sstr;
    sstr << "LD R" << (int)Rd << ", -X ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_X_incr::Trace() {
    std::stringstream sstr;
    sstr << "LD R" << (int)Rd << ", X+ ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_Y_decr::Trace() {
    std::stringstream sstr;
    sstr << "LD R" << (int)Rd << ", -Y ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_Y_incr::Trace() {
    std::stringstream sstr;
    sstr << "LD R" << (int)Rd << ", Y+ " ;
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_Z_incr::Trace() {
    std::stringstream sstr;
    sstr << "LD R" << (int)Rd << ", Z+ ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_LD_Z_decr::Trace() {
    std::stringstream sstr;
    sstr << "LD R" << (int)Rd << ", -Z";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_LPM_Z::Trace() {
    std::stringstream sstr;
    sstr << "LPM R" << (int)Rd << ", Z ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();

    /* Z is R31:R30 */
    unsigned int Z = core->GetRegZ();
    string sym(core->Flash->GetSymbolAtAddress(Z));
    traceOut << "FLASH[0x" << hex << Z << dec << "," << sym << "] ";

    return ret;
}

int avr_op_LPM::Trace() {
    std::stringstream sstr;
    sstr << "LPM R0, Z ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0); 
    int ret = this->operator()();

    /* Z is R31:R30 */
    unsigned int Z = core->GetRegZ();
    string sym(core->Flash->GetSymbolAtAddress(Z));
    traceOut << "FLASH[0x" << hex << Z << dec << "," << sym << "] ";

    return ret;
}

int avr_op_LPM_Z_incr::Trace() {
    std::stringstream sstr;
    sstr << "LPM R" << (int)Rd << ", Z+ " ;
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    /* Z is R31:R30 */
    unsigned int Z = core->GetRegZ();
    int ret = this->operator()();
    
    string sym(core->Flash->GetSymbolAtAddress(Z));
    traceOut << "FLASH[0x" << hex << Z << dec << "," << sym << "] ";
    return ret;
}

int avr_op_LSR::Trace() {
    std::stringstream sstr;
    sstr << "LSR R" << (int)Rd << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_MOV::Trace() {
    std::stringstream sstr;
    sstr << "MOV R" << (int)R1 << ", R" << (int)R2 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_MOVW::Trace() {
    std::stringstream sstr;
    sstr << "MOVW R" << (int)Rd << ", R" << (int)Rs << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_MUL::Trace() {
    std::stringstream sstr;
    sstr << "MUL R" << (int)Rd << ", R" << (int)Rr << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_MULS::Trace() {
    std::stringstream sstr;
    sstr << "MULS R" << (int)Rd << ", R" << (int)Rr << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_MULSU::Trace() {
    std::stringstream sstr;
    sstr << "MULSU R" << (int)Rd << ", R" << (int)Rr << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_NEG::Trace() {
    std::stringstream sstr;
    sstr << "NEG R" << (int)Rd <<" ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_NOP::Trace() {
    std::stringstream sstr;
    sstr << "NOP ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_OR::Trace() {
    std::stringstream sstr;
    sstr << "OR R" << (int)Rd << ", R" << (int)Rr << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_ORI::Trace() {
    std::stringstream sstr;
    sstr << "ORI R" << (int)R1 << ", " << HexChar(K) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_OUT::Trace() {
    std::stringstream sstr;
    sstr << "OUT " << HexChar(ioreg) << ", R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_POP::Trace() {
    std::stringstream sstr;
    sstr << "POP R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_PUSH::Trace() {
    std::stringstream sstr;
    sstr << "PUSH R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_RCALL::Trace() {
    std::stringstream sstr;
    sstr << "RCALL " << HexShort((core->PC + K + 1) << 1) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_RET::Trace() {
    std::stringstream sstr;
    sstr << "RET " ;
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_RETI::Trace() {
    std::stringstream sstr;
    sstr << "RETI ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_RJMP::Trace() {
    std::stringstream sstr;
    sstr << "RJMP " << HexShort((core->PC + K + 1) << 1) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_ROR::Trace() {
    std::stringstream sstr;
    sstr << "ROR R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_SBC::Trace() {
    std::stringstream sstr;
    sstr << "SBC R" << (int)R1 << ", R" << (int)R2 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_SBCI::Trace() {
    std::stringstream sstr;
    sstr << "SBCI R" << (int)R1 << ", " << HexChar(K) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_SBI::Trace() {
    std::stringstream sstr;
    sstr << "SBI " << HexChar(ioreg) << ", " << (int)Kbit << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_SBIC::Trace() {
    std::stringstream sstr;
    sstr << "SBIC " << HexChar(ioreg) << ", " << (int)Kbit << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_SBIS::Trace() {
    std::stringstream sstr;
    sstr << "SBIS " << HexChar(ioreg) << ", " << (int)Kbit << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_SBIW::Trace( ) {
    std::stringstream sstr;
    sstr << "SBIW R" << (int)R1 << ", " << HexChar(K) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret=this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_SBRC::Trace() {
    std::stringstream sstr;
    sstr << "SBRC R" << (int)R1 << ", " << (int)Kbit << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_SBRS::Trace() {
    std::stringstream sstr;
    sstr << "SBRS R" << (int)R1 << ", " << (int)Kbit << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_SLEEP::Trace() {
    std::stringstream sstr;
    sstr << "SLEEP " ;
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_SPM::Trace() {
    std::stringstream sstr;
    sstr << "SPM " ;
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_STD_Y::Trace() {
    std::stringstream sstr;
    sstr << "STD Y+" << (int)K << ", R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_STD_Z::Trace() {
    std::stringstream sstr;
    sstr << "STD Z+" << (int)K << ", R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_STS::Trace() {
    word offset = core->Flash->ReadMemWord((core->PC + 1) * 2);  //this is k!
    std::stringstream sstr;
    sstr << "STS " << "0x" << hex << offset << dec << ", R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_X::Trace() {
    std::stringstream sstr;
    sstr << "ST X, R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_X_decr::Trace() {
    std::stringstream sstr;
    sstr << "ST -X, R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_X_incr::Trace() {
    std::stringstream sstr;
    sstr << "ST X+, R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_Y_decr::Trace() {
    std::stringstream sstr;
    sstr << "ST -Y, R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_Y_incr::Trace() {
    std::stringstream sstr;
    sstr << "ST Y+, R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_Z_decr::Trace() {
    std::stringstream sstr;
    sstr << "ST -Z, R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_ST_Z_incr::Trace() {
    std::stringstream sstr;
    sstr << "ST Z+, R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_SUB::Trace() {
    std::stringstream sstr;
    sstr << "SUB R" << (int)R1 << ", R" << (int)R2 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_SUBI::Trace() {
    std::stringstream sstr;
    sstr << "SUBI R" << (int)R1 << ", " << HexChar(K) << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    TraceSREG();
    return ret;
}

int avr_op_SWAP::Trace() {
    std::stringstream sstr;
    sstr << "SWAP R" << (int)R1 << " ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_WDR::Trace() {
    std::stringstream sstr;
    sstr << "WDR ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_BREAK::Trace() {
    std::stringstream sstr;
    sstr << "BREAK ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

int avr_op_ILLEGAL::Trace() {
    std::stringstream sstr;
    sstr << "Invalid Instruction! ";
    traceOut << std::setw(instrWidth) << sstr.str() << std::setw(0);
    int ret = this->operator()();
    return ret;
}

/* EOF */
