#include <cassert>
#include <cstdio>
#include <Windows.h>

#include "visa.h"
#pragma comment(lib, "visa32.lib")

// This example demonstrates 
// 1. Using VISA to connect to the VSG60 software 
// 2. Configuring the software for amplitude modulation
// 3. Wait for the *OPC bit to be set in the status register to indicate 
//    the waveform is being output.

static void wait_for_opc(ViSession inst)
{
    viPrintf(inst, "*OPC\n");
    while(true) {
        int esr;
        viQueryf(inst, "*ESR?\n", "%d", &esr);
        if(esr) {
            break;
        }
        Sleep(16);
    }
}

void scpi_vsg_multitone()
{
    ViSession rm, inst;
    ViStatus rmStatus;

    // Get the VISA resource manager
    rmStatus = viOpenDefaultRM(&rm);
    assert(rmStatus == 0);

    // Open a session to the VSG software which must be running at this point
    ViStatus instStatus = viOpen(rm, "TCPIP::localhost::5024::SOCKET", VI_NULL, VI_NULL, &inst);
    assert(instStatus == 0);

    // For SOCKET programming, we want to tell VISA to use a terminating character 
    //   to end a read operation. In this case we want the newline character to end a 
    //   read operation. The termchar is typically set to newline by default. Here we
    //   set it for illustrative purposes.
    viSetAttribute(inst, VI_ATTR_TERMCHAR_EN, VI_TRUE);
    viSetAttribute(inst, VI_ATTR_TERMCHAR, '\n');

    // Disable modulation
    viPrintf(inst, "OUTPUT ON\n");
    viPrintf(inst, "OUTPUT:MOD OFF\n");

    // Configure freq/ampl
    viPrintf(inst, "FREQ 100MHz\n");
    viPrintf(inst, "POW -20\n");

    // Configure modulation
    viPrintf(inst, "MTONE ON\n");
    viPrintf(inst, "MTONE:PHASE RANDOM\n");
    viPrintf(inst, "MTONE:PHASE:SEED 12\n");
    viPrintf(inst, "MTONE:NTONES 1001\n");
    viPrintf(inst, "MTONE:FSPACING 10kHz\n");
    viPrintf(inst, "MTONE:FNOTCH 1MHz\n");

    // Enable modulation
    viPrintf(inst, "OUTPUT:MOD ON\n");

    // Wait for operation complete
    wait_for_opc(inst);

    // Make some measurement here
    // ...

    // Done
    viClose(inst);
    return;
}