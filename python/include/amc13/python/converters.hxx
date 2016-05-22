/* 
 * File:   converters_exceptions.hxx
 * Author: ale
 *
 * Created on March 23, 2015, 2:18 PM
 */

#ifndef AMC13_PYTHON_CONVERTERS_EXCEPTIONS_HXX
#define	AMC13_PYTHON_CONVERTERS_EXCEPTIONS_HXX


namespace amc13py {
//------------------------------------------//
// ---  Converter_std_vector_from_list  --- //
//------------------------------------------//

//---
template<class T>
Converter_std_vector_from_list<T>::Converter_std_vector_from_list() {
    boost::python::converter::registry::push_back ( &convertible, &construct, boost::python::type_id< std::vector<T> >() );
} 


//---
template <class T>
void* 
Converter_std_vector_from_list<T>::convertible ( PyObject* obj_ptr ) {
  if ( !PyList_Check ( obj_ptr ) ) {
    return 0;
  } else {
    return obj_ptr;
  }
}


//---
template <class T>
void 
Converter_std_vector_from_list<T>::construct ( PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data ) {
  namespace bpy = boost::python;
  // Grab pointer to memory in which to construct the new vector
  void* storage = ( ( bpy::converter::rvalue_from_python_storage< std::vector<T> >* ) data )->storage.bytes;
  // Grab list object from obj_ptr
  bpy::list py_list ( bpy::handle<> ( bpy::borrowed ( obj_ptr ) ) );
  // Construct vector in requested location, and set element values.
  // boost::python::extract will throw appropriate exception if can't convert to type T ; boost::python will then call delete itself as well.
  size_t nItems = bpy::len ( py_list );
  std::vector<T>* vec_ptr = new ( storage ) std::vector<T> ( nItems );

  for ( size_t i = 0; i < nItems; i++ ) {
    vec_ptr->at ( i ) = bpy::extract<T> ( py_list[i] );
  }

  // If successful, then register memory chunk pointer for later use by boost.python
  data->convertible = storage;
}


//----------------------------------------//
// ---  Converter_std_vector_to_list  --- //
//----------------------------------------//

//---
template <class T>
PyObject*
Converter_std_vector_to_list<T>::convert ( const std::vector<T>& vec ) {
  namespace bpy = boost::python;
  bpy::list theList;

  for ( typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end(); it++ ) {
    theList.append ( bpy::object ( *it ) );
  }

  return bpy::incref ( theList.ptr() );
}


}

#endif	/* AMC13_PYTHON_CONVERTERS_EXCEPTIONS_HXX */

