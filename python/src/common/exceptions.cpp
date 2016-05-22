
#include "amc13/python/exceptions.hpp"


#include "boost/python/extract.hpp"

#include "amc13/Exception.hh"


using namespace amc13py;
namespace bpy = boost::python ;


PyObject* amc13py::create_exception_class(const std::string& aExceptionName, PyObject* aBaseExceptionPyType)
{
  std::string scopeName = bpy::extract<std::string> ( bpy::scope().attr ( "__name__" ) );
  std::string qualifiedExcName = scopeName + "." + aExceptionName;
  PyObject* typeObj = PyErr_NewException ( const_cast<char*> ( qualifiedExcName.c_str() ) , aBaseExceptionPyType, 0 );

  if ( !typeObj )
  {
    bpy::throw_error_already_set();
  }

  bpy::scope().attr ( aExceptionName.c_str() ) = bpy::handle<> ( bpy::borrowed ( typeObj ) );
  return typeObj;
}


void amc13py::wrap_exceptions()
{
  // Base amc13 exception (fallback for derived exceptions if not wrapped)
  PyObject* baseExceptionPyType = wrap_exception_class<amc13::Exception::exBase>("exBase", PyExc_Exception);

  // Derived amc13 exceptions
  wrap_exception_class<amc13::Exception::BadChip>("BadChip", baseExceptionPyType);
  wrap_exception_class<amc13::Exception::NULLPointer>("NULLPointer", baseExceptionPyType);
  wrap_exception_class<amc13::Exception::UnexpectedRange>("UnexpectedRange", baseExceptionPyType);
  wrap_exception_class<amc13::Exception::BadFileFormat>("BadFileFormat", baseExceptionPyType);
  wrap_exception_class<amc13::Exception::BadAMC13>("BadAMC13", baseExceptionPyType);
  wrap_exception_class<amc13::Exception::BadValue>("BadValue", baseExceptionPyType);
  wrap_exception_class<amc13::Exception::BadFile>("BadFile", baseExceptionPyType);
}
