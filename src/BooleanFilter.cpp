//////////////////////////////////////////////////////////////////////////////
//    Copyright 2004-2014, SenseGraphics AB
//
//    This file is part of H3D API.
//
//    H3D API is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    H3D API is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with H3D API; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//    A commercial license is also available. Please contact us at 
//    www.sensegraphics.com for more information.
//
//
/// \file BooleanFilter.cpp
/// \brief CPP file for BooleanFilter, X3D scene-graph node
///
//
//
//////////////////////////////////////////////////////////////////////////////

#include <H3D/BooleanFilter.h>

using namespace H3D;

// Add this node to the H3DNodeDatabase system.
H3DNodeDatabase BooleanFilter::database( 
                                   "BooleanFilter", 
                                   &(newInstance<BooleanFilter>), 
                                   typeid( BooleanFilter ),
                                   &X3DChildNode::database );

namespace BooleanFilterInternals {
  FIELDDB_ELEMENT( BooleanFilter, set_boolean, INPUT_ONLY );
  FIELDDB_ELEMENT( BooleanFilter, inputTrue, OUTPUT_ONLY );
  FIELDDB_ELEMENT( BooleanFilter, inputFalse, OUTPUT_ONLY );
  FIELDDB_ELEMENT( BooleanFilter, inputNegate, OUTPUT_ONLY );
}

BooleanFilter::BooleanFilter( Inst< SFNode       > _metadata,
                              Inst< SetBoolean   > _set_boolean,
                              Inst< SFBool       > _inputFalse,
                              Inst< NegateSFBool > _inputNegate,
                              Inst< SFBool       > _inputTrue ):
  X3DChildNode( _metadata ),
  set_boolean( _set_boolean ),
  inputFalse( _inputFalse ),
  inputNegate( _inputNegate ),
  inputTrue( _inputTrue ) {

  type_name = "BooleanFilter";
  database.initFields( this );

  set_boolean->setValue( true, id );
  set_boolean->route( inputNegate, id );
}

