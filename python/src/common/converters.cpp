#include "amc13/python/converters.hpp"

#include <boost/python/to_python_converter.hpp>
#include <boost/python/scope.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/converter/registry.hpp>

namespace bpy = boost::python;

namespace amc13py {

void register_converters() {
  
    // Pointer to boost python converter's register
    const bpy::converter::registration* reg;

    // std::vector<uint64_t>
    // Ensure the converter for this type is not already registered
    reg = bpy::converter::registry::query(bpy::type_id< std::vector<uint64_t> >());
    if (reg == NULL or (*reg).m_to_python == NULL) {
    // Duplicated converter
        Converter_std_vector_from_list<uint64_t>();
        bpy::to_python_converter< std::vector<uint64_t>, amc13py::Converter_std_vector_to_list<uint64_t> >();
    }
    
}

} // namespace amc13py
