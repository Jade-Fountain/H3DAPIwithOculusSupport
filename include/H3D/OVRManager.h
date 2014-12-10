//////////////////////////////////////////////////////////////////////////////
//	  Author: Jake Fountain 2014
//    This file is part of H3D API Oculus rift extension.
// 	  The code for this file has been created based on the code exhibited in the OVR Developer Documentation v0.4.4
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
/// \file OVRManager.h
/// \brief H file for OVRManager.cpp
///
//
//
//////////////////////////////////////////////////////////////////////////////
#ifndef VIRTUALREALITY_OVRManager_H
#define VIRTUALREALITY_OVRManager_H

#include "OVR.h"

namespace virtualreality {

	class OVRManager {
	public:
		void initialise();
		void destroy();
		
	private:
		ovrHmd hmd;

		bool ovrHMDPresent;


		ovrPoseStatef getPoseOfHMD();

		bool checkHealthWarningState();
	};

}

#endif