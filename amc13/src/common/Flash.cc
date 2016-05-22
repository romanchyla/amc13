#include "amc13/Flash.hh"
#include <boost/algorithm/string.hpp>

#define FLASH_RETRY 3

// #define DEBUG

namespace amc13{
  
  // AMC13_flash Class Constructor
  Flash::Flash( uhal::HwInterface* T2)
  {
    
    if (T2 == NULL) {
      Exception::NULLPointer e ;
      throw e ;
    }
    FlashT2 = T2;
  }
  
  Flash::~Flash() { }

  
  // AMC13_flash Class Functions
  // add=0x000000 for header
  //     0x080000 for spartan backup (for spartan 45 chips)
  //     0x100000 for spartan backup (for spartan 25 chips)
  //     0x200000 for spartan
  //     0x400000 for virtex/kintex

  
  // 'readFlashPage()' reads one page from flash starting at byte address add
  std::vector<uint32_t> Flash::readFlashPage(uint32_t add) {

    //amc13->write(amc13->spartan, "FLASH_WBUF", 0x0b << 24 | add);
    FlashT2 -> getNode( "FLASH_WBUF" ).write( 0x0b << 24 | add );
    FlashT2 -> dispatch() ; 

    flashDoCommand(260);
    std::vector<uint32_t> stdVec;
    uint32_t buffer[65] ;
    uint32_t * ptrbuffer = buffer ;

    //amc13->read(amc13->spartan, 0x1080, 64, ptrbuffer);
    read( 0x1080, 64, ptrbuffer);
    
    stdVec.resize(64);
    memcpy ( &(stdVec[0]), ptrbuffer, 64*sizeof(uint32_t) );
    return stdVec;
  }
  
  // 'firmwareFromFlash()' reads entire flash contents into vector
  // add is starting address pages is number of pages
  std::vector<uint32_t> Flash::firmwareFromFlash(uint32_t add, int pages) {
    std::vector<uint32_t> in, out;
    for (int i=0; i < pages; ++i){
      if (i==0)
	std::cout << "reading flash at address " << std::hex << "0x" << add << std::endl;
      if((i+1)%3000==0)
	std::cout << "reading flash at address " << std::hex << "0x" << add << "\t % done = " << std::dec << (i+50)*100/pages << std::endl;	
      in = readFlashPage(add);
      for (int j=0; j < 64; ++j){
	out.push_back(in[j]);
      }
      add +=256;
      in.clear();
    }
    return out;
  } 
  
  // 'eraseFlashSector()' erases one sector of the flash 
  void Flash::eraseFlashSector(uint32_t s) {
    enableFlashWrite();
    uint32_t add = s * (1 << 18);
    std::cout << "Erasing flash sector at address 0x" << std::hex << add << std::endl;
    
    //amc13->write(amc13->spartan, "FLASH_WBUF", 0xD8 << 24 | add);
    FlashT2 -> getNode( "FLASH_WBUF" ).write( 0xD8 << 24 | add );
    FlashT2 -> dispatch() ; 

    flashDoCommand(3);
    waitForWriteDone();
  } 
  
  
  // 'verifyFlash(const std::string&)' takes a MCS filename 
  // calculates the appropriate offset using 'parseMcsFile()' and 'offset'
  // and verify from that address in the flash
  void Flash::verifyFlash(const std::string& mcsFileName) {
    parseMcsFile( mcsFileName) ;
    uint32_t add = offset( chip_no, version, chip_type) ;
    printf("Verifying file %s to flash address 0x%x...\n", mcsFileName.c_str(), add);
    verifyFlash( mcsFileName, add ) ;
    return;
  }

  // verifyFlash() verifies flash programming
  void Flash::verifyFlash(const std::string& mcsFileName, uint32_t add){
    std::vector<uint32_t> flash,file;
    file =firmwareFromMcs(mcsFileName);
    if(file.size()==0){
      std::cout << "file does not exist" << std::endl;
      return;
    }
    int pages = file.size()/64;
    if(file.size()%64!=0) pages +=1;
    flash=firmwareFromFlash(add,pages);
    int imax = file.size(); 
    std::cout << "Verifying flash against " << mcsFileName << ", num pages: " << std::dec << pages << std::endl;
    for (int i=0; i < imax; ++i){
      //int k = i/64; 
      if (file[i]!=flash[i]) {
	std::cout << "flash verification error page = " << std::dec << (i/64) << " word = " <<std::hex <<  i << " flash = " 
		  << std::hex << flash[i] << " file = " << std::hex << file[i] << std::endl;
	return;
      } 
    }
    std::cout << "Successfully verified flash programing:" << std::endl << "file = " 
	      << mcsFileName << "   pages = "<< std::dec << pages << std::endl;
    return;
  } 
    
  // 'programFlash(const std::string&)' takes a MCS filename 
  // calculates the appropriate offset using 'parseMcsFile()' and 'offset'
  // and programs to that address in the flash
  void Flash::programFlash(const std::string& mcsFileName) {
    parseMcsFile( mcsFileName) ;
    uint32_t add = offset( chip_no, version, chip_type) ;
    printf("Programming %s to flash address 0x%x...\n", mcsFileName.c_str(), add);
    programFlash( mcsFileName, add ) ;
    return;
  }

  // 'programFlash(const std::string&, uint32_t)' takes a MCS filename 
  // (from current directory) and flash address offset to program 
  // from MCS to the flash 
  void Flash::programFlash(const std::string& mcsFileName, uint32_t add) {
    std::vector<uint32_t> flash,file;
    file =firmwareFromMcs(mcsFileName);
    if(file.size() == 0) {
      std::cout << "file does not exist" << std::endl;
      return;
    }
    uint32_t dataWords=file.size();
    uint32_t remainder=dataWords%64;
    uint32_t pages = dataWords/64;
    if(file.size()%64 != 0) {
      pages +=1;
      //pad zeros to fill out even multiple of 64 words
      uint32_t imax=64-remainder;
      for (uint32_t i=0; i<imax; ++i) {
	file.push_back(0);
      }
    }
    // flash is programmed one page (256 bytes) at a time
    // pages is the number of pages to program
    // the starting byte address of the page of flash data is
    // stored in programAddress 
    // and written to register 0x1000 ("FLASH_WBUF")
    // spartan flash starts at 0x0 and virtex at 0x200000
    // As per instructions of Mr. Wu, we write the pages in reverse order.
    uint32_t programAddress = 256*pages+add;
    // zero the flash
    // flash is zeroed (0xffffffff) in units of 1024 pages
    uint32_t kpages=pages/1024;
    uint32_t koffset=add/1024/256;
    remainder=kpages%1024;
    if (remainder !=0) 
      kpages +=1;
    if(kpages ==0) 
      kpages +=1;
    for(uint32_t k=0; k<kpages; ++k) {
#ifdef FLASH_RETRY
      int ntry = FLASH_RETRY;
      while( ntry--) {
	try {
	  eraseFlashSector(k+koffset);
	  break;
	} catch (uhal::exception::exception & e) {
	  printf("Flash threw... tries left: %d\n", ntry);
	}
      }
      if( ntry == 0) {
	printf("Flash erase failed after %d tries\n", FLASH_RETRY);
	exit(1);
      }
#else
      eraseFlashSector(k+koffset);
#endif
    }
    // dataIndex is marks the location of the first word of flash data in file
    uint32_t dataIndex=file.size();
    for(uint32_t p=0; p < pages; ++p){
      if (p==0)
	std::cout << "\n\nprogramming flash at address " << std::hex << "0x" << programAddress << std::endl;
      if((p+1)%3000==0)
	std::cout << "programming flash at address " << std::hex << "0x" << programAddress << "\t % done = " << std::dec << (p+50)*100/pages << std::endl;
      programAddress -=256;
      dataIndex -=64;
      enableFlashWrite();
      
      //amc13->write(amc13->spartan, "FLASH_WBUF", 0x02 << 24 | programAddress);
      FlashT2 -> getNode( "FLASH_WBUF" ).write( 0x02 << 24 | programAddress );
      FlashT2 -> dispatch() ;

      // Data is written in blocks of 64 32-bit words
      std::vector<uint32_t> wrData;
      for(uint32_t i = 0; i < 64; i++) {
	wrData.push_back(file[dataIndex+i]);
      }
      // work around, copy wrData to wrData32
      uint32_t wrData32[ wrData.size() ];
      for(uint32_t i = 0; i < wrData.size(); i++) {
	wrData32[i] = wrData[i];
      }

#ifdef FLASH_RETRY
      int ntry = FLASH_RETRY;
      while( ntry--) {
	try {
	  
	  //amc13->write( amc13->spartan, 0x1001, wrData.size(), wrData32 ); //block write
	  write( 0x1001, wrData.size(), wrData32 );

	  flashDoCommand(259);
	  waitForWriteDone();
	  break;
	} catch (uhal::exception::exception & e) {
	  printf("Flash threw... tries left: %d\n", ntry);
	}
      }
      if( ntry == 0) {
	printf("Flash erase failed after %d tries\n", FLASH_RETRY);
	exit(1);
      }
#else
      
      //amc13->write( amc13->spartan, 0x1001, wrData.size(), wrData32 ); //block write
      write( 0x1001, wrData.size(), wrData32 );
      
      flashDoCommand(259);
      waitForWriteDone();
#endif
    }
    return;
  }
  
  
  // 'flashDoCommand()' executes a flash command
  // parameter is 0 for enableFlashWrite
  //              3 for eraseFlashSector
  //            3+n for flashProgram, writing n words to flash
  //              1 for waitForWriteDone
  //            4+n for flashRead, reading n words from flash
  void Flash::flashDoCommand(int parameter) {
    
    //amc13->write(amc13->T2,"CONF.FLASH.CMD", parameter);
    FlashT2 -> getNode( "CONF.FLASH.CMD" ).write( parameter );
    FlashT2 -> dispatch() ;

    //int z = amc13->read(amc13->spartan, "CONF.FLASH.CMD");
    int z = read("CONF.FLASH.CMD");

    //while (z != 0) z = amc13->read(amc13->spartan,"CONF.FLASH.CMD"); 
    while (z != 0) z = read("CONF.FLASH.CMD");

  } 
  
  // 'enableFlashWrite()' enables flash for writing
  void Flash::enableFlashWrite(){
    
    //amc13->write(amc13->spartan, "FLASH_WBUF", 0x06 << 24);
    FlashT2 -> getNode( "FLASH_WBUF" ).write( 0x06 << 24 );
    FlashT2 -> dispatch() ;
    
    flashDoCommand(0);
  }
  
  // 'waitForWriteDone()' ensures previous write has finished before continuing
  void Flash::waitForWriteDone(){
    
    //amc13->write(amc13->spartan, "FLASH_WBUF", 0x05000000);
    FlashT2 -> getNode( "FLASH_WBUF" ).write( 0x05000000 );
    FlashT2 -> dispatch() ;

    flashDoCommand(1);

    //uint32_t z = amc13->read(amc13->spartan, "FLASH_RBUF");
    uint32_t z = read("FLASH_RBUF");

    while (z & 0x01000000){
      flashDoCommand(1);

      //z = amc13->read(amc13->spartan, "FLASH_RBUF");
      z = read("FLASH_RBUF");

    }
  }   
  
  void Flash::loadFlashT1(){

    //amc13->write(amc13->T2, 0, 0x10);
    FlashT2 -> getClient().write( 0, 0x10 );
    FlashT2 -> getClient().dispatch();
    
  }
  
  void Flash::loadFlash(){

    //amc13->write(amc13->T2, 0, 0x100);
    FlashT2 -> getClient().write( 0, 0x100 );
    FlashT2 -> getClient().dispatch();
    
  }
  
  
  // 'firmwareFromMcs()' parses the AMC13 firmware and puts it in a vector. 
  // The vector may be used to verify and/or program the flash memory.
  // The firmware format is (all 1 byte except the address)
  // : (no. data bytes) (address, 2 bytes) (rectype) (data) (check sum) 
  // 
  // the output is in 32 bit words
  //
  // use case:
  //    std::vector<uint32_t> file;
  //    file =firmwareFromMcs(mcsFileName);
  std::vector<uint32_t> Flash::firmwareFromMcs(const std::string& mcsFileName) {
    std::vector<uint32_t> out;
    std::ifstream file(mcsFileName.c_str());
    if(!file.is_open()) return out;
    std::string line;
    uint32_t nBytes,addr,recType,checkSum,data,byteSum;
    uint32_t temp;
    while(file.good()) {
      getline(file, line);
      //assert(line.size());
      if(line.size() != 0) {
	assert(line.at(0)==":"[0]);
	recType  = intFromString(line, 7, 2);
	if(recType) continue; //flash data only if recType is 0
	nBytes   = intFromString(line, 1, 2);
	addr     = intFromString(line, 3, 4);
	checkSum = intFromString(line, 9+2*nBytes, 2);
	byteSum  = nBytes+recType+checkSum;
	byteSum += intFromString(line, 3, 2) + intFromString(line, 5, 2);
	for(unsigned int iByte=0; iByte < nBytes; ++iByte) {
	  data = intFromString(line, 9+2*iByte, 2);
	  byteSum += data;
	  uint32_t nBits = 8*(iByte%4);
	  // keep order of the 32 bit word identical to firmware file 
	  temp |= data<<(24 - nBits);
	  if ((iByte+1)%4==0) {
	    out.push_back(temp);
	    temp=0;
	  }
	}
	if(nBytes%4 !=0) out.push_back(temp);
	assert(!(byteSum&0xff));
      }
    }
    file.close();
    return out;
  }
 
  // Select mcs file and parse file for validity and save information to internal variables for use
  void Flash::parseMcsFile( std::string p_name ) {
    file_name = p_name ;
    std::string name = p_name; 
    name = name.substr(name.find_last_of("/") + 1);
    name = boost::to_upper_copy( name ) ;
    // Check that filename is sufficiently long 
    if ( name.size() < 16 ) {
      clearThrow( "File name shorter than 16 char.\n" ) ;
    }
    // Check that file starts w/ "AMC13T"
    if ( name.substr(0, 6) != "AMC13T") {
      clearThrow( "File name does not start w/ 'AMC13T'.\n" ) ;
    }
    // Check that file ends w/ ".MCS"
    if ( name.substr( name.length()-4, name.length()-1 ) != ".MCS" ) {
      clearThrow( "File name does not end w/ '.mcs'.\n" ) ;
    }
    // Determine chip_no
    chip_no = name[6] - '0' ; // convert ascii char to int
    if ( chip_no == 2) 
      chip_no = AMC13Simple::T2 ;   
    else if ( chip_no == 1)
      chip_no = AMC13Simple::T1 ;
    else {
      clearThrow( "File name does have T1 or T2 in correct format.\n" ) ;  
    }
    name.erase( 0, 7 ) ; // erase AMC13T# from name  
    // Determine version
    if ( name[0] != 'H' && name[0] != 'G' && name[0] != 'V' ) {
      clearThrow( "File name has unexpected version format at 7th character.\n" ) ;     
    }
    int mark ;
    mark = name.std::string::find_first_of( "_.", 0) ;
    version = name.substr( 0, mark ) ;
    name.erase( 0, mark ) ; // erase version from name  
    // Determine chip_type
    if ( name[0] == '_' ) {
      mark = name.std::string::find_first_of( ".", 0 ) ;
      chip_type = name.substr( 1, mark-1 ) ;
    }    
#ifdef DEBUG
    printf( "chip_no: %d\n", chip_no ) ;
    printf( "version: %s\n", version.c_str() );
    printf( "chip_type: %s\n", chip_type.c_str() ) ;
#endif
    return;
  }

  std::string Flash::selectMcsFile( int chipNo, std::string ver, std::string chipType) {
     std::string dir = "./";
     std::vector<std::string> validFiles;

     // Open the directory and store its MCSfile names as MCSParse instances
     DIR *dp;
     struct dirent *dirp;
     if((dp  = opendir(dir.c_str())) == NULL)
       std::cout << "No files in this directory" << std::endl;
     while ((dirp = readdir(dp)) != NULL) {     
       std::string dname = dirp->d_name;
#ifdef DEBUG
       printf("----> %s file\n", dname.c_str() );
#endif   
       //check if this is atleast an MCS file (case independent)
       if ((dname.size() >= 4) &&   // name big enough for ".mcs"
	   //case insensitive check of .mcs
	   ( boost::to_upper_copy<std::string>(dname.substr(dname.size()-4)).find(".MCS") != std::string::npos)){ 
	 try {
	   parseMcsFile( dname );
	   if ( chip_no == chipNo &&  chip_type == chipType) {
	     std::string fver6 = version.substr(0,6); // First 6 chars of file version
	     if (ver == "GOLDEN" || ver=="HEADER") { // check for match in file version
	       if( boost::iequals( fver6, ver)) { // compare only first 6 chars, ignore rest
		 validFiles.push_back( dname ) ;	       
	       }
	     } else {		// not ver or golden, skip those
	       if( !(boost::iequals( fver6, "GOLDEN") || boost::iequals( fver6, "HEADER")))
		 validFiles.push_back( dname ) ;
	     }
	   }  
	 } catch( const amc13::Exception::BadFileFormat &e){
	   std::cout << "Incorrectly named file: " << dname 
		     << ". " << e.Description() << std::endl;
	 }
       }
     }
     closedir(dp);
     printf("\n");

     // Check that there validFiles is not empty
     if ( !validFiles.size() ) {
       printf("No matching files. Returning to menu.\n");
       return "";
     }
#ifdef DEBUG
     printf("size of validFiles: %zu\n", validFiles.size() );
#endif     
     
     // print all filenames in validFiles numbered in 1-based 
     for ( size_t i = 0; i < validFiles.size(); i++ ){
       printf("%zu. %s\n", i+1, validFiles[i].c_str() ) ;
     }

     // getline user input
     std::string fileNumStr;
     printf("\nEnter number for MCS file to select (0 to exit): ");

     // check that number is within range
     bool inRange = false;
     while( !inRange ){
       getline( std::cin, fileNumStr );
       int fileNumInt = atoi( fileNumStr.c_str() ) -1 ; //-1 from input to match revert to 0-based 
       if ( fileNumInt >= 0  && fileNumInt < (int) validFiles.size() ) { //comparing int to unsigned
	 inRange = true;
	 return validFiles[ fileNumInt].c_str() ;
       }
       else if ( fileNumInt == -1) { //user input 0
	 return "";
       }
       else {
	 printf( "\nSorry not in range. Try again: "); // ask for the number again 
       }
     }
     return "";
   }


  uint32_t Flash::offset( int chipNo, std::string ver, std::string chipType) {
    switch( chipNo ) {
      case -1: // No chipNo
	clearThrow( "No chipNo specified.\n" );
	break;
      case AMC13Simple::T1:
	flash_address = 0x400000;
	return 0x400000;
	break;
      case AMC13Simple::T2:
	if ( ver == "HEADER" ) 
	  return 0x000000;
	else if ( ver == "GOLDEN" ) {
	    if ( chipType == "6SLX45T" ) 
	      return 0x080000;
	    else if ( chipType == "6SLX25T")
	      return 0x100000;
	    else 
	      clearThrow( "Unexpected T2 chip_type suffix.\n" ); 
	}
	else {
	  return 0x200000;
	}
	break;
      default:
	clearThrow( "chipNo out of expected range.\n" );
	break;	
    }	
    return 0 ;
  }

  std::string Flash::chipTypeFromSN( int chipNo, int sn ) {
    switch( chipNo ) {
      case -1: // No chipNo
	clearThrow( "No chipNo specified.\n" );
	break;
      case AMC13Simple::T1:
	if ( sn > 31) { // Kintex 7
	  return "7K325T";
	}
	else if ( sn < 13 ) { // Virtex 6 lx130t
	  return "6VLX130T";
	}
	else { //Virtex 6 lx240t
	  return "6VLX240T";
	}
	break;
      case AMC13Simple::T2:
	if ( sn > 47) { // Spartan 6 lx45t
	  return "6SLX45T";
	}
	else { // Spartan 6 lx25t
	  return "6SLX25T";
	}
	break;
      default:
	clearThrow( "chipNo out of expected range.\n" );
	break;	
    }
    return "";
  }


  void Flash::clearThrow( std::string appendstr ) {
    amc13::Exception::BadFileFormat e ;
    e.Append( appendstr ) ;
    clear() ;
    throw e ;    
  }

  void Flash::clear() {
    file_name.clear() ;
    chip_no = -1 ;
    version.clear() ;
    chip_type.clear() ;
  }
 
  bool Flash::parseChipType(std::string chip_type) {
    int pos = 0;
    int pos1;
    pos1 = chip_type.std::string::find_first_of("VSK", pos);
    //Get the series
    if (pos1==int(std::string::npos)) {
      error = "Missing 'family' character in Chip Type";
      return false;
    }
    else if (pos1-pos != 1) {
      error = "'Series' digit is not one character long!";
      return false;
    }
    else if (isdigit(chip_type[pos])) {
      series = chip_type[pos];
      pos = pos1;
    }
    else {
      error = "'Series' character is not a digit!";
      return false;
    }
    pos1 = chip_type.std::string::find_first_not_of("VSK", pos);
    //Get the family
    if (pos1-pos != 1) {
      error = "Invalid 'family' type";
      return false;
    }
    else {
      family = chip_type[pos];
      pos = pos1;
    }
    pos1 = chip_type.std::string::find_first_of("0123456789", pos);
    //Get the type
    if (pos1-pos != 0) { // Optional 'type'  present
      type = chip_type.substr(pos, pos1-pos);
      pos = pos1;
    }
    pos1 = chip_type.std::string::find_first_not_of("0123456789", pos);
    //Get the size
    if (pos1-pos != 0) {
      size = chip_type.substr(pos, pos1-pos);
      pos = pos1;
    }
    else {
      error = "Logic Size missing from file name";
      return false;
    }
    pos1 = chip_type[std::string::npos];
    //Get the suffix
    if (pos1-pos != 0) { // Optional 'type_suffix' present
      type_suffix = chip_type.substr(pos, pos1-pos);
      return true; // End of the chip_type string
    }
    else 
      return true; // End of the chip_type string
  }
  
  // Helper function converts string to unsigned int
  uint32_t Flash::intFromString(const std::string& s, unsigned int pos, unsigned int n) {
    assert(n<=8);
    return strtoul(s.substr(pos,n).c_str(),NULL, 16);
  }
  
  // Register Read
  uint32_t Flash::read(const std::string& reg) {
    uhal::ValWord<uint32_t> ret; 
    ret = FlashT2 -> getNode( reg ).read() ;
    FlashT2 -> dispatch() ;
    return ret ;  
  }

                                                         
  // Block Read from Address
  size_t Flash::read(uint32_t addr, size_t nWords, uint32_t* buffer) {
    
    //Make sure we have a valid buffer
    if ( buffer==NULL ) {
      throw amc13::Exception::NULLPointer() ;
    }
    
    uhal::ValVector<uint32_t> retVec ;
    std::vector<uint32_t> stdVec ;
    uint32_t offset = 0 ;
    int remainingWords = nWords ;
    
    //Do the read, pass on uhal exceptions
    while( remainingWords ) {
      uint32_t wordsToRead = remainingWords > MAX_BLOCK_READ_SIZE ? MAX_BLOCK_READ_SIZE : remainingWords ; 
      retVec = FlashT2 -> getClient(). readBlock( ( addr+offset ), wordsToRead, uhal::defs::INCREMENTAL );
      FlashT2 -> getClient(). dispatch() ;
      std::copy( retVec.begin(), retVec.end(), std::back_inserter( stdVec ) ) ;
      remainingWords -= wordsToRead ;
      offset += wordsToRead ;
    }
    
    // Make sure stdVec is equal or smaller than the number of requested words
    if ( stdVec.size() > nWords) {
      throw amc13::Exception::UnexpectedRange();
    }
    
    // Copy Block read (stdVec) to buffer array
    for ( size_t i=0; i <  stdVec.size(); i++) {
      buffer[i] = stdVec[i] ;
    }
    return stdVec.size() ;
  }

  // Block Write to Address
  void Flash::write( uint32_t addr, size_t nWords, uint32_t* data) {

    //Make sure we have a valid buffer
    if ( data==NULL ) {
      throw amc13::Exception::NULLPointer();
    }

    //Create writeVector and load it with our data                                                                                                                     
    std::vector<uint32_t> writeVec;
    writeVec.resize(nWords);
    memcpy( &(writeVec[0]), data, nWords*sizeof(uint32_t) );
    FlashT2 -> getClient().writeBlock( addr, writeVec, uhal::defs::INCREMENTAL );
    FlashT2 -> dispatch();
  }

}
