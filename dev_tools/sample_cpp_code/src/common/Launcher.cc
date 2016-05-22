
#include "Launcher.hh"

  Launcher::Launcher()
  {
    LoadCommandList(); //Load list
    hw = NULL;
  }
  
  Launcher::~Launcher()
  {
    if( hw != NULL){
      delete hw;
    }
  }
  
  int Launcher::EvaluateCommand(std::vector<std::string> command)
  {
    //Handle no command
    if(command.size() == 0) {
      return 0;
    }
    
    //Search for command
    int commandIndex = FindCommand(command[0]);
    if(commandIndex >= 0) {
      //Command found!
      
      //Build the string & uint64_t argument vector
      std::vector<std::string> argListString;
      std::vector<uint64_t> argListUint64_t;
      for(size_t iArg = 1; iArg < command.size();iArg++) {
	argListString.push_back(command[iArg]);
	argListUint64_t.push_back(strtoul(command[iArg].c_str(),NULL,0));
      }
      
      //Call the function associated with this command
      //If the return value is negative, we will quit
      return (*this.*(commandPtr[commandIndex]))(argListString,argListUint64_t);
      
    } else { //command not found
      printf("Bad Command: ");
      if( command.size())
	printf("%s\n", command[0].c_str());
      else
	printf("\n");
      return 0;
    }
  }
  
  int Launcher::FindCommand(std::string command)
  {
    for(size_t iCmd = 0; iCmd < commandName.size();iCmd++)
      {
	//Check against each internal command
	if((command.size() == commandName[iCmd].size()) &&
	   (command.compare(commandName[iCmd]) == 0) )
	  {
	    return iCmd; //found command (return index)
	  }
      }
    return -1; //Not found
  }
  
  void Launcher::AddCommandAlias( std::string alias, std::string existingCommand )
  {
    int cmd = FindCommand( existingCommand);
    if( cmd >= 0) {
      AddCommand( alias, commandPtr[cmd], commandHelp[cmd], commandAutoComplete[cmd]);
    } else {
      fprintf( stderr, "Tried to add alias %s to command %s which doesn't exist!\n",
	       alias.c_str(), existingCommand.c_str());
    }
  }
  
  void Launcher::AddCommand(std::string name, 
			    int (Launcher::* fPtr)(std::vector<std::string>,std::vector<uint64_t>),
			    std::string help, 
			    std::string (Launcher::* acPtr)(std::vector<std::string> const &,std::string const &,int))
  {
    //Checks for consistency 
    assert(commandName.size() == commandPtr.size());
    assert(commandName.size() == commandHelp.size());
    assert(commandName.size() == commandAutoComplete.size());
    
    commandName.push_back(name);
    commandPtr.push_back(fPtr);
    commandHelp.push_back(help);
    commandAutoComplete.push_back(acPtr);
  }


std::string Launcher::AutoCompleteCommand(std::string const & line,int state) 
{
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
      if(commandName[pos].find(line) == 0){
	return commandName[pos];
      }
      //compare
    }
  //not found
  return std::string("");
}

std::string Launcher::AutoCompleteSubCommand(std::vector<std::string> const & line,
					     std::string const & currentToken,int state) 
{
  //find command
  if(line.size() >0){
    int iCommand = FindCommand(line[0]);
    if(iCommand >= 0){
      //Call the command's auto complete function
      if (commandAutoComplete[iCommand] != NULL) {
	return (*this.*(commandAutoComplete[iCommand]))(line,currentToken,state);
      }
      return std::string("");
    }
  }

  return std::string("");
}

