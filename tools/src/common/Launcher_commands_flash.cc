#include "amc13/Launcher.hh"
#include "amc13/Flash.hh"
#include <boost/algorithm/string.hpp>

// #define DEBUG

namespace amc13{
   
  bool ok_to_continue( const char *message) {
    char buff[5];
    printf("%s\n", message);
    printf("Enter 'Yes' to continue-->");
    fgets( buff, 4, stdin);
    if( !strncmp( buff, "Yes", 3))
      return true;
    else
      return false;
  }

  int Launcher::AMC13PrintFlash(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    
    if( intArg.size() < 1) {
      printf("Need a page address\n");
      return 0;
    }

    std::vector<uint32_t> data = defaultAMC13()->getFlash()->readFlashPage( intArg[0]);
    printf("Read %lld words from flash at 0x%lx\n", (unsigned long long)data.size(), intArg[0]);
    for( unsigned i=0; i<data.size(); i++)
      printf("%4d: 0x%08x\n", i, data[i]);


    return 0;
  }
   
  int Launcher::AMC13VerifyFlash(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    if( strArg.size() < 2) {
      printf("need filename and address\n");
      return 0;
    }

    defaultAMC13()->getFlash()->verifyFlash( strArg[0], intArg[1]);

    return 0;
  }

  int Launcher::AMC13ProgramFlash(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    
    char buff[6];

    if( !ok_to_continue("EXPERT ONLY!  You probably don't want to use this command.\n" \
		       "use 'pv', 'pk' or 'ps' instead!\n"))
      return 0;

    if( strArg.size() < 2) {
      printf("need filename and address\n");
      return 0;
    }

    bool progOK = false;
    if( strArg.size() >= 3 && strArg[2] == "Y")
      progOK = true;
    else {
      printf("Propose to program file %s to flash address 0x%lx... ok? ", strArg[0].c_str(), intArg[1]);
      fgets( buff, 5, stdin);
      if( toupper( *buff) == 'Y')
	progOK = true;
    }

    if( progOK)
      defaultAMC13()->getFlash()->programFlash( strArg[0], intArg[1]);
    else
      printf("Aborted\n");
    return 0;
  }

  int Launcher::AMC13ReloadFPGAs(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    defaultAMC13()->getFlash()->loadFlash();
    printf("Best exit/restart tool after 10s or so\n");

    return 0;
  }

  int Launcher::AMC13VerifyFlashFile(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    if( strArg.size() < 1) {
      printf("need filename\n");
      return 0;
    }

    defaultAMC13()->getFlash()->verifyFlash( strArg[0] );

    return 0;
  }

  int Launcher::AMC13ProgramFlashFile(std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {    
    char buff[6];

    if( !ok_to_continue("EXPERT ONLY!  You probably don't want to use this command.\n" \
		       "use 'pv', 'pk' or 'ps' instead!\n"))
      return 0;

    if( strArg.size() < 1) {
      printf("need filename\n");
      return 0;
    }

    bool progOK = false;
    if( strArg.size() >= 2 && strArg[1] == "Y")
      progOK = true;
    else {
      printf("Propose to program file %s to flash... ok? ", strArg[0].c_str());
      fgets( buff, 5, stdin);
      if( toupper( *buff) == 'Y')
	progOK = true;
    }
    
    if( progOK ) 
      defaultAMC13()->getFlash()->programFlash( strArg[0] );
    
    else
      printf("Aborted\n");
    return 0;
  }

  int Launcher::AMC13SelectFileTest( std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    if( strArg.size() < 1) {
      printf("need filename\n");
      return 0;
    }
    //    defaultAMC13()->getFlash()->parseMcsFile( strArg[0].c_str() );
    std::string useFile = defaultAMC13()->getFlash()->selectMcsFile( AMC13Simple::T2, "HEADER", "6SLX45T");
    printf("Chose file: %s\n", useFile.c_str() );
    
    return 0; 
  }

  int Launcher::AMC13VerifyFlashHeader( std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    std::string chipType;
    if( strArg.size() < 1) {
      printf("chip_type not specified, using default from serial number...");
      // check serial number and use chip_type corresponding 
      uhal::ValWord<uint32_t> sn = defaultAMC13()->getT2()->getNode("STATUS.SERIAL_NO").read();
      defaultAMC13()->getT2()->dispatch();
      //printf("serial no: %d\n", sn.value() );
      chipType = defaultAMC13()->getFlash()->chipTypeFromSN( AMC13Simple::T2, sn ) ;
    }
    else {
      chipType = boost::to_upper_copy( strArg[0] );
    }
    printf( "Searching for files with T2, Header, and %s...\n", chipType.c_str() );
    std::string selectFile = defaultAMC13()->getFlash()->selectMcsFile( AMC13Simple::T2, "HEADER", chipType);
    if (selectFile != "") {
      printf("Verifying against file: %s...\n", selectFile.c_str() );
      defaultAMC13()->getFlash()->verifyFlash( selectFile );
    }
    return 0;    
  }

  int Launcher::AMC13VerifyFlashGolden( std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    std::string chipType;
    if( strArg.size() < 1) {
      printf("chip_type not specified, using default from serial number...\n");
      // check serial number and use chip_type corresponding 
      uhal::ValWord<uint32_t> sn = defaultAMC13()->getT2()->getNode("STATUS.SERIAL_NO").read();
      defaultAMC13()->getT2()->dispatch();
      //printf("serial no: %d\n", sn.value() );
      chipType = defaultAMC13()->getFlash()->chipTypeFromSN( AMC13Simple::T2, sn ) ;
    }
    else {
      chipType = boost::to_upper_copy( strArg[0] );
    }
    printf( "Searching for files with T2, Golden, and %s...\n", chipType.c_str() );
    std::string selectFile = defaultAMC13()->getFlash()->selectMcsFile( AMC13Simple::T2, "GOLDEN", chipType);
    if (selectFile != "") {
      printf("Verifying against file: %s...\n", selectFile.c_str() );
      defaultAMC13()->getFlash()->verifyFlash( selectFile );
    }
    return 0;    
  }


  int Launcher::AMC13VerifyFlashT2( std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    std::string chipType;
    if( strArg.size() < 1) {
      printf("chip_type not specified, using default from serial number...\n");
      // check serial number and use chip_type corresponding 
      uhal::ValWord<uint32_t> sn = defaultAMC13()->getT2()->getNode("STATUS.SERIAL_NO").read();
      defaultAMC13()->getT2()->dispatch();
#ifdef DEBUG
      printf("serial no: %d\n", sn.value() );
#endif
      chipType = defaultAMC13()->getFlash()->chipTypeFromSN( AMC13Simple::T2, sn ) ;
    }
    else {
      chipType = boost::to_upper_copy( strArg[0] );
    }
    printf( "Searching for files with T2, and %s...\n", chipType.c_str() );
    std::string selectFile = defaultAMC13()->getFlash()->selectMcsFile( AMC13Simple::T2, "", chipType);
    if (selectFile != "") {
      printf("Verifying against file: %s...\n", selectFile.c_str() );
      defaultAMC13()->getFlash()->verifyFlash( selectFile );
    }
    return 0;    
  }


  int Launcher::AMC13VerifyFlashT1( std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    // chipType should be used to handle virtex or kintex. Currently defaults to using kintex
    std::string chipType;
    if( strArg.size() < 1) {
      printf("chip_type not specified, using default from serial number...\n");
      // check serial number and use chip_type corresponding 
      uhal::ValWord<uint32_t> sn = defaultAMC13()->getT2()->getNode("STATUS.SERIAL_NO").read();
      defaultAMC13()->getT2()->dispatch();
      //printf("serial no: %d\n", sn.value() );
      chipType = defaultAMC13()->getFlash()->chipTypeFromSN( AMC13Simple::T1, sn ) ;
    }
    else {
      chipType = boost::to_upper_copy( strArg[0] );
    }
    printf( "Searching for files with T1 and %s...\n", chipType.c_str() );
    std::string selectFile = defaultAMC13()->getFlash()->selectMcsFile( AMC13Simple::T1, "", chipType);
    if (selectFile != "") {
      printf("Verifying against file: %s...\n", selectFile.c_str() );
      defaultAMC13()->getFlash()->verifyFlash( selectFile );
    }
    return 0;    
  }


  int Launcher::AMC13ProgramFlashHeader( std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    std::string chipType;
    if( strArg.size() < 1) {
      printf("chip_type not specified, using default from serial number...\n");
      // check serial number and use chip_type corresponding 
      uhal::ValWord<uint32_t> sn = defaultAMC13()->getT2()->getNode("STATUS.SERIAL_NO").read();
      defaultAMC13()->getT2()->dispatch();
      //printf("serial no: %d\n", sn.value() );
      chipType = defaultAMC13()->getFlash()->chipTypeFromSN( AMC13Simple::T2, sn ) ;
    }
    else {
      chipType = boost::to_upper_copy( strArg[0] );
    }
    printf( "Searching for files with T2, Header, and %s...\n", chipType.c_str() );
    std::string selectFile = defaultAMC13()->getFlash()->selectMcsFile( AMC13Simple::T2, "HEADER", chipType);
    if (selectFile != "") {
      printf("Programming against file: %s...\n", selectFile.c_str() );
      if( ok_to_continue("program flash header, really an expert thing!"))
	defaultAMC13()->getFlash()->programFlash( selectFile );
    }
    return 0;    
  }

  int Launcher::AMC13ProgramFlashGolden( std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    std::string chipType;
    if( strArg.size() < 1) {
      printf("chip_type not specified, using default from serial number...\n");
      // check serial number and use chip_type corresponding 
      uhal::ValWord<uint32_t> sn = defaultAMC13()->getT2()->getNode("STATUS.SERIAL_NO").read();
      defaultAMC13()->getT2()->dispatch();
      //printf("serial no: %d\n", sn.value() );
      chipType = defaultAMC13()->getFlash()->chipTypeFromSN( AMC13Simple::T2, sn ) ;
    }
    else {
      chipType = boost::to_upper_copy( strArg[0] );
    }
    printf( "Searching for files with T2, Golden, and %s...\n", chipType.c_str() );
    std::string selectFile = defaultAMC13()->getFlash()->selectMcsFile( AMC13Simple::T2, "GOLDEN", chipType);
    if (selectFile != "") {
      printf("Programming against file: %s...\n", selectFile.c_str() );
      if( ok_to_continue("program flash golden (backup image), really an expert thing!"))
	defaultAMC13()->getFlash()->programFlash( selectFile );
    }
    return 0;    
  }


  int Launcher::AMC13ProgramFlashT2( std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    std::string chipType;
    if( strArg.size() < 1) {
      printf("chip_type not specified, using default from serial number...\n");
      // check serial number and use chip_type corresponding 
      uhal::ValWord<uint32_t> sn = defaultAMC13()->getT2()->getNode("STATUS.SERIAL_NO").read();
      defaultAMC13()->getT2()->dispatch();
      //printf("serial no: %d\n", sn.value() );
      chipType = defaultAMC13()->getFlash()->chipTypeFromSN( AMC13Simple::T2, sn ) ;
    }
    else {
      chipType = boost::to_upper_copy( strArg[0] );
    }
    printf( "Searching for files with T2, and %s...\n", chipType.c_str() );
    std::string selectFile = defaultAMC13()->getFlash()->selectMcsFile( AMC13Simple::T2, "", chipType);
    if (selectFile != "") {
      printf("Programming against file: %s...\n", selectFile.c_str() );
      if( ok_to_continue("programing T2 (spartan) flash now")) {
	defaultAMC13()->getFlash()->programFlash( selectFile );
	printf("Done.\n");
      }
    }
    return 0;    
  }


  int Launcher::AMC13ProgramFlashT1( std::vector<std::string> strArg,
			       std::vector<uint64_t> intArg) {
    // chipType should be used to handle virtex or kintex. Currently defaults to using kintex
    std::string chipType;
    if( strArg.size() < 1) {
      printf("chip_type not specified, using default from serial number...\n");
      // check serial number and use chip_type corresponding 
      uhal::ValWord<uint32_t> sn = defaultAMC13()->getT2()->getNode("STATUS.SERIAL_NO").read();
      defaultAMC13()->getT2()->dispatch();
      //printf("serial no: %d\n", sn.value() );
      chipType = defaultAMC13()->getFlash()->chipTypeFromSN( AMC13Simple::T1, sn ) ;
    }
    else {
      chipType = boost::to_upper_copy( strArg[0] );
    }
    printf( "Searching for files with T1 and %s...\n", chipType.c_str() );
    std::string selectFile = defaultAMC13()->getFlash()->selectMcsFile( AMC13Simple::T1, "", chipType);
    if (selectFile != "") {
      printf("Programming against file: %s...\n", selectFile.c_str() );
      if( ok_to_continue("programing T1 (virtex/kintex) flash now")) {
	defaultAMC13()->getFlash()->programFlash( selectFile );
	printf("Done.\n");
      }
    }
    return 0;    
  }

}

