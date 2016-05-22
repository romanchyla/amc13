//
// Program to dump the status items as an HTML table for preliminary documentation
//

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "amc13/Status.hh"
#include "amc13/Module.hh"

#include <fstream>
#include <iostream>

void make_table_matching( uhal::HwInterface* brd, std::string type, int level);

int main(int argc, char *argv[]) {
  amc13::Module pMod;
  char *atp = getenv("AMC13_ADDRESS_TABLE_PATH");

  //===================
  //Command line args
  //===================
  if (argc < 2) {
    std::cout << "Usage:  printDocoTable.exe <connection_file>|<ip-address> [<dump-level>]" << std::endl;
    return 0;
  }

  int level = 99;
  if( argc > 2) {
    level = atoi( argv[2]);
  }

  //Disable the logging
  uhal::disableLogging();

  if( atp == NULL) {
    printf("Please set AMC13_ADDRESS_TABLE_PATH to location of address table files\n");
    exit( 1);
  }

  // HTML style
  printf("<!DOCTYPE html>\n");
  printf("<html>\n");
  printf("<head>\n");
  printf("<style>\n");
  printf("table, th, td { border: 1px solid black; }\n");
  printf("table { width: 100%%; }\n");
  printf("td { padding: 3px; }\n");
  printf("th { background-color: gray; color: white; }\n");
  printf("table { border-collapse: collapse; }\n");

  printf("#tbl { color: red; font-weight: bold; }\n");
  printf("#col { color: DarkGreen; font-weight: bold; }\n");
  printf("#row { color: blue; font-weight: bold; }\n");
  printf("#name { font-family: courier; color: DarkSlateGray; }\n");
  printf("#descr { font-family: times; }\n");

  printf("</style><body>\n");

  pMod.Connect( argv[1], atp);
  amc13::AMC13* amc13 = pMod.amc13;
  uhal::HwInterface * t1 = amc13->getT1();
  uhal::HwInterface * t2 = amc13->getT2();

  printf("<h1>Kintex (T1)</h1>\n");

  make_table_matching( t1, "STATUS", level);
  make_table_matching( t1, "CONF", level);

  printf("<h1>Spartan (T2)</h1>\n");
  make_table_matching( t2, "STATUS", level);
  make_table_matching( t2, "CONF", level);
}

void make_table_matching( uhal::HwInterface* brd, std::string type, int level) {

  // deal with C++ madness with some typedefs
  typedef std::pair<std::string, std::string> a_spair;
  typedef std::map<std::string, a_spair> a_map1s; // map string, pair
  typedef std::map<std::string, a_map1s> a_map2s;     // 2nd level map
  typedef std::map<std::string, a_map2s> a_map3s;     // 3rd level map
  
  a_map3s mat;			// declare our big matrix

  printf("<h2>%s Group</h2>\n", type.c_str());

  printf("<table>\n");
  printf("<tr><th>Tbl<th>Col<th>Row<th>Name<th>Description\n");

  std::vector<std::string> nodes = brd->getNodes( type + "\\..*");

  for( unsigned int i=0; i<nodes.size(); i++) {
    bool show = true;

    const uhal::Node& node = brd->getNode( nodes[i]);

    // filter out the iterated things
    if( nodes[i].find("AMC1") != std::string::npos)
      show = false;
    if( nodes[i].find("AMC0") != std::string::npos && nodes[i].find("AMC01") == std::string::npos)
      show = false;
    if( nodes[i].find("LTRIG.SAMPLE") != std::string::npos &&
	nodes[i].find("LTRIG.SAMPLE00") == std::string::npos)
      show = false;
    if( nodes[i].find("SFP1") != std::string::npos || nodes[i].find("SFP2") != std::string::npos)
      show = false;
    if( nodes[i].find("L1A_HISTORY_") != std::string::npos)
      show = false;

    // remove things ending in _nnn or _nn where nn is not "00"
    static const boost::regex ex("_\\d?\\d\\d$");
    if( regex_search( nodes[i], ex))
      show = false;
    
    if( !nodes[i].compare( nodes[i].length() - 3, 3, "_00"))
      show = true;
    if( !nodes[i].compare( nodes[i].length() - 4, 4, "_000"))
      show = true;

    // check params
    if( show) {

      const boost::unordered_map<std::string,std::string> params = node.getParameters();
      const std::string desc = node.getDescription();

      // only keep those with "Status" parameter above specified level
      if( params.find("Status") != params.end() && (atoi(params.find("Status")->second.c_str()) <= level) ) {

	std::string table = params.find("Table")->second;
	std::string row = params.find("Row")->second;
	std::string column = params.find("Column")->second;

	std::vector<std::string> wordz;
	boost::split( wordz, nodes[i], boost::is_any_of("."));

	// make a copy of the name with the prefix stripped
	std::string mod_name = nodes[i].substr( nodes[i].find(".")+1);

	// replace _d with d'th dotted word in column name
	if( column.find("_") == 0) {
	  int n = atoi( column.substr(1).c_str() );
	  column = wordz[n-1];
	}

	// finally store it in the big map
	a_spair val( mod_name, desc);
	mat[table][column][row] = val;
	
      }
    }
  }

  // now dump the 3-level map
  for( a_map3s::iterator table=mat.begin(); table != mat.end(); table++) {
    std::cout << "<tr><td id=tbl colspan=5> Table: " << table->first << std::endl;
    for( a_map2s::iterator column=table->second.begin(); column != table->second.end(); column++) {
      std::cout << "<tr><td>&nbsp;<td id=col colspan=4> Column: " << column->first << std::endl;
      for( a_map1s::iterator row=column->second.begin(); row != column->second.end(); row++) {
	a_spair val = row->second;
	std::cout << "<tr><td colspan=2> &nbsp; <td id=row>" << row->first << "<td id=name>"
		  << val.first << "<td id=descr>"<< val.second << std::endl;
      }
    }
  }


  printf("</table>\n");

}
