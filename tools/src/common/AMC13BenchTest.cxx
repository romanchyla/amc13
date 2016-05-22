#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <unistd.h> //For Sleep function
#include <iomanip>
#include <ctime>

#include "amc13/Module.hh"
#include "tclap/CmdLine.h"

bool isCharInt(char ch) {
  return (ch >= '0' && ch <='9');
}

//For testing if the given serial number is actually a serial number
bool isStringUInt(const std::string& str) {
  for (std::string::const_iterator iter = str.begin(); iter != str.end(); iter++) {
    if (!isCharInt(*iter)) return false;
  }
  return true;
}

//Returns a string with the default IP Address for the given serial no
std::string getAddress(int SN, amc13::AMC13::Board t) {
  int lastBit = 0;
  if (t == amc13::AMC13::T1) {
    lastBit = 255 - 2*(SN%128);
  } else {
    lastBit = 254 - 2*(SN%128);
  }
  int thirdBit = SN/128+1;
  std::ostringstream stream;
  stream << "192.168." << thirdBit << "." << lastBit;
  return stream.str();
}

//Returns the name of the connection file based off of the serial no
std::string getFileName(int SN) {
  std::ostringstream stream;
  stream << "connectionSN";
  if (SN >= 100) {
    stream << SN;
  } else if (SN < 100 && SN >= 10) {
    stream << "0" << SN;
  } else {
    stream << "00" << SN;
  }
  stream << ".xml";
  return stream.str();
}

//creates a connection file
void makeFile(int SN, std::string atp) {
  std::ofstream connectionFile(getFileName(SN).c_str());
  connectionFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n\n";
  connectionFile << "<connections>" << "\n";
  
  connectionFile << "  " << "<connection id=\"T1\" uri=\"ipbusudp-2.0://" << getAddress(SN,amc13::AMC13::T1) 
		 << ":50001\" address_table=\"file://" << atp << "AMC13XG_T1.xml\" />" << "\n";
  
  connectionFile << "  " << "<connection id=\"T2\" uri=\"ipbusudp-2.0://" << getAddress(SN,amc13::AMC13::T2) 
		 << ":50001\" address_table=\"file://" << atp << "AMC13XG_T2.xml\" />" << "\n";
  
  connectionFile << "\n\n" << "</connections>" << "\n";
  connectionFile.close();
}

//Check if the numbers given are identical or all different
enum Expectation {
  DIFF,
  SAME,
  VAL
};

bool check(uint32_t vals[], Expectation e, uint32_t expectedVal = 0) {
  if (e == DIFF) {
    return (vals[0] != vals[1] && vals[1] != vals[2] && vals[0] != vals[2]);
  } else if ( e == SAME) {
    return (vals[0] == vals[1] && vals[1] == vals[2]);
  } else {
    return (vals[0] == expectedVal && vals[1] == expectedVal && vals[2] == expectedVal);
  }
}
  
bool check3Vals(amc13::Module& module, Expectation e,amc13::AMC13::Board t, uint32_t addr,const std::string& title, std::ofstream& file) {
  std::cout << "Checking " << title << "...\n";
  file << "Checking " << title << "...\n";
  bool result;
  uint32_t vals[3];
  for (int i = 0; i < 3; i++) {
    vals[i] = module.amc13->read(t,addr);
  }
  for (int i = 0; i < 3; i++) {
    if (t == amc13::AMC13::T1) {
      file << "T1: ";
    } else {
      file << "T2: ";
    }
    file << "0x" << std::hex << addr << ": " << "0x" << std::hex << vals[i] << "\n";
  }
  if (check(vals,e)) {
    std::cout << title << " is ";
    file << title << " is ";
    if (e == DIFF) {
      std::cout << "changing!\n";
      file      << "changing!\n";
    } else {
      std::cout << "constant!\n";
      file      << "constant!\n";
    }
    result = true;
  } else {
    std::cerr << "ERROR: " <<title << " should be ";
    file      << "ERROR: " <<title << " should be ";
    if (e == DIFF) {
      std::cout << "changing";
      file      << "changing";
    } else {
      std::cout << "constant";
      file      << "constant";
    }
    std::cout << ", but it isn't... \n";
    file      << ", but it isn't... \n";
    result = false;
  }
  file << "\n";
  std::cout << "\n";
  return result;
}

void prompt() {
  std::string response("");
  std::cout << "Press c to continue, q to quit: ";
  std::cin >> response;
  std::cout << "\n";
  if (response == "q") {
    throw "Quitting testing\n";
  }
}

void memoryTest(int sn, const std::string& addressTable, std::ofstream& logfile) {
  
  //===============Memory Test==================
  amc13::Module module;
  std::string msg("");
  msg = "Connecting...\n";
  std::cout << msg;
  logfile << msg;
  module.Connect(getFileName(sn),addressTable);
  std::cout << "\n";
  
  msg = "\nChecking T2 AMC Link Error Counter...\n";
  std::cout << msg;
  logfile << msg;
  
  uint32_t vals[3];
  for (int i = 0; i < 3; i++) {
    vals[i] = module.amc13->read(amc13::AMC13::T2,0x7);
    logfile << "T2: 0x" << std::hex << 0x7 << ": 0x" << vals[i] << "\n";
  }
  if (check(vals,VAL,0)) {
    msg = "T2 AMC Link Error Counter is zero!\n";
    std::cout << msg;
    logfile << msg;
  } else if (check(vals,SAME,0)) {
    msg =  "T2 AMC Link Error detected, but counter is not changing.\n";
    std::cout << msg;
    logfile << msg;
      } else {
    msg = "ERROR: T2 AMC Link Error Counter is changing, it should be zero, at least not changing...\n";
    std::cerr << msg;
    logfile << msg;
    prompt();
  }
  std::cout << "\n";
  logfile << "\n";
  
  //check3Vals(amc13::Module& mod, Expecation e,amc13::AMC13::Board t, uint32_t addr,const std::string& title, const std::ofstream& file)
  if (!check3Vals(module,DIFF,amc13::AMC13::T2, 0x4, "T2 to AMC Data",logfile)) {
    prompt();
  }
  
  if (!check3Vals(module,DIFF,amc13::AMC13::T2, 0x5, "AMC to T2 Data",logfile)) {
    prompt();
  }
  
  std::cout << "Checking if T1 is ready...\n";
  uint32_t t1_ready = module.amc13->read(amc13::AMC13::T2,"STATUS.TTC.LAST_ORN") & 1;
  if (t1_ready == 1) {
    msg = "T1 is ready!\n";
    std::cout << msg;
    logfile << msg;
  } else {
    msg = "T1 is not ready...\n";
    std::cerr << msg;
    logfile << msg;
    prompt();
  }
  std::cout << "\n";
  logfile << "\n";
  
  //T1 READS BEGIN HERE
  if (!check3Vals(module,DIFF,amc13::AMC13::T1, 0x1f, "TTC Counter",logfile)) {
    prompt();
  }
  
  std::cout << "Enabling memory test\n\n";
  logfile << "Enabling memory test\n";
  module.amc13->write(amc13::AMC13::T1, 0xf, 020000); //Enable memory test
  logfile << "Writing to T1 register 0x" << std::hex << 0xf << " : 0" << std::oct << 020000 << "\n"; 
  
  if (!check3Vals(module,DIFF,amc13::AMC13::T1,0xa, "Memory Status",logfile)) {
    prompt();
  }
  
  msg = "Checking AMC Counters...\nChanging AMC_RxSel and AMC_TxSel to different values...\n\n";
  std::cout << msg;
  logfile << msg;
  module.amc13->write(amc13::AMC13::T1, 0xf, 020012);
  logfile << "Writing to T1 register 0x" << std::hex << 0xf << " : 0"<< std::oct << 020012 << "\n";
  
  for (uint32_t i = 0x10; i < 0x1c; i++) {
    std::ostringstream stream;
    stream << "AMC" << i - 0xf << " counter";
    if (!check3Vals(module,DIFF,amc13::AMC13::T1, i,stream.str(),logfile)) {
      prompt();
    }
  }
  
  msg = "Changing AMC_RxSel and AMC_TxSel to identical values...\n";
  std::cout << msg <<"\n";
  logfile << msg;
  module.amc13->write(amc13::AMC13::T1, 0xf, 020011);
  logfile << "Writing to T1 register 0x" << std::hex << 0xf << " : 0"<< std::oct << 020011 << "\n";
  
  for (uint32_t i = 0x10; i < 0x1c; i++) {
    std::ostringstream stream;
    stream << "AMC" << i - 0xf << " counter";
    if (!check3Vals(module,SAME,amc13::AMC13::T1, i,stream.str(),logfile)) {
      prompt();
    }
  }
  
  msg = "Changing SFP_RxSel and SFP_TxSel to identical values...\n";
  std::cout << msg  << "\n";
  logfile << msg;
  module.amc13->write(amc13::AMC13::T1, 0xf, 021211);
  logfile << "Writing to T1 register 0x" << std::hex << 0xf << " : 0"<< std::oct << 021211 << "\n";
  for (uint32_t i = 0x1c; i < 0x1f; i++) {
    std::ostringstream stream;
    stream << "SFP" << i - 0x1b << " counter";
    if (!check3Vals(module,DIFF,amc13::AMC13::T1, i , stream.str(),logfile)) {
      prompt();
    }
  }
  
  module.amc13->write(amc13::AMC13::T1, 0xf, 010000); //Reset counters
  logfile << "Writing to T1 register 0x" << std::hex << 0xf << " : 0"<< std::oct << 010000 << "\n";
  std::cout << "=============Memory Test Done=============\n\n";
  logfile   << "=============Memory Test Done=============\n\n";
}

//Runs the AMC13ToolFlash program to program the flash
void flashProgramming(int sn, const std::string& addressTable, std::ofstream& logfile) {
  char formatString[100];
  sprintf(formatString,"AMC13ToolFlash.exe -c %s -f %s", getAddress(sn,amc13::AMC13::T2).c_str(),"%s");
  char flashCommands[4][100];
  sprintf(flashCommands[0],formatString,"pfh");                    //Program Flash Header
  sprintf(flashCommands[1],formatString,"pbs");                    //Program Spartan Backup
  sprintf(flashCommands[2],formatString,"ps");                     //Program Spartan
  sprintf(flashCommands[3],formatString,"pk");                     //Program Virtex/Kintex
  //Flash programming
  for (int i = 0; i < 4; i++) {
    std::cout << "\n";
    system(flashCommands[i]);
    std::cout << "\n";
  }
  std::cout << "\n";
  std::cout << "Please power cycle the AMC13 and wait 20 seconds.\n";
  prompt();
  sleep(20);
  std::cout << "\n\n";
  std::cout << "================Firmware Programming complete==================\n\n";
  amc13::Module module;
  module.Connect(getFileName(sn),addressTable);
  logfile << "T1 Firmware: " << std::hex <<module.amc13->read(amc13::AMC13::T1,"STATUS.FIRMWARE_VERS") << "\n";
  logfile << "T2 Firmware: " << std::hex <<module.amc13->read(amc13::AMC13::T2,"STATUS.FIRMWARE_VERS") << "\n";
}

void L1ATest(int sn, const std::string& addressTable, std::ofstream& logfile) { 
  //=======================L1A test=====================
  std::string msg("");
  amc13::Module module;
  module.Connect(getFileName(sn),addressTable);
  //initialize the amc
  try {
    module.amc13->endRun();
    msg = "Enabling fake data\n";
    std::cout << msg;
    logfile << msg;
    module.amc13->fakeDataEnable(true);              //Enable Fake Data
    msg = "Enabling TTC loop back\n";
    std::cout << msg;
    logfile << msg;
    module.amc13->localTtcSignalEnable(true);        //Enable TTS as TTC Loopback
    msg = "Disabling monitor buffer back pressure\n";
    std::cout << msg;
    logfile << msg;
    module.amc13->monBufBackPressEnable(false);      //Disable Monitor Buffer Backpressure
    msg = "Enabling AMCs 1-12\n\n";
    std::cout << msg;
    logfile << msg;
    module.amc13->AMCInputEnable(0xfff);             //Enable all AMCs
    module.amc13->startRun();
    module.amc13->configureLocalL1A(true,0,1,1,0);     //Configure local L1A. mode = O, rate = 1, burst = 1, rules = 0
  } catch (uhal::exception::exception& e) {
    std::cerr << "Exception was thrown while setting up the local triggers\n";
    logfile << "Exception was thrown while setting up the local triggers\n";
    throw;
  }
  std::cout << "Sending 5 local triggers\n";
  logfile << "Sending 5 local triggers\n";
  for (int i = 0 ; i <= 5; i++) {
    std::cout << "Trigger: " << i << "\n";
    logfile << "Trigger: " << i << "\n";
    module.amc13->sendL1ABurst();                    //Send the L1As
  }
  
  uint32_t L1ACount = module.amc13->read(amc13::AMC13::T1,"STATUS.GENERAL.L1A_COUNT_LO");
  logfile << "5 Triggers sent, " << L1ACount << " L1As recieved\n";
  if (L1ACount == 5) {
    std::cout << "\n5 L1As recieved!\n";
  } else {
    std::cout << "\n" << L1ACount << "L1As recieved...\n";
    }
  
  module.amc13->reset(amc13::AMC13::T1);
  module.amc13->resetCounters();
  //DNA Numbers
  std::cout << "Please enter the following DNA numbers into the board database:\n";
  std::cout << "T1 DNA: " << std::setfill('0') << std::setw(8) << std::hex <<module.amc13->read(amc13::AMC13::T1,"STATUS.FPGA.DNA_LO") << " " 
	    << std::hex << std::setfill('0') << std::setw(8) << module.amc13->read(amc13::AMC13::T1,"STATUS.FPGA.DNA_HI") << "\n";
  std::cout << "T2 DNA: " << std::setfill('0') << std::setw(8) << std::hex << module.amc13->read(amc13::AMC13::T2,"STATUS.FPGA.DNA_LO") << " "
	    << std::hex << std::setfill('0') << std::setw(8) << module.amc13->read(amc13::AMC13::T2,"STATUS.FPGA.DNA_HI") <<"\n";
}




int main(int argc, char** argv) {

  int sn = 0;
  std::string addressTable("");
  try {
    //Set up command line
    TCLAP::CmdLine cmd("Tool for testing and programming the AMC13 module.",' ',"AMC13 Testing Tool");
    TCLAP::ValueArg<std::string> serialNo("s",                                                  //One Char Flag
					  "serial",                                             //full flag name
					  "Serial Number of the AMC13 you wish to test.",       //description
					  true,                                                 //required
					  "",
					  "positive integer",                                   //type
					  cmd);

    TCLAP::ValueArg<std::string> addrTablePath( "p", "path", "address table path", false, "", "string", cmd);
    cmd.parse(argc,argv);
    
    //Inhibit most noise from uhal
    uhal::setLogLevelTo(uhal::Error());
    
    char* atp_env;
    if (addrTablePath.isSet() ) {
      addressTable = addrTablePath.getValue();
      std::cout << "Address Table Path " << "/" << addressTable << "/ set on command line\n";
    } else if ( (atp_env=getenv("AMC13_ADDRESS_TABLE_PATH")) != NULL) {
      addressTable = atp_env;
      std::cout << "Address Table Path " << "/" << addressTable << "/ set from AMC13_ADDRESS_TABLE_PATH\n";
    } else {
      std::cout << "No Address Table Path Specified\n";
    }

    std::string snString = serialNo.getValue();
    if (isStringUInt(snString)) {
      sn = atoi(snString.c_str());
      std::cout << "Testing AMC13 number " << sn << "\n";
    } else {
      std::cerr << "Invalid Serial No\n";
      return 1;
    }
   } catch (TCLAP::ArgException& e) {
    std::cerr << "Error " << e.error() << " for arg " << e.argId() << "\n";
    return 1;
  }

  // ensure addressTable has one trailing "/"
  if( addressTable[addressTable.size()-1] != '/')
    addressTable = addressTable + "/";
  
  makeFile(sn,addressTable);       //Create connection file
    
  //Create the log file
  time_t rawtime;
  tm* timeinfo;
  char date[100];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(date,100,"_%Y-%m-%d-%I:%M:%S",timeinfo);
  std::ostringstream logfileName;
  logfileName << "logFileSN" << sn << date << ".txt";
  std::ofstream logfile(logfileName.str().c_str());

    //==================================================Testing Begins Here=================================
  try {  
    std::cout << "Please program the MMC with AVR studio, the FPGA's with iMPACT, amd place loop-back fibers into the amc13\n";
    logfile      << "Please program the MMC with AVR studio, the FPGA's with iMPACT, amd place loop-back fibers into the amc13\n";
    std::string response("");

    prompt();
    
    std::string msg = "Would you like to skip the memory test? ";
    std::cout << msg;
    logfile   << msg; 
    std::string skipResponse("");
    std::cin >> skipResponse;
    logfile << skipResponse << "\n";
    if (skipResponse != "yes" && skipResponse != "y" ) { 
      memoryTest(sn, addressTable, logfile);
    } else {
      std::cout << "\n";
    }

    msg = "Please use an oscilloscope to look at signals at the terminals of resistors on the test stand.\n You should see 40 MHz LVDs\
 level signals on them.\n\nPlease program the T2 Spartan chip with the appropriate configuration file. ";
    std::cout << msg;
    logfile << msg << "\n";

    prompt();

    flashProgramming(sn, addressTable, logfile);
    L1ATest(sn, addressTable, logfile);
  } catch (uhal::exception::exception& e) {
    std::cerr << "Connection error: ";
    std::cerr << e.what() << "\n";
    logfile.close();
    return 1;
  } catch (amc13::Exception::exBase& e) {
    std::cerr << "AMC13 Exception caught: ";
    std::cerr << e.what();
    logfile.close();
    return 1;
  } catch (std::exception& e) {
    std::cerr << "Exception caught: ";
    std::cerr << e.what() << "\n";
    logfile.close();
    return 1;
  } catch (const char* e) {
    std::cout << e << "\n";
    logfile << e << "\n";
    logfile.close();
    return 0;
  }
  logfile.close();
}
