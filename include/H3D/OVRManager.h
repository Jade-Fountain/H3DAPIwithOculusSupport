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
#ifndef H3D_OVRManager_H
#define H3D_OVRManager_H

#include "OVR.h"

#include "H3D/StereoInfo.h"
#include "H3D/X3DViewpointNode.h"
#include <GL/glew.h>
#include "OVR_CAPI_GL.h"

namespace H3D {

	class OVRManager {
	public:
		OVRManager() : ovrHMDPresent(false),
					   oculusRiftTextureID(0),
					   renderTargetSize(){}

		void initialise();
		void destroy();
		
		bool ovrHMDPresent;

    	//Get IPD, FOV, pose, etc
		void getHMDInfo(H3D::StereoInfo* info);

		bool checkHealthWarningState();

		void configureRenderSettings(HWND window, HDC hdc);

		void createRenderTexture(int width, int height, int samples);
		
		void startFrame();
		
		void endFrame();

		void setProjectionMatrix(H3D::X3DViewpointNode::EyeMode eye_mode);

		void setViewMatrix(H3D::X3DViewpointNode::EyeMode eye_mode);

		void drawBuffer(H3D::X3DViewpointNode::EyeMode eye_mode);
		
		void unbindBuffers();

		std::string getConsoletext();

	private:		

		std::stringstream console;

		ovrHmd hmd;

		ovrEyeRenderDesc EyeRenderDesc[2];
		ovrPosef headPoses[2];
		ovrGLTexture eyeTextures[2];
		ovrRecti eyeViewports[2];
		OVR::Sizei renderTargetSize;

		GLuint oculusRiftTextureID;
		GLuint oculusFramebufferID;
		GLuint oculusDepthbufferID;

		ovrPoseStatef getPoseOfHMD();

		ovrEyeType H3DEyeModeToOVREyeType(H3D::X3DViewpointNode::EyeMode eye_mode);
		
		//Utilities
		GLfloat* getColumnMajorRepresentation(OVR::Matrix4f m);

		std::string getString(OVR::Matrix4f m);
	};

}

#endif