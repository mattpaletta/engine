#pragma once

#include <memory>
#include <set>
#include <Eigen/Core>
#include <glad/glad.h>

#include <constants/shader.hpp>

#include "openvr.h"

#include "frame_buffer_desc.hpp"
#include "vertex_data_window.hpp"

// See: https://github.com/ValveSoftware/openvr/blob/master/samples/hellovr_opengl/hellovr_opengl_main.cpp

class VRApplication {
private:
	// Projection matrices, setup in constructor
	Eigen::Matrix4f leftProjection;
	Eigen::Matrix4f rightProjection;
	Eigen::Matrix4f leftEyePos;
	Eigen::Matrix4f rightEyePos;

	// Render
	unsigned int renderWidth; // These are kept as unsigned int, because of the VR library support
	unsigned int renderHeight;
	FramebufferDesc leftFrameBufferDesc;
	FramebufferDesc rightFrameBufferDesc;
	GLuint companionWindowVAO;
	GLuint companionWindowIDVertBuffer;
	GLuint companionWindowIDIndexBuffer;
	std::size_t companionWindowIndexSize;
	FramebufferDesc CreateEyeFrameBuffer(int width, int height);
	void CreateCompanionWindow();

	vr::IVRSystem* vr_pointer;

	// Shutdown
	bool has_shutdown = false;
	bool should_shutdown = false;

	// Controller Device ID's
	vr::TrackedDeviceIndex_t left;
	vr::TrackedDeviceIndex_t right;
	std::set<vr::TrackedDeviceIndex_t> trackers;

	void FindController(const vr::VREvent_t& event, const bool did_find);
	void FindTracker(const vr::VREvent_t& event, const bool did_find);
	void HandleButtonEvent(const vr::VREvent_t& event);

	vr::HmdVector3_t GetPosition(const vr::HmdMatrix34_t& matrix);
	vr::HmdQuaternion_t GetRotation(const vr::HmdMatrix34_t& matrix);
	vr::HmdVector3_t ProcessRotation(const vr::HmdMatrix34_t& matrix);

	// Utility Functions
	std::string getEnglishTrackingResultForPose(const vr::TrackedDevicePose_t& pose);
	std::string getEnglishPoseValidity(const vr::TrackedDevicePose_t& pose);
	std::string getPoseXYZString(const vr::TrackedDevicePose_t& pose);

	Eigen::Matrix4f GetHMDMatrixProjection(const vr::Hmd_Eye& nEye, const float nearClip = 0.1f, const float farClip = 30.0f);
	Eigen::Matrix4f GetHMDMatrixPoseEye(const vr::Hmd_Eye& nEye);

public:
	VRApplication() = default;
	~VRApplication();

	void Init();
	void Shutdown();
	bool ShouldShutdown();

	void GetInput();
	void GetTrackingPose();

	// Should be in range: [0 - 3999]
	void RunVibration(const unsigned short leftStrength, const unsigned short rightStrength);
};
