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

	void OVRManager::dismissHealthWarning(){
		//Health and Saftey Warning Display State
		ovrHSWDisplayState hswDisplayState;
		ovrHmd_GetHSWDisplayState(hmd, &hswDisplayState);
		if (hswDisplayState.Displayed)
		{
			ovrHmd_DismissHSWDisplay(hmd);
		}
	}

	bool OVRManager::checkHealthWarningState(){
		//Health and Saftey Warning Display State
		ovrHSWDisplayState hswDisplayState;
		ovrHmd_GetHSWDisplayState(hmd, &hswDisplayState);
		// Dismiss the warning if the user pressed the appropriate key or if the user
		// is tapping the side of the hmd.
		if (hswDisplayState.Displayed)
		{
			//If the warning is still displayed, check for triggers to disable

			//TODO: add key presses to turn off health and safety warning
			// If the user has requested to dismiss the warning via keyboard or controller input...
			// if (Util_GetAndResetHSWDismissedState()){
			// 	ovrHmd_DismissHSWDisplay(hmd);
			// 	return false;
			// }
			// else
			//End TODO
		
			// Detect a moderate tap on the side of the hmd.
			ovrTrackingState ts = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds());
			if (ts.StatusFlags & ovrStatus_OrientationTracked)
			{

				const OVR::Vector3f v(ts.RawSensorData.Accelerometer.x,
									  ts.RawSensorData.Accelerometer.y,
									  ts.RawSensorData.Accelerometer.z);

				// Arbitrary value and representing moderate tap on the side of the DK2 Rift.
				if (v.LengthSq() > 250.f) {
					ovrHmd_DismissHSWDisplay(hmd);
					//The HSW is not displayed
					return false;
				}

			}
			//The HSW is still displayed
			return true;
		}
		//The HSW is not displayed
		return false;
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

		if(separateEyeTextures){
			//Use one texture per eye for rendering prior to distortion
			eyeViewports[ovrEye_Left].Pos = OVR::Vector2i(0, 0);
			eyeViewports[ovrEye_Left].Size = recommenedTexSize_Left;
			eyeViewports[ovrEye_Right].Pos = OVR::Vector2i(0, 0);
			eyeViewports[ovrEye_Right].Size = recommenedTexSize_Right;

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
			Sizei renderTarget;
			renderTarget.w = recommenedTexSize_Left.w + recommenedTexSize_Right.w;	
			//renderTarget height is max of the left and right eye texture heights		
			renderTarget.h = (int(recommenedTexSize_Left.h)>int(recommenedTexSize_Right.h)) ? int(recommenedTexSize_Left.h) : int(recommenedTexSize_Right.h);

			eyeViewports[ovrEye_Left].Pos = OVR::Vector2i(0, 0);
			eyeViewports[ovrEye_Left].Size = OVR::Sizei((renderTarget.w + 1) / 2, renderTarget.h); 
			eyeViewports[ovrEye_Right].Pos =  OVR::Vector2i(renderTarget.w / 2, 0);
			eyeViewports[ovrEye_Right].Size = eyeViewports[ovrEye_Left].Size;

			genBufferIDs(1);
			createRenderTextureForEye(renderTarget.w, renderTarget.h, ovrEye_Left);
			//TODO:The actual RT size may be different due to HW limits.
			//checkViewport(eyeViewports[ovrEye_Left]);
			//finally, unbind
			unbindBuffers();
		}

		for (int eye = 0; eye < 2; eye++ ){
			eyeTextures[eye].OGL.Header.API = ovrRenderAPI_OpenGL;
			eyeTextures[eye].OGL.Header.RenderViewport = eyeViewports[eye];
			eyeTextures[eye].OGL.Header.TextureSize = eyeViewports[eye].Size;
			eyeTextures[eye].OGL.TexId = oculusRiftTextureID[eye];
			eyeTextures[eye].Texture.Header.API = ovrRenderAPI_OpenGL;
			eyeTextures[eye].Texture.Header.RenderViewport = eyeViewports[eye];
			eyeTextures[eye].Texture.Header.TextureSize = eyeViewports[eye].Size;
		}

		// Configure OpenGL.
		ovrGLConfig cfg;

		const int backBufferMultisample = 1;
		cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
		cfg.OGL.Header.BackBufferSize = OVR::Sizei(hmd->Resolution.w, hmd->Resolution.h);
		cfg.OGL.Header.Multisample = backBufferMultisample;
		cfg.Config.Header.API = ovrRenderAPI_OpenGL;
		cfg.Config.Header.BackBufferSize = OVR::Sizei(hmd->Resolution.w, hmd->Resolution.h);
		cfg.Config.Header.Multisample = backBufferMultisample;
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
		
//TODO DELETE ONCE WORKING:
		// // //glBindFramebuffer(GL_READ_FRAMEBUFFER, oculusReadFramebufferID[i]);
		// glBindFramebuffer(GL_FRAMEBUFFER, flippedFramebufferID[eye]);
		// //Init backbuffer flipped textures
		// // "Bind" the newly created texture : all future texture functions will modify this texture
		// glBindTexture(GL_TEXTURE_2D, flippedRiftTextureID[eye]);
		// // Give an empty image to OpenGL ( the last "0" )
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		// // Poor filtering. Needed !
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		// // The depth buffer
		// glBindRenderbuffer(GL_RENDERBUFFER, oculusDepthbufferID[eye]);
		// glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		// glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, oculusDepthbufferID[eye]);

		// // Set "flippedRiftTextureID" as our colour attachement #0; draw to flipped first
		// glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, flippedRiftTextureID[eye], 0);
		// // Set the list of draw buffers.
		// // GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
		// // glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
// ENDTODO


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

		// Set the list of draw buffers.
		// GLenum DrawBuffers2[1] = {GL_COLOR_ATTACHMENT0};
		// glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
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

		//Is this introducing latency?
		
		headPoses[ovrEye_Left] = ovrHmd_GetHmdPosePerEye(hmd, ovrEye_Left);
		headPoses[ovrEye_Right] = ovrHmd_GetHmdPosePerEye(hmd, ovrEye_Right);
		
	}

	void OVRManager::endFrame(){
		unbindBuffers();
		ovrHmd_EndFrame(hmd, headPoses, &eyeTextures[0].Texture);
	}

	void OVRManager::setProjectionMatrix(X3DViewpointNode::EyeMode eye_mode) {
		ovrEyeType eye = H3DEyeModeToOVREyeType(eye_mode);
		OVR::Matrix4f proj = OVR::Matrix4f(ovrMatrix4f_Projection(EyeRenderDesc[eye].Fov, near_distance, far_distance, true));
		glMultMatrixf(getColumnMajorRepresentation(proj));

		// For some reason the eye textures are inverted between drawing to oculusRiftEyeTexture and the screen. It would be good to figure out why
		// The health warning is the correct way up. 
		// This flips clips space and tells open gl that the polygons are now coiled clockwise
      	glScalef( 1, -1, 1 );
      	glFrontFace( GL_CW );  
	}

	void OVRManager::setViewMatrix(X3DViewpointNode::EyeMode eye_mode){
		ovrEyeType eye = H3DEyeModeToOVREyeType(eye_mode);
		OVR::Quatf orientation = OVR::Quatf(headPoses[eye].Orientation);
		OVR::Matrix4f view = OVR::Matrix4f(orientation.Inverted()) * OVR::Matrix4f::Translation(-headPoses[eye].Position.x,-headPoses[eye].Position.y,-headPoses[eye].Position.z); 
		glMultMatrixf(getColumnMajorRepresentation(OVR::Matrix4f::Translation(EyeRenderDesc[eye].HmdToEyeViewOffset) * view));
	}

	void OVRManager::setViewport(H3D::X3DViewpointNode::EyeMode eye_mode){
		ovrEyeType eye = H3DEyeModeToOVREyeType(eye_mode);
		glViewport( eyeViewports[eye].Pos.x, eyeViewports[eye].Pos.y, 
					eyeViewports[eye].Pos.x + eyeViewports[eye].Size.w, eyeViewports[eye].Pos.y + eyeViewports[eye].Size.h );
		//glViewport( width->getValue(), 0, width->getValue(), height->getValue() );
	}

	void OVRManager::drawBuffer(H3D::X3DViewpointNode::EyeMode eye_mode){
		ovrEyeType eye = H3DEyeModeToOVREyeType(eye_mode);
		//Render texture info for rendering and distortion

		glBindFramebuffer(GL_FRAMEBUFFER, oculusFramebufferID[eye]);
	}

	void OVRManager::unbindBuffers(){
		glBindTexture(GL_TEXTURE_2D, 0);
   		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

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
}
