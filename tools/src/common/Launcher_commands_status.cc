#include "amc13/Launcher.hh"

namespace amc13{

  bool IsNumberHelper(const std::string & str){
      bool levelIsNumber = true;
      for(size_t iDigit = 0; iDigit < str.size();iDigit++){
	//Check if char is 0-9,'-',or '.'.
	levelIsNumber = levelIsNumber && (isdigit(str[iDigit]) || str[iDigit] == '-' || str[iDigit] == '.');
      }
      return levelIsNumber;
  }

    
  int Launcher::Status(std::vector<std::string> strArg,
		       std::vector<uint64_t> intArg)
  {
    bool toFile = (defaultModule()->isFileOpen());
    // choose stream for output, default to std::cout
    std::ostream& stream = (toFile) ? defaultModule()->getStream() : std::cout;

    if(intArg.size()==0){
      //default to level 1 display
      defaultAMC13()->getStatus()->Report(1,stream);
    } else if(intArg.size()==1){
      //arg 0 should be an integer
      if (IsNumberHelper(strArg[0])){
	defaultAMC13()->getStatus()->Report(intArg[0],stream);
      } else {
	std::cout << "Error: \"" << strArg[0] << "\" is not a valid print level\n";
      }
    } else if(intArg.size() > 1){
      //arg 0 should be an integer
      if (IsNumberHelper(strArg[0])){
	defaultAMC13()->getStatus()->Report(intArg[0],stream,strArg[1]);
      } else {
	std::cout << "Error: \"" << strArg[0] << "\" is not a valid print level\n";
      }
    }
    if (toFile) {
      std::cout << "Wrote to " << defaultModule()->getFileName() << std::endl;
    }
    return 0;
  }

  int Launcher::StatusHTML(std::vector<std::string> strArg,
		       std::vector<uint64_t> intArg)
  {
    // first handle the arguments
    int a_level = 1;
    std::string a_table = "";
    bool a_bare = false;

    if( strArg.size() > 0) {
      for( size_t i=0; i<strArg.size(); i++) {
	char first = strArg[i].c_str()[0];
	if( isdigit( first)) {
	  a_level = intArg[i];
	} else if( first == '-') {
	  switch( toupper( strArg[i].c_str()[1])) {
	  case 'B':
	    a_bare = true;
	    break;
	  default:
	    std::cout << "unknown option " << strArg[i] << std::endl;
	  }
	} else {
	  a_table = strArg[i];
	  std::cout << "Outputting only HTML table " << a_table << std::endl;
	}
      }
    }

    defaultAMC13()->getStatus()->SetHTML();
    bool toFile = (defaultModule()->isFileOpen());
    std::ostream& stream = (toFile) ? defaultModule()->getStream() : std::cout; 
    try{
      if( a_bare) {
	stream << defaultAMC13()->getStatus()->ReportBare( a_level, a_table);
      } else {
	defaultAMC13()->getStatus()->Report(a_level,stream,a_table);
      }
    }catch (std::exception e){
      //We want to make sure the HTML gets unset if something bad happens
    }
    if (toFile) {
      std::cout << "Wrote to " << defaultModule()->getFileName() << std::endl;
    }
    defaultAMC13()->getStatus()->UnsetHTML();
    return 0;
  }

  int Launcher::OpenFile(std::vector<std::string> strArg,
			 std::vector<uint64_t> intArg)
  {
    if (strArg.size() == 0) {
      printf("No file given\n");
      return 0;
    } else {
      defaultModule()->setStream(strArg[0].c_str());
      printf("status will be streamed to file\n");
      return 0;
    }
  }

  int Launcher::CloseFile(std::vector<std::string> strArg,
			  std::vector<uint64_t> intArg) 
  {
    defaultModule()->closeStream();
    printf("Status file closed\n");
    return 0;
  }


  int Launcher::MrWuRegisterDump(std::vector<std::string> strArg,
			         std::vector<uint64_t> intArg)
  {
    
    std::string filename;
    uint32_t sleepTime = 2;
    uint32_t repetitions = 2;
    uint32_t statusLevel = 9;

    /// for default filename ///
    time_t curTime;
    struct tm * timeinfo;
    time( &curTime );
    timeinfo = localtime( &curTime );
    char datetime [34];
    strftime( datetime, 34, "AMC13XGDump_%F_%H%M%S.txt", timeinfo );

    // Starting with largest number and excluding "break;" until the case 1
    // has us start at case strArg.size() and execute all assignments for
    // the variables in lower positions.
    switch ( strArg.size() ) {
    case 4:
      statusLevel = intArg.at(3);
    case 3:
      sleepTime = intArg.at(2);
    case 2:
      repetitions = intArg.at(1);
    case 1:
      filename = strArg.at(0);
      break;
    default:
      filename = datetime;
    }

    if ( filename.length() > 4 ) {
      if ( filename.substr( filename.length()-4,4 ) != ".txt" ) {
	printf("Please give a .txt file\n");
	return 0;
      }
    }
    else {
      printf("Please give a .txt file\n");
      return 0;
    }

    // check if file already exists
    std::ifstream xfile( filename.c_str() );
    if ( xfile.good() ) {
      printf("File name already in use\n");
      xfile.close();
      return 0;
    }

    std::ofstream outputFile( filename.c_str(), std::ofstream::out );

    for ( uint32_t iREP = 0; iREP < repetitions; ++iREP ) {
      printf("Dumping registers\n");
      MrWuRegDumpHelper( outputFile, iREP );
      if ( iREP != repetitions - 1 ) {
	printf( "Waiting for %u seconds between register dumps...\n", sleepTime );
	clock_t waiter = sleepTime*CLOCKS_PER_SEC + clock();
	while ( waiter > clock() ); // waits for sleepTime seconds
      }
    }
    printf("Register dumps completed\n");
    
    outputFile << "\n";
    defaultAMC13()->getStatus()->Report( statusLevel, outputFile );

    /// close file ///
    outputFile.close();

    printf( "Adding \">status\" output with verbosity level %u\n", statusLevel );

    char cwd[256];
    getcwd(cwd, 256);
    printf( "Output found at %s/%s\n", cwd, filename.c_str() );
    printf( "Please e-mail wusx@bu.edu with details of the conditions under which the dump was captured and the problems experienced.\n" );

    return 0;
  }

  void Launcher::MrWuRegDumpHelper(std::ofstream& filestream, uint32_t iteration) {
    // Will be called repeatedly from MrWuRegisterDump. Adds next register dump to file

    uint32_t outBufferV[0x2000];
    uint32_t outBufferS[0x1000];
    defaultAMC13()->read( AMC13Simple::T1, 0, 0x2000, outBufferV );
    defaultAMC13()->read( AMC13Simple::T2, 0, 0x1000, outBufferS );

    filestream << std::hex;

    filestream << "Dump " << iteration+1 << ": --- T1 Registers ---\n";
    for ( uint16_t vWORD = 0; vWORD < 0x2000; ++vWORD ) {

      if ( vWORD % 8 == 0 ) {
	// ternary operator prevents first line of file from being blank
	filestream << ((vWORD>0) ? "\n" : "") << std::setw(8) << std::setfill('0') << vWORD << ": ";
      }
      filestream << " " << std::setw(8) << std::setfill('0') << outBufferV[vWORD];
    }

    filestream << "\n\n";

    filestream << "Dump " << iteration+1 << ": --- T2 Registers ---";
    for ( uint16_t sWORD = 0; sWORD < 0x1000; ++sWORD ) {

      if ( sWORD % 8 == 0 ) {
	filestream << "\n" << std::setw(8) << std::setfill('0') << sWORD << ": ";
      }
      filestream << " " << std::setw(8) << std::setfill('0') << outBufferS[sWORD];

    }

    filestream << "\n\n";

    return;
  }


      static void statusTableAutoCompleteHelper(uhal::Node const & baseNode,std::vector<std::string> & completionList,std::string const & currentToken)
  {
    //process all the nodes and look for table names
    std::map<std::string,int> matches;
    for(uhal::Node::const_iterator itNode = baseNode.begin();
	itNode != baseNode.end();
	itNode++){
      boost::unordered_map<std::string,std::string> parameters = itNode->getParameters();
      boost::unordered_map<std::string,std::string>::iterator itTable;
      itTable = parameters.find("Table");
      if(itTable != parameters.end()){	
	if(itTable->second.find(currentToken) == 0){
	  matches[itTable->second] = 0;
	}
      }
    }
    for(std::map<std::string,int>::iterator it = matches.begin();
	it != matches.end();
	it++){
      completionList.push_back(it->first);
    }
  }
  std::string Launcher::autoComplete_StatusTable(std::vector<std::string> const & line,std::string const &currentToken,int state)
  {  
    if(line.size() >= 2){
      static size_t pos;
      static std::vector<std::string> completionList;
      if(!state) {
	completionList.clear();
	//Check if we are just starting out
	pos = 0;
	uhal::HwInterface* hw;

	hw = defaultAMC13()->getChip(AMC13Simple::T1);
	if(hw != NULL){
	  statusTableAutoCompleteHelper(hw->getNode(),completionList,currentToken);
	}
	hw = defaultAMC13()->getChip(AMC13Simple::T2);
	if(hw != NULL){
	  statusTableAutoCompleteHelper(hw->getNode(),completionList,currentToken);	
	}
	
      } else {
	//move forward in pos
	pos++;
      }
      
      
      if(pos < completionList.size()){
	return completionList[pos];
      }
    }
    //not found
    return std::string("");  
  }

}
