#ifndef STRING_BIMAP_H
#define STRING_BIMAP_H

#include <boost/bimap.hpp>

#include "utilities.h"

template<class property_type> struct string_bimap : public boost::bimaps::bimap<mstring, property_type>
{                                                                                                 
    typedef boost::bimaps::bimap<mstring, property_type> Base;

    string_bimap() : Base() {}
    template<class InputIterator> string_bimap( InputIterator first, InputIterator last ) : Base( first, last ) {}

    const property_type& get_specifier( cstring& specifier_name ) const
    {                                                                 
        typename Base::left_const_iterator it = Base::left.find( specifier_name );

        if ( it == Base::left.end() ) throw std::runtime_error( "Not a valid property specifier name: " + specifier_name );

        return it->second;
    }                     

    cstring& get_specifier( const property_type& specifier_value ) const
    {                                                                  
        typename Base::right_const_iterator it = Base::right.find( specifier_value );

        if ( it == Base::right.end() ) throw std::runtime_error( "Not a valid property specifier value: " + utilities::to_s( specifier_value ) );

        return it->second;
    }                     
};

#endif // STRING_BIMAP_H
