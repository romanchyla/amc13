#ifndef __STATUS_HH__
#define __STATUS_HH__

//AMC13 & uhal interface
#include "amc13/AMC13Simple.hh"

#include <ostream>
#include <iostream> //for std::cout
#include <vector>
#include <string>
#include <map>
#include <boost/unordered_map.hpp>
#include <boost/tokenizer.hpp> //for tokenizer

namespace amc13{

  const char ParameterParseToken = '_';
  const std::string DefaultFormat = "X";
  
  std::string TableStrip(const std::string & s1);
  bool TableNameCompare(const std::string & s1, const std::string & s2);

  /*! \brief One cell in a status display.
   *
   * One cell corresponds to one item from the AMC13 address table,
   * along with all the additional information required to display it.
   * see http://bucms.bu.edu/twiki/bin/view/BUCMSPublic/AMC13AddressTable.
   */
  class Cell{
  public:
    ///! Constructor
    Cell(){Clear();};
    ///! Fill in values for a cell
    void Setup(std::string const & _address,  /// address table name stripped of Hi/Lo
	       std::string const & _description, /// long description
	       std::string const & _row,	 /// display row
	       std::string const & _col,	 /// display column
	       std::string const & _format,	 /// display format
	       std::string const & _rule,	 /// nz, z, nzr etc
	       std::string const & _statusLevel); /// status level to display
    ///! store a value plus a shift for a multi-word value
    void Fill(uhal::ValWord<uint32_t> value,
	      size_t bitShift = 0);

    ///! Determine if this cell should be displayed based on rule, statusLevel
    bool Display(int level,bool force = false) const;
    ///! Determine if this cell's row should be suppressed based on rule="nzr"
    bool SuppressRow( bool force) const;
    int DisplayLevel() const;
    std::string Print(int width,bool html = false) const;
    std::string const & GetRow() const;
    std::string const & GetCol() const;
    std::string const & GetDesc() const;
    std::string const & GetAddress() const;

    void SetAddress(std::string const & _address);
    uint32_t const & GetMask() const;
    void SetMask(uint32_t const & _mask);

  private:    
    void Clear();
    void CheckAndThrow(std::string const & name,
		       std::string & thing1,
		       std::string const & thing2) const;
    uint64_t ComputeValue() const;

    std::string address;
    std::string description;
    std::string row;
    std::string col;

    uint32_t mask;
    
    std::vector<uhal::ValWord<uint32_t> > valWord;
    std::vector<int> valWordShift;
    std::string format;
    std::string displayRule;   
    int statusLevel;
  };

  enum StatusMode {
    TEXT,
    HTML,
    BAREHTML,
    LATEX
  };

  class SparseCellMatrix{
  public:
    SparseCellMatrix(){Clear();};
    ~SparseCellMatrix(){Clear();};
    void Add(std::string chipName,uhal::Node const & node);
    void Render(std::ostream & stream,int status,StatusMode statusMode = TEXT) const;
    std::vector<std::string> GetTableRows() const;
    std::vector<std::string> GetTableColumns() const;
    const Cell* GetCell(const std::string& row, const std::string& col) const;
 private:
    void Clear();
    void CheckName(std::string const & newName);
    std::string ParseRow(boost::unordered_map<std::string,std::string> & parameters,
			 std::string const & addressBase) const;
    std::string ParseCol(boost::unordered_map<std::string,std::string> & parameters,
			 std::string const & addressBase) const;

    std::vector<Cell*> row(std::string const &);
    std::vector<Cell*> col(std::string const &);


    void Print(std::ostream & stream,int status,bool force,int headerColWidth,
	       std::map<std::string,bool> & rowDisp,std::vector<int> & colWidth) const;
    void PrintHTML(std::ostream & stream,int status,bool force,int headerColWidth,
		   std::map<std::string,bool> & rowDisp,std::vector<int> & colWidth) const;
    void PrintLaTeX(std::ostream & stream) const;

    std::string name;
    std::map<std::string,Cell*> cell;
    std::map<std::string,std::map<std::string,Cell *> > rowColMap;
    std::map<std::string,std::map<std::string,Cell *> > colRowMap;    
    mutable std::vector<std::string> rowName;
    mutable std::vector<std::string> colName;
  };
  
  class Status{
  public:
    /*! \brief Construct an AMC13 status object */
    Status(AMC13Simple * _amc13,int _version);


    void ReportHeader(std::ostream & stream) const;
    std::string ReportHeader() const;
    void ReportStyle(std::ostream & stream) const;
    std::string ReportStyle() const;
    void ReportBody(size_t level,std::ostream & stream, std::string const & singleTable = std::string(""));
    std::string ReportBody(size_t level, std::string const & singleTable = std::string(""));
    void ReportTrailer(std::ostream & stream) const;
    std::string ReportTrailer() const;


    /*! \brief Emit a status report to output stream
     *
     * Generate a status report using extra information in the XML address
     * table and output to stream.  By default the format is plain text, best
     * rendered with a fixed-width font.  Other options currently available are
     * HTML (with styling), bare HTML (no header or styling) and LaTeX source.
     *
     * \param level Verbosity level, from 1 to 9
     * \param stream Output stream
     * \param singleTable Optional name of one HTML table to emit (see address table for table names)
     */
    void Report(size_t level,
		std::ostream & stream=std::cout,
		std::string const & singleTable = std::string(""));
    /*! \brief Emit a bare HTML report for one HTML table
     *
     * Wrapper function which provides a convenient interface to
     * emit bare HTML for a single status table.  This is suited to
     * embedding the display in a parent HTML page
     *
     * \param level Verbosity level, from 1 to 9
     * \param stream Output stream
     * \param singleTable name of one HTML table to emit (see address table for table names)
     */
    std::string ReportBare( size_t level, std::string const & singleTable = std::string(""));
    /*! Select output mode */
    void SetOutputMode(StatusMode mode);
    /*! Get output mode */
    StatusMode GetOutputMode() const ;
    /*! Select "bare HTML" output mode */
    void SetBareHTML();
    /*! Unselect "bare HTML" output tmode */
    void UnsetBareHTML();
    /*! Select HTML output mode */
    void SetHTML();
    /*! Unselect HTML output mode */
    void UnsetHTML();
    /*! Select LaTeX source output mode */
    void SetLaTeX();
    /*! Unselect LaTeX source output mode */
    void UnsetLaTeX();
    /*! Get a const pointer to a SparseCellMatrix item */
    const SparseCellMatrix* GetTable(const std::string& table) const;
    /*! Get a list of tables */
    std::vector<std::string> GetTableList() const;
    /*! Get a list of Rows in a given table */
    std::vector<std::string> GetTableRows(std::string const & table) const;
    /*! Get a list of Columns in a givenTable */
    std::vector<std::string> GetTableColumns(std::string const & table) const;
    /*! Get a const pointer to the cell in the given position */
    const Cell* GetCell(const std::string & table, const std::string & row, const std::string & col) const; 
  private:
    void ProcessChip(AMC13Simple::Board chip,std::string const & singleTable);
    void SetAMCMask(uint32_t mask);
    void SetSFPMask(uint32_t mask);
    bool debug;
    StatusMode statusMode;
    int version;
    AMC13Simple * amc13;
    std::map<std::string,SparseCellMatrix> tables;
  };
}
#endif
