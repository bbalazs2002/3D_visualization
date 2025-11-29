vec4 CameraViewProj(vec4 pos) {
	return cameraData.viewProj * pos;
}