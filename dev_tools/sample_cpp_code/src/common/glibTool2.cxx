#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdlib.h>


//TCLAP parser
#include "tclap/CmdLine.h"


#include "Launcher.hh"
#include "CLI.hh"

std::string LimitStringLines(std::string source,size_t beginLineCount = 5,size_t endLineCount = 2) {
  //Load the first beginLineCount lines.
  if((source.size() > 0)&&(source.find('\n') == std::string::npos)){
    source=source+'\n';
  }
  std::string beginString;
  while( beginLineCount && !source.empty()) {
    //Find the next new line
    size_t pos = source.find('\n');
    if(pos == std::string::npos) {
      source.clear();
      break;
    }
    
    //append the line associated with it to our begin string with a tab at the beginning
    beginString += std::string("\t") + source.substr(0,pos) + std::string("\n");
    //Move past the newline
    pos++;
    //trim string
    source = source.substr(pos,source.size()-pos);
    
    beginLineCount--;
  }

  std::string endString;
  while(endLineCount && !source.empty()) {
    //Find the next new line
    size_t pos = source.rfind('\n');
    
    if(pos == std::string::npos) {
      //We didn't find a newline, so this is the last line
      pos = 0;
    } else if(++pos == source.size()) { //Move past the null line, but catch if it was the last char.
      source.resize(source.size()-1);
      continue;
    }
    
    //reverse append the line associated with it to our begin string with a tab at the beginning
    endString = std::string("\t") + source.substr(pos,source.size()-pos) + 
      std::string("\n") + endString;
    
    //trim source string
    if(pos >0) {
      pos--; //Move back to the newline
      source = source.substr(0,pos); //trim up to the newline
    } else { // nothing left, so clear
      source.clear();
    }
    
    endLineCount--;
  }
  
  //BUild final string
  if(!source.empty()) {
    //Count the number of skipped lines if non-zero
    size_t skippedLineCount = 1;
    for(size_t iStr = 0; iStr < source.size();iStr++) {
      if(source[iStr] == '\n')
	skippedLineCount++;
    }
    std::ostringstream s;
    s << "*** Skipping " << skippedLineCount << " lines! ***\n";
    beginString += s.str();
  }
  beginString += endString;
  return beginString;
}

int main(int argc, char* argv[]) 
{
  //Create CLI
  CLI cli;

  //Create Command launcher (early, so we can set things)
  Launcher launcher;

  try {
    TCLAP::CmdLine cmd("Simple tool for talking to glibs",
		       ' ',
		       "glibTool v2");
    
    //glib connections
    TCLAP::MultiArg<std::string> Connections("c",                   //one char flag
					     "connect",             // full flag name
					     "connection file",     //description
					     true,                 //required
					     "string",              // type
					     cmd);
    //Script files
    TCLAP::ValueArg<std::string> scriptFile("X",                   //one char flag
					    "script",              // full flag name
					    "script filename",     //description
					    false,                 //required
					    std::string(""),       //Default is empty
					    "string",              // type
					    cmd);

    // path for address table files
    TCLAP::ValueArg<std::string> adrTblPath( "p", "path", "address table path", false, "", "string", cmd);

    //Parse the command line arguments
    cmd.parse(argc,argv);

    //--- inhibit most noise from uHAL
    uhal::setLogLevelTo(uhal::Error());
    
    char *atp_env;

    if( adrTblPath.isSet()) {
      launcher.SetAddressTablePath( adrTblPath.getValue());
      printf("Address table path \"%s\" set on command line\n", launcher.GetAddressTablePath().c_str() );
    } else if( (atp_env = getenv("GLIB_ADDRESS_TABLE_PATH")) != NULL) {
      launcher.SetAddressTablePath( atp_env);
      printf("Address table path \"%s\" set from GLIB_ADDRESS_TABLE_PATH\n", launcher.GetAddressTablePath().c_str() );      
    } else {
      printf("No address table path specified.\n");
    }
    
    //setup connections
    for(std::vector<std::string>::const_iterator it = Connections.getValue().begin(); 
	it != Connections.getValue().end();
	it++)
      {
	cli.ProcessString("connect " + *it);
      }
  

    //Load scripts
    if(scriptFile.getValue().size()){
      cli.ProcessFile(scriptFile.getValue());
    }



  } catch (TCLAP::ArgException &e) {
    fprintf(stderr, "Error %s for arg %s\n",
	    e.error().c_str(), e.argId().c_str());
    return 0;
  }


  //============================================================================
  //Main loop
  //============================================================================

  //--- inhibit most noise from uHAL
  uhal::disableLogging();

  bool running = true;
  while (running) 
    {
      try {
	//Get parsed command
	std::vector<std::string> command = cli.GetInput(&launcher);
	
	//Launch command function
	if(launcher.EvaluateCommand(command) < 0)
	  {
	    //Shutdown tool
	    running = false;
	  }     
      }catch (uhal::exception::exception & e){
	std::cout << "\n\n\nCaught microHAL exception.\n";
	std::cout << LimitStringLines(e.what()) << std::endl;
	if(cli.InScript()) //Fail to command line if in script
	  running = false;
	cli.ClearInput(); //Clear any scripted commands
      }catch (std::exception  & e){
	std::string errorstr(e.what());
	errorstr.erase(std::remove(errorstr.begin(), 
				   errorstr.end(), 
				   '\n'), 
		       errorstr.end());
	
	std::cout << "\n\n\nCaught std::exception " << errorstr << ". Shutting down.\n";
	running = false;
	cli.ClearInput(); //Clear any scripted commands
      }
    }
  return 0;
}
