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


	void OVRManager::initialise(){
		std::cerr << "Initialising Oculus Rift...";
		ovr_Initialize();
		hmd = ovrHmd_Create(0);
		if (hmd)
		{
			ovrHMDPresent = true;
			// Start the sensor which provides the Rift’s pose and motion.
			bool tracking_initialised = ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);
			console << "Rift Found!" << std::endl;
		} else {
			ovrHMDPresent = false;
			console << "Rift NOT Found." << std::endl;
		}	
	}

	void OVRManager::destroy(){
		ovrHmd_Destroy(hmd);
		ovr_Shutdown();
	}

	ovrPoseStatef OVRManager::getPoseOfHMD(){
		// Query the HMD for the current tracking state.
		console << "Getting OR Pose" << std::endl;
		ovrTrackingState tracking_state = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds());
		if (tracking_state.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked))
		{
			ovrPoseStatef pose = tracking_state.HeadPose;
			return pose;
		}
		return ovrPoseStatef();		
	}

	bool OVRManager::checkHealthWarningState(){
		// Health and Safety Warning display state.
		ovrHSWDisplayState hswDisplayState;
		ovrHmd_GetHSWDisplayState(hmd, &hswDisplayState);
		// Dismiss the warning if the user pressed the appropriate key or if the user
		// is tapping the side of the hmd.
		if (hswDisplayState.Displayed)
		{
			// If the user has requested to dismiss the warning via keyboard or controller input...

			//TODO: add key presses to turn off health and safety warning
			// if (Util_GetAndResetHSWDismissedState()){
			// 	ovrHmd_DismissHSWDisplay(hmd);
			// 	return false;
			// }
			// else
		
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
					return false;
				}
			}
			return true;
		}
		return false;
	}

	void OVRManager::configureRenderSettings(HWND window, HDC hdc){
		//Configure Stereo settings.
		OVR::Sizei recommenedTex0Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, hmd->DefaultEyeFov[0], 1.0f);
		OVR::Sizei recommenedTex1Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, hmd->DefaultEyeFov[1], 1.0f);
		
		renderTargetSize.w = recommenedTex0Size.w + recommenedTex1Size.w;
		renderTargetSize.h = (int(recommenedTex0Size.h)>int(recommenedTex1Size.h)) ? int(recommenedTex0Size.h) : int(recommenedTex1Size.h);

		const int eyeRenderMultisample = 1;
		// pRendertargetTexture = pRender->CreateTexture(Texture_RGBA | Texture_RenderTarget | eyeRenderMultisample, renderTargetSize.w, renderTargetSize.h, NULL);
		createRenderTexture(renderTargetSize.w, renderTargetSize.h, eyeRenderMultisample);
		//The actual RT size may be different due to HW limits.
		//TODO
		//renderTargetSize.w = pRendertargetTexture->GetWidth();
		//renderTargetSize.h = pRendertargetTexture->GetHeight();

		// Configure OpenGL.
		ovrGLConfig cfg;

		const int backBufferMultisample = 1;
				
		cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
		cfg.OGL.Header.BackBufferSize = OVR::Sizei(hmd->Resolution.w, hmd->Resolution.h);
		cfg.OGL.Header.Multisample = backBufferMultisample;
		cfg.OGL.Window = window;
		cfg.OGL.DC = hdc;
		ovrBool result = ovrHmd_ConfigureRendering(hmd, &cfg.Config, hmd->DistortionCaps, hmd->DefaultEyeFov, EyeRenderDesc);
	}

	void OVRManager::createRenderTexture(int width, int height, int samples){
		//One texture for both eyes
		glGenTextures(1, &oculusRiftTextureID);
   		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, oculusRiftTextureID);
   		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, oculusRiftTextureID, 0);

  //  		//Viewports info
		eyeViewports[ovrEye_Left].Pos = OVR::Vector2i(0, 0);
		eyeViewports[ovrEye_Left].Size = OVR::Sizei(renderTargetSize.w / 2, renderTargetSize.h); //TODO refactor
		eyeViewports[ovrEye_Right].Pos = OVR::Vector2i((renderTargetSize.w + 1) / 2, 0);
		eyeViewports[ovrEye_Right].Size = eyeViewports[ovrEye_Left].Size;

		//Render texture info for rendering and then distortion
		eyeTextures[ovrEye_Left].OGL.Header.API = ovrRenderAPI_OpenGL;
		eyeTextures[ovrEye_Left].OGL.Header.RenderViewport = eyeViewports[ovrEye_Left];
		eyeTextures[ovrEye_Left].OGL.Header.TextureSize = renderTargetSize;
		eyeTextures[ovrEye_Left].OGL.TexId = oculusRiftTextureID;
		eyeTextures[ovrEye_Left].Texture.Header.API = ovrRenderAPI_OpenGL;
		eyeTextures[ovrEye_Left].Texture.Header.RenderViewport = eyeViewports[ovrEye_Left];
		eyeTextures[ovrEye_Left].Texture.Header.TextureSize = renderTargetSize;

		eyeTextures[ovrEye_Right] = eyeTextures[ovrEye_Left];
		eyeTextures[ovrEye_Right].OGL.Header.RenderViewport = eyeViewports[ovrEye_Right];
		eyeTextures[ovrEye_Right].Texture.Header.RenderViewport = eyeViewports[ovrEye_Right];
	}

	void OVRManager::getHMDInfo(H3D::StereoInfo* info){
		// info->interocularDistance->setValue(2 * std::fabs(ovrEyeRenderDesc::HmdToEyeViewOffset[0]));
		//TODO load in correct IPD as above
		info->interocularDistance->setValue(0.062); 
		info->focalDistance->setValue(99999999999999);
		ovrQuatf orientation = getPoseOfHMD().ThePose.Orientation;

		Quaternion q = Quaternion(orientation.x,orientation.y,orientation.z,orientation.w);

		info->headTilt->setValue(Rotation(q));
	}

	void OVRManager::startFrame(){
		// ovrFrameTiming hmdFrameTiming =
		ovrHmd_BeginFrame(hmd, 0);
	}

	void OVRManager::endFrame(){
		ovrHmd_EndFrame(hmd, headPoses, &eyeTextures[0].Texture); //must convert to opengl texture pointer; hence &..[0].Texture
	}

	void OVRManager::setProjectionMatrix(X3DViewpointNode::EyeMode eye_mode){
		//TODO: customize eye render order
		ovrEyeType eye = H3DEyeModeToOVREyeType(eye_mode);
		float near_distance = 0.001f;
		float far_distance = 100.0f;
		OVR::Matrix4f proj = OVR::Matrix4f(ovrMatrix4f_Projection(EyeRenderDesc[eye].Fov, near_distance, far_distance, true));
		glMultMatrixf(getColumnMajorRepresentation(proj));
	}

	void OVRManager::setViewMatrix(X3DViewpointNode::EyeMode eye_mode){
		ovrEyeType eye = H3DEyeModeToOVREyeType(eye_mode);
		headPoses[eye] = ovrHmd_GetHmdPosePerEye(hmd, eye);
		OVR::Quatf orientation = OVR::Quatf(headPoses[eye].Orientation);
		OVR::Matrix4f view = OVR::Matrix4f(orientation.Inverted()) * OVR::Matrix4f::Translation(-headPoses[eye].Position.x,-headPoses[eye].Position.y,-headPoses[eye].Position.z);
		glMultMatrixf(getColumnMajorRepresentation(OVR::Matrix4f::Translation(EyeRenderDesc[eye].HmdToEyeViewOffset) * view));
	}

	void OVRManager::drawBuffer(H3D::X3DViewpointNode::EyeMode eye_mode){
		glBindFramebuffer(GL_FRAMEBUFFER, oculusRiftTextureID);
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

	std::string OVRManager::getString(OVR::Matrix4f m){
	   	std::stringstream s;
	   	for (int i = 0; i < 4 ;i ++){
	   		for (int j = 0; j < 4 ;j ++){
	   			s << m.M[j][i];
	   		}
	   		s << std::endl;
	   	}
		return s.str();
	} 

	std::string OVRManager::getConsoletext(){
		std::string s = console.str();
		console.str(std::string()); //Clear stream
		return s;
	}


}
