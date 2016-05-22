#include "Launcher.hh"
#include <boost/tokenizer.hpp>

  void Launcher::LoadCommandList()
  {
    
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
    
    AddCommand("connect",&Launcher::Connect,
	       "Connect to glib\n");
    AddCommand("r",&Launcher::Read,
	       "Read from glib\n");
    AddCommand("w",&Launcher::Write,
	       "Write to glib\n");

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

    //Build a map of commands to the names that are connected to them.
    //The order of the vector will be the same as the order of the 
    //If only one command is desired, only save command names that point to the same function
    std::map<size_t,std::vector<size_t> > helpMap;
    for(size_t iEntry = 0; iEntry < commandPtr.size();iEntry++){
      //Find the first instance of this actual function
      size_t baseCommand = iEntry;
      for(size_t iSearch = 0; iSearch < baseCommand;iSearch++){
	if(commandPtr[iSearch] == commandPtr[baseCommand]){
	  baseCommand = iSearch;
	}
      }
      if(thisCommand == -1){
	helpMap[baseCommand].push_back(iEntry);
      }else if(size_t(thisCommand) == iEntry){
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

