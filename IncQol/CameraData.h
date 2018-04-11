#ifndef CAMERA_DATA_H
#define CAMERA_DATA_H

#include <d3dx9.h>
class CameraData {
public:
	D3DXVECTOR3 position;
	D3DXVECTOR3 viewVec;
	D3DXVECTOR3 upVec;
	D3DXVECTOR3 lookAt;
	float fovy;
};
#endif