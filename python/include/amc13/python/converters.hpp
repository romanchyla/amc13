/* 
 * File:   converters.hpp
 * Author: ale
 *
 * Created on March 22, 2015, 10:25 AM
 */

#ifndef AMC13_PYTHON_CONVERTERS_EXCEPTIONS_HPP
#define	AMC13_PYTHON_CONVERTERS_EXCEPTIONS_HPP


// C++ Headers
#include <vector>

// Boost Headers
#include <boost/python/object.hpp>
#include <boost/python/dict.hpp>
#include <boost/python/converter/rvalue_from_python_data.hpp>

namespace amc13py {

  // CONVERTERS
  template<class T>
  struct Converter_std_vector_from_list {

    // Default CTOR. Registers this converter to boost::python
    Converter_std_vector_from_list();

    // Determine if obj_ptr can be converted to vector<T>
    static void* convertible ( PyObject* obj_ptr );

    // Convert obj_ptr to a C++ vector<T>
    static void construct ( PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data );
  };
  
  
  //------------------//
  template <class T>
  struct Converter_std_vector_to_list {
    static PyObject* convert ( const std::vector<T>& v );
  };


  
  void register_converters();

} // namespace amc13py

#include "amc13/python/converters.hxx"

#endif	/* CONVERTERS_EXCEPTIONS_HPP */

