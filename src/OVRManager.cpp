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
/// \file OVRManager.cpp
/// \brief CPP file for OVRManager.h
///
//
//
//////////////////////////////////////////////////////////////////////////////

#include <H3D/OVRManager.h>
#include "H3DUtil/Quaternion.h"
#include "H3DUtil/Rotation.h"
#include <iostream>

 
namespace H3D {

	using H3DUtil::ArithmeticTypes::Quaternion;
	using H3DUtil::ArithmeticTypes::Rotation;
	using H3D::X3DViewpointNode;
	
	using OVR::Sizei;

	void OVRManager::initialise(){

		ovr_Initialize();

		//Create head mounted display object
		hmd = ovrHmd_Create(0);
		if (hmd)
		{
			// Start the sensor which provides the Rift’s pose and motion.
			bool tracking_initialised = ovrHmd_ConfigureTracking(hmd, 
																hmd->TrackingCaps, //Set tracking capabilities 
																0); //Do not require tracking
		}
	}

	bool OVRManager::ovrHMDPresent(){
		return bool(hmd);
	}

	void OVRManager::destroy(){
		ovrHmd_Destroy(hmd);
		ovr_Shutdown();
	}

	void OVRManager::configureRenderSettings(HWND window, HDC hdc, bool separateEyeTextures_){
		//Break if HMD not initialised
		if(!hmd) return;

		//Set number of eye textures
		separateEyeTextures = separateEyeTextures_;

		//Set window for ovr to draw to
		ovrHmd_AttachToWindow(hmd, window, NULL, NULL);

		//Set capabilities of HMD
		ovrHmd_SetEnabledCaps(hmd, hmd->HmdCaps);

		//Configure texture sizes for each eye.
		OVR::Sizei recommenedTexSize_Left = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, hmd->DefaultEyeFov[0], 1.0f);
		OVR::Sizei recommenedTexSize_Right = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, hmd->DefaultEyeFov[1], 1.0f);

		//Multisampling 
		const int eyeRenderMultisample = 1;
		
		Sizei renderTarget[2] = { recommenedTexSize_Left, recommenedTexSize_Right};
		if(separateEyeTextures){
			//Use one texture per eye for rendering prior to distortion
			eyeViewports[ovrEye_Left].Pos = OVR::Vector2i(0, 0);
			eyeViewports[ovrEye_Left].Size = recommenedTexSize_Left;
			eyeViewports[ovrEye_Right].Pos = OVR::Vector2i(0, 0);
			eyeViewports[ovrEye_Right].Size = recommenedTexSize_Right;

			//Store render target sizes for later configuration
			renderTarget[ovrEye_Left] = eyeViewports[ovrEye_Left].Size;
			renderTarget[ovrEye_Right] = eyeViewports[ovrEye_Right].Size;

			genBufferIDs(2);
			createRenderTextureForEye(eyeViewports[ovrEye_Left].Size.w, eyeViewports[ovrEye_Left].Size.h, ovrEye_Left);			
			//TODO:The actual RT size may be different due to HW limits.
			//checkViewport(eyeViewports[ovrEye_Left]);
			//finally, unbind
			
			createRenderTextureForEye(eyeViewports[ovrEye_Right].Size.w, eyeViewports[ovrEye_Right].Size.h, ovrEye_Right);
			//TODO:The actual RT size may be different due to HW limits.
			//checkViewport(eyeViewports[ovrEye_Right]);
			//finally, unbind
			unbindBuffers();

		} else {
			//Use one texture for both eyes for rendering prior to distortion
			renderTarget[ovrEye_Left].w = recommenedTexSize_Left.w + recommenedTexSize_Right.w;	
			//renderTarget height is max of the left and right eye texture heights		
			renderTarget[ovrEye_Right].h = (int(recommenedTexSize_Left.h)>int(recommenedTexSize_Right.h)) ? int(recommenedTexSize_Left.h) : int(recommenedTexSize_Right.h);
			//Both render target textures are the same size as they are the same texture
			renderTarget[ovrEye_Right] = renderTarget[ovrEye_Left];

			eyeViewports[ovrEye_Left].Pos = OVR::Vector2i(0, 0);
			eyeViewports[ovrEye_Left].Size = OVR::Sizei(renderTarget[ovrEye_Left].w / 2, renderTarget[ovrEye_Left].h); 
			eyeViewports[ovrEye_Right].Pos =  OVR::Vector2i(renderTarget[ovrEye_Left].w / 2, 0);
			eyeViewports[ovrEye_Right].Size = eyeViewports[ovrEye_Left].Size;

			genBufferIDs(1);
			createRenderTextureForEye(renderTarget[ovrEye_Left].w, renderTarget[ovrEye_Left].h, ovrEye_Left);
			//TODO:The actual RT size may be different due to HW limits.
			//checkViewport(eyeViewports[ovrEye_Left]);
			//finally, unbind
			unbindBuffers();
		}
		
		for (int eye = 0; eye < 2; eye++ ){
			//Render texture info for rendering and distortion
			eyeTextures[eye].OGL.Header.API = ovrRenderAPI_OpenGL;
			eyeTextures[eye].OGL.Header.RenderViewport = eyeViewports[eye];
			eyeTextures[eye].OGL.Header.TextureSize = renderTarget[eye];
			eyeTextures[eye].OGL.TexId = oculusRiftTextureID[eye];
		}

		// Configure OpenGL.
		ovrGLConfig cfg;

		const int backBufferMultisample = 1;
		cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
		cfg.OGL.Header.BackBufferSize = OVR::Sizei(hmd->Resolution.w, hmd->Resolution.h);
		cfg.OGL.Header.Multisample = backBufferMultisample;
		cfg.OGL.Window = window;
		cfg.OGL.DC = hdc;
		
		ovrBool success = ovrHmd_ConfigureRendering(hmd, &cfg.Config, hmd->DistortionCaps, hmd->DefaultEyeFov, EyeRenderDesc);

	}

	void OVRManager::genBufferIDs(int number_of_buffers){
		glGenTextures(number_of_buffers, &oculusRiftTextureID[0]);

		glGenFramebuffers(number_of_buffers, &oculusFramebufferID[0]);

		glGenRenderbuffers(number_of_buffers, &oculusDepthbufferID[0]);

		if( number_of_buffers == 1 ){
			//Single texture per eye
			oculusRiftTextureID[1] = oculusRiftTextureID[0]; 
			oculusFramebufferID[1] = oculusFramebufferID[0];
			oculusDepthbufferID[1] = oculusDepthbufferID[0];		
		}
	}

	void OVRManager::createRenderTextureForEye(int width, int height, int eye){
			//The texture which will be drawn to
		glBindFramebuffer(GL_FRAMEBUFFER, oculusFramebufferID[eye]);
		
		//Init output textures
		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, oculusRiftTextureID[eye]);
		// Give an empty image to OpenGL ( the last "0" )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		// Poor filtering. Needed !
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// The depth buffer
		glBindRenderbuffer(GL_RENDERBUFFER, oculusDepthbufferID[eye]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, oculusDepthbufferID[eye]);

		// Set "oculusRiftTextureID" as our colour attachement #0
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, oculusRiftTextureID[eye], 0);
	}

	OVR::Sizei OVRManager::getTextureSizei(){
		OVR::Sizei s;
		int miplevel = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &s.w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &s.h);
		return s;
	}

	void OVRManager::startFrame(){
		ovrHmd_BeginFrame(hmd, 0);		
	}

	void OVRManager::endFrame(){
		unbindBuffers();
		ovrHmd_EndFrame(hmd, headPoses, &eyeTextures[0].Texture);
	}

	void OVRManager::setProjectionMatrix(ovrEyeType eye) {
		OVR::Matrix4f proj = OVR::Matrix4f(ovrMatrix4f_Projection(EyeRenderDesc[eye].Fov, near_distance, far_distance, true));
		glMultMatrixf(getColumnMajorRepresentation(proj));
		// For some reason the eye textures are inverted between drawing to oculusRiftEyeTexture and the screen. It would be good to figure out why
		// The health warning is the correct way up. 
		// This flips clips space and tells open gl that the polygons are now coiled clockwise
      	glScalef( 1, -1, 1 );
      	glFrontFace( GL_CW );  
	}

	Matrix4f OVRManager::getHeadPose(){
		ovrPosef headPose = ovrHmd_GetHmdPosePerEye(hmd, ovrEye_Left);
		OVR::Quatf orientation = OVR::Quatf(headPose.Orientation);
		OVR::Matrix4f view = OVR::Matrix4f::Translation(headPose.Position.x,headPose.Position.y,headPose.Position.z) * OVR::Matrix4f(orientation); 
		return Matrix4f(view.M[0][0], view.M[0][1], view.M[0][2], view.M[0][3], 
				        view.M[1][0], view.M[1][1], view.M[1][2], view.M[1][3], 
				        view.M[2][0], view.M[2][1], view.M[2][2], view.M[2][3], 
				        view.M[3][0], view.M[3][1], view.M[3][2], view.M[3][3]);
	}

	void OVRManager::setViewMatrix(ovrEyeType eye){
		headPoses[eye] = ovrHmd_GetHmdPosePerEye(hmd, eye);
		OVR::Quatf orientation = OVR::Quatf(headPoses[eye].Orientation);
		OVR::Matrix4f view = OVR::Matrix4f(orientation.Inverted()) * OVR::Matrix4f::Translation(-headPoses[eye].Position.x,-headPoses[eye].Position.y,-headPoses[eye].Position.z); 
		OVR::Matrix4f camToCalibrationlessSpace = /*OVR::Matrix4f::Translation(EyeRenderDesc[eye].HmdToEyeViewOffset) */ view;
		OVR::Matrix4f camToWorld =camToCalibrationlessSpace * worldToCalibration;
		glMultMatrixf(getColumnMajorRepresentation(camToWorld));
	}

	void OVRManager::setViewport(ovrEyeType eye){
		glViewport( eyeViewports[eye].Pos.x, eyeViewports[eye].Pos.y, 
					eyeViewports[eye].Size.w, eyeViewports[eye].Size.h );
	}

	void OVRManager::drawBuffer(ovrEyeType eye){
		glBindFramebuffer(GL_FRAMEBUFFER, oculusFramebufferID[eye]);
	}

	void OVRManager::unbindBuffers(){
		glBindTexture(GL_TEXTURE_2D, 0);
   		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	ovrEyeType OVRManager::getEyeOrder(int eye_number){
		return hmd->EyeRenderOrder[eye_number];
	}

	//Currently redundant
	ovrEyeType OVRManager::H3DEyeModeToOVREyeType(X3DViewpointNode::EyeMode eye_mode){
		if (eye_mode == X3DViewpointNode::EyeMode::MONO || eye_mode == X3DViewpointNode::EyeMode::LEFT_EYE){
			return ovrEye_Left;
		} else {
			return ovrEye_Right;
		}
	}

	GLfloat* OVRManager::getColumnMajorRepresentation(OVR::Matrix4f m){
	    GLfloat M[16] = {m.M[0][0], m.M[1][0], m.M[2][0], m.M[3][0],
						m.M[0][1], m.M[1][1], m.M[2][1], m.M[3][1],
						m.M[0][2], m.M[1][2], m.M[2][2], m.M[3][2],
						m.M[0][3], m.M[1][3], m.M[2][3], m.M[3][3] };
		return M;
	}


	//CALIBRATION

	void OVRManager::setCalibrationSamples(const std::vector<OVR::Matrix4f>& HMDSamples_, const std::vector<OVR::Matrix4f>& PenSamples_){
		HMDSamples = HMDSamples_;
		PenSamples = PenSamples_;
		bestError = 9999999999999999999;
	}

	double OVRManager::calibrate(int iterations){
		
		for (int i = 0; i < iterations; i++){
			OVR::Matrix4f newRobotToOVR = deltaMat(bestRobotToOVR);
			OVR::Matrix4f newPenToHMD = deltaMat(bestPenToHMD);
			double newError = computeCalibrationSquareError(newRobotToOVR,newPenToHMD);
			if (newError < bestError){
				bestError = newError;
				bestRobotToOVR = newRobotToOVR;
				bestPenToHMD = newPenToHMD;
			}
		}

		return bestError;
	}

	double OVRManager::computeCalibrationSquareError(const OVR::Matrix4f& robotToOVR, const OVR::Matrix4f& penToHMD){
		double error = 0;
		for (int i = 0; i < PenSamples.size(); i++){
			OVR::Matrix4f errorMat = (HMDSamples[i] - robotToOVR * PenSamples[i] * penToHMD.Inverted());
			for(int j = 0; j < 4; j++){
				for(int k = 0; k < 4; k++){
					error += errorMat.M[j][k] * errorMat.M[j][k];
				}
			}
		}
		return error;
	}

	OVR::Matrix4f OVRManager::deltaMat(const OVR::Matrix4f& m){
		float theta = 2 * M_PI * rand() / float(RAND_MAX);
		float phi = M_PI * rand() / float(RAND_MAX);
		OVR::Vector3f axis = OVR::Vector3f(std::cos(theta) * std::cos(phi), std::sin(theta) * std::cos(phi), std::sin(phi));
		float angle = angleLearningRate * rand() / float(RAND_MAX);
		OVR::Quatf qr = OVR::Quatf(axis, angle) * OVR::Quatf(m);
		OVR::Matrix4f r(qr);

		OVR::Vector3f translation = OVR::Vector3f(translationLearningRate * rand() / float(RAND_MAX), translationLearningRate * rand() / float(RAND_MAX), translationLearningRate * rand() / float(RAND_MAX));
		r.SetTranslation(m.GetTranslation() + translation);
		
		return r;
	}

}
