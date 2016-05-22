/*
 *
 * File: amc13/python/exceptions.hpp
 * Author: Tom Williams
 * Date: March 2015
 *   (Largely copy of corresponding file in uHAL Python bindings)
 */

#ifndef AMC13_PYTHON_EXCEPTIONS_HPP
#define AMC13_PYTHON_EXCEPTIONS_HPP


#include "boost/python/exception_translator.hpp"


namespace amc13py{

  //! Defines a Python exception class in the current scope
  /*!
    @param aExceptionName Name of the Python class
    @param aBaseExceptionPyType Base class that the newly-created exception class will be derived from
    @return The PyObject* pointer for this exception class 
  */
  PyObject* create_exception_class(const std::string& aExceptionName, PyObject* aBaseExceptionPyType);


  //! Functor that can be registered with boost python, and is thereby used to translate exceptions over the C-Python boundary
  template<class ExceptionType>
  class ExceptionTranslator {
  public:
    //! Creates functor which will translate the C++ exception class ExceptionType to the Python exception class aExceptionPyType 
    ExceptionTranslator(PyObject* aExceptionPyType);

    //! Translation function called at the C-Python boundary when an exception of type ExceptionType is thrown from C++. Essentially, it converts the C++ exception into a Python exception
    void operator() (const ExceptionType& e) const;

  private:
    //! Pointer to the Python exception class corresponding to ExceptionType
    PyObject* mExceptionPyType;
  };


  //! Create Python exception class in the current scope, and register an exception translator which converts C++ class ExceptionType to that Python exception class
  /*!
    @param aExceptionName Name of the Python class
    @param aBaseExceptionPyType Base class of the Python exception class
    @return The Python exception class
   */
  template<class ExceptionType>
  PyObject* wrap_exception_class(const std::string& aExceptionName, PyObject* aBaseExceptionPyType);


  //! Wraps all amc13 exceptions (i.e. creates corresponding Python classes, and regsiters appropriate exception translators)
  void wrap_exceptions();

}


#include "amc13/python/exceptions.hxx"

#endif
