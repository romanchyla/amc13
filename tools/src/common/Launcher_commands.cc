#include "amc13/Launcher.hh"
#include "amc13/Flash.hh"
#include <boost/tokenizer.hpp>


namespace amc13{
  
  void Launcher::LoadCommandList()
  {
    //Very special commands that are really in the command line processor. 
    AddCommand("include",&Launcher::InvalidCommandInclude,"Read a script file"); //This command exists inside of the CLI class and should never be called.  
                                                     //InvalidCommandInclude is the least wrong thing to put here, it will never be called.
    AddCommandAlias("load","include");               //This is the only allowed alias right now and is only allowed because it is also part of the CLI class
                                                     //DO NOT ADD ANY MORE ALIASES TO include/load UNLESS THEY ARE IN THE CLI CLASS! 
                                                     //LISTEN TO THE LINE ABOVE! YES YOU! -Dan dgastler@bu.edu
    // general commands (Launcher_commands)
    AddCommand("help",&Launcher::Help,"List commands. 'help <command>' for additional info and usage",&Launcher::autoComplete_Help);
    AddCommandAlias( "h", "help");
    AddCommand("quit",&Launcher::Quit,"Close program");
    AddCommandAlias("q", "quit");
    AddCommand("exit",&Launcher::Quit,"Close program");
    AddCommand("echo",&Launcher::Echo,
	       "Echo to screen\n" \
	       "  Usage:\n" \
	       "  echo <text>");
    AddCommand("sleep",&Launcher::Sleep,
	       "Delay in seconds \n" \
	       "  Usage:\n" \
	       "  sleep <integer number of seconds to be delayed>");
    
    // manage AMC13 connections (Launcher_commands_control)
    AddCommand("connect",     &Launcher::AMC13Connect, 
	       "Connect to amc13 module\n" \
	       "  Usage:\n" \
       "  connect <AMC13 connection file>,\n" \
	       "  connect <T2 IP address>,\n" \
	       "  connect <T2 Serial Number>\n");
    AddCommand("list",        &Launcher::AMC13List,    "List connected amc13s and firmware versions");
    AddCommandAlias( "fv", "list");
    AddCommand("selectAMC13", &Launcher::AMC13Select,  
	       "Select active amc13 \n"	\
	       "  Usage:\n" \
	       "  selectAMC13 <AMC13 number from list> \n");
    AddCommandAlias("sel", "selectAMC13");
    
    // basic read and write (Launcher_commands_io)
    AddCommand("readT1",      &Launcher::AMC13ReadT1,  
	       "Read from AMC13 T1 board\n" \
	       "  Usage:\n" \
	       "  readT1 <addr> [<count>] [<flags>]\n" \
	       "    flags:  D - read double-words (64 bits)\n" \
	       "            N - display only non-zero values",
	       &Launcher::autoComplete_T1AddressTable);
    AddCommandAlias( "rv", "readT1");
    
    AddCommand("readT2",      &Launcher::AMC13ReadT2,  
	       "Read from AMC13 T2 board\n"\
	       "  Usage:\n" \
	       "  readT2 <addr> [<count>] [<flags>]\n" \
	       "    flags:  D - read double-words (64 bits)\n" \
	       "            N - display only non-zero values",
	       &Launcher::autoComplete_T2AddressTable);
    AddCommandAlias( "rs", "readT2");
    
    AddCommand("writeT1",      &Launcher::AMC13WriteT1,  
	       "Write from AMC13 T1 board\n" \
	       "  Usage:\n" \
	       "  writeT1 <T2 address to be written to>",
	       &Launcher::autoComplete_T1AddressTable);
    AddCommandAlias( "wv", "writeT1");
    
    AddCommand("writeT2",      &Launcher::AMC13WriteT2,  
	       "Write from AMC13 T2 board\n" \
	       "  Usage:\n" \
	       "  writeT2 <T2 address to be written to>",
	       &Launcher::autoComplete_T2AddressTable);
    AddCommandAlias( "ws", "writeT2");
    
    // control functions (Launcher_commands_control)
    AddCommand("i",           &Launcher::AMC13Initialize,
	       "Initialize AMC13 and TTC distribution\n" \
	       "  Usage:\n"	    \
	       "  i <inputs> <options>\n"	   \
	       "    inputs:   list of inputs e.g. '1-12'\n" \
	       "              '*' enables all inputs which show AMC link ready\n" \
	       "    options:  T to enable loop-back TTC on TTS output fiber\n" \
	       "              F to generate fake events in AMC13\n" \
	       "              B to enable monitor buffer backpressure (stops EvB when full)\n" \
	       "              N to leave AMC13 out of run mode after initialization");
    AddCommandAlias("en", "i");

    AddCommand("daq",      &Launcher::AMC13ConfigDAQ,
	       "Enable DAQ outputs\n" \
	       "  Usage:\n" \
	       "  daq 0|1|2|3|d (L)\n" \
	       "      1  - enable SFP0 (top) DAQ fiber for AMC1-AMC12 readout\n" \
	       "      2  - enable SFP0 for AMC1-AMC6, SFP1 for AMC7-AMC12\n" \
	       "      3  - enable SFP0 for AMC1-AMC4, SFP1 for AMC5-AMC8, SFP2 for AMC9-AMC12\n" \
	       "     0|d - disable all fibers\n" \
	       "      L  - enable local mode (optional)\n");

    AddCommand("localL1A", &Launcher::AMC13ConfigL1A,
	       "Configure local L1A generator\n" \
	       " Usage:\n" \
	       " localL1A <options> <burst> <rate>\n" \
	       " options:  o specifies one trigger per <rate> orbits\n" \
	       "           b specifies one trigger per <rate> BX\n" \
	       "           r specifies random triggers in <rate> Hz\n" \
	       "           d disable local L1A\n" \
	       "   burst:  number of triggers per burst in single-burst mode\n" \
	       "    rate:  number of orbits or BX between triggers or random rate in 2Hz units");
    
    AddCommand("setOcrCommand", &Launcher::AMC13SetOcrCommand,
	       "Choose TTC broadcast command to use for orbit count reset\n" \
	       " Usage:\n" \
	       " setOcrCommand <cmd> [<mask>]\n" \
	       "Command must be 0-255 with low bit 0" \
	       "Optional mask specifies bits to ignore in command match\n");

    AddCommand("setResyncCommand", &Launcher::AMC13SetResyncCommand,
	       "Choose TTC broadcast command to use for resync operation\n" \
	       " Usage:\n" \
	       " setResyncCommand <cmd> [<mask>]\n" \
	       "Command must be 0-255 with low bit 0" \
	       "Optional mask specifies bits to ignore in command match\n");

    AddCommand("setOrbitGap", &Launcher::AMC13SetOrbitGap,
	       "Set begin and end of orbit gap for locally-generated triggers\n" \
	       " Usage:\n" \
	       " setOrbitGap <begin> <end>\n" \
	       " <begin>    BX for start of gap (not included) range 0-3562\n" \
	       " <end>      BX for end of gap (included) range 1-3563\n" \
	       "L1A generated by e.g. the internal random trigger generator will not\n" \
	       "occur within this BX range");

    AddCommand("prescale", &Launcher::AMC13Prescale,
	       "set prescale mode and factor\n" \
	       " Usage:\n" \
	       " prescale <mode> <factor>\n" \
	       "   mode:  0 for simple prescale (record every n events, n from 1...0x10000)\n" \
	       "          1 to match EvN with n low bits =0 where n from 5..20\n" \
	       " factor:  if mode=0, 1...0x10000\n" \
	       "          if mode=1, 5..20");

    AddCommand("lt",     &Launcher::AMC13LocalTrig,
	       "Send/enable/disable local triggers\n"	\
	       "  lt <num> [<delay>]    send <num> local trigger (bursts) in software loop\n" \
	       "                        delay <delay> ms between triggers (default 1)\n" \
	       "  lt e                  enable trigger generator\n" \
	       "  lt d                  disable trigger generator\n" \
	       "  lt c                  enable continuous mode (configure with localL1A)\n"
	       );

    AddCommand("rg",     &Launcher::AMC13ResetGeneral,
	       "General reset\n" \
	       "  For firmwares before 0x20f/0x4006 this resets DAQ links too\n" \
	       "  After this version, use 'rd' to reset DAQ separately\n"
	       );

    AddCommand("rc",     &Launcher::AMC13ResetCounters,
	       "Counter reset\n");

    AddCommand("rd",     &Launcher::AMC13ResetDAQ,
	       "DAQ Link reset (firmware 0x20f/0x4006 and above)\n" \
	       "  after issuing this command the link must be recofigured\n" \
	       "  from the receiving end\n");
    
    AddCommand("start",  &Launcher::AMC13Start, "Enable run mode");
    AddCommand("stop",   &Launcher::AMC13Stop, "Disable run mode");
    
    AddCommand("nodes", &Launcher::ListNodes, "List address table nodes -- list {T1|T2} <regex> <options>\n" \
	       "   <regex>    understands '*' to match any string, e.g. STATUS.TTC.*\n" \
	       "   <options>  'v' to display long descriptions for items\n" \
 	       "              'd' for debug dump of all address table parameters\n");

    AddCommand("re_old", &Launcher::AMC13ReadEvent, "Read next event from SDRAM");
    AddCommand("rev_old", &Launcher::AMC13ReadEventVector, "Read next event from SDRAM using new vector interface");
    
    AddCommand("readEvent", &Launcher::AMC13ReadEventMultiFED, "Read next event from SDRAM with multiple DAQ channels/Event Builders/FEDs\n" \
	       "usage:\n" \
	       "  re                 - prints event size, header, and trailer for each active DAQ channel \n" \
	       "  re all             - prints all the 64 bit words from the event \n" );
    AddCommandAlias("re", "readEvent");

    AddCommand("dfMulti", &Launcher::AMC13DumpMultiFEDEvent, "Read events and write to binary file -- df file(s) [count]\n" \
	       "number of file names must match number of active SFP channels\n");

    AddCommand("df", &Launcher::AMC13DumpEvent, "Read events and write to binary file -- df file [count]\n" \
	       "In normal (non-over-write mode):\n" \
	       "  df <filename> [event_count]\n\n" \
	       "In overwrite mode (CONF.EVB.MON_FULL_OVERWRITE = 1):\n" \
	       "  df <filename> event_count [max_size]]\n" \
	       "    event_count is number of blocks up to the last one\n" \
	       "    max_size is number of bytes to dump for each block (default 0x200000)\n");

    AddCommand("bgo", &Launcher::AMC13ConfigBGO, "Configure TTC command generator (BGO channels)\n" \
	       "usage:\n" \
	       "  bgo fire              - trigger all 'single' enabled BGOs once\n" \
	       "  bgo list              - show current settings\n" \
	       "  bgo <chan> short|long <cmd> <bcn> <prescale> <repeat>\n");

    AddCommand("ttc", &Launcher::AMC13TTCHistory, "Configure/read TTC history capture on T2\n" \
	       "Usage:\n" \
	       "  ttc h on                 - ttc history enable\n" \
	       "  ttc h off                - ttc history disable\n" \
	       "  ttc h clr                - ttc history clear\n" \
	       "  ttc h d <num>            - ttc history display <num> items or all\n" \
	       "  ttc f on                 - ttc filter enable\n" \
	       "  ttc f off                - ttc filter disable\n" \
	        "  ttc f clr                - ttc filter clear\n" \
	       "  ttc f s <n> <cmd> <mask> - set TTC filter list item\n" \
	       "          <n> is item 0-15 to set\n" \
	       "          <cmd>  is value to match for filtering commands\n" \
	       "          <mask> is bits to ignore when filtering commands\n" \
	       "  ttc f list               - list currently-defined filters\n" \
	       "  ttc f ena <n>            - enable specific filter by number\n" \
	       "  ttc f dis <n>            - disable specific filter by number\n");

    AddCommand("l1a", &Launcher::AMC13L1AHistory, "Read recent L1A history\n" \
	       "Usage:\n" \
	       " l1a [count]               - display last [count] L1A\n" \
	       "\n" \
	       " C - HCAL calibration code\n" \
	       " T - TTS state (8=RDY 4=BSY 2=SYN 1=OFW)\n" \
	       " Y - Laser setting?\n" \
	       " G - Laser calibration trigger\n" \
	       " P - Pedestal calibration trigger\n" \
	       " W - trigger in calibration gap window range\n" \
	       " A - Accepted calibration trigger (laser or pedestal) in window\n" \
	       " E - Event type (1=normal 2=calibration)\n"
	       );

    //Calibration Trigger commands
    AddCommand("cal",&Launcher::AMC13CalTriggerWindow, "Configure calibration events\n" \
	       "Usage:\n"\
	       "  cal <on|off> <lowerLimit> <upperLimit>;");

    // flash commands (Launcher_commands_flash)

    AddCommand("reconfigureFPGAs", &Launcher::AMC13ReloadFPGAs, "reconfigure FPGAs from flash");


    AddCommand("vfh", &Launcher::AMC13VerifyFlashHeader, 
	       "verify flash header\n" \
	       "Usage:\n" \
	       "  vfh OR vfh <chip_type>\n" \
	       "  chip_type:    specifies firmware file suffix denoting T2 chip type");
    AddCommand("vbs", &Launcher::AMC13VerifyFlashGolden, 
	       "verify flash backup spartan (golden) \n" \
	       "Usage:\n" \
	       "  vbs OR vbs <chip_type>\n" \
	       "  chip_type:    specifies firmware file suffix denoting T2 chip type");
    AddCommand("vs", &Launcher::AMC13VerifyFlashT2, 
	       "verify flash spartan\n" \
	       "Usage:\n" \
	       "  vs OR vs <chip_type>\n" \
 	       "  chip_type:    specifies firmware file suffix denoting T2 chip type");
    AddCommand("vv", &Launcher::AMC13VerifyFlashT1, 
	       "verify flash virtex/kintex\n" \
	       "Usage:\n" \
	       "  vv OR vv <chip_type>\n" \
	       "  chip_type:    specifies firmware file suffix denoting T2 chip type");
    AddCommandAlias( "vk", "vv");

    AddCommand("pfh", &Launcher::AMC13ProgramFlashHeader, 
	       "program flash header\n" \
	       "Usage:\n" \
	       "  pfh OR pfh <chip_type>\n" \
	       "  chip_type:    specifies firmware file suffix denoting T2 chip type");   
    AddCommand("pbs", &Launcher::AMC13ProgramFlashGolden, 
	       "program flash backup spartan (golden)\n" \
	       "Usage:\n" \
	       "  pbs OR pbs <chip_type>\n" \
	       "  chip_type:    specifies firmware file suffix denoting T2 chip type");
    AddCommand("ps", &Launcher::AMC13ProgramFlashT2, 
	       "program flash spartan\n" \
	       "Usage:\n" \
	       "  ps OR ps <chip_type>\n" \
	       "  chip_type:    specifies firmware file suffix denoting T2 chip type");
    AddCommand("pv", &Launcher::AMC13ProgramFlashT1, 
	       "program flash virtex/kintex\n" \
	       "Usage:\n" \
	       "  pv OR pv <chip_type>\n" \
	       "  chip_type:    specifies firmware file suffix denoting T2 chip type");
    AddCommandAlias( "pk", "pv");

    AddCommand("id", &Launcher::AMC13SetID, 
	       "Set AMC13 IDs (OBSOLETE! Use \"fed\" and \"slink\" for multiple DAQ-links.)\n"		\
	       "usage:\n" \
	       "  id [fed <fed_id>] [slink <slink_id>]\n" \
	       "    <fed_id> is 12-bit FED number sent in event header\n" \
	       "    <slink_id> is 16-bit physical DAQ link ID (low 2 bits set by AMC13)\n");
    AddCommand("fed",&Launcher::AMC13FedID,
	       "Set an AMC13's DAQ-link's fed ID" \
	       "usage:\n" \
	       "  fed <DAQ-link> <fed_id> \n"\
	       "    <DAQ-link> is 0,1,2 \n"\
	       "    <fed_id> is a 12-bit FED number send in each event header\n");
    AddCommand("slink",&Launcher::AMC13SlinkID,
	       "Set AMC13 Slink ID" \
	       "usage:\n" \
	       "  slink <slink_id> \n"\
	       "    <slink_id> is 16-bit physical DAQ link ID (low 2 bits set by AMC13)\n");
    

    // commands under development
    AddCommand("status", &Launcher::Status, "Display AMC13 Status\n"\
	       "usage:\n"\
	       "status <level> table_name\n"\
	       "  level from 1..9 with 1 being least verbose (99 displays everything)\n" \
	       "  table_name limits the display to the table named\n",
 	       &Launcher::autoComplete_StatusTable);
    AddCommandAlias("st","status");

    AddCommand("statusHTML", &Launcher::StatusHTML, "Display AMC13 status in basic HTML format\n" \
	       "usage:\n"\
	       "statusHTML <level> table_name\n"\
	       "  level from 1..9 with 1 being least verbose (99 displays everything)\n" \
	       "  table_name limits the display to the table named\n",	       
	       &Launcher::autoComplete_StatusTable);

    AddCommand("openStatusFile", &Launcher::OpenFile, "Open a text file to stream status info to\n");
    AddCommand("closeStatusFile", &Launcher::CloseFile, "Close the text file\n"); 

    AddCommand("printFlash", &Launcher::AMC13PrintFlash,
	       "EXPERT ONLY\n" \
	       "print flash data for testing only");
    AddCommand("verifyFlash", &Launcher::AMC13VerifyFlash, "EXPERT ONLY\n" \
	       "verify flash data from file to specified address\n" \
	       "Usage:\n" \
	       "  verifyFlash <file> <address>\n" \
	       "  file:      firmware file to verify against\n" \
	       "  address:   starting address of relavent flash sector to verify");
    AddCommand("programFlash", &Launcher::AMC13ProgramFlash, "EXPERT ONLY\n" \
	       "program flash data from file to specified address\n" \
	       "Usage:\n" \
	       "  programFlash <file> <address>\n" \
	       "  file:      firmware file to program in\n" \
	       "  address:   starting address of relavent flash sector to program");

    AddCommand("verifyFlashFile", &Launcher::AMC13VerifyFlashFile,  "EXPERT ONLY\n" \
	       "verify flash data from file\n" \
	       "Usage:\n" \
	       "  verifyFlash <file>\n"
	       "  file:     firmware file to verify against\n"
	       "Note: address of flash sector to verify determined from file name");
    AddCommand("programFlashFile", &Launcher::AMC13ProgramFlashFile,  "EXPERT ONLY\n" \
	       "program flash data from file\n" \
	       "Usage:\n" \
	       "  programFlash <file>\n"
	       "  file:     firmware file to verify against\n"
	       "Note: address of flash sector to program determined from file name");
    AddCommand("dump", &Launcher::MrWuRegisterDump, "Dump all the registers in Mr. Wu's format \n" \
	       "Usage:\n" \
	       "  dump (<file>) (<repetitions>) (<wait time>) (<statusLevel>)\n" \
	       "    file: (optional) .txt file to store dumped register data\n" \
	       "      (default file is AMC13XGDump_[year]-[month]-[day]_[hour][minute][second].txt)\n" \
	       "    repetitions: (optional) number of times to dump the registers\n" \
	       "      (default number of repetitions is 2)\n" \
	       "    wait time: (optional) the amount of time between repetitions\n" \
	       "      (default wait time is 2 seconds)\n" \
               "    statusLevel: (optional) sets the verbosity of the status command\n" \
               "      (default statusLevel = 9)\n");
    AddCommandAlias("wu", "dump");
    AddCommand("SFP", &Launcher::SFPDump, "Dump info on an SFP\n" \
	       "Usage:\n"\
	       "  SFP (<SFP#>)\n"\
	       "   SFP# (0-3) default 0\n");

    // AddCommand("selectFileTest", &Launcher::AMC13SelectFileTest, "test function for MCS file parsing -- selectFileTest file");

  }

  int Launcher::InvalidCommandInclude(std::vector<std::string>,std::vector<uint64_t>)
  {
    return 0;
  }
  
  int Launcher::Quit(std::vector<std::string>,
		     std::vector<uint64_t>)
  {
    //Quit CLI so return -1
    return -1;
  }
  
  int Launcher::Echo(std::vector<std::string> strArg,
		     std::vector<uint64_t> intArg) {
    for(size_t iArg = 0; iArg < strArg.size();iArg++)
      {
	printf("%s ",strArg[iArg].c_str());
      }
    printf("\n");
    return 0;
  }

  int Launcher::Help(std::vector<std::string> strArg,
		     std::vector<uint64_t> intArg) 
  {
    //Check the arguments to see if we are looking for
    //  simple help:    all commands listed with 1-line help
    //  specific help:  full help for one command
    //  full help:      full help for all commands
    bool fullPrint = false;
    int thisCommand = -1;
    if(strArg.size() > 0){      
      if(strArg[0][0] == '*'){
	//Check if we want a full print and skip to the end if we do
	fullPrint = true;
      } else {
	//We want a per command help
	int cmd = FindCommand(strArg[0]);	
	if(cmd >=0){
	  //Store this command ptr for the search through the command vector
	  thisCommand = cmd;
	  fullPrint = true;
	}
      }
    }

    //Build a map of commands to the CLI names that are connected to them.
    //The order of the vector will be the same as the order of the 
    //If only one command is desired, only save command names that point to the same function
    std::map<size_t,std::vector<size_t> > helpMap;
    for(size_t iEntry = 0; iEntry < commandPtr.size();iEntry++){
      //Find the first instance of this actual function
      size_t baseCommand = iEntry;
      for(size_t iSearch = 0; iSearch < baseCommand;iSearch++){
	if(commandPtr[iSearch] == commandPtr[baseCommand]){
	  baseCommand = iSearch; //This effectively breaks out of this loop
	}
      }
      if(thisCommand == -1){ //for generat help
	helpMap[baseCommand].push_back(iEntry);
      }else if(size_t(thisCommand) == iEntry){ //for help with only one command
	helpMap[thisCommand].push_back(iEntry);
      }
    }
    
    //Print all of our command helps
    typedef std::map<size_t,std::vector<size_t> >::iterator itHelp;    
    for( itHelp itCommand = helpMap.begin();
	 itCommand != helpMap.end();
	 itCommand++){
      //Build a list of main name and aliases. 
      std::string nameString(commandName[itCommand->first]);      
      for(size_t iSubName = 1;iSubName < itCommand->second.size();iSubName++){
	if(iSubName == 1){
	  nameString+=" (";
	}
	nameString += commandName[itCommand->second[iSubName]];	
	if(iSubName != (itCommand->second.size() -1)){
	  nameString += ",";
	}else{
	  nameString += ")";
	}
      }

      //print the help for this command
      std::string & helpString = commandHelp[itCommand->second[0]];
      if(!fullPrint && helpString.find('\n')){ 
	printf(" %-20s:   %s\n",nameString.c_str(),
	       helpString.substr(0,helpString.find('\n')).c_str());
      } else { // Print full help
	printf(" %-20s:   ",nameString.c_str());
	boost::char_separator<char> sep("\n");
	boost::tokenizer<boost::char_separator<char> > tokens(helpString, sep);
	boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin();
	if(it != tokens.end()){
	  printf("%s\n",(*it).c_str());
	  it++;
	}
	for ( ;it != tokens.end();++it){	    
	  printf("                         %s\n",(*it).c_str());
	}
      }
      
    }
    return 0;
  }


std::string Launcher::autoComplete_Help(std::vector<std::string> const & line,std::string const & currentToken ,int state)
{  
  if(line.size() > 0){
    if((line.size() > 1) && (currentToken.size() == 0)){
	return std::string("");
      }
    static size_t pos;
    if(!state) {
      //Check if we are just starting out
      pos = 0;
    } else {
      //move forward in pos
      pos++;
    }
    for(;pos < commandName.size();pos++)
      {
	if(commandName[pos].find(currentToken) == 0){
	  return commandName[pos];
	}
      }
  }
  //not found
  return std::string("");  
}

 
  int Launcher::Sleep(std::vector<std::string> strArg,
		     std::vector<uint64_t> intArg) {
    if( strArg.size() < 1) {
      printf("Need a delay time in seconds\n");
    } else {
      double s_time = strtod( strArg[0].c_str(), NULL);
      if( s_time > 5.0) {
	sleep( intArg[0]);
      } else {
	usleep( (useconds_t)(s_time * 1e6) );
      }
    }
    return 0;
  }

}

