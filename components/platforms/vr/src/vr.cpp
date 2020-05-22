#include "vr.hpp"

#include <Eigen/LU>

#include <iostream>
#include <sstream>
#include <cassert>

namespace {
	template <typename T>
	std::string to_string_with_precision(const T a_value, const int n = 3) {
		std::ostringstream out;
		out.precision(n);
		out << std::fixed << a_value;
		return out.str();
	}
}

void VRApplication::Init() {
	vr::EVRInitError eError = vr::VRInitError_None;
	this->vr_pointer = vr::VR_Init(&eError, vr::VRApplication_Scene);
	if (eError != vr::VRInitError_None) {
		this->vr_pointer = nullptr;
		std::cerr << "ERROR: Unable to start VR runtime" << VR_GetVRInitErrorAsEnglishDescription(eError) << std::endl;
		std::terminate();
	} else {
#if ENGINE_DEBUG_VR
		std::cout << "Successfully started VR Runtime" << std::endl;
#endif
	}

	// Setup Projection & Pose Matrices
	this->leftProjection = this->GetHMDMatrixProjection(vr::Eye_Left);
	this->rightProjection = this->GetHMDMatrixProjection(vr::Eye_Right);
	this->leftEyePos = this->GetHMDMatrixPoseEye(vr::Eye_Left);
	this->rightEyePos = this->GetHMDMatrixPoseEye(vr::Eye_Right);

	// Setup render targets
	this->vr_pointer->GetRecommendedRenderTargetSize(&this->renderWidth, &this->renderHeight);

	// Setup frame buffers
	this->leftFrameBufferDesc = this->CreateEyeFrameBuffer(this->renderWidth, this->renderHeight);
	this->rightFrameBufferDesc = this->CreateEyeFrameBuffer(this->renderWidth, this->renderHeight);

	// Setup Companion window
	this->CreateCompanionWindow();

	// Create VR Compositor
	vr::EVRInitError peError = vr::VRInitError_None;
	if (!vr::VRCompositor()) {
		std::cerr << "Compositor initialization failed" << std::endl;
		std::terminate();
	}
}

FramebufferDesc VRApplication::CreateEyeFrameBuffer(unsigned int width, unsigned int height) {
	FramebufferDesc frameBuffer;
	glGenFramebuffers(1, &frameBuffer.renderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.renderFramebufferId);

	glGenRenderbuffers(1, &frameBuffer.depthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer.depthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frameBuffer.depthBufferId);

	glGenTextures(1, &frameBuffer.renderTextureId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, frameBuffer.renderTextureId);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, width, height, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, frameBuffer.renderTextureId, 0);

	glGenFramebuffers(1, &frameBuffer.resolveFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer.resolveFramebufferId);

	glGenTextures(1, &frameBuffer.resolveTextureId);
	glBindTexture(GL_TEXTURE_2D, frameBuffer.resolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBuffer.resolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR: Failed to create VR framebuffer" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return frameBuffer;
}

void VRApplication::CreateCompanionWindow() {
	if (!this->vr_pointer) {
		return;
	}
	
	std::vector<VertexDataWindow> vVerts;

	// left eye verts
	vVerts.emplace_back(Position2d(-1, -1), glm::vec2(0, 1));
	vVerts.emplace_back(Position2d( 0, -1), glm::vec2(1, 1));
	vVerts.emplace_back(Position2d(-1,  1), glm::vec2(0, 0));
	vVerts.emplace_back(Position2d( 0,  1), glm::vec2(1, 0));

	// right eye verts
	vVerts.emplace_back(Position2d(0, -1), glm::vec2(0, 1));
	vVerts.emplace_back(Position2d(1, -1), glm::vec2(1, 1));
	vVerts.emplace_back(Position2d(0,  1), glm::vec2(0, 0));
	vVerts.emplace_back(Position2d(1,  1), glm::vec2(1, 0));

	const GLushort vIndices[] = { 0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6 };
	this->companionWindowIndexSize = _countof(vIndices);

	glGenVertexArrays(1, &this->companionWindowVAO);
	glBindVertexArray(this->companionWindowVAO);

	glGenBuffers(1, &this->companionWindowIDVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, this->companionWindowIDVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, vVerts.size() * sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW);

	glGenBuffers(1, &this->companionWindowIDIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->companionWindowIDIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->companionWindowIndexSize * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void*) offsetof(VertexDataWindow, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void*) offsetof(VertexDataWindow, texCoord));

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Eigen::Matrix4f VRApplication::GetHMDMatrixPoseEye(const vr::Hmd_Eye& nEye) {
	if (!this->vr_pointer) {
		return Eigen::Matrix4f();
	}

	vr::HmdMatrix34_t matEyeRight = this->vr_pointer->GetEyeToHeadTransform(nEye);
	Eigen::Matrix<float, 4, 4> matrixObj;
	matrixObj <<
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f;

	Eigen::Matrix<float, 4, 4> inverse;
	bool invertible;

	matrixObj.computeInverseWithCheck(inverse, invertible);
	if (invertible) {
		return inverse;
	} else {
		return Eigen::Matrix4f::Identity();
	}
}

Eigen::Matrix4f VRApplication::GetHMDMatrixProjection(const vr::Hmd_Eye& nEye, const float nearClip, const float farClip) {
	if (!this->vr_pointer) {
		return Eigen::Matrix4f();
	}
	const auto m = this->vr_pointer->GetProjectionMatrix(nEye, nearClip, farClip).m;
	Eigen::Map<Eigen::Matrix4f> map(*m);
	return map;
}


VRApplication::~VRApplication() {
	if (!this->has_shutdown) {
		this->Shutdown();
		this->vr_pointer = nullptr;
	}
}

bool VRApplication::ShouldShutdown() {
	return this->should_shutdown;
}

void VRApplication::Shutdown() {
	if (this->vr_pointer) {
		vr::VR_Shutdown();
		this->has_shutdown = true;
	}
}

void VRApplication::FindTracker(const vr::VREvent_t& event, const bool did_find) {
	vr::ETrackedDeviceClass trackedDeviceClass = this->vr_pointer->GetTrackedDeviceClass(event.trackedDeviceIndex);
	if (trackedDeviceClass != vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker) {
		return; //this is a placeholder, but there isn't a tracker 
				// involved so the rest of the snippet should be skipped
	}

	vr::ETrackedDeviceClass clazz = this->vr_pointer->GetTrackedDeviceClass(event.trackedDeviceIndex);// If we `didn't find it`, we forget about the ID, otherwise update with the new id.

	if (clazz == vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker) {
		// Left hand
		if (did_find) {
			this->trackers.insert(event.trackedDeviceIndex);
		} else {
			this->trackers.erase(event.trackedDeviceIndex);
		}
	}
}

void VRApplication::FindController(const vr::VREvent_t& event, const bool did_find) {
	vr::ETrackedDeviceClass trackedDeviceClass = this->vr_pointer->GetTrackedDeviceClass(event.trackedDeviceIndex);
	if (trackedDeviceClass != vr::ETrackedDeviceClass::TrackedDeviceClass_Controller) {
		return; //this is a placeholder, but there isn't a controller 
				// involved so the rest of the snippet should be skipped
	}

	vr::ETrackedControllerRole role = this->vr_pointer->GetControllerRoleForTrackedDeviceIndex(event.trackedDeviceIndex);
	// If we `didn't find it`, we forget about the ID, otherwise update with the new id.

	if (role == vr::TrackedControllerRole_Invalid) {
		// The controller is probably not visible to a base station.
		// Invalid role comes up more often than you might think.
	} else if (role == vr::TrackedControllerRole_LeftHand) {
		// Left hand
		this->left = did_find ? event.trackedDeviceIndex : 0;
	} else if (role == vr::TrackedControllerRole_RightHand) {
		// Right hand
		this->right = did_find ? event.trackedDeviceIndex : 0;
	}
}


void VRApplication::GetInput() {
	vr::VREvent_t event;
	// This returns false if there is no events
	if (this->vr_pointer->PollNextEvent(&event, sizeof(event))) {
		switch (event.eventType) {
		////////////////////
		// Device
		////////////////////
		case vr::VREvent_TrackedDeviceActivated:
#if ENGINE_DEBUG_VR
			std::cout << "EVENT (OpenVR) Device: " << event.trackedDeviceIndex << " attached" << std::endl;
#endif
			this->FindController(event, true);
			this->FindTracker(event, true);
			break;
		case vr::VREvent_TrackedDeviceDeactivated:
#if ENGINE_DEBUG_VR
			std::cout << "EVENT (OpenVR) Device: " << event.trackedDeviceIndex << " deattached" << std::endl;
#endif
			this->FindController(event, false);
			this->FindTracker(event, false);
			break;
		case vr::VREvent_TrackedDeviceUpdated:
#if ENGINE_DEBUG_VR
			std::cout << "EVENT (OpenVR) Device: " << event.trackedDeviceIndex << " updated" << std::endl;
#endif
			break;
		////////////////////
		// Quit
		///////////////////
		case vr::VREvent_ProcessQuit:
		case vr::VREvent_Quit:
			// The engine can use this on the render loop
			this->should_shutdown = true;
			break;
		////////////////////
		// Buttons
		////////////////////
		case vr::VREvent_ButtonPress:
		case vr::VREvent_ButtonUnpress:
		case vr::VREvent_ButtonTouch:
		case vr::VREvent_ButtonUntouch:
#if ENGINE_DEBUG_VR
			std::cout << "Got Button Event" << std::endl;
#endif
			this->HandleButtonEvent(event);
			break;
		default:
#if ENGINE_DEBUG_VR
			std::cout << "EVENT (OpenVR) Event: " << event.eventType << std::endl;
#endif
			break;
		}
	}
}

void VRApplication::HandleButtonEvent(const vr::VREvent_t& event) {
	switch (event.data.controller.button) {
	case vr::k_EButton_Grip:
		switch (event.eventType) {
		case vr::VREvent_ButtonPress:
			break;
		case vr::VREvent_ButtonUnpress:
			break;
		}
		break;
	case vr::k_EButton_SteamVR_Trigger:
		switch (event.eventType) {
		case vr::VREvent_ButtonPress:
			break;
		case vr::VREvent_ButtonUnpress:
			break;
		}
		break;
	case vr::k_EButton_SteamVR_Touchpad:
		switch (event.eventType) {
		case vr::VREvent_ButtonPress:
			break;
		case vr::VREvent_ButtonUnpress:
			break;
		case vr::VREvent_ButtonTouch:
			break;
		case vr::VREvent_ButtonUntouch:
			break;
		}
		break;
	case vr::k_EButton_ApplicationMenu:
		switch (event.eventType) {
		case vr::VREvent_ButtonPress:
			break;
		case vr::VREvent_ButtonUnpress:
			break;
		}
		break;
	}
}

void VRApplication::RunVibration(const int leftStrength, const int rightStrength) {
	// Should be in range: [0 - 3999]
#if ENGINE_DEBUG_VR
	assert(leftStrength >= 0);
	assert(leftStrength <= 3999);

	assert(rightStrength >= 0);
	assert(rightStrength <= 3999);
#endif
	if (leftStrength + rightStrength > 0) {
		this->vr_pointer->TriggerHapticPulse(this->left, 0, leftStrength);
		this->vr_pointer ->TriggerHapticPulse(this->right, 0, rightStrength);
	}
}

void VRApplication::GetTrackingPose() {
	// Seconds to predict the pose for, 0 for now.
	constexpr float sec_time_from_now = 0.f;
	vr::TrackedDevicePose_t trackedDevicePose;

	// Standing or Seating
	// Get HMD Pose
	vr_pointer->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, sec_time_from_now, &trackedDevicePose, 1);

	auto position = GetPosition(trackedDevicePose.mDeviceToAbsoluteTracking);
	auto rot = GetRotation(trackedDevicePose.mDeviceToAbsoluteTracking);

#if ENGINE_DEBUG_VR
	std::cout << "HMD " << this->getPoseXYZString(trackedDevicePose, 0) << std::endl;
	std::cout << "HMD: " << this->getEnglishTrackingResultForPose(trackedDevicePose) << " " << this->getEnglishPoseValidity(trackedDevicePose) << std::endl;
	std::cout << ("HMD: qw:" + to_string_with_precision(rot.w, 2) +
		" qx:" + to_string_with_precision(rot.x, 2) +
		" qy:" + to_string_with_precision(rot.y, 2) +
		" qz:" + to_string_with_precision(rot.z, 2)) << std::endl;
#endif

	vr::TrackedDevicePose_t LtrackedDevicePose;
	vr::VRControllerState_t LcontrollerState;
	if (this->left != 0) {
		vr_pointer->GetControllerStateWithPose(vr::TrackingUniverseStanding, this->left, &LcontrollerState, sizeof(LcontrollerState), &LtrackedDevicePose);
	}
	
	vr::TrackedDevicePose_t RtrackedDevicePose;
	vr::VRControllerState_t RcontrollerState;
	if (this->right != 0) {
		vr_pointer->GetControllerStateWithPose(vr::TrackingUniverseStanding, this->right, &RcontrollerState, sizeof(RcontrollerState), &RtrackedDevicePose);
	}

	for (const auto& tracker : this->trackers) {
		vr::TrackedDevicePose_t trackerDevicePose;
		vr::VRControllerState_t trackerState;
		this->vr_pointer->GetControllerStateWithPose(vr::TrackingUniverseStanding, tracker, &trackerState, sizeof(trackerState), &trackerDevicePose);
	}
}

vr::HmdVector3_t VRApplication::GetPosition(const vr::HmdMatrix34_t& matrix) {
	vr::HmdVector3_t vector;

	vector.v[0] = matrix.m[0][3];
	vector.v[1] = matrix.m[1][3];
	vector.v[2] = matrix.m[2][3];

	return vector;
}

vr::HmdQuaternion_t VRApplication::GetRotation(const vr::HmdMatrix34_t& matrix) {
	vr::HmdQuaternion_t q;

	q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
	q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
	q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
	return q;
}

vr::HmdVector3_t VRApplication::ProcessRotation(const vr::HmdMatrix34_t& matrix) {
	vr::HmdVector3_t ret;
	ret.v[0] = atan2(matrix.m[1][2], matrix.m[2][2]);
	ret.v[1] = atan2(-matrix.m[0][2],
		sqrt(matrix.m[1][2] * matrix.m[1][2] + matrix.m[2][2] * matrix.m[2][2]));
	ret.v[2] = atan2(matrix.m[0][1], matrix.m[0][0]);

	return ret;
}


std::string VRApplication::getEnglishTrackingResultForPose(const vr::TrackedDevicePose_t& pose) {
	switch (pose.eTrackingResult) {
	case vr::ETrackingResult::TrackingResult_Uninitialized:
		return "Invalid tracking result";
	case vr::ETrackingResult::TrackingResult_Calibrating_InProgress:
		return "Calibrating in progress";
	case vr::ETrackingResult::TrackingResult_Calibrating_OutOfRange:
		return "Calibrating Out of range";
	case vr::ETrackingResult::TrackingResult_Running_OK:
		return "Running OK";
	case vr::ETrackingResult::TrackingResult_Running_OutOfRange:
		return "WARNING: Running Out of Range";
	default:
		return "Default";
	}
}

std::string VRApplication::getEnglishPoseValidity(const vr::TrackedDevicePose_t& pose) {
	return pose.bPoseIsValid ? "Valid" : "Invalid";
}

std::string VRApplication::getPoseXYZString(const vr::TrackedDevicePose_t& pose, int hand) {
	vr::HmdVector3_t pos = this->ProcessRotation(pose.mDeviceToAbsoluteTracking);
	if (pose.bPoseIsValid) {
		return   "x:" + to_string_with_precision(pos.v[0], 3) +
				" y:" + to_string_with_precision(pos.v[1], 3) +
				" z:" + to_string_with_precision(pos.v[2], 3);
	} else {
		return "            INVALID";
	}
}