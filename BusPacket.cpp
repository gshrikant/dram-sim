/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland 
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/

//
// BusPacket.cpp
// Class file for bus packet object
//

#include "BusPacket.h"
#include <assert.h>

using namespace DRAMSim;
using namespace std;

/**
 * @brief 
 *
 * @param packtype
 * @param physicalAddr
 * @param col
 * @param rw
 * @param r
 * @param b
 * @param dat
 * @param dramsim_log_
 */
BusPacket::BusPacket(BusPacketType packtype, uint64_t physicalAddr, 
		unsigned col, unsigned rw, unsigned r, unsigned b, void *dat, 
		ostream &dramsim_log_) :
	dramsim_log(dramsim_log_),
	busPacketType(packtype),
	column(col),
	row(rw),
	bank(b),
	rank(r),
	physicalAddress(physicalAddr),
	data(dat)
{}

void BusPacket::print(uint64_t currentClockCycle, bool dataStart)
{
    assert(this != NULL);

    if (VERIFICATION_OUTPUT)
	{
		switch (busPacketType)
		{
		case READ:
			cmd_verify_out << currentClockCycle << ": read ("<<rank<<","<<bank<<","<<column<<",0);"<<endl;
			break;
		case READ_P:
			cmd_verify_out << currentClockCycle << ": read ("<<rank<<","<<bank<<","<<column<<",1);"<<endl;
			break;
		case WRITE:
			cmd_verify_out << currentClockCycle << ": write ("<<rank<<","<<bank<<","<<column<<",0 , 0, 'h0);"<<endl;
			break;
		case WRITE_P:
			cmd_verify_out << currentClockCycle << ": write ("<<rank<<","<<bank<<","<<column<<",1, 0, 'h0);"<<endl;
			break;
		case ACTIVATE:
			cmd_verify_out << currentClockCycle <<": activate (" << rank << "," << bank << "," << row <<");"<<endl;
			break;
		case PRECHARGE:
			cmd_verify_out << currentClockCycle <<": precharge (" << rank << "," << bank << "," << row <<");"<<endl;
			break;
		case REFRESH:
			cmd_verify_out << currentClockCycle <<": refresh (" << rank << ");"<<endl;
			break;
		case DATA:
			//TODO: data verification?
			break;
		default:
			ERROR("Trying to print unknown kind of bus packet");
			exit(-1);
		}
	}
}

void BusPacket::print()
{
    assert(this != NULL);
    char msg[1024];
    const char fmt[] ="\"BP [%s] pa[0x\"<<hex<<physicalAddress<<dec<<\"] r[\"<<rank<<\"] b[\"<<bank<<\"] row[\"<<row<<\"] col[\"<<column<<\"]\""; 

    switch (busPacketType)
    {
        case READ:
            std::sprintf(msg, fmt, "READ");
            PRINT(msg);
            cout << msg;
            break;
        case READ_P:
            std::sprintf(msg, fmt, "READ_P");
            PRINT(msg);
            cout << msg;
            break;
        case WRITE:
            std::sprintf(msg, fmt, "WRITE");
            PRINT(msg);
            break;
        case WRITE_P:
            std::sprintf(msg, fmt, "WRITE_P");
            PRINT(msg);
            break;
        case ACTIVATE:
            std::sprintf(msg, fmt, "ACT");
            PRINT(msg);
            break;
        case PRECHARGE:
            std::sprintf(msg, fmt, "PRE");
            PRINT(msg);
            break;
        case REFRESH:
            std::sprintf(msg, fmt, "REF");
            PRINT(msg);
            break;
        case DATA:
            PRINTN("BP [DATA] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"] data["<<data<<"]=");
            printData();
            PRINT("");
            break;
        default:
            ERROR("Trying to print unknown kind of bus packet");
            exit(-1);
    }
}

void BusPacket::printData() const 
{
	if (data == NULL)
	{
		PRINTN("NO DATA");
		return;
	}
	PRINTN("'" << hex);
	for (int i=0; i < 4; i++)
	{
		PRINTN(((uint64_t *)data)[i]);
	}
	PRINTN("'" << dec);
}
