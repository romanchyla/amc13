// Parses the address table and prints out LaTeX tables for documentations purposes

#include "amc13/Status.hh"
#include "amc13/Module.hh"
#include <fstream>
#include <iostream>

int main(int argc, char *argv[]) {
  //===================
  //Command line args
  //===================
  if (argc < 3) {
    std::cout << "usage:  LaTeXprint.exe <ip_addr> <address_table_path>" << std::endl;
    return 0;
  }

  //Disable the logging
  uhal::disableLogging();


  //create AMC13 module
  amc13::Module* mod = new amc13::Module();
  mod->Connect( argv[1], argv[2]);

  //create the pointertr
  amc13::Status::Status sts( mod->amc13, 0);
  sts.SetLaTeX();
  std::ofstream file("table.tex"); 
  sts.Report(99,file);
  file.close();
  std::cout << "Wrote to table.tex\n"; 
}
