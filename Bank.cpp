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

/**
 * @file Bank.cpp
 * @brief Class file for Bank.
 *
 * @synopsis
 *
 * The bank class is just a glorified sparse storage data structure that keeps
 * track of written data in case the simulator wants a functional DRAM model.
 *
 * A vector of size NUM_COLS keeps a FIFO ordered linked list of rows and their
 * associated values. A column can be viewed as a linked list of rows contained
 * within it.
 *
 * write() adds an entry to the proper linked list or replaces the value in a
 * row that was already written
 *
 * read() searches for a node with the right row value, if not found returns
 * the tracer value 0xDEADBEEF.
 * 
 * TODO: if anyone wants to actually store data, see the 'data_storage' branch
 *       and perhaps try to merge that into master
 */

#include "Bank.h"
#include "BusPacket.h"

using namespace std;
using namespace DRAMSim;

// Ctor
Bank::Bank(ostream &dramsim_log_):
		currentState(dramsim_log_), 
		rowEntries(NUM_COLS),
		dramsim_log(dramsim_log_)
{}

Bank::DataStruct *Bank::searchForRow(unsigned row, DataStruct *head)
{
	while (head != NULL)
	{
		if (head->row == row)
		{
			return head;
		}

		head = head->next;
	}

	// If we get here, didn't find it
	return NULL;
}


void Bank::read(BusPacket *busPacket)
{
	DataStruct *rowHeadNode = rowEntries[busPacket->column];
	DataStruct *foundNode = NULL;

	if ((foundNode = Bank::searchForRow(busPacket->row, rowHeadNode)) == NULL)
	{
		// The row hasn't been written before, so it isn't in the list
		void *garbage = calloc(BL * (JEDEC_DATA_BUS_BITS/8),1);
		((long *)garbage)[0] = 0xdeadbeef; // Tracer value
		busPacket->data = garbage;
	}
	else
	{
        // Found it
        busPacket->data = foundNode->data;
	}

	// The return packet should be a data packet, not a read packet
	busPacket->busPacketType = DATA;
}


void Bank::write(const BusPacket *busPacket)
{
	/* TODO: Move all the error checking to BusPacket so once we have a bus
     * packet we know the fields are all legal.
     */

	if (busPacket->column >= NUM_COLS)
	{
		ERROR("== Error - Bus Packet column "
              << busPacket->column 
              <<" out of bounds");
		exit(-1);
	}

	// Head of the list we need to search
	DataStruct *rowHeadNode = rowEntries[busPacket->column];
	DataStruct *foundNode = NULL;

	if ((foundNode = Bank::searchForRow(busPacket->row, rowHeadNode)) == NULL)
	{
		// Not found
		DataStruct *newRowNode = (DataStruct *)malloc(sizeof(DataStruct));

		// Insert at the head for speed
		// TODO: Optimize this data structure for speedier lookups?
		newRowNode->row = busPacket->row;
		newRowNode->data = busPacket->data;
		newRowNode->next = rowHeadNode;
		rowEntries[busPacket->column] = newRowNode;
	}
	else
	{
		// found it, just plaster in the new data
		foundNode->data = busPacket->data;

		if (DEBUG_BANKS)
		{
			PRINTN(" -- Bank "
                    << busPacket->bank
                    <<" writing to physical address 0x" 
                    << hex << busPacket->physicalAddress<< dec
                    <<":");
			busPacket->printData();
			PRINT("");
		}
	}
}
