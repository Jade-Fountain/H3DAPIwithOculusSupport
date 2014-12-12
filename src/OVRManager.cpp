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
#include "OVR_CAPI_GL.h"

 
namespace H3D {

	using H3DUtil::ArithmeticTypes::Quaternion;
	using H3DUtil::ArithmeticTypes::Rotation;
	using H3D::X3DViewpointNode;


	void OVRManager::initialise(){
		console << "Initialising Oculus Rift...";
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
		
		OVR::Sizei renderTargetSize;
		renderTargetSize.w = recommenedTex0Size.w + recommenedTex1Size.w;
		renderTargetSize.h = std::max(int(recommenedTex0Size.h),int(recommenedTex1Size.h));

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
		ovrBool result = ovrHmd_ConfigureRendering(hmd, &cfg.Config, distortionCaps, eyesFov, EyeRenderDesc);
				
	}

	void OVRManager::createRenderTexture(int width, int height, int samples){
		//TODO
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

	void OVRManager::setProjectionMatrix(X3DViewpointNode::EyeMode eye_mode){
		//TODO: customize eye render order
		ovrEyeType eye = H3DEyeModeToOVREyeType(eye_mode);
		ovrMatrix4f proj = ovrMatrix4f_Projection(EyeRenderDesc[eye].Fov, 0.01f, 10000.0f, true);
		// * Test code *
		// Assign quaternion result directly to view (translation is ignored).
		pRender->SetProjection(proj);
	}

	void OVRManager::setViewMatrix(X3DViewpointNode::EyeMode eye_mode){
		ovrEyeType eye = H3DEyeModeToOVREyeType(eye_mode);
		ovrPosef headPose = ovrHmd_GetHmdPosePerEye(hmd, eye);
		OVR::Quatf orientation = OVR::Quatf(headPose.Orientation);
		OVR::Matrix4f view = OVR::Matrix4f(orientation.Inverted()) * OVR::Matrix4f::Translation(-headPose.Position.x,-headPose.Position.y,-headPose.Position.z);
		glMultMatrixf(getColumnMajorRepresentation(OVR::Matrix4f::Translation(EyeRenderDesc[eye].HmdToEyeViewOffset) * view));
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
