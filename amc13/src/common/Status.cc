#include "amc13/Status.hh"
#include "amc13/Exception.hh"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm> //std::max
#include <iomanip> //for std::setw
#include <ctype.h> // for isDigit()

//For PRI macros
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

namespace amc13{

  using boost::algorithm::iequals;

  //=============================================================================
  //===== Status Class
  //=============================================================================
  Status::Status(AMC13Simple * _amc13, int _version = 0)
  {
    if(_amc13 == NULL){
      Exception::NULLPointer e;
      e.Append("Null pointer given to Status constructor\n");
      throw e;
    }
    //store local copy of the svn version so we can reference it while debugging
    version = _version;
    //Save the amc weare talking to
    amc13 = _amc13;  
    statusMode = TEXT;
    debug = false;
  }

  void Status::ProcessChip(AMC13Simple::Board chip,std::string const & singleTable)
  {
    //Get the list of nodes out of the hardware
    uhal::HwInterface * hw = amc13->getChip(chip);
    if(hw == NULL){
      Exception::NULLPointer e;
      e.Append("AMC13::Status::Processchip() - Bad hw interface for chip\n");
      throw e;
    }
    uhal::Node const & baseNode = hw->getNode();

    std::string chipName("U");
    if(chip == AMC13Simple::T1){
      chipName.assign("T1");
    }else if(chip == AMC13Simple::T2){
      chipName.assign("T2");
    }
    
    // Look for enable masks, only in T1
    if( chipName == "T1") {
      uint32_t my_mask;

      my_mask = amc13->read( chip, "CONF.AMC.ENABLE_MASK");
      SetAMCMask( my_mask);

      my_mask = amc13->read( chip, "CONF.SFP.ENABLE_MASK");
      SetSFPMask( my_mask);

    }

    //process all the nodes and build table structure
    for(uhal::Node::const_iterator itNode = baseNode.begin();
	itNode != baseNode.end();
	itNode++){
      //Get the list of parameters for this node
      boost::unordered_map<std::string,std::string> parameters = itNode->getParameters();      
      //Look for a Status parameter
      if(parameters.find("Status") != parameters.end()){	
	std::string const & tableName = parameters["Table"];
	//Add this Address to our Tables if it matches our singleTable option, or we are looking at all tables
	//	if( singleTable.empty() || iequals(tableName,singleTable)){
	if( singleTable.empty() || TableNameCompare(tableName,singleTable)){
	  tables[tableName].Add(chipName,*itNode);
	}
      }
    }
    //Force all the value reads
    hw->dispatch();
  }

  std::string TableStrip(const std::string & s1){
    std::string strip(s1);
    //Look for an underscore
    size_t _pos = s1.find('_');
    if( (_pos != std::string::npos) &&  //found a '_'
	(_pos < (s1.size()-1) ) && // '_' isn't the last char
	(_pos > 0) ) {     // '_' isn't the first char
      //check if there are only numbers before the '_'
      bool isSortPrefix = true;
      for(int iChar = (int) _pos-1; (iChar >= 0) && isSortPrefix ; iChar--){	
	isSortPrefix = isdigit(s1[iChar]) && isSortPrefix; //check if digit is a number
      }
      if(isSortPrefix){
	strip = strip.substr(_pos+1,strip.size());
      }
    }
    return strip;
  }
  bool TableNameCompare(const std::string & s1, const std::string & s2){
    return iequals(TableStrip(s1),TableStrip(s2));
  }

  void Status::ReportHeader(std::ostream & stream) const
  {
    if(statusMode == LATEX) {			
      stream << "\\documentclass[a4paper,10pt]{article}" << "\n";
      stream << "\\usepackage[margin=0.5in]{geometry}" << "\n";
      stream << "\\title{AMC13 Address Table Documentation}" << "\n";
      stream << "\\author{Eric Hazen, Daniel Gastler, Alexander Guldemond, David Zou}" << "\n"; 
      stream << "\\begin{document}" << "\n";
      stream << "\\maketitle" << "\n\n";
      stream << "\\section{Introduction}" << "\n";
      stream << "Introduction goes here";   
    }
    else if(statusMode == HTML){ 
      stream << "<!DOCTYPE html><html><head><style>\n";
    }
  }

  void Status::ReportStyle(std::ostream & stream) const 
  {
    if (statusMode == HTML) {
      std::string head_color = "lightblue";
      std::string cell_color = "lightgreen";
      std::string err_color = "#FB412d"; //red                                                                                                                                                              
      std::string null_color = "lightgrey";
      stream << "table { float: left; margin: 10px;}\n"; //Allows multiple tables on the same line with spacing                                                                                                                             
      stream << "th { font-size: smaller; background-color:" << head_color << ";}\n"; // Sets header font size and background color                                                                                                         
      stream << "th.name {font-size: 20px; }\n";  // Increases the font size of the cell containing the name of the tables                                                                                                                  
      stream << "td { background-color:" << null_color << "; text-align: right;}\n"; // Sets the background color of null cells to grey                                                                                                     
      stream << "td.nonerror { background-color:" << cell_color << ";}\n" ; // sets the background color of data cells                                                                                                                      
      stream << "td.error { background-color:" << err_color << ";}\n"; // sets the background color of error cells                                                                                                                          
      stream << "td.null { background-color:" << null_color << ";} </style></head><body>\n"; // sets the background color of null cells
    }
  }

  void Status::ReportBody(size_t level, std::ostream & stream, std::string const & singleTable)
  {
    //Clear any entries
    tables.clear();
    
    //Process both chips on the AMC13
    ProcessChip(AMC13Simple::T1,singleTable);
    ProcessChip(AMC13Simple::T2,singleTable);
    
    // Now output the content, looping over the tables
    // Eventually calls one of PrintLaTeX(), PrintHTML() or Print() for each table
    for(std::map<std::string,SparseCellMatrix>::iterator itTable = tables.begin();
	itTable != tables.end();
	itTable++){
      itTable->second.Render(stream,level,statusMode);
    }
  }

  void Status::ReportTrailer(std::ostream & stream) const
  {
    // Now any post-amble
    if (statusMode == LATEX) {
      stream << "\\section{Version}" << "\n";
      stream << "Using svn version: " << version << ".\n";
      stream << "\\end{document}\n";
    } else if(statusMode == HTML){
      //Display the svn version for this release
      stream << "<table><tr><td>SVN</td><td>" << version << "</td></tr></table>\n";
      stream << "</body></html>\n";
    } else if (statusMode == TEXT) {
      stream << "SVN: " << version << "\n";
    }  
  }

  std::string Status::ReportHeader() const {
    std::stringstream str;
    ReportHeader(str);
    return str.str();
  }

  std::string Status::ReportStyle() const {
    std::stringstream str;
    ReportStyle(str);
    return str.str();
  }

  std::string Status::ReportBody(size_t level, std::string const & singleTable)  {
    std::stringstream str;
    ReportBody(level,str,singleTable);
    return str.str();
  }

  std::string Status::ReportTrailer() const{
    std::stringstream str;
    ReportTrailer(str);
    return str.str();
  }
  
  void Status::Report(size_t level,std::ostream & stream,std::string const & singleTable)
  {
    ReportHeader(stream);
    ReportStyle(stream);
    ReportBody(level,stream,singleTable);
    ReportTrailer(stream);
  }

  std::string Status::ReportBare(size_t level,std::string const & singleTable) {
    std::stringstream str;
    StatusMode saveMode = statusMode;    
    statusMode = BAREHTML;
    Report( level, str, singleTable);
    statusMode = saveMode;
    return str.str();
  }

  const SparseCellMatrix* Status::GetTable(const std::string & table) const {
    if (tables.find(table) == tables.end()) {
      Exception::BadValue e;
      char buffer[50];
      snprintf(buffer,49,"Table %s not found\n",table.c_str());
      e.Append(buffer);
      throw e;
    }
    return &tables.at(table);
  }

  std::vector<std::string> Status::GetTableList() const {
    std::vector<std::string> tableList;
    for (std::map<std::string,SparseCellMatrix>::const_iterator it = tables.begin(); it != tables.end(); it++) {
      tableList.push_back(it->first);
    }
    return tableList;
  }

  std::vector<std::string> Status::GetTableRows(std::string const & table) const {
    return GetTable(table)->GetTableRows();
  }

  std::vector<std::string> Status::GetTableColumns(std::string const & table) const {
    return GetTable(table)->GetTableColumns();
  }

  const Cell* Status::GetCell(const std::string& table, const std::string& row, const std::string& col) const {
    return GetTable(table)->GetCell(row,col);
  }

  //Channel mask (static for this run)
  static std::map<std::string,bool> AMCMask; 
  static std::map<std::string,bool> SFPMask;

  void Status::SetAMCMask(uint32_t mask)
  {
    char buffer[] = "AMC00";
    AMCMask.clear();//clear the current mask
    for(size_t iBit = 0; iBit < 12;iBit++){
      if(mask & (1<<iBit)){
	snprintf(buffer,sizeof(buffer),"AMC%02zu",iBit+1);
	AMCMask[buffer] = true;
      }
    }
  }

  void Status::SetSFPMask(uint32_t mask) {
    char buffer[] = "SFP00";
    SFPMask.clear();//clear the current mask
    for (size_t iBit = 0; iBit < 3; iBit++) {
      if (mask & (1<<iBit)) {
	snprintf(buffer,sizeof(buffer),"SFP%01zu",iBit);
	SFPMask[buffer] = true;
      }
    }
  }

  void Status::SetOutputMode(StatusMode mode) { statusMode = mode;}
  /*! Get output mode */
  StatusMode Status::GetOutputMode() const { return statusMode;}
  /*! Select "bare HTML" output mode */
  void Status::SetBareHTML(){statusMode = BAREHTML;}
  /*! Unselect "bare HTML" output tmode */
  void Status::UnsetBareHTML(){statusMode = TEXT;}
  /*! Select HTML output mode */
  void Status::SetHTML(){statusMode = HTML;}
  /*! Unselect HTML output mode */
  void Status::UnsetHTML(){statusMode = TEXT;}
  /*! Select LaTeX source output mode */
  void Status::SetLaTeX() {statusMode = LATEX;}
  /*! Unselect LaTeX source output mode */
  void Status::UnsetLaTeX() {statusMode = TEXT;}
  

  //=============================================================================
  //===== SparseCellMatrix Class
  //=============================================================================


  void SparseCellMatrix::Clear()
  {
    //Clear the name
    name.clear();
    //Clear the maps
    rowColMap.clear();
    colRowMap.clear();
    rowName.clear();
    colName.clear();
    //Deallocate the data
    for(std::map<std::string,Cell*>::iterator itCell = cell.begin();
	itCell != cell.end();
	itCell++){
      if(itCell->second == NULL){
	delete itCell->second;
	itCell->second = NULL;
      }
    }
    //clear the vector
    cell.clear();
  }

  std::vector<std::string> SparseCellMatrix::GetTableRows() const {
    std::vector<std::string> rows;
    for (std::map<std::string,std::map<std::string, Cell *> >::const_iterator it = rowColMap.begin(); 
	 it != rowColMap.end(); it++) {
      rows.push_back(it->first);
    }
    return rows;
  }

  std::vector<std::string> SparseCellMatrix::GetTableColumns() const {
    std::vector<std::string> cols;
    for (std::map<std::string,std::map<std::string, Cell *> >::const_iterator it = colRowMap.begin();
	 it != colRowMap.end();it++) {
      cols.push_back(it->first);
    }
    return cols;
  }

  const Cell* SparseCellMatrix::GetCell(const std::string & row, const std::string & col) const {
    if (rowColMap.find(row) == rowColMap.end() || colRowMap.find(col) == colRowMap.end()) {
      Exception::BadValue e;
      char buffer[50];
      snprintf(buffer,49,"No cell in (\"%s\",\"%s\") position\n",row.c_str(),col.c_str());
      e.Append(buffer);
      throw e;
    }
    return rowColMap.at(row).at(col);
  }

  void SparseCellMatrix::Add(std::string ChipName,uhal::Node const & node)
  {
    //Get name from node
    boost::unordered_map<std::string,std::string> parameters = node.getParameters();

    if(parameters.find("Table") == parameters.end()){
      Exception::BadValue e;
      char tmp[256];
      snprintf(tmp, 255, "Missing Table value for node (%s) %s\n", ChipName.c_str(), node.getId().c_str());
      e.Append( tmp);
      throw e;
    }
    CheckName(parameters["Table"]);
    
    //Determine address
    std::string address = ChipName+node.getPath();
    boost::to_upper(address);

    //Check if the rows/columns are the same
    //Determine row and column
    std::string row = ParseRow(parameters,address);
    std::string col = ParseCol(parameters,address);    
    int bitShift = 0;
    
    //Check if address contains a "_HI" or a "_LO"    
    if((address.find("_LO") == (address.size()-3)) ||
       (address.find("_HI") == (address.size()-3))){
      //Search for an existing base address
      std::string baseAddress;
      if(address.find("_LO") == (address.size()-3)){
	baseAddress = address.substr(0,address.find("_LO"));
	bitShift = 0;
      }
      if(address.find("_HI") == (address.size()-3)){
	baseAddress = address.substr(0,address.find("_HI"));
	bitShift = 32;
	std::string LO_address(baseAddress);
	LO_address.append("_LO");
	//Check if the LO word has already been placed
	if (cell.find(LO_address) != cell.end()) {
	  uint32_t mask = cell.at(LO_address)->GetMask();
	  while ( (mask & 0x1) == 0) {
	    mask >>= 1;
	  }
	  int count = 0;
	  while ( (mask & 0x1) == 1) {
	    count++;
	    mask >>= 1;
	  }
	  bitShift = count; //Set bitShift t be the number of bits in the LO word
	}
	//If LO word hasn't been placed into the table yet, assume 32
      }
      std::map<std::string,Cell*>::iterator itCell;
      if(((itCell = cell.find(baseAddress)) != cell.end()) ||  //Base address exists alread
	 ((itCell = cell.find(baseAddress+std::string("_HI"))) != cell.end()) || //Hi address exists alread
	 ((itCell = cell.find(baseAddress+std::string("_LO"))) != cell.end())){ //Low address exists alread
	if(iequals(itCell->second->GetRow(),row) && 
	   iequals(itCell->second->GetCol(),col)){
	  //We want to combine these entries so we need to rename the old one
	  Cell * ptrCell = itCell->second;
	  cell.erase(ptrCell->GetAddress());
	  cell[baseAddress] = ptrCell;
	  ptrCell->SetAddress(baseAddress);	  
	  address=baseAddress;

	}
      }
    }
 
    
    //Get description,format,rule, and statuslevel
    std::string description = node.getDescription();
    std::string statusLevel = parameters["Status"]; //No checks because this must be here
    std::string rule = parameters["Show"]; boost::to_upper(rule);
    std::string format;
    if(parameters.find("Format") != parameters.end()){      
      format.assign(parameters["Format"]);      
    }else{
      format.assign(DefaultFormat);
    }

    Cell * ptrCell;
    //Add or append this entry
    if(cell.find(address) == cell.end()){
      ptrCell = new Cell;
      cell[address] = ptrCell;
    }else{
      ptrCell = cell[address];
    }
    ptrCell->Setup(address,description,row,col,format,rule,statusLevel);
    //Read the value if it is a non-zero status level
    //A status level of zero is for write only registers
    if(ptrCell->DisplayLevel() > 0){
      ptrCell->Fill(node.read(),bitShift);
    }
    //Setup should have thrown if something bad happened, so we are safe to update the search maps
    rowColMap[row][col] = ptrCell;
    colRowMap[col][row] = ptrCell;    
    ptrCell->SetMask( node.getMask());
  }

  void SparseCellMatrix::CheckName(std::string const & newTableName)
  {
    //Check that this name isn't empty
    if(newTableName.empty()){
      Exception::BadValue e;
      std::string error("Bad table name \""); error+=newTableName ;error+="\"\n";
      e.Append(error);
      throw e;
    }

    //Strip the leading digits off of the table name
    int loc = newTableName.find("_");
    bool shouldStrip = true;
    for (int i = 0; i < loc; i++) {
      if (!isdigit(newTableName[i])) {
        shouldStrip = false;
        break;
      }
    }
    std::string modName;
    if (shouldStrip) {
      modName = newTableName.substr(loc+1);
    }
    else {
      modName = newTableName;
    }
    
    //Setup table name if not yet set
    if(name.empty()){
      name = modName;
    }else if(!iequals(modName,name)){
      Exception::BadValue e;
      std::string error("Tried adding entry of table \"");
      error += modName + " to table " + name;
      e.Append(error.c_str());
      throw e;
    }
  }
  std::string SparseCellMatrix::ParseRow(boost::unordered_map<std::string,std::string> & parameters,
					 std::string const & addressBase) const
  {
    std::string newRow;
    //Row
    if(parameters.find("Row") != parameters.end()){
      //Grab the row name and store it
      newRow.assign(parameters["Row"]);
      boost::to_upper(newRow);
      //Check if we have a special character at the beginning to tell us what to use for row
      if((newRow.size() > 1)&&
	 (newRow[0] == ParameterParseToken)){
	size_t token = strtoul(newRow.substr(1).c_str(),NULL,0);
	//Build a BOOST tokenizer for the address name
	//This is for use with undefined rows and columns. 
	//This does not tokenize until .begin() is called
	boost::char_separator<char> sep(".");
	boost::tokenizer<boost::char_separator<char> > tokenizedAddressName(addressBase,sep);
	boost::tokenizer<boost::char_separator<char> >::iterator itTok;
	if(token == 0){	
	  newRow.assign(addressBase);
	} else { //Parse the name by "."s for the correct substr
	  itTok = tokenizedAddressName.begin();
	  for(;token>1;token--){itTok++;};  //Move the tokenizer forward

	  //Check that this is a valid value 
	  if(itTok == tokenizedAddressName.end()){
	    Exception::BadValue e;	    
	    std::string error("Bad row for ");
	    error += addressBase + " with token " + newRow;
	    e.Append(error.c_str());
	    throw e;
	  }
	  //Assign the row to the token string
	  newRow.assign(*itTok);
	}
      } 
    }else{
      //Missing row
      Exception::BadValue e;
      std::string error("Missing row for ");
      error += addressBase;
      e.Append(error.c_str());
      throw e;
    }
    return newRow;
  }
  std::string SparseCellMatrix::ParseCol(boost::unordered_map<std::string,std::string> & parameters,
					 std::string const & addressBase) const
  {    
    std::string newCol;
    //Col
    if(parameters.find("Column") != parameters.end()){
      //Grab the col name and store it
      newCol.assign(parameters["Column"]);
      boost::to_upper(newCol);
      //Check if we have a special character at the beginning to tell us what to use for col
      if((newCol.size() > 1)&&
	 (newCol[0] == ParameterParseToken)){
	size_t token = strtoul(newCol.substr(1).c_str(),NULL,0);
	//Build a BOOST tokenizer for the address name
	//This is for use with undefined cols and columns. 
	//This does not tokenize until .begin() is called
	boost::char_separator<char> sep(".");
	boost::tokenizer<boost::char_separator<char> > tokenizedAddressName(addressBase,sep);
	boost::tokenizer<boost::char_separator<char> >::iterator itTok;
	if(token == 0){	
	  newCol.assign(addressBase);
	} else { //Parse the name by "."s for the correct substr
	  itTok = tokenizedAddressName.begin();
	  for(;token>0;token--){itTok++;};  //Move the tokenizer forward

	  //Check that this is a valid value 
	  if(itTok == tokenizedAddressName.end()){
	    Exception::BadValue e;	    
	    std::string error("Bad col for ");
	    error += addressBase + " with token " + newCol;
	    e.Append(error.c_str());
	    throw e;
	  }
	  //Assign the col to the token string
	  newCol.assign(*itTok);
	}
      } 
    }else{
      //Missing col
      Exception::BadValue e;
      std::string error("Missing col for ");
      error += addressBase;
      e.Append(error.c_str());
      throw e;
    }
    return newCol;
  }

  // render one table
  void SparseCellMatrix::Render(std::ostream & stream,int status,StatusMode statusMode) const
  {
    bool forceDisplay = (status >= 99) ? true : false;

    //==========================================================================
    //Rebuild our col/row map since we added something new
    //==========================================================================
    std::map<std::string,std::map<std::string,Cell *> >::const_iterator it;
    rowName.clear();
    for(it = rowColMap.begin();it != rowColMap.end();it++){      
      rowName.push_back(it->first);
    }
    colName.clear();
    for(it = colRowMap.begin();it != colRowMap.end();it++){
      colName.push_back(it->first);
    }	    


    //==========================================================================
    //Determine the widths of each column, decide if a column should be printed,
    // and decide if a row should be printed
    //==========================================================================
    std::vector<int> colWidth(colName.size(),-1);
    bool printTable = false;
    // map entry defined to display this row, passed on to print functions
    std::map<std::string,bool> rowDisp; 
    // map entry "true" to kill this row, used only locally
    std::map<std::string,bool> rowKill;
    
    for(size_t iCol = 0;iCol < colName.size();iCol++){
      //Get a vector for this row
      const std::map<std::string,Cell*> & inColumn = colRowMap.at(colName[iCol]);
      for(std::map<std::string,Cell*>::const_iterator itColCell = inColumn.begin();
	  itColCell != inColumn.end();
	  itColCell++){
	//Check if we should display this cell
	if(itColCell->second->Display(status,forceDisplay)){
	  //This entry will be printed, 
	  //update the table,row, and column display variables
	  printTable = true;
	  // check if this row is already marked to be suppressed
	  if( rowKill[itColCell->first])
	    // yes, delete entry in rowDisp if any
	    rowDisp.erase(itColCell->first);
	  else 
	    // nope, mark it for display
	    rowDisp[itColCell->first] = true;
	  // deal with the width
	  int width = itColCell->second->Print(-1).size();
	  if(width > colWidth[iCol]){
	    colWidth[iCol]=width;
	  }
	}
	// now check if this cell should cause the row to be suppressed
	if( itColCell->second->SuppressRow( forceDisplay) || rowKill[itColCell->first] ) {
	  rowKill[itColCell->first] = true;
	  rowDisp.erase(itColCell->first);
	}
      }
    }

    if(!printTable){
      return;
    }
    
    //Determine the width of the row header
    int headerColWidth = 16;
    if(name.size() > size_t(headerColWidth)){
      headerColWidth = name.size();
    } 
    for(std::map<std::string,bool>::iterator itRowName = rowDisp.begin();
	itRowName != rowDisp.end();
	itRowName++){
      if(itRowName->first.size() > size_t(headerColWidth)){
	headerColWidth = itRowName->first.size();
      }
    }

        
    //Print out the data
    if (statusMode == LATEX) {
      PrintLaTeX(stream);
    }else if(statusMode == HTML || statusMode == BAREHTML){
      PrintHTML(stream,status,forceDisplay,headerColWidth,rowDisp,colWidth);
    }else{
      Print(stream,status,forceDisplay,headerColWidth,rowDisp,colWidth);
    }
  }
  void SparseCellMatrix::Print(std::ostream & stream,
			       int status,
			       bool forceDisplay,
			       int headerColWidth,
			       std::map<std::string,bool> & rowDisp,
			       std::vector<int> & colWidth) const
  {
    //=====================
    //Print the header
    //=====================
    //Print the rowName
    stream << std::right << std::setw(headerColWidth+1) << name << "|";	
    for(size_t iCol = 0; iCol < colWidth.size();iCol++){
      if(colWidth[iCol] > 0){
	size_t columnPrintWidth = std::max(colWidth[iCol],int(colName[iCol].size()));
	stream<< std::right  
	      << std::setw(columnPrintWidth+1) 
	      << colName[iCol] << "|";
      }	  
    }
    //Draw line
    stream << std::endl << std::right << std::setw(headerColWidth+2) << "--|" << std::setfill('-');
    for(size_t iCol = 0; iCol < colWidth.size();iCol++){
      if(colWidth[iCol] > 0){
	size_t columnPrintWidth = std::max(colWidth[iCol],int(colName[iCol].size()));
	stream<< std::right  
	      << std::setw(columnPrintWidth+2) 
	      << "|";
      }	  
    }
    stream << std::setfill(' ') << std::endl;
    
    //=====================
    //Print the data
    //=====================
    for(std::map<std::string,bool>::iterator itRow = rowDisp.begin();
	itRow != rowDisp.end();
	itRow++){
      //Print the rowName
      stream << std::right << std::setw(headerColWidth+1) << itRow->first << "|";
      //Print the data
      const std::map<std::string,Cell*> & colMap = rowColMap.at(itRow->first);
      for(size_t iCol = 0; iCol < colName.size();iCol++){	  
	if(colWidth[iCol] > 0){
	  size_t width = std::max(colWidth[iCol],int(colName[iCol].size()));
	  std::map<std::string,Cell*>::const_iterator itMap = colMap.find(colName[iCol]);
	  if((itMap != colMap.end()) &&
	     (itMap->second->Display(status,forceDisplay))){
	    stream << std::right  
		   << std::setw(width+1)
		   << itMap->second->Print(colWidth[iCol]) << "|";	   
	  }else{
	    stream << std::right 
		   << std::setw(width+2) 
		   << " |";
	  }
	}
      }
      stream << std::endl;
    }

    //=====================
    //Print the trailer
    //=====================
    stream << std::endl;
  }
  void SparseCellMatrix::PrintHTML(std::ostream & stream,
				   int status,
				   bool forceDisplay,
				   int headerColWidth,
				   std::map<std::string,bool> & rowDisp,
				   std::vector<int> & colWidth) const
  {
    //=====================
    //Print the header
    //=====================
    //Print the rowName
    stream << "<table border=\"1\" >" << "<tr>" << "<th class=\"name\">" << name << "</th>";    
    for(size_t iCol = 0; iCol < colWidth.size();iCol++){
      if(colWidth[iCol] > 0){
	stream << "<th>" << colName[iCol] << "</th>";
      }	  
    }
    stream << "</tr>\n";   	
    //=====================
    //Print the data
    //=====================
    for(std::map<std::string,bool>::iterator itRow = rowDisp.begin();
	itRow != rowDisp.end();
	itRow++){
      stream << "<tr><th>" << itRow->first << "</th>";
      //Print the data
      const std::map<std::string,Cell*> & colMap = rowColMap.at(itRow->first);
      for(size_t iCol = 0; iCol < colName.size();iCol++){	  
	if(colWidth[iCol] > 0){
	  std::map<std::string,Cell*>::const_iterator itMap = colMap.find(colName[iCol]);
	  if(itMap != colMap.end()){
	    //sets the class for the td element for determining its color
	    std::string tdClass = (itMap->second->GetDesc().find("error") != std::string::npos ? "error" : "nonerror") ;
	    tdClass = (itMap->second->Print(colWidth[iCol],true) == "0") ? "null" : tdClass; 
	    if(itMap->second->Display(status,forceDisplay)){
	      stream << "<td title=\"" << itMap->second->GetDesc()  << "\" class=\"" << tdClass << "\">" 
		     << itMap->second->Print(colWidth[iCol],true) << "</td>";
	    }else{
	      stream << "<td title=\"" << itMap->second->GetDesc()  << "\" class=\"" << tdClass << "\">" << " " << "</td>";
	    }
	  }else{
	    stream << "<td>" << " " << "</td>";
	  }
	}
      }
      stream << "</tr>\n";
    }
    //=====================
    //Print the trailer
    //=====================
    stream << "</table>\n";
  }

  void SparseCellMatrix::PrintLaTeX(std::ostream & stream)  const {
    //=====================
    //Print the header
    //=====================
    std::string headerSuffix = "";
    std::string cols = "";
    std::vector<std::string> modColName;

    //Shrink the column list to not include redundant columns
    for (std::vector<std::string>::const_iterator colIt = colName.begin() ; colIt != colName.end(); ++colIt) {
      if (modColName.empty()) {
	modColName.push_back(*colIt);
      } else if (modColName.back().substr(0,3).compare((*colIt).substr(0,3))==0 && ((*colIt).substr(0,3) == "AMC" ||
										    (*colIt).substr(0,3) == "SFP")) {
	
      } else {
	modColName.push_back(*colIt);
      }
    }
	    
    //Remove underscores from column names and create the suffix for the header
    for (std::vector<std::string>::iterator it = modColName.begin(); it != modColName.end(); ++it) {
      headerSuffix = headerSuffix + "|l";
      if (it->find("_") != std::string::npos) {
	cols = cols +" & " + (*it).replace(it->find("_"),1," ");
      } else {
	cols = cols + " & " + *it;
      }
    }

    //Strip underscores off table name
    std::string strippedName = name;
    while (strippedName.find("_") != std::string::npos) {
      strippedName = strippedName.replace(strippedName.find("_"),1," ");
    }


    headerSuffix = headerSuffix + "|l|";
    stream << "\\section{" << strippedName << "}\n";
    stream << "\\begin{center}\n";
    stream << "\\begin{tabular}" <<"{" << headerSuffix << "}" <<"\n";
    stream << "\\hline\n";
    
    //=========================================================================
    //Print the first row, which contains the table name and the column names
    //=========================================================================
    stream << strippedName + cols + " \\\\\n";
    stream << "\\hline\n";

    //========================
    //Print subsequent rows
    //========================
    for (std::vector<std::string>::const_iterator itRow = rowName.begin(); itRow != rowName.end(); itRow++) {
      std::string thisRow = *itRow; 
      char s_mask[10];

      while (thisRow.find("_") != std::string::npos) {
	thisRow = thisRow.replace(thisRow.find("_"),1," ");
      }

      for(std::vector<std::string>::iterator itCol = modColName.begin(); itCol != modColName.end(); itCol++) {
	std::string addr;

	if (true) {
	  if (rowColMap.at(*itRow).find(*itCol) == rowColMap.at(*itRow).end()) {
	    addr = " ";
	    *s_mask = '\0';
	  }
	  else {
	    addr = rowColMap.at(*itRow).at(*itCol)->GetAddress();
	    uint32_t mask = rowColMap.at(*itRow).at(*itCol)->GetMask();
	    snprintf( s_mask, 10, "0x%x", mask );

	    if (addr.find(".") != std::string::npos) {
	      addr = addr.substr(addr.find_last_of(".")+1);
	    }
	    while (addr.find("_") != std::string::npos) {
	      addr = addr.replace(addr.find("_"),1," ");
	    }
	  }


	  thisRow = thisRow + " & " + s_mask + "/" + addr;
	} else {
	  thisRow = thisRow + " & " + " ";
	}
      }
      
      
      stream << thisRow + "\\\\\n";
      stream << "\\hline\n";
    }
    
    //=====================
    //Print trailer
    //=====================
    stream << "\\end{tabular}\n";
    stream << "\\end{center} \n";
    stream << "Documentation goes here" << "\n\n\n";
  }


  //=============================================================================
  //===== Cell Class
  //=============================================================================
  void Cell::Clear()
  {
    address.clear();
    description.clear();
    row.clear();
    col.clear();
    valWord.clear();
    valWordShift.clear();
    format.clear();
    displayRule.clear();
    statusLevel = 0;
  }
  void Cell::Setup(std::string const & _address,  //stripped of Hi/Lo
		   std::string const & _description,
		   std::string const & _row, //Stripped of Hi/Lo
		   std::string const & _col, //Stripped of Hi/Lo
		   std::string const & _format,
		   std::string const & _rule,
		   std::string const & _statusLevel)
  {
    //These must all be the same
    CheckAndThrow("Address",address,_address);
    CheckAndThrow(address + " row",row,_row);
    CheckAndThrow(address + " col",col,_col);
    CheckAndThrow(address + " format",format,_format);
    CheckAndThrow(address + " rule",displayRule,_rule);
    
    //Append the description for now
    description += _description;

    //any other formatting
    statusLevel = strtoul(_statusLevel.c_str(),
		     NULL,0);
  }

  void Cell::Fill(uhal::ValWord<uint32_t> value,
		  size_t bitShift)
  {
    valWord.push_back(value);
    valWordShift.push_back(bitShift);
  }

  int Cell::DisplayLevel() const {return statusLevel;}

  bool Cell::SuppressRow( bool force) const
  {
    // Compute the full value for this entry
    uint64_t val = ComputeValue();
    bool suppressRow = (iequals( displayRule, "nzr") && (val == 0)) && !force;
    return suppressRow;
  }

  std::string const & Cell::GetRow() const {return row;}
  std::string const & Cell::GetCol() const {return col;}
  std::string const & Cell::GetDesc() const {return description;}
  std::string const & Cell::GetAddress() const {return address;}

  void Cell::SetAddress(std::string const & _address){address = _address;}
  uint32_t const & Cell::GetMask() const {return mask;}
  void Cell::SetMask(uint32_t const & _mask){mask = _mask;}


  bool Cell::Display(int level,bool force) const
  {
    // Compute the full value for this entry
    uint64_t val = ComputeValue();

    // Decide if we should display this cell
    bool display = (level >= statusLevel) && (statusLevel != 0);

    // Check against the print rules
    if(iequals(displayRule,"nz")){
      display = display & (val != 0); //Show when non-zero
    } else if(iequals(displayRule,"z")){
      display = display & (val == 0); //Show when zero
    }

    //Check if this column is an AMCXX column 
    if(col.find("AMC") != std::string::npos){
      if(AMCMask.find(col) == AMCMask.end()){
	display = false;
      }
    }

    //Check if this column is an SFPXX column
    if (col.find("SFP") != std::string::npos) {
      if (SFPMask.find(col) == SFPMask.end()){
	display = false;
      }
    }

    //Force display if we want
    display = display || force;
    return display;
  }
  uint64_t Cell::ComputeValue() const
  {
    //Compute full value
    uint64_t val = 0;
    for(size_t i = 0; i < valWord.size();i++){
      if(valWord.size() > 1){//If we have multiple values to merge
	val += (uint64_t(valWord[i].value()) << valWordShift[i]);
      }else{//If we have just one value
	val += uint64_t(valWord[i].value());
      }
    }
    return val;
  }

  std::string Cell::Print(int width = -1,bool html) const
  { 
    //If this cell does not display a number
    if (iequals(format,std::string("TTSRaw")) || iequals(format,std::string("TTSEnc"))) {
      std::map<uint64_t,std::string> TTSOutputs;
      std::string bad;

      //Construct a map of desired outputs
      if (iequals(format,std::string("TTSRaw"))) { 
	TTSOutputs[0] = "DIS";
	TTSOutputs[1] = "OFW";
	TTSOutputs[2] = "SYN";
	TTSOutputs[4] = "BSY";
	TTSOutputs[8] = "RDY";
	TTSOutputs[15] = "DIS";
	bad = "ERR";
      } else {
	TTSOutputs[0] = "RDY";
	TTSOutputs[1] = "OFW";
	TTSOutputs[2] = "BSY";
	TTSOutputs[4] = "SYN";
	TTSOutputs[8] = "ERR";
	TTSOutputs[16] = "DIS";
	bad = "BAD";
      }
      
      uint64_t val = ComputeValue();
      std::string output = "";
      for (std::map<uint64_t,std::string>::iterator it = TTSOutputs.begin(); it != TTSOutputs.end(); it++) {
	if (it->first == val) {
	  output = it->second;
	  break;
	}
	output = bad;
      }
      // for now include hex value too
      char tmp[10];
      snprintf( tmp, sizeof(tmp), "%s (0x%" PRIx64 ")", output.c_str(), val);
      output = tmp;
      return output;
    }
   	
    char buffer[21];  //64bit integer can be max 20 ascii chars (as a signed int)

    //Build the format string for snprintf
    std::string fmtString("%");
    if(iequals(format,std::string("x")) && ComputeValue() >= 10){
      fmtString.assign("0x%");
      if(width >= 0){
	width -= 2;
      }
    } else if (iequals(format,std::string("x")) && ComputeValue() < 10) {
      // get rid of the leading zeros, looks better
      fmtString.assign("%");
    }
    
    //if we are specifying the width, add a *
    if(width >= 0){
      fmtString.append("*");
    }

    //add the PRI stuff for our uint64_t
    if(iequals(format,std::string("x"))){
      fmtString.append(PRIX64);
    }else if(iequals(format,std::string("d"))){
      fmtString.append(PRId64);
    }else if(iequals(format,std::string("u"))){
      fmtString.append(PRIu64);
    }
 

    //Generatethe string
    if(width == -1){      
      snprintf(buffer,sizeof(buffer),
	       fmtString.c_str(),ComputeValue());
    }else{
      snprintf(buffer,sizeof(buffer),	       
	       fmtString.c_str(),width,ComputeValue());
    }
    //return the string
    return std::string(buffer);
  }

  void Cell::CheckAndThrow(std::string const & name,
			   std::string & thing1,
			   std::string const & thing2) const
  {
    //Checks
    if(thing1.size() == 0){
      thing1 = thing2;
    } else if(!iequals(thing1,thing2)) {
      Exception::BadValue e;
      e.Append(name);
      e.Append(" mismatch: "); 
      e.Append(thing1); e.Append(" != ");e.Append(thing2);
      throw e;
    }    
  }
  
  
}
