//
// Simple C++ program which accesses AMC13 using libraries
//

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdlib.h>

#include <time.h>

#define TCLAP_SETBASE_ZERO 1

//TCLAP parser
#include "tclap/CmdLine.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "amc13/AMC13.hh"
#include "amc13/Module.hh"

int main(int argc, char* argv[])
{
  amc13::Module pMod;
  char *atp = getenv("AMC13_ADDRESS_TABLE_PATH");

  bool externalTTC = false;
  int randRate = 100;
  int numAMC = 12;
  int fakeSize = 0;

  time_t t0;

  //--- inhibit most noise from uHAL
  uhal::setLogLevelTo(uhal::Error());

//   if( argc < 2) {
//     printf("usage:  TakeData {<ip-address>|<connection_file>}\n");
//     exit( 1);
//   }

  try {
    TCLAP::CmdLine cmd("Test data taking program",
		       ' ',
		       "TakeData v0.1");
    
    //AMC 13 connections
    TCLAP::ValueArg<std::string> argConnection("c", "connect", "IP address, connection file or serial no", 
					    true,  std::string(""), "string", cmd);

    TCLAP::SwitchArg argExtTTC( "t", "ExternalTTC", "Use external TTC input", cmd, false);

    TCLAP::ValueArg<int> argRandRate( "r", "RandomRate", "Set random internal trigger rate",
				      false, 100, "int", cmd);

    TCLAP::ValueArg<int> argFakeSize( "f", "FakeSize", "Set fake event size per AMC in 64-bit words",
				      false, 0x400, "int", cmd);

    TCLAP::ValueArg<int> argNumAMC( "a", "NumAMC", "Number of fake AMCs to enable",
				 false, 12, "int", cmd);

    //Parse the command line arguments
    cmd.parse(argc,argv);

    pMod.Connect( argConnection.getValue().c_str(), atp);
    externalTTC = argExtTTC.getValue();
    randRate = argRandRate.getValue();
    numAMC = argNumAMC.getValue();
    fakeSize = argFakeSize.getValue();

  } catch (TCLAP::ArgException &e) {
    fprintf(stderr, "Error %s for arg %s\n",
	    e.error().c_str(), e.argId().c_str());
    return 0;
  }


  if( atp == NULL) {
    printf("Please set AMC13_ADDRESS_TABLE_PATH to location of address table files\n");
    exit( 1);
  }

  amc13::AMC13* amc13 = pMod.amc13;

  uint32_t serno = amc13->read( amc13::AMC13Simple::T1, "STATUS.SERIAL_NO");
  printf("Connected to AMC13 serial number %d\n", serno);

  // initialize the AMC13
  amc13->endRun();		// take out of run mode
  printf("AMC13 out of run mode\n");
  amc13->fakeDataEnable( true);
  
  if( externalTTC) {
    printf("Expecting external TTC\n");
    amc13->localTtcSignalEnable( false);
    amc13->enableLocalL1A( false);
  } else {
    printf("Using fake ttc\n");
    amc13->localTtcSignalEnable( true);
    amc13->enableLocalL1A( true);
  }

  amc13->monBufBackPressEnable( true);
  // figure out how many AMCs to enable
  if( numAMC < 1 || numAMC > 12) {
    printf("number of AMCs must be in the range 1-12\n");
    exit(1);
  }
  uint32_t aMask = 0;
  for( int b=1, i=0; i<numAMC; i++, b<<=1)
    aMask |= b;
  printf("AMC count %d converted to mask 0x%03x\n", numAMC, aMask);
  amc13->AMCInputEnable( aMask);

  printf("Setting fake data size to %d 64-bit words (%d bytes)\n", fakeSize, fakeSize*8);
  amc13->write( amc13::AMC13::T1, "CONF.AMC.FAKE_DATA_SIZE", fakeSize);

  amc13->startRun();
  printf("AMC13 is back in run mode and ready\n");

  printf("Set local L1A to enabled, random, burst=1, rate=%d Hz\n", randRate);
  amc13->configureLocalL1A( true, 2, 1, randRate, 0);
  amc13->enableLocalL1A( true);
  amc13->startContinuousL1A();

  uint32_t lastEvN = 0;
  t0 = time(NULL);

  // read as fast as we can, monitor rate and EvN
  while( 1) {
    int nevt = amc13->read( amc13::AMC13Simple::T1, "STATUS.MONITOR_BUFFER.UNREAD_BLOCKS");
    //    printf("Unread events: %d\n", nevt);
    if( nevt) {
      std::vector<uint64_t> evt = amc13->readEvent();
      uint32_t EvN = (evt[0] >> 32) & 0xffffff;
      if( EvN != lastEvN+1) {
	printf("error, expected EvN=0x%x saw EvN=0x%x\n", lastEvN+1, EvN);
	exit( 1);
      }
      lastEvN = EvN;
      if( (EvN % randRate) == 0 || time(NULL) != t0) {
	int rate = amc13->read( amc13::AMC13Simple::T1, "STATUS.GENERAL.L1A_RATE_HZ");
	printf( "%d (%d Hz)\n", EvN, rate);
	t0 = time(NULL);
      }
    } else {
      printf("Waiting...\n");
      sleep( 1);
    }
  }


}

