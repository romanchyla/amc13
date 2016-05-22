
/*
 *
 * File: amc13/python/exceptions.hxx
 * Author: Tom Williams
 * Date: March 2015
 *   (Largely copy of corresponding file in uHAL Python bindings)
 */

#ifndef AMC13_PYTHON_EXCEPTIONS_HXX
#define AMC13_PYTHON_EXCEPTIONS_HXX


#include "boost/python/object.hpp"
#include "boost/python/scope.hpp"



template<class ExceptionType>
amc13py::ExceptionTranslator<ExceptionType>::ExceptionTranslator(PyObject* aExceptionPyType) : 
 mExceptionPyType(aExceptionPyType) { 
}


template<class ExceptionType>
void amc13py::ExceptionTranslator<ExceptionType>::operator() (const ExceptionType& e) const {
  namespace bpy = boost::python;
  bpy::object pyException(bpy::handle<> (bpy::borrowed(mExceptionPyType)));

  // Add exception::what() string as ".what" attribute of Python exception
  pyException.attr("what") = e.what();
  pyException.attr("Description") = e.Description();
  PyErr_SetObject(mExceptionPyType, pyException.ptr());
}


template<class ExceptionType>
PyObject* amc13py::wrap_exception_class(const std::string& aExceptionName, PyObject* aBaseExceptionPyType)
{
  PyObject* exceptionPyType = amc13py::create_exception_class(aExceptionName, aBaseExceptionPyType);
  boost::python::register_exception_translator<ExceptionType>(amc13py::ExceptionTranslator<ExceptionType>(exceptionPyType));

  return exceptionPyType;
}


#endif
