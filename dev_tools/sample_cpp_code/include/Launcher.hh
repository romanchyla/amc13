#ifndef HCAL_AMC13_COMMANDS
#define HCAL_AMC13_COMMANDS


#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include <stdint.h>

#include "uhal/uhal.hpp"


//-- non-member functions
void ReplaceStringInPlace(std::string& subject, const std::string& search,
			  const std::string& replace);
  
  
class Launcher {
public:
  Launcher();
  ~Launcher();
  
  int EvaluateCommand(std::vector<std::string> command);
  
  //For CLI to auto complete commands
  std::string AutoCompleteCommand(std::string const & line,int state);  
  std::string AutoCompleteSubCommand(std::vector<std::string> const & line,
				     std::string const & currentToken,int state);

  // set/get address table path
  void SetAddressTablePath( std::string p) { addressTablePath = p; }
  std::string GetAddressTablePath() { return addressTablePath; }

private:
  //A mapping between command string and function to call
  //The called function returns an int and takes an AMC13 class, a vector of argument strings and a vector of uint64_t conversions of those strings
  void AddCommand(std::string name, 
		  int (Launcher::*)(std::vector<std::string>,std::vector<uint64_t>),
		  std::string help, 
		  std::string (Launcher::*)(std::vector<std::string> const &,std::string const &,int)=NULL);
  void AddCommandAlias( std::string alias, std::string existingCommand );
  int FindCommand(std::string command);
  /*! \brief read a chip from a command (t1 or t2)          
     
   */

  typedef int (Launcher::*CommandFcnPtr)(std::vector<std::string>,std::vector<uint64_t>);
  std::vector<std::string> commandName;
  std::vector<CommandFcnPtr> commandPtr;
  std::vector<std::string> commandHelp;
  std::vector<std::string (Launcher::*)(std::vector<std::string> const &,std::string const &,int)> commandAutoComplete;
    
  // address table path (a bit ugly, but has to go somewhere!)
  std::string addressTablePath;

  uhal::HwInterface *hw;

    
  //====================================================
  //Add your commands here
  //====================================================
    
  //Here is where you update the map between string and function
  void LoadCommandList();
    
  //Add new command functions here
  int Help(std::vector<std::string>,std::vector<uint64_t>);	   
  int Quit(std::vector<std::string>,std::vector<uint64_t>);	   
  int Echo(std::vector<std::string>,std::vector<uint64_t>);	   
  int Sleep(std::vector<std::string>,std::vector<uint64_t>);	   

  int Connect(std::vector<std::string>,std::vector<uint64_t>);	   
  int Read(std::vector<std::string>,std::vector<uint64_t>);      
  int Write(std::vector<std::string>,std::vector<uint64_t>);      

  int OpenFile(std::vector<std::string>,std::vector<uint64_t>);
  int CloseFile(std::vector<std::string>,std::vector<uint64_t>);
  int MrWuRegisterDump(std::vector<std::string>,std::vector<uint64_t>);


  //Add new command (sub command) auto-complete files here
  std::string autoComplete_Help(std::vector<std::string> const &,std::string const &,int);
//  std::string autoComplete_T1AddressTable(std::vector<std::string> const &,std::string const &,int);
//  std::string autoComplete_T2AddressTable(std::vector<std::string> const &,std::string const &,int);
//  std::string autoComplete_StatusTable(std::vector<std::string> const &,std::string const &,int);
};

#endif
  
