
#include "amc13/Launcher.hh"
#include "amc13/Flash.hh"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
// for usleep() in local trigger gen
#include <unistd.h>

// split a string on a delimiter
//
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

namespace amc13{
    
  int Launcher::AMC13Connect(std::vector<std::string> strArg,
			     std::vector<uint64_t> intArg)
  {
    //Check for a filename
    if(strArg.size() ==  0) {
      printf("AMC13Connect: Missing connection file.\n");
      return 0;
    }
    
    //create AMC13 module
    Module* mod = new Module();
    if(strArg.size() > 1) {
      mod->Connect( strArg[0], addressTablePath, strArg[1]);
    } 
    else{
      mod->Connect( strArg[0], addressTablePath);
    }
    AMCModule.push_back(mod);    

    return 0;
  }
  
  int Launcher::AMC13List(std::vector<std::string> strArg,
			  std::vector<uint64_t> intArg) {
    printf("Connected AMC13s\n");
    for(size_t iAMC = 0; iAMC < AMCModule.size();iAMC++) {
      printf("%c%zu: %s\n", (iAMC == defaultAMC13no) ? '*' : ' ',
	     iAMC, AMCModule[iAMC]->Show().c_str() );
    }
    return 0;
  }
  
  int Launcher::AMC13Select(std::vector<std::string> strArg,
			    std::vector<uint64_t> intArg) {
    if(intArg.size() == 0) {
      printf("AMC13Select: Missing AMC13 number\n");
      return 0;
    }
    if(intArg[0] >= AMCModule.size()) {
      printf("AMC13Select: Bad AMC13 number\n");
      return 0;
    }
    defaultAMC13no = intArg[0];
    printf("Setting default AMC13 to %zu\n",defaultAMC13no);
    return 0;
  }
  
  //
  // initialize with list of inputs and other options
  //
  int Launcher::AMC13Initialize(std::vector<std::string> strArg,
				std::vector<uint64_t> intArg) {
    
    uint32_t amcMask = 0;
    AMC13* amc13 = defaultAMC13(); // for convenience
    
    if( strArg.size() ==0) {
      printf("usage: i <inputs> <options>\n");
      return 0;
    }
      
    // keep track of the selected options
    bool fakeTTC = false;	// "T" option for loop-back TTC
    bool fakeData = false;	// "F" for fake data
    bool monBufBack = false;    // "B" for monitor buffer backpressure
    bool runMode = true;	// "N" to disable run mode

    for( size_t i=0; i<strArg.size(); i++) {
	
      // first character for categorizing the arguments
      char fchar = strArg[i].c_str()[0];

      // check if it starts with a digit
      if( isdigit( fchar) ) {
	amcMask = amc13->parseInputEnableList( strArg[i], true); // use 1-based numbering
	printf("parsed list \"%s\" as mask 0x%x\n", strArg[i].c_str(), amcMask);
      } else if( fchar == '*') {    // special case
	// read mask
	amcMask = amc13->read( AMC13Simple::T1, "STATUS.AMC_LINK_READY_MASK");
	printf("Generated mask 0x%03x from STATUS.AMC_LINK_READY_MASK\n", amcMask);
      } else if( isalpha( fchar) && (strArg[i].size() == 1)) {			// it's a letter
	// should be single-letter options
	switch( toupper(strArg[i].c_str()[0])) {
	case'T':		// TTS outputs TTC
	  printf("Enabling TTS as TTC for loop-back\n");
	  fakeTTC = true;
	  break;
	case 'F':		// fake data
	  printf("Enabling fake data\n");
	  fakeData = true;
	  break;
	case 'B':		// monitor buffer backpressure
	  printf("Enabling monitor buffer backpressure, EvB will stop when MB full\n");
	  monBufBack = true;
	  break;
	case 'N':		// no run mode
	  printf("Disable run mode\n");
	  runMode = false;
	  break;
	default:
	  printf("Error: Unknown option: %s. Not initializing AMC13!\n", strArg[i].c_str());
	  return 0;
	}
      } else {
	  printf("Error: Unknown option: %s. Not initializing AMC13!\n", strArg[i].c_str());
	  return 0;
      }
    }

    // Must have at least one AMC enabled for each DAQ channel.
    // We can check this with a bitwise and, since the the first 12 bits of amcMask
    // simply represent if an AMC is enabled or not. Leaving the number to & with
    // in binary is a better visual representation than using a hex number.
    switch ( defaultAMC13()->read(AMC13::T1, "CONF.SFP.ENABLE_MASK" ) ) {
      case 0:
      case 1:
	if ( amcMask > 0 ) {
	  break;
	}
	else {
	  printf("Must enable at least one AMC1-AMC12 slot\n");
	  return 0;
	}
      case 3:
	if ( ((amcMask & 0b000000111111) > 0) && ((amcMask & 0b111111000000) > 0) ) {
	  break;
	}
	else {
	  printf("2 DAQ channels: must enable at least one AMC1-AMC6 and one AMC7-AMC12\n");
	  return 0;
	}
      case 7:
	if ( ((amcMask & 0b000000001111) > 0) && ((amcMask & 0b000011110000) > 0) && ((amcMask & 0b111100000000) > 0) ) {
	  break;
	}
	else {
	  printf("3 DAQ channels: must enable at least one from each AMC1-AMC4, AMC5-AMC8, AMC9-AMC12\n");
	  return 0;
	}
      default:
	amc13::Exception::UnexpectedRange e;
	e.Append("Error in AMC13Initialize - CONF.SFP.ENABLE_MASK not 0, 1, 3, or 7\n");
	throw e;
    }
      
    try {
	
      // initialize the AMC13
      amc13->endRun();		// take out of run mode
      printf("AMC13 out of run mode\n");
      amc13->fakeDataEnable( fakeData);
      amc13->localTtcSignalEnable( fakeTTC);
      amc13->monBufBackPressEnable(monBufBack);
      amc13->AMCInputEnable( amcMask);
      if( runMode) {
	amc13->startRun();
	printf("AMC13 is back in run mode and ready\n");
      } else {
	printf("AMC13 is *not* in run mode.  Use \"start\" to start run\n");
      }
    } catch (uhal::exception::exception & e) {
      printf("Argh!  Something threw in the control functions\n");
    }
      
    return 0;
  }

  int Launcher::AMC13ConfigDAQ(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    if( strArg.size() == 0 ) {
      printf("Need link count 0/1/2/3 or d\n");
      return 0;
    }

    if( strArg.size() > 2 ) {
      printf("Only up to 2 arguments allowed, see 'help daq' for proper usage.\n");
      return 0;
    }

    if( isdigit( strArg[0].c_str()[0])) {
      switch( atoi( strArg[0].c_str())) {
      case 0:
	defaultAMC13()->sfpOutputEnable( 0);
	defaultAMC13()->daqLinkEnable( false);
	printf("DAQ outputs disabled\n");
	break;
      case 1:
	defaultAMC13()->sfpOutputEnable( 1);
	defaultAMC13()->daqLinkEnable( true);
	printf("SFP0 enabled\n");
	break;
      case 2:
	defaultAMC13()->sfpOutputEnable( 3);
	defaultAMC13()->daqLinkEnable( true);
	printf("SFP0 and SFP1 enabled\n");
	break;
      case 3:
	defaultAMC13()->sfpOutputEnable( 7);
	defaultAMC13()->daqLinkEnable( true);
	printf("SFP0-SFP2 enabled\n");
	break;
      default:
	printf("Link count must be 0-3\n");
      }
    } else {
      if( toupper(strArg[0].c_str()[0]) == 'D') {
	printf("DAQ outputs disabled\n");
	defaultAMC13()->daqLinkEnable( false);
	defaultAMC13()->sfpOutputEnable( 0);
      }
    }

    if ( strArg.size() > 1 ) {
      if ( toupper( strArg[1].c_str()[0] == 'L' ) ) {
	  defaultAMC13()->daqLinkEnable( false );
	  printf( "Local mode enabled\n" );
	}
      else {
	printf("Command not accepted, see 'help daq' for proper usage.\n");
	return 0;
      }
    }

    printf("Best to do a DAQ reset (rd) after changing link settings\n");

    return 0;
  }


  int Launcher::AMC13SetOcrCommand(std::vector<std::string> strArg,
				   std::vector<uint64_t> intArg) {
    if( intArg.size() == 2) {
      defaultAMC13()->setOcrCommand( intArg[0], intArg[1]);
    } else if( intArg.size() == 1) {
      defaultAMC13()->setOcrCommand( intArg[0]);
    } else
      printf("usage:  setOcrCommand <cmd> [<mask>]\n");
    return 0;
  }

  int Launcher::AMC13SetResyncCommand(std::vector<std::string> strArg,
				   std::vector<uint64_t> intArg) {
    if( intArg.size() == 2) {
      defaultAMC13()->setResyncCommand( intArg[0], intArg[1]);
    } else if( intArg.size() == 1) {
      defaultAMC13()->setResyncCommand( intArg[0]);
    } else
      printf("usage:  setResyncCommand <cmd> [<mask>]\n");
    return 0;
  }



  int Launcher::AMC13SetOrbitGap(std::vector<std::string> strArg,
				   std::vector<uint64_t> intArg) {
    if( intArg.size() != 2) {
      printf("Need begin and end BX for gap\n");
      return 0;
    }
    defaultAMC13()->setOrbitGap( intArg[0], intArg[1]);
    return 0;
}

  int Launcher::AMC13Prescale(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    if( intArg.size() != 2) {
      printf("need mode and factor after command\n");
      return 0;
    }
    printf("Setting prescale mode %ld factor %ld\n", intArg[0], intArg[1]);
    defaultAMC13()->configurePrescale( intArg[0], intArg[1]);
    return 0;
  }
  
  
  //
  // configure local L1A
  //
  int Launcher::AMC13ConfigL1A(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {

    bool ena = true;
    uint32_t rate = 1;
    uint32_t burst = 1;
    int mode = 0;
    int rules = 0;

    if( strArg.size() < 1) {
      printf("Need at least one option\n");
      return 0;
    }

    switch( toupper(strArg[0].c_str()[0])) {
    case 'O':
      mode = 0;
      break;
    case 'B':
      mode = 1;
      break;
    case 'R':
      mode = 2;
      break;
    case 'D':
      ena = false;
      break;
    default:
      printf("Unknown option %c... should be one of O, B, R, d\n", strArg[0].c_str()[0]);
      break;
    }

    // if only one argument supplied, it should be the rate
    // burst defaults to 1

    if( intArg.size() == 2) {
      rate= intArg[1];
      printf("burst defaulted to 1\n");
    } else if( intArg.size() == 3) {
      burst = intArg[1];
      rate = intArg[2];
    } else {
      printf("Need a mode and 1 or 2 integer arguments\n");
    }

    printf("Configure LocalL1A %s mode=%d burst=%d rate=%d rules=%d\n", ena ? "enabled" : "disabled",
	   mode, burst, rate, rules);
    defaultAMC13()->configureLocalL1A( ena, mode, burst, rate, rules);

    return 0;
  }

  //
  // send one or more local triggers
  // enable/disable trigger generator and burst mode
  //
  int Launcher::AMC13LocalTrig(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    int ntrig = 1;
    int delay_ms = 1;
    bool doTriggers = false;

    if( intArg.size() > 0) {
      // first character for categorizing the arguments
      char fchar = strArg[0].c_str()[0];
      
      if( isdigit( fchar)) {
	printf("detected number after 'lt'\n");
	ntrig = intArg[0];
	doTriggers = true;
	if( intArg.size() > 1)
	  delay_ms = intArg[1];
      } else if( toupper( fchar) == 'E') {
	printf("Enable local triggers\n");
	defaultAMC13()->enableLocalL1A( true);
      } else if( toupper( fchar) == 'D') {
	printf("Disable continuous local triggers\n");
	defaultAMC13()->stopContinuousL1A();
      } else if( toupper( fchar) == 'C') {
	printf("Enable continuous local triggers\n");
	defaultAMC13()->enableLocalL1A( true);
	defaultAMC13()->startContinuousL1A();
      }
      
    } else
      doTriggers = true;

    if( doTriggers) {
      printf("Sending %d local triggers\n", ntrig);
      while( ntrig--) {
	if( (ntrig % 100) == 0)
	  printf("Trigger: %d left\n", ntrig);
	defaultAMC13()->sendL1ABurst();
	usleep( 1000*delay_ms);
      }
    }
    return 0;
  }
  
  
  
  // start and stop AMC13
  int Launcher::AMC13Start(std::vector<std::string> strArg,
			   std::vector<uint64_t> intArg) {
    printf("AMC13 run start\n");
    defaultAMC13()->startRun();
    return 0;
  }
  
  int Launcher::AMC13Stop(std::vector<std::string> strArg,
			  std::vector<uint64_t> intArg) {
    printf("AMC13 run stop (out of run mode)\n");
    defaultAMC13()->endRun();
    return 0;
  }
  

  int Launcher::AMC13ResetGeneral(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    printf("General reset\n");
    defaultAMC13()->reset( AMC13Simple::T1);
    return 0;
  }

  int Launcher::AMC13ResetCounters(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    printf("Counter reset\n");
    defaultAMC13()->resetCounters();
    return 0;
  }

  int Launcher::AMC13ResetDAQ(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    printf("DAQ reset\n");
    defaultAMC13()->resetDAQ();
    return 0;
  }

  int Launcher::AMC13FedID(std::vector<std::string> strArg,
			   std::vector<uint64_t> intArg){
    if( strArg.size() != 2 ) {
      printf("usage: <DAQ-link> <fed_id>\n");
      return 0;
    }
    defaultAMC13()->setFEDid(intArg[0], intArg[1]);
    return 0;
  }

  int Launcher::AMC13SlinkID(std::vector<std::string> strArg,
			   std::vector<uint64_t> intArg){
    if( strArg.size() != 1 ) {
      printf("usage: <slink_id>\n");
      return 0;
    }
    defaultAMC13()->setSlinkID(intArg[0]);    
    return 0;
  }

  int Launcher::AMC13SetID(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    printf("OBSOLETE! Use \"fed\" and \"slink\" for multiple DAQ-links.\n");

    const char *usage = "usage:  id [fed <fed_id>] [slink <slink_id>]\n";
    if( !strArg.size() || (strArg.size() % 2)) {
      printf(usage);
      return 0;
    }
    for( int i=0; i < (int) strArg.size(); i+=2) {
      if( !strcasecmp( strArg[i].c_str(), "fed")){
	defaultAMC13()->setFEDid( intArg[i+1]);
      }
      else if( !strcasecmp( strArg[i].c_str(), "slink")){
	defaultAMC13()->setSlinkID( intArg[i+1]);
      }
      else{
	printf( usage);
      }
    }
    return 0;
  }

  int Launcher::ListNodes(std::vector<std::string> strArg,
			  std::vector<uint64_t> intArg) {
    std::vector<std::string> nodes;
    std::string rx;
    uhal::HwInterface * brd = NULL;    
    bool debug = false;
    bool describe = false;
    bool help = false;

    if( strArg.size() < 2) {
      printf("Need T1/T2 and regular expression after command\n");
      return 0;
    }
    
    if( strArg.size() > 2) {
      switch( toupper(strArg[2].c_str()[0])) {
      case 'D':
	debug = true;
	break;
      case 'V':
	describe = true;
	break;
      case 'H':
	help = true;
	break;
      }
    }

    if( strArg[0] == "T1" || strArg[0] == "t1") {
      brd = defaultAMC13()->getT1();
      nodes = myMatchNodes( brd, strArg[1]);
    } else if( strArg[0] == "T2" || strArg[0] == "t2") {
      brd = defaultAMC13()->getT2();      
      nodes = myMatchNodes( brd, strArg[1]);
    } else
      printf("Need T1 or T2 after command\n");
    
    int n = nodes.size();
    printf("%d nodes matched\n", n);
    if( n)
      for( int i=0; i<n; i++) {
	// get various node attributes to display
	const uhal::Node& node = brd->getNode( nodes[i]);
	uint32_t addr = node.getAddress();
	uint32_t mask = node.getMask();
	uint32_t size = node.getSize();
	uhal::defs::BlockReadWriteMode mode = node.getMode();
	uhal::defs::NodePermission perm = node.getPermission();
	std::string s = "";
	const std::string descr = node.getDescription();
	switch( mode) {
	case uhal::defs::INCREMENTAL:
	  s += " inc";
	  break;
	case uhal::defs::NON_INCREMENTAL:
	  s += " non-inc";
	  break;
	case uhal::defs::HIERARCHICAL:
	case uhal::defs::SINGLE:
	default:
	  break;
	}
	switch( perm) {
	case uhal::defs::READ:
	  s += " r";
	  break;
	case uhal::defs::WRITE:
	  s += " w";
	  break;
	case uhal::defs::READWRITE:
	  s += " rw";
	  break;
	default:
	  ;
	}
	if( size > 1) {
	  char siz[20];
	  snprintf( siz, 20, " size=0x%08x", size);
	  s += siz;
	}
	printf("  %3d: %-60s (addr=%08x mask=%08x) %s\n", i, nodes[i].c_str(), addr,
	       mask, s.c_str());
	// print long description
	if( describe)
	  printf("       %s\n", descr.c_str());

	// print documentation mode with table/row/column
	if( help) {
	  
	}

	// dump params
	if( debug) {
	  const boost::unordered_map<std::string,std::string> params = node.getParameters();
	  for( boost::unordered_map<std::string,std::string>::const_iterator it = params.begin();
	       it != params.end();
	       it++) {
	    printf( "   %s = %s\n", it->first.c_str(), it->second.c_str());
	  }
	}
      }
    
    return 0;
  }
  
  //
  // return a list of nodes matching regular expression
  // convert regex so "." is literal, "*" matches any string
  // "perl:" prefix leaves regex unchanged
  //
  std::vector<std::string> myMatchNodes( uhal::HwInterface* hw, const std::string regex)
  {
    std::string rx = regex;
    
    std::transform( rx.begin(), rx.end(), rx.begin(), ::toupper);
    
    if( rx.size() > 6 && rx.substr(0,5) == "PERL:") {
      printf("Using PERL-style regex unchanged\n");
      rx = rx.substr( 5);
    } else {
      ReplaceStringInPlace( rx, ".", "#");
      ReplaceStringInPlace( rx, "*",".*");
      ReplaceStringInPlace( rx, "#","\\.");
    }
    
    return hw->getNodes( rx);
  }
  
  
  
  // it's wacko that the std library doesn't have this!
  void ReplaceStringInPlace(std::string& subject, const std::string& search,
			    const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
    }
  }
  
  
  
  int Launcher::AMC13DumpEvent(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    uint64_t* pEvt;
    uint64_t head[2];
    size_t siz;
    int rc;
    FILE *fp;

    int nevt = 0x400;
    int nwrote = 0;

    int esize = 0x200000;	// only for overwrite mode

    bool over_mode = false;

    // parse options
    if( strArg.size() < 1) {
      printf("Need a file name after df\n");
      return 0;
    }

    if( (fp = fopen( strArg[0].c_str(), "w")) == NULL) {
      printf("Error opening %s for output\n", strArg[0].c_str());
      return 0;
    }
    if( intArg.size() > 1)
      nevt = intArg[1];

    if( intArg.size() > 2)
      esize = intArg[2];

    // see what mode the AMC13 is in
    int mon_over = defaultAMC13()->read( AMC13Simple::T1, "CONF.EVB.MON_FULL_OVERWRITE");
    if( mon_over == 1) {
      printf("NOTE:  monitor buffer is in overwrite mode, using special readout mode\n");
      printf("       each event will require 128k bytes by default\n");
      printf("       event count is LAST n events\n");
      over_mode = 1;
    }

    printf("Trying to read %d events\n", nevt);

    if( mon_over) {		// ---- special overwrite mode, write fixed size pages ----

      // find pointer to last written event in monitor buffer
      uint32_t page = defaultAMC13()->read( AMC13Simple::T1, "STATUS.MONITOR_BUFFER.UNREAD_BLOCKS");

      printf( "Writing %d events each %d bytes will require %d bytes total (plus headers)\n",
	      nevt, esize, nevt * esize);
      printf( "(hit ^C if not ok!)\n");
      
      // back the pointer up nevt pages, taking care of wraps
      uint32_t ptr = (0x400 + page - nevt) & 0x3ff;

      printf("Reading pointer range from 0x%x up to 0x%x\n", ptr, page);

      for( int i=0; i<nevt; i++) { // loop over events
	uint32_t off = 0x8000000 + ptr * 0x20000; // find offset of this event (sorry for constants!)
	// attempt a block read of the whole thing
	uhal::ValVector<uint32_t> biggie;
	biggie = defaultAMC13()->getT1()->getClient().readBlock(off, esize, uhal::defs::INCREMENTAL);
	defaultAMC13()->getT1()->getClient().dispatch();
	printf("page 0x%03x addr 0x%08x: %08x %08x\n", page, ptr, biggie[1], biggie[0]);

	// write to file in simple-minded format
	head[0] = 0xdeadfacefeedbeef;
	head[1] = esize;
	// have to write the effing val-thingy one word at a time?
	fwrite( head, sizeof(uint64_t), 2, fp);
	for( int i=0; i<esize; i++)
	  fwrite( &biggie[i], sizeof(uint32_t), 1, fp);
	ptr = (ptr + 1) & 0x3ff;
	++nwrote;
      }

    } else {

      for( int i=0; i<nevt; i++) {
	if( (i % 100) == 0)
	  printf("calling readEvent (%d)...\n", i);
	pEvt = defaultAMC13()->readEvent( siz, rc);

	if( rc == 0 && siz > 0 && pEvt != NULL) {
	  head[0] = 0xbadc0ffeebadcafe;
	  head[1] = siz;
	  fwrite( head, sizeof(uint64_t), 2, fp);
	  fwrite( pEvt, sizeof(uint64_t), siz, fp);
	  ++nwrote;
	} else {
	  printf("No more events\n");
	  break;
	}
  
	if( pEvt)
	  free( pEvt);
      }
    }
    
    printf("Wrote %d events to %s\n", nwrote, strArg[0].c_str());

    fclose( fp);
    return 0;
  }


  int Launcher::AMC13DumpMultiFEDEvent(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {

    bool emptyCheck = 1;

    size_t numDAQs;
    switch ( defaultAMC13()->read( AMC13::T1, "CONF.SFP.ENABLE_MASK" ) ) {
    case 0:
    case 1:
      numDAQs = 1;
      break;
    case 3:
      numDAQs = 2;
      break;
    case 7:
      numDAQs = 3;
      break;
    default:
      amc13::Exception::UnexpectedRange e;
      e.Append("In AMC13DumpMultiFEDEvent - CONF.SFP.ENABLE_MASK not 0, 1, 3, or 7\n");
      throw e;
    }

    for ( size_t iDAQ = 0; iDAQ < numDAQs; ++iDAQ ) {
      std::string SFPregister = "STATUS.MONITOR_BUFFER.WORDS_SFP";
      char SFPnumber[2];
      snprintf( SFPnumber, 2, "%lu", iDAQ );
      SFPregister.append( SFPnumber );

      emptyCheck &= ( defaultAMC13()->read(AMC13::T1,SFPregister) == 0 ) ? 1 : 0;

    }

    if ( emptyCheck ) {
      printf( "No events to dump\n" );
      return 0;
    }

    FILE *f[3];
    uint64_t head[3][2];

    uint32_t nEvents = 9999;
    uint32_t nWrote = 0;

    // parse options
    if( (strArg.size() < numDAQs) || ( (strArg.size() == numDAQs) && isdigit( strArg[strArg.size()-1].c_str()[0] ) ) ) {
      printf( "Need %lu file name(s) after df\n", numDAQs );
      return 0;
    }

    for ( size_t iDAQ = 0; iDAQ < numDAQs; ++iDAQ ) {
      if ( ( f[iDAQ] = fopen( strArg[iDAQ].c_str(), "w" ) ) == NULL ) {
	printf("Error opening %s for output\n", strArg[iDAQ].c_str() );
	return 0;
      }
    }

    if( intArg.size() > numDAQs ) {
      nEvents = intArg[numDAQs];
    }

    printf("Trying to read %u events\n", nEvents);
    

    for ( uint32_t iEvent = 0; iEvent < nEvents; ++iEvent ) {

      if ( ( iEvent % 100 ) == 0 ) {
	printf( "Calling readEventMultiFED (events read = %d)...\n", iEvent );
      }

      // reads data into AMC13 member MonitorBufferData, removing need to 
      // copy vectors or reallocate memory to fill new vector
      defaultAMC13()->ReadEventMultiFED();

      for ( size_t jDAQ = 0; jDAQ < numDAQs; ++jDAQ ) {
	emptyCheck = 1;

	if ( defaultAMC13()->MonitorBufferEventData().at(jDAQ).size() == 0 ) {
	  emptyCheck &= 1;
	 }
	else {
	  emptyCheck &= 0;
	  head[jDAQ][0] = 0xbadc0ffeebadcafe;
	  head[jDAQ][1] = defaultAMC13()->MonitorBufferEventData().at(jDAQ).size();
	  fwrite( head[jDAQ], sizeof(uint64_t), 2, f[jDAQ] );
	  fwrite( defaultAMC13()->MonitorBufferEventData().at(jDAQ).data(), sizeof(uint64_t), defaultAMC13()->MonitorBufferEventData().at(jDAQ).size(), f[jDAQ] );
	}

      }

      if ( emptyCheck ) {
	printf( "Nothing more to read. Read %d events\n", nWrote );
	return 0;
      }

      ++nWrote;

    }

    std::string fNames = "";

    for ( size_t iDAQ = 0; iDAQ < numDAQs; ++iDAQ ) {
      char fname [60];
      snprintf( fname, 60, "%s ", strArg[iDAQ].c_str() );
      fNames.append( fname );
      
      fclose( f[iDAQ] );
    }
    
    printf( "Wrote %u events to %s\n", nWrote, fNames.c_str() );

    return 0;
  }


  int Launcher::AMC13ReadEvent(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    uint64_t* pEvt;
    size_t siz;
    int rc;
    bool readAll = false;
    
    if( strArg.size())
      readAll = true;

    printf("calling readEvent...");
    pEvt = defaultAMC13()->readEvent( siz, rc);
    if( rc)
      printf("error rc=%d\n", rc);
    if( pEvt == NULL)
      printf("null ptr\n");
    else if( siz == 0)
      printf("size=0\n");
    else
      printf("AOK\n");
    
    if( rc == 0 && siz > 0 && pEvt != NULL) {
      printf("Read %lld words\n", (long long)siz);
      if( readAll) {
	for( unsigned int i=0; i<siz; i++)
	  printf("%4d: %016lx\n", i, pEvt[i]);
      } else {
	for( unsigned int i=0; i<((siz>10)?10:siz); i++)
	  printf("%4d: %016lx\n", i, pEvt[i]);
	if( siz > 10) {
	  printf(" ...\n");
	  for( unsigned int i=siz-5; i<siz; i++)
	    printf("%4d: %016lx\n", i, pEvt[i]);
	}
      }
    }
    if( pEvt)
      free( pEvt);
    
    return 0;
  }




  
  int Launcher::AMC13ReadEventVector(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    uint64_t* pEvt;
    size_t siz;
    
    std::vector<uint64_t> evtVec;

    printf("calling readEvent...");
    evtVec = defaultAMC13()->readEvent();

    pEvt = evtVec.data();
    siz = evtVec.size();

    if( siz == 0)
      printf("No event to read\n");
    else
      printf("AOK\n");
    
    if( siz > 0 && pEvt != NULL) {
      printf("Read %lld words\n", (long long)siz);
      for( unsigned int i=0; i<((siz>10)?10:siz); i++)
	printf("%4d: %016lx\n", i, pEvt[i]);
      if( siz > 10) {
	printf(" ...\n");
	for( unsigned int i=siz-5; i<siz; i++)
	  printf("%4d: %016lx\n", i, pEvt[i]);
      }
    }
    
    return 0;
  }



  int Launcher::AMC13ReadEventMultiFED(std::vector<std::string> strArg,
				       std::vector<uint64_t> intArg) {

    bool dispAll = false;

    std::string fname;

    if ( strArg.size() > 0 ) {
      if ( toupper(strArg[0].c_str()[0]) == 'A' ) {
	dispAll = true;
      }
    }

    printf("Calling readEventMultiFED...");

    // this method operates on the private vector of vectors of the AMC13
    // this prevents the need to declare a vector in the Launcher methods
    // and it also prevents a large copying of data
    defaultAMC13()->ReadEventMultiFED();

    printf("Complete.\n");

    uint32_t eventNumber = 0;
    if ( !defaultAMC13()->MonitorBufferEventData()[0].empty() ) {
      eventNumber = (defaultAMC13()->MonitorBufferEventData()[0][0]>>32) & 0xffffff;
    }

    size_t numDAQs = defaultAMC13()->MonitorBufferEventData().size();
    bool emptyCheck = 1;
    short namc[3];
    short longHead = 0;
    uint32_t maxSize = 0;
    uint32_t sizeCheck[3];

    std::string FEDid    = "|  FED ID      |";
    std::string chanName = "|  Channel     |";
    std::string bnchx    = "|  Bunch X-ing |";
    std::string orbnum   = "|  Orbit #     |";
    std::string evnnum   = "|  Event #     |";
    std::string evSize   = "|  Event Size  |";
    std::string spacer   = "+--------------+";
    std::string header   = "|  Header      |";
    std::string tailer   = "|  Trailer     |";
    std::string blank    = "|              |";
    std::string empty    = "";

    for ( size_t iDAQ = 0; iDAQ < numDAQs; ++iDAQ ) {

      char fedIDregister [23];
      snprintf( fedIDregister, 23, "CONF.ID.SFP%lu.SOURCE_ID", iDAQ );
      char fedIDappender [20];
      snprintf( fedIDappender, 20, "%7s0x%03x%6s|", empty.c_str(), defaultAMC13()->read( AMC13::T1, fedIDregister ), empty.c_str() );
      FEDid.append( fedIDappender );

      char channelName [20];
      snprintf( channelName, 20, "%8sSFP%lu%6s|", empty.c_str(), iDAQ, empty.c_str() );
      chanName.append( channelName );

      char eventNumb [20];
      snprintf( eventNumb, 20, "%4s%8u%6s|", empty.c_str(), eventNumber, empty.c_str() );
      evnnum.append( eventNumb );

      char spacerAppend [20];
      snprintf( spacerAppend, 20, "------------------+" );
      spacer.append( spacerAppend );
      
      char blanker [20];
      snprintf( blanker, 20, "%18s|", empty.c_str() );
      header.append( blanker );
      tailer.append( blanker );
      blank.append( blanker );

      char eventSize [20];
      char bunchXing [20];
      char orbitNum [20];
      if ( !defaultAMC13()->MonitorBufferEventData().at(iDAQ).empty() ) {
	sizeCheck[iDAQ] = defaultAMC13()->MonitorBufferEventData().at(iDAQ).size();
	snprintf( eventSize, 20, "%12d%6s|", sizeCheck[iDAQ], empty.c_str() );
	if ( sizeCheck[iDAQ] > maxSize ) { maxSize = sizeCheck[iDAQ]; }

	snprintf( bunchXing,  20, "%12lu%6s|", (defaultAMC13()->MonitorBufferEventData().at(iDAQ)[0]>>20)&0xfff, empty.c_str() );
	snprintf( orbitNum,  20, "%12lu%6s|", (defaultAMC13()->MonitorBufferEventData().at(iDAQ)[1]>>4)&0xffffffff, empty.c_str() );

	emptyCheck &= 0;
	namc[iDAQ] = ( defaultAMC13()->MonitorBufferEventData().at(iDAQ)[1] >> 52 ) & 0xf;
	if ( namc[iDAQ] > longHead ) { longHead = namc[iDAQ]; }
      }
      else {
	snprintf( eventSize, 20, "%12lu%6s|", (unsigned long)0, empty.c_str() );
	snprintf( bunchXing,  20, "%12lu%6s|", (unsigned long)0, empty.c_str() );
	snprintf( orbitNum,  20, "%12lu%6s|", (unsigned long)0, empty.c_str() );

	emptyCheck &= 1;
      }
      evSize.append( eventSize );
      bnchx.append( bunchXing );
      orbnum.append( orbitNum );
     
    }

    printf( "%s \n%s \n%s \n%s \n%s \n%s \n%s \n%s \n", spacer.c_str(), FEDid.c_str(), chanName.c_str(), orbnum.c_str(), bnchx.c_str(), evnnum.c_str(), evSize.c_str(), spacer.c_str() );

    if ( emptyCheck ) {
      printf( "No event to read.\n" );
      return 0;
    }

    if ( !dispAll ) {
    
      longHead += 2;

      printf( "%s \n", header.c_str() );

      for ( short iHDR = 0; iHDR < longHead; ++iHDR ) {
	std::string line = "|";
	char liner [16];
	snprintf( liner, 16, "%14s|", empty.c_str() );
	line.append( liner );
	for ( size_t jDAQ = 0; jDAQ < numDAQs; ++jDAQ ) {
	  char ln [20];
	  if ( iHDR < ( 2 + namc[jDAQ] ) ) {
	    snprintf( ln, 20, " %016lx |", defaultAMC13()->MonitorBufferEventData().at(jDAQ)[iHDR] );
	  }
	  else {
	    snprintf( ln, 20, "%18s|", empty.c_str() );
	  }
	  line.append( ln );
	}
	line.append( "\n" );
	printf( "%s", line.c_str() );
      }
      
      printf( "%s \n%s \n", blank.c_str(), tailer.c_str() );
      
      std::string tail = "|              |";
      for ( size_t iDAQ = 0; iDAQ < numDAQs; ++iDAQ ) {
	char tl [20];
	snprintf( tl, 20, " %016lx |", defaultAMC13()->MonitorBufferEventData().at(iDAQ).back() );
	tail.append( tl );
      }
      printf( "%s \n%s \n", tail.c_str(), spacer.c_str() );
      
    }
    else {
      
      for ( uint32_t iWRD = 0; iWRD < maxSize; ++iWRD ) {
	std::string line = "|";
	char liner [16];
	snprintf( liner, 16, "%4s%6u%4s|", empty.c_str(), iWRD, empty.c_str() );
	line.append( liner );
	for ( size_t jDAQ = 0; jDAQ < numDAQs; ++jDAQ ) {
	  char ln [20];
	  if ( iWRD < sizeCheck[jDAQ] ) {
	    snprintf( ln, 20, " %016lx |", defaultAMC13()->MonitorBufferEventData().at(jDAQ)[iWRD] );
	  }
	  else {
	    snprintf( ln, 20, "%18s|", empty.c_str() );
	  }
	  line.append( ln );
	}
	line.append( "\n" );
	printf( "%s", line.c_str() );
      }
      printf( "%s\n", spacer.c_str() );

    }

    return 0;

  }


  // format a hex string, blank if zero
  void sxnz( uint32_t v, int lo, int wid) {
    int fwid = wid / 4 + ((wid % 4) != 0);
    uint32_t ev = (v>>lo) & ((1<<wid)-1);
    if( ev)
      printf( " %*x", fwid, ev);
    else
      printf( " %*s", fwid, " ");
  }
    

  int Launcher::AMC13L1AHistory(std::vector<std::string> strArg,
				std::vector<uint64_t> intArg) {
    unsigned int nh = 128;
    if( intArg.size())
      nh = intArg[0];
    if( nh < 1 || nh > 128) {
      printf("Number of requested history items must be 1-128\n");
      return 0;
    }
    std::vector<uint32_t> hist = defaultAMC13()->getL1AHistory( nh);
    size_t ng = hist.size() / 4;
    printf("%zd history items retrieved\n", ng);
    if( nh > ng)
      nh = ng;
    printf("Num Orbit    BcN  EvN   C T Y G P W A E\n");
    printf("--- -------- --- ------ - - - - - - - -\n");
    for( unsigned int i=0; i<nh; i++) {
      uint32_t cal = hist[4*i+3];
      printf( "%3d %08" PRIx32 " %03" PRIx32" %06" PRIx32, i,
	      hist[4*i], hist[4*i+1], hist[4*i+2]);
      sxnz( cal, 16, 4);
      sxnz( cal, 12, 4);
      sxnz( cal, 8, 4);
      sxnz( cal, 7, 1);
      sxnz( cal, 6, 1);
      sxnz( cal, 5, 1);
      sxnz( cal, 4, 1);
      sxnz( cal, 0, 4);
      printf("\n");
    }
    return 0;
  }


  int Launcher::AMC13TTCHistory(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    if( strArg.size() < 2) {
      printf("At least two arguments required\n");
      return 0;
    }

    std::transform( strArg[0].begin(), strArg[0].end(), strArg[0].begin(), ::toupper);
    std::transform( strArg[1].begin(), strArg[1].end(), strArg[1].begin(), ::toupper);

    if( strArg[0] == "H") {
      if( strArg[1] == "ON") {
	defaultAMC13()->setTTCHistoryEna( true);
      } else if( strArg[1] == "OFF") {
	defaultAMC13()->setTTCHistoryEna( false);
      } else if( strArg[1] == "CLR") {
	defaultAMC13()->clearTTCHistory();
      } else if( strArg[1] == "D") {
	// display the history
	unsigned n = defaultAMC13()->getTTCHistoryCount();
	printf("History buffer has %d entries\n", n);
	if( n) {
	  if( strArg.size() > 2) {
	    if( intArg[2] < n)
	      n = intArg[2];
	  }
	  //	  uint32_t hist[4*n];	// whence cometh this memory?  Only the devil knows!
	  std::vector<uint32_t> hist;
	  hist = defaultAMC13()->getTTCHistory( n);
	  printf("NOTE:  TTC history capture disabled before readout\n");
	  unsigned int ngot = hist.size()/4;
	  if( ngot != n)
	    printf("Only retrieved %d items from history list\n", ngot);
	  printf("    Cmd --Orbit- BcN --EvN-\n");
	  for( unsigned i=0; i<ngot; i++)
	    printf("%3d: %02x %08x %03x %06x\n", i, hist[4*i], hist[4*i+1], hist[4*i+2], hist[4*i+3]);
	}
      } else {
	printf("Unknown option '%s' after 'TTC H'\n", strArg[1].c_str());
      }

    } else if( strArg[0] == "F") {

      if( strArg[1] == "ON") {
	defaultAMC13()->setTTCFilterEna( true);
      } else if( strArg[1] == "OFF") {
	defaultAMC13()->setTTCFilterEna( false);
      } else if( strArg[1] == "CLR") {
	defaultAMC13()->clearTTCHistoryFilter();
      } else if( strArg[1] == "S") {
	if( strArg.size() < 5)
	  printf("Need <n> <cmd> <mask> after 'TTC F S'\n");
	else {
	  defaultAMC13()->setTTCHistoryFilter( intArg[2],  0x10000 | (intArg[3] & 0xff) | ((intArg[4] << 8) & 0xff00) );
	  printf("History item %ld set and enabled\n", intArg[2]);
	}
      } else if( strArg[1] == "LIST") {
	printf("Item Ena CMD Mask\n");
	for( int i=0; i<16; i++) {
	  uint32_t f = defaultAMC13()->getTTCHistoryFilter( i);
	  printf("  %2d %s  %02x %02x\n", i, (f & 0x10000) ? "On " : "Off", f & 0xff, (f >> 8) & 0xff);
	}
      } else if( strArg[1] == "ENA") {
	if( strArg.size() < 3)
	  printf("Need <n> after TTC F ENA\n");
	else {
	  uint32_t f = defaultAMC13()->getTTCHistoryFilter( intArg[2]);
	  f |= 0x10000;
	  defaultAMC13()->setTTCHistoryFilter( intArg[2], f);
	}
      } else if( strArg[1] == "DIS") {
	if( strArg.size() < 3)
	  printf("Need <n> after TTC F DIS\n");
	else {
	  uint32_t f = defaultAMC13()->getTTCHistoryFilter( intArg[2]);
	  f &= 0xffff;
	  defaultAMC13()->setTTCHistoryFilter( intArg[2], f);
	}
      } else {
	printf("Unknownd option '%s' after 'TTC F'\n", strArg[1].c_str());
      }
    }



    return 0;
  }

  int Launcher::AMC13ConfigBGO(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {

    if( strArg.size() < 1) {
      printf("Need a channel number 0-3 or \"fire\" after bgo\n");
      return 0;
    }

    // handle single options
    if( strArg.size() == 1) {

      // fire single commands
      if( strArg[0] == "fire") {
	defaultAMC13()->sendBGO();
	return 0;

      // list current settings
      } else if( strArg[0] == "list") {

	printf("  ch ena len  --cmd--- bcn pscl single\n");
	for( int ch=0; ch<4; ch++) {
	  std::vector<uint32_t> v = defaultAMC13()->getBGOConfig( ch);
	  printf("  %d %s %s %08x %03x %04x %s\n",
		 ch, v[0] ? "yes" : "no ", v[1] ? "long" : "shrt",
		 v[2], v[3], v[4], v[5] ? "single" : "repeat");
		 
	}

	return 0;
      }
    }

    // now we need a channel number plus other stuff
    if( strArg.size() < 2 || intArg[0] > 3) {
      printf("Need channel 0-3 plus option after bgo\n");
      return 0;
    }

    int ch = intArg[0];
    if( strArg[1] == "on") {
      defaultAMC13()->enableBGO(ch);
      return 0;
    }

    if( strArg[1] == "off") {
      defaultAMC13()->disableBGO(ch);
      return 0;
    }

    //  void AMC13::configureBGOShort( int chan, uint8_t cmd, uint16_t bx, uint16_t prescale, bool repeat)
    // temporary format:  bgo <chan> short|long <cmd> <bx> <prescale> yes|no
    if( strArg.size() != 6) {
      printf("Usage: bgo <chan> short|long <cmd> <bx> <prescale> yes|no\n");
      return 0;
    }

    if( strArg[1] == "short") {
      printf("Configuring short command\n");
      defaultAMC13()->configureBGOShort( ch, intArg[2], intArg[3], intArg[4], (strArg[5] == "yes"));
    } else {
      printf("Configuring long command\n");
      defaultAMC13()->configureBGOLong( ch, intArg[2], intArg[3], intArg[4], (strArg[5] == "yes"));
    }
    return 0;
  }

  //Calibration Trigger Functions
  int Launcher::AMC13CalTriggerWindow(std::vector<std::string> strArg, std::vector<uint64_t> intArg) {
    if (strArg.size() == 0) {
      const char* en = defaultAMC13()->getCalTrigEnable() ? "enabled" : "disabled";
      uint16_t lo = defaultAMC13()->getCalTrigWindowLow();
      uint16_t hi = defaultAMC13()->getCalTrigWindowHigh();
      printf("Calibration events are %s\nCalibration window lower limit: %u (0x%x)\nCalibration window upper limit: %u (0x%x)\n",
	     en,lo,lo,hi,hi);
      return 0;
    } else if (strArg.size() == 1) {
      if (strArg[0] == "on") {
	defaultAMC13()->calTrigEnable(true);
      } else if (strArg[0] == "off") {
	defaultAMC13()->calTrigEnable(false);
      } else {
	printf("Unknown option %s\n",strArg[0].c_str());
      }
    } else if (strArg.size() == 2) {
      defaultAMC13()->setCalTrigWindow(intArg[0],intArg[1]);
    } else if (strArg.size() == 3) {
      if (strArg[0] != "on" && strArg[2] != "on") {
	printf("Can only pass three arguments if enabling calibration events\n");
      } else if (strArg[0] == "on") {
	defaultAMC13()->calTrigEnable(true);
	defaultAMC13()->setCalTrigWindow(intArg[1],intArg[2]);
      } else {
	defaultAMC13()->calTrigEnable(true);
	defaultAMC13()->setCalTrigWindow(intArg[0],intArg[1]);
      }
    }
    return 0;
 }


  std::string Launcher::autoComplete_T1AddressTable(std::vector<std::string> const & line,std::string const &currentToken,int state)
  {  
    static size_t pos;
    static std::vector<std::string> completionList;
    if(!state) {
      //Check if we are just starting out
      pos = 0;
      uhal::HwInterface* hw = defaultAMC13()->getChip( AMC13Simple::T1);
      if(hw == NULL){
	return std::string("");
      }
      completionList = myMatchNodes(hw,currentToken+std::string("*"));
    } else {
      //move forward in pos
      pos++;
    }


    if(pos < completionList.size()){
      return completionList[pos];
    }
    //not found
    return std::string("");  
  }
  std::string Launcher::autoComplete_T2AddressTable(std::vector<std::string> const & line,std::string const &currentToken,int state)
  {  
    static size_t pos;
    static std::vector<std::string> completionList;
    if(!state) {
      //Check if we are just starting out
      pos = 0;
      uhal::HwInterface* hw = defaultAMC13()->getChip( AMC13Simple::T2);
      if(hw == NULL){
	return std::string("");
      }
      completionList = myMatchNodes(hw,currentToken+std::string("*"));
    } else {
      //move forward in pos
      pos++;
    }


    if(pos < completionList.size()){
      return completionList[pos];
    }
    //not found
    return std::string("");  
  }


  int Launcher::SFPDump(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
    //Get the SFP number (default to 0)
    uint32_t SFP = 0;
    if(intArg.size() > 0){
      //If the SFP number is between 0 and 3
      if(intArg[0] == (0x00000003 & (intArg[0]))){
	SFP = intArg[0];
      }
    }

    uint32_t data,addr;

    //Get bitrate
    addr = 0x100 + 0x20*SFP + 3;
    uint32_t bit_rate_Mbps = 100*uint32_t(defaultAMC13()->read( AMC13Simple::T1,addr)&0xFF);

    //Get Vendor Name
    std::string vendor_name;
    addr = 0x100 + 0x20*SFP + 5; //beginning of vendor name
    for(int iRd = 0; iRd < 4; iRd++){      
      data = defaultAMC13()->read( AMC13Simple::T1,addr);    
      for(int i = 0; i < 4;i++){
	char ASCII = char((data >> (8*i))&0xFF);
	if (isprint(ASCII)){
	  vendor_name.append(1,ASCII);
	}else{
	  vendor_name.push_back(' ');
	}
      }
      addr++;
    }

    //Get Vendor Part Number
    std::string vendor_part;
    addr = 0x100 + 0x20*SFP + 10; //beginning of vendor name
    for(int iRd = 0; iRd < 4; iRd++){      
      data = defaultAMC13()->read( AMC13Simple::T1,addr);    
      for(int i = 0; i < 4;i++){
	char ASCII = char((data >> (8*i))&0xFF);
	if (isprint(ASCII)){
	  vendor_part.append(1,ASCII);
	}else{
	  vendor_part.push_back(' ');
	}
      }
      addr++;
    }

    //Get Vendor Part Number
    std::string vendor_rev;
    addr = 0x100 + 0x20*SFP + 14; //beginning of vendor name
    data = defaultAMC13()->read( AMC13Simple::T1,addr);    
    for(int i = 0; i < 4;i++){
      char ASCII = char((data >> (8*i))&0xFF);
      if (isprint(ASCII)){
	vendor_rev.append(1,ASCII);
      }else{
	vendor_rev.push_back(' ');
      }
    }

    
    std::cout << "SFP " << SFP << std::endl
	      << "   BitRate: " << bit_rate_Mbps << "Mbps\n"
	      << "   Vendor:  " << vendor_name << std::endl
	      << "   Part #:  " << vendor_part << std::endl
	      << "   Rev # :  " << vendor_rev << std::endl;
    return 0;
  }
    
}

