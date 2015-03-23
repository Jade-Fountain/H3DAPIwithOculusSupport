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

#include <string>
#include <fstream>
#include <streambuf>

#define _USE_MATH_DEFINES //For M_PI
#include <math.h>

namespace H3D {

	class OVRManager {
	public:
		bool separateEyeTextures;
		OVRManager() : separateEyeTextures(true),
				   	   near_distance(0.001f),
					   far_distance(100.0f),
					   hmd(0)
					   {
					   	//Translate in the real world
					   	worldToCalibration = OVR::Matrix4f::Translation(0,-0.33,-0.310);
					   	//Scale to to match haptics
					   	worldToCalibration *= OVR::Matrix4f::Scaling(1);

					   	bestPenToHMD = OVR::Matrix4f::Identity();
						bestRobotToOVR = OVR::Matrix4f::Identity();
						bestError = 9999999999999999999;	

						angleLearningRate = M_PI;
						translationLearningRate = 0.1;
					   }


		void initialise();

		bool ovrHMDPresent();
		
		void destroy();

		void createShaders();		

		void dismissHealthWarning();

		bool checkHealthWarningState();

		void configureRenderSettings(HWND window, HDC hdc, bool separateEyeTextures_ = true);

		void genBufferIDs(int number_of_buffers);

		void createRenderTextureForEye(int width, int height, int eye);

		OVR::Sizei getTextureSizei();
		
		void startFrame();
		
		void endFrame();

		void setProjectionMatrix(ovrEyeType eye);

		Matrix4f getHeadPose();
		
		void setViewMatrix(ovrEyeType eye);

		void setViewport(ovrEyeType eye);

		void drawBuffer(ovrEyeType eye);
		
		void unbindBuffers();

		std::string getConsoletext();

		ovrEyeType getEyeOrder(int eye_number);

		const float near_distance;
		const float far_distance;

		//CALIBRATION

		void setCalibrationSamples(const std::vector<OVR::Matrix4f>& HMDSamples_, const std::vector<OVR::Matrix4f>& PenSamples_);
		
		double calibrate(int iterations = 100);
		
		double computeCalibrationSquareError(const OVR::Matrix4f& robotToOVR, const OVR::Matrix4f& penToHMD);
		
		OVR::Matrix4f deltaMat(const OVR::Matrix4f& m);
	private:		
		//TODO comment
		ovrHmd hmd;

		ovrEyeRenderDesc EyeRenderDesc[2];

		ovrPosef headPoses[2];

		ovrGLTexture eyeTextures[2];
		ovrRecti eyeViewports[2];

		GLuint oculusRiftTextureID[2];
		GLuint oculusFramebufferID[2];
		GLuint oculusDepthbufferID[2];

		OVR::Matrix4f worldToCalibration;

		ovrEyeType H3DEyeModeToOVREyeType(H3D::X3DViewpointNode::EyeMode eye_mode);

		//Utilities
		GLfloat* getColumnMajorRepresentation(OVR::Matrix4f m);

		//Calibration variables
		OVR::Matrix4f bestPenToHMD;
		OVR::Matrix4f bestRobotToOVR;
		double bestError;
		float angleLearningRate;
		float translationLearningRate;
		std::vector<OVR::Matrix4f> PenSamples;
		std::vector<OVR::Matrix4f> HMDSamples;
	};

}

#endif