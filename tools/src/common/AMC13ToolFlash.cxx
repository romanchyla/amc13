#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>

//For parsing connection inputs. 
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

//TCLAP parser
#include "tclap/CmdLine.h"

//#include "amc13/CLI.hh"
#include "amc13/Module.hh"


//_________Executable for initial programming of flash (works if only T2 avaiable)_________________


int VerifySector( amc13::Flash * flash, int chip, std::string ver ) {
  // Get chip_type
  std::string chipType;

  //uhal::ValWord<uint32_t> sn = pAMC13->getT2()->getNode("STATUS.SERIAL_NO").read();
  //pAMC13->getT2()->dispatch();
  uint32_t sn = flash->read("STATUS.SERIAL_NO");  
  
  printf("Serial Number: %d\n", sn );
  chipType = flash->chipTypeFromSN( chip, sn );

  // Select file 
  std::string selectFile = flash->selectMcsFile( chip, ver, chipType);
  if (selectFile != "") {
    printf("Verifying against file: %s...\n", selectFile.c_str() );
    flash->verifyFlash( selectFile );
  }else{
    printf("No MCS file selected\n");
    return 0;
  }
  return 0;
}

int ProgramSector( amc13::Flash * flash, int chip, std::string ver) {
  // Get chip_type
  std::string chipType;
  
  // check serial number and use chip_type corresponding 
  //uhal::ValWord<uint32_t> sn = pAMC13->getT2()->getNode("STATUS.SERIAL_NO").read();
  //pAMC13->getT2()->dispatch();
  uint32_t sn = flash->read("STATUS.SERIAL_NO"); 

  printf("Serial no: %d\n", sn );
  chipType = flash->chipTypeFromSN( chip, sn ) ;
  
  //printf( "Searching for files with T2, and %s...\n", chipType.c_str() );
  
  std::string selectFile = flash->selectMcsFile( chip, ver, chipType);
  if (selectFile != "") {
    printf("Programming against file: %s... \n", selectFile.c_str() );
    flash->programFlash( selectFile );
    flash->verifyFlash(selectFile);
  }else{
    printf("No MCS file selected\n");
    return 0;
  }
  return 0;    
}

//Checks if file matches chip, ver, and chipType
bool ValidateMCS(int chip, std::string ver, std::string chipType, std::string file) {
  std::string origName = file;
  file = file.substr(file.find_last_of("/")+1);          //Remove path name
  file = boost::to_upper_copy(file);

  std::string suffix = ".MCS";
  //Check if ends with .mcs
  if (!(file.size() >= suffix.size() && 
	file.compare(file.size() - suffix.size(), suffix.size(), suffix) == 0)) { 
    printf("ERROR: %s is not a .mcs file\n",origName.c_str());
    return false;
  }
  //Check if sufficiently long
  if (file.size() < 16) {
    printf("ERROR: %s is shorter than 16 characters\n",origName.c_str());
    return false;
  }
  //Check if starts with AMC13T
  if (file.substr(0,6) != "AMC13T") {
    printf("ERROR: %s does not begin with AMC13T\n",origName.c_str());
    return false;
  }
  //Check if next char is 1 or 2
  if (file[6] != '1' && file[6] != '2') {
    printf("ERROR: %s does not have T1 or T2 in correct format\n",origName.c_str());
    return false;
  }
  int fileChip = (file[6] == '1') ? 1 : 0;
  //Check if programming to T1 or T2 and that the file given is for given tongue
  if (fileChip != chip) {
    printf("ERROR: %s is meant for T%c\n",origName.c_str(),file[6]);
    return false;
  }
  file.erase(0,7);
  //Check if next char is v H or G
  if (file[0] != 'V' && file[0] != 'H' && file[0] != 'G') {
    printf("ERROR: %s has unexpected verion format as seventh character\n",origName.c_str());
    return false;
  }
  if (file[0] == 'V' && ver != "") {
    printf("ERROR: %s has unexpected verion format as seventh character\n",origName.c_str());
    return false;
  }
  if (file[0] == 'H' && ver != "HEADER") {
    printf("ERROR: %s has unexpected verion format as seventh character\n",origName.c_str());
    return false;
  }
  if (file[0] == 'G' && ver != "GOLDEN") {
    printf("ERROR: %s has unexpected verion format as seventh character\n",origName.c_str());
    return false;
  }
  file = file.substr(file.find("_")+1, file.size() -file.find("_") - 1 - suffix.size() );
  if (file != chipType) {
    printf("ERROR: %s is not for %s chip type\n",origName.c_str(),chipType.c_str());
    return false;
  }
  return true;
}

int VerifySector(amc13::Flash * flash, int chip, std::string ver, std::string selectFile) {
  std::string chipType;
  uint32_t sn = flash->read("STATUS.SERIAL_NO");

  printf("serial no: %d\n", sn);
  chipType = flash->chipTypeFromSN(chip,sn);
  if (ValidateMCS(chip, ver, chipType, selectFile)) {
    printf("Verifying against file: %s...\n", selectFile.c_str() );
    flash->verifyFlash( selectFile );
  } else {
    printf("MCS File given on command line is invalid\n");
    return 0;
  }
  return 0;
}

int ProgramSector(amc13::Flash * flash, int chip, std::string ver, std::string selectFile) {
  std::string chipType;
  uint32_t sn = flash->read("STATUS.SERIAL_NO");

  printf("Serial no: %d\n", sn);
  chipType = flash->chipTypeFromSN(chip,sn);
  if (ValidateMCS(chip, ver, chipType, selectFile)) {
    printf("Programming against file: %s... \n", selectFile.c_str() );
    flash->programFlash(selectFile);
    flash->verifyFlash(selectFile);
  } else {
    printf("MCS File given on command line is invalid\n");
    return 0;
  }
  return 0;
}


//User sets up list of MCS files and then all sectors are programmed. 
int ProgramAll(amc13::Flash * flash){
  //Order: pfh, pbs, ps, pv
  std::string chipType;
  std::string MCSFile[4];

  uint32_t sn = flash->read("STATUS.SERIAL_NO"); 
  printf("Serial number: %d\n", sn );  

  //Set up arrays with chip number and version names
  int chip [4] = {amc13::AMC13::T2, amc13::AMC13::T2, amc13::AMC13::T2, amc13::AMC13::T1};
  std::string version [4] = {"HEADER", "GOLDEN", "", ""};
  std::string sectorName [4] = {"Header", "Golden", "Spartan", "Virtex"}; //For printing only

  //Make array of MCS files
  for(int iSector=0;iSector<4;iSector++){
    std::cout<<"-------------------------------------------------"<<std::endl;
    chipType = flash->chipTypeFromSN( chip[iSector], sn ) ;
    std::cout<<"Select "<<sectorName[iSector]<<" MCS file"<<std::endl;
    MCSFile[iSector] = flash->selectMcsFile( chip[iSector], version[iSector], chipType);
  }

  //Program and verify all sectors from MCS array
  for(int iSector=0;iSector<4;iSector++){
    std::cout<<"-------------------------------------------------"<<std::endl;
    if (MCSFile[iSector] != "") {
      //std::cout<<"Sector:"<<iSector<<std::endl<<"Chip:"<<chip[iSector]<<std::endl<<"Version:"<<version[iSector]<<std::endl;
      //std::cout<<"chipType:"<<chipType<<std::endl;
      std::cout<<"Programming "<<sectorName[iSector]<<" against file:\n"<<MCSFile[iSector]<<"..."<<std::endl;
      flash->programFlash( MCSFile[iSector] );
      flash->verifyFlash(MCSFile[iSector]);
    }else{
      std::cout<<"No MCS file selected for "<<sectorName[iSector]<<std::endl;
    } 
  }
  std::cout<<std::endl<<"Done programming all sectors."<<std::endl<<std::endl;
  return 0;
}

//Check if a given file exists
inline bool checkFile(const std::string& str) {
  std::ifstream f(str.c_str());
  return f.good();
}

int main(int argc, char* argv[]){

  char *atp_env;
  std::string addrtable;  
  std::vector <int> flashChip;
  std::vector <std::string> flashVersion;
  std::vector <int> flashSwitch;
  std::vector <std::string> flashSectorName;
  std::vector <std::string> firmwareFile;
  static char T1_uri[256]; //"ipbusudp-2.0://"+*amc13Connections.getValue().begin()+":50001";
  static char T2_uri[256];
  bool reloadFPGA;
  bool listFirmware;
  bool programFlash;
  uhal::HwInterface * hwT1 = NULL;
  uhal::HwInterface * hwT2 = NULL;
  amc13::Flash *flash = NULL;
  bool debug = false;
    
  try{
    TCLAP::CmdLine cmd("Tool for programming flash to AMC13 modules.",
		       ' ',
		       "AMC13 Flash Tool");
    // Tool currently supports IP address connection and SN, connectionfile and prefix handling may come in the future
    //AMC 13 connections

    TCLAP::ValueArg<std::string> amc13Connections("c",                 //one char flag
						  "connect",           //full flag name
						  "IP address (numeric) or serial number. " \
						  "Append \"/c\" to use control hub. " \
						  "(currently connection files and URIs are not supported)",
						  false,               //required
						  "",                  //type
						  "string",
						  cmd);
    
    // path for address table files
    TCLAP::ValueArg<std::string> adrTblPath( "p", "path", "address table path", false, "", "string", cmd);

    //AMC 13 flash command
    TCLAP::MultiArg<std::string> flashCommand("f",                   //one char flag
					      "flash",               //full flag name
					      "Program options: pfh(flash header), pbs(backup spartan), ps(spartan), pv/pk(virtex/kintex), pa(program all). All verify flash when programming is finished. Verify alone can be called with vfh, vbs, vs, vv/vk.",//description
					      false,                 //required
					      "string",              //type
					      cmd);

    //AMC13 Firmware file. List files to be used
    TCLAP::MultiArg<std::string> mcsFile("m",                        //one char flag
					 "mcs",                      //full flag name
					 "Firmware versions to program into flash. Will check compatibility of firmware with specified flash options",//description
					 false,                      //required
					 "string",
					 cmd);

    //Command to reload FPGAs after programming flash. Default false, -r for true. If true no need for -f flag.  
    TCLAP::SwitchArg reloadCommand("r",
                                   "ReloadFPGAs",
                                   "Reload FPGAs after flash, default off, -r ro reload, no need for -f when true",
                                   cmd,
                                   false);

    //Disply firmware versions.  
    TCLAP::SwitchArg firmwareCommand("l",
				     "ListFirmware",
				     "Lists T1 and T2 in the flash memory. May not be current being used by chips.",
				     cmd,
				     false);

    /*
    //Not implementing this for now. Use the -c flag with an ip address or SN.
    //AMC 13 uri
    TCLAP::ValueArg<std::string> amc13URI("u",                          //one char flag
					  "uri",                        //full flag name
					  "protocol,ip,port",          //description
					  false,                        //required
					  "",
					  "string",                     //type
					  cmd);
    */    

    cmd.parse(argc,argv);
    reloadFPGA = reloadCommand.getValue();
    listFirmware = firmwareCommand.getValue();
    programFlash = flashCommand.isSet();
    
    //Argument conflict check:
    if(reloadCommand.isSet() && firmwareCommand.isSet()){
      printf("Only use -r or -l, not both, in the same call of AMC13ToolFlash\n");
      return 0;
    }

    //--- inhibit most noise from uHAL
    uhal::setLogLevelTo(uhal::Error());
     
    // Set address table path
    if( adrTblPath.isSet() ) {
      addrtable = adrTblPath.getValue() ;
      printf("Address table path \"%s\" set on command line\n", addrtable.c_str() );   
    } else if( (atp_env = getenv("AMC13_ADDRESS_TABLE_PATH")) != NULL) {
      addrtable = atp_env;      
    } else {
      printf("No address table path specified.\n");
      return 0;
    }

    //Check for slash at end of address path
    if(addrtable[addrtable.size()-1] != '/'){
      addrtable = addrtable + "/";
    }
    std::cout<<"Address table path "<<addrtable<<" set from AMC13_ADDRESS_TABLE_PATH"<<std::endl;      

    // Parse -f flash command and set chip no, version, and if verifying or programming (flash_flag). 
    if( flashCommand.isSet() ) {
      flashChip.clear();
      flashVersion.clear();
      flashSwitch.clear();
      for(std::vector<std::string>::const_iterator it = flashCommand.getValue().begin();
	  it != flashCommand.getValue().end();
	  it++){
	//Program all memory
	if(*it == "pa"){
	  flashSectorName.push_back("all memory");
	  flashChip.push_back(amc13::AMC13::T1);
	  flashVersion.push_back("");
	  flashSwitch.push_back(0);
	  //Program virtex/kintex memory
	}else if( (*it == "pv") || (*it == "pk") || (*it == "vk") || (*it == "vv")) {
	  flashChip.push_back(amc13::AMC13::T1);
	  flashVersion.push_back("");
	  if( (*it == "pv") || (*it == "pk")){
	    flashSectorName.push_back("Virtex/Kintex");
	    flashSwitch.push_back(1);
	  }else{
	    flashSectorName.push_back("Virtex/Kintex");
	    flashSwitch.push_back(-1);
	  }
	  //Program T2 memory
	}else {
	  flashChip.push_back(amc13::AMC13::T2);
	  if( (*it == "pfh") || (*it == "vfh")) {
	    flashVersion.push_back("HEADER");
	    if( (*it == "pfh")){
	      flashSectorName.push_back("flash header");
	      flashSwitch.push_back(1);
	    }else{
	      flashSectorName.push_back("flash header");
	      flashSwitch.push_back(-1);
	    }
	  }else if( (*it == "pbs") || (*it == "vbs")) {
	    flashVersion.push_back("GOLDEN"); 
	    if( (*it == "pbs")){
	      flashSectorName.push_back("backup Spartan(Golden)");
	      flashSwitch.push_back(1);
	    }else{
	      flashSectorName.push_back("backup Spartan(Golden)");
	      flashSwitch.push_back(-1);
	    }
	  }else if( (*it == "ps") ||  (*it == "vs")) {
	    flashVersion.push_back("");
	    if( (*it == "ps")){
	      flashSectorName.push_back("Spartan");
	      flashSwitch.push_back(1);
	    }else{
	      flashSectorName.push_back("Spartan");
	      flashSwitch.push_back(-1);
	    }
	  }else{
	    printf("Please use valid flash command (pfh,vfh,pbs,vbs,ps,vs,pv,vv,pk,vk,pa)\n");
	    return 0;
	  }
	}
      }    
    }
    

    //Check to see if the files given exist
    if (mcsFile.isSet()) {
      for (std::vector<std::string>::const_iterator it = mcsFile.getValue().begin(); 
	   it != mcsFile.getValue().end(); it++) {
	if (checkFile(*it)) {
	  firmwareFile.push_back(*it);
	} else {
	  printf("File %s not found, exiting tool\n", it->c_str());
	  return 0;
	}
      }
    }

    //Check valid connection
    //if ( (!amc13Connections.isSet() || connSize != 1) && !amc13URI.isSet() ){
    if ( !amc13Connections.isSet() ){
      printf("Please specify (-c) an IP address or SN.\n");
      return 0;
    } else { 
      //Generate URI from IP address. Can use /c for control hub. 
      //For reference: "ipbusudp-2.0://"+*amc13Connections.getValue().begin()+":50001"
      std::string connInput=amc13Connections.getValue();
      std::string IPAddress = "";
      uint8_t oct_last=0;
      bool use_ch=0;

      // Check for numeric IP address or Serial Number using Boost and check /c for control hub
      // separate last octet so we can increment it
      static const boost::regex IP("(\\d{1,3}.\\d{1,3}.\\d{1,3}.)(\\d{1,3})(/[cC])?");
      static const boost::regex SN("(\\d{1,3})(/[cC])?");
      static const boost::regex fileXml("\\S*\\.[xX][mM][lL]");
      static boost::cmatch what;
           
      //Connection with IP address:
      if( boost::regex_match( connInput.c_str(), what, IP)) {
	std::string ip_addr(what[1].first, what[1].second); // extract 1st 3 octets
	IPAddress = ip_addr; 
	std::string ip_last(what[2].first, what[2].second); // extract last octet  
	oct_last = atoi(ip_last.c_str());
	use_ch = what[3].matched; // check for /c suffix
      }
      //Connection with SN:
      else if( boost::regex_match( connInput.c_str(), what, SN)){
	int SNInput = atoi(what[1].first);
	if (SNInput<128){
	  IPAddress ="192.168.1.";
	  oct_last = 254-2*SNInput;
	}else if (SNInput<256){
	  IPAddress ="192.168.2.";
	  oct_last = 254-2*SNInput;
	}else{
	  IPAddress ="192.168.3.";
	  oct_last = 254-2*SNInput%128;
	}
	use_ch = what[2].matched;
      }else{
	printf("No regular expression matches\n");
	return 0;
      }
	
      // specify protocol prefix
      if( use_ch) 
	printf("Using control hub true\n");
      else
	printf("Using control hub false\n");
      
      std::string proto = use_ch ? "chtcp-2.0://localhost:10203?target=" : "ipbusudp-2.0://";
      
      snprintf(T2_uri, 255, "%s%s%d:50001", proto.c_str(), IPAddress.c_str(), oct_last);
      snprintf(T1_uri, 255, "%s%s%d:50001", proto.c_str(), IPAddress.c_str(), oct_last+1);
      printf("Created URI from IP address:\n  T2: %s\n  T1: %s\n",T2_uri,T1_uri);
    }
  }catch (TCLAP::ArgException &e) {
    fprintf(stderr, "Error %s for arg %s\n",
	    e.error().c_str(), e.argId().c_str());
    return 0;
  }

  //____________Connect to the board and run flash commands_____________//         

  if(!debug){
    try{
      // Build the Hardware Interface using URI and xml file.
      std::cout << "Connecting to the board..."<<std::endl;
      //Connect to T2. Should always throw and shut down if not connected.
      hwT2 = new uhal::HwInterface( uhal::ConnectionManager::getDevice( "T2", T2_uri, "file://"+addrtable+"AMC13XG_T2.xml" ) );
      //Connect to T1. This will throw if connection cannot be made, but the tool will continue without it.
      try{
	hwT1 = new uhal::HwInterface( uhal::ConnectionManager::getDevice( "T1", T1_uri, "file://"+addrtable+"AMC13XG_T1.xml" ) );
      }catch(uhal::exception::exception & e){
	std::cout<<"Cannot connect to T1. Will continue without."<<std::endl;
      }
	
      //Create a flash object with the T2 hardware interface:
      flash = new amc13::Flash(hwT2);

      std::cout<<"---------------------------------------------------"<<std::endl;

      // Program/verify flash
      if( programFlash ){
	for(size_t i=0;i<flashSwitch.size();i++){
	  if(flashSwitch[i]>0){
	    std::cout<<"Programming "<<flashSectorName[i]<<std::endl;
	    if ( i < firmwareFile.size() ) {
	      ProgramSector( flash, flashChip[i], flashVersion[i], firmwareFile[i]);
	    } else {
	      ProgramSector( flash, flashChip[i], flashVersion[i]);
	    }
	  }	  
	  else if(flashSwitch[i]<0){
	    std::cout<<"Verifying "<<flashSectorName[i]<<std::endl;	    
	    if ( i < firmwareFile.size() ) {
	      VerifySector( flash, flashChip[i], flashVersion[i], firmwareFile[i]);
	    } else {
	      VerifySector( flash, flashChip[i], flashVersion[i]);
	    }
	  }	    
	  else{
	    std::cout<<"Programming "<<flashSectorName[i]<<std::endl;
	    ProgramAll(flash);
	  }
	  std::cout<<"---------------------------------------------------"<<std::endl;
	}       
      }

      // Reload FPGAs
      if(reloadFPGA){
	printf("Reloading FPGAs from flash...\n");
	printf("User should wait 10 seconds before connecting again.\n");
	flash->loadFlash();
      }

      //List firmware.
      if(listFirmware){
	printf("Firmware Version: ");
        if(hwT1 != NULL){
	  uhal::ValWord<uint32_t> ret = hwT1->getNode("STATUS.FIRMWARE_VERS").read();
	  hwT1->dispatch();
	  uint32_t T1Firmware = ret;
	  printf("T1=%04x",T1Firmware);
	}else{
	  printf("T1=Did not connect");
	}
	if(hwT2 != NULL)
	  printf("   T2=%04x\n",flash->read("STATUS.FIRMWARE_VERS"));
	else
	  printf("   T2=null\n");
      }
      
      if(!programFlash && !reloadFPGA && !listFirmware)
	printf("No command entered! Use --help for command options.\n");
      
    }catch (std::exception & e){
      if(hwT1!=NULL)
	delete hwT1;
      if(hwT2!=NULL)
	delete hwT2;
      if(flash!=NULL)
	delete flash;
      std::string errorstr(e.what());
      std::cout<< "\n\n\nCaught exception"<<std::endl<< errorstr << "Shutting down.\n";
      return 0;
    }
  } 
  if(hwT1!=NULL)
    delete hwT1;
  if(hwT2!=NULL)
    delete hwT2;
  if(flash!=NULL)
    delete flash;
  return 0;
}
