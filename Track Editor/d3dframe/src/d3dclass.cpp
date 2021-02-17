/*
**-----------------------------------------------------------------------------
** Name:    D3DClass.cpp
** Purpose: Various D3D support classes
** Notes:
**
** Copyright (c) 1995 - 1997 by Microsoft, all rights reserved.
**-----------------------------------------------------------------------------
*/

// Note:  Must Define D3D_OVERLOADS to get C++ version of D3DMATRIX
#define D3D_OVERLOADS
#include <float.h>
#include <math.h>

#include "D3DClass.h"

const float pi = 3.141592654f;

/*
	Class Methods
*/

/*
**-----------------------------------------------------------------------------
**  Name:       Light::Light
**  Purpose:	Constructor
**-----------------------------------------------------------------------------
*/

Light::Light(LPDIRECT3D3 lpD3D)
{
    memset(&light, 0, sizeof(D3DLIGHT2));
	light.dwSize = sizeof(D3DLIGHT2);
    lpD3D->CreateLight(&lpD3DLight, NULL);
	changed = 1;

	// default to no attenuation with distance
    light.dvAttenuation0 = 1.0f;
    light.dvAttenuation1 = 0.0f;
    light.dvAttenuation2 = 0.0f;
	// default to no max distance
	light.dvRange = D3DLIGHT_RANGE_MAX;
	// default to linear ramp from inner to outer cone
	light.dvFalloff = 1.0f;
	// default to on
	light.dwFlags = D3DLIGHT_ACTIVE;
} // end of Light::Light

/*
**-----------------------------------------------------------------------------
**  Name:       Light::~Light
**  Purpose:	Destructor
**-----------------------------------------------------------------------------
*/

Light::~Light()
{
	if (lpD3DLight) {
		lpD3DLight->Release();
		lpD3DLight = NULL;
	}
} // end Light::~Light
  
/*
**-----------------------------------------------------------------------------
**  Name:       PointLight::PointLight
**  Purpose:	Constructor
**-----------------------------------------------------------------------------
*/

PointLight::PointLight(LPDIRECT3D3		lpD3D, 
					   const D3DVECTOR&	color, 
					   const D3DVECTOR&	position) : Light(lpD3D)
{
    light.dltType = D3DLIGHT_POINT;
    light.dcvColor.r = color[0];
    light.dcvColor.g = color[1];
    light.dcvColor.b = color[2];
    light.dvPosition.x = position[0];
    light.dvPosition.y = position[1];
    light.dvPosition.z = position[2];

	changed = 1;
} // end PointLight::PointLight
  
/*
**-----------------------------------------------------------------------------
**  Name:       SpotLight::SpotLight
**  Purpose:	Constructor
**-----------------------------------------------------------------------------
*/

SpotLight::SpotLight(LPDIRECT3D3		lpD3D, 
					 const D3DVECTOR&	color, 
					 const D3DVECTOR&	position, 
					 const D3DVECTOR&	direction, 
					 const float		umbra_angle,
					 const float		penumbra_angle) : Light(lpD3D)
{
    light.dltType = D3DLIGHT_SPOT;
    light.dcvColor.r = color[0];
    light.dcvColor.g = color[1];
    light.dcvColor.b = color[2];
    light.dvPosition.x = position[0];
    light.dvPosition.y = position[1];
    light.dvPosition.z = position[2];
    light.dvDirection.x = direction[0];
    light.dvDirection.y = direction[1];
    light.dvDirection.z = direction[2];
	light.dvTheta = umbra_angle;
	light.dvPhi = penumbra_angle;

	changed = 1;
} // end of SpotLight::SpotLight
  
/*
**-----------------------------------------------------------------------------
**  Name:       DirectionalLight::DirectionalLight
**  Purpose:	Constructor
**-----------------------------------------------------------------------------
*/

DirectionalLight::DirectionalLight(LPDIRECT3D3		lpD3D, 
								   const D3DVECTOR&	color, 
								   const D3DVECTOR&	direction) : Light(lpD3D)
{
    light.dltType = D3DLIGHT_DIRECTIONAL;
    light.dcvColor.r = color[0];
    light.dcvColor.g = color[1];
    light.dcvColor.b = color[2];
    light.dvDirection.x = direction[0];
    light.dvDirection.y = direction[1];
    light.dvDirection.z = direction[2];

	changed = 1;
}	// end of DirectionalLight::DirectionalLight

/*
**-----------------------------------------------------------------------------
**  Name:       ParallelLight::ParallelLight
**  Purpose:	Constructor
**-----------------------------------------------------------------------------
*/

ParallelPointLight::ParallelPointLight(LPDIRECT3D3			lpD3D, 
									   const D3DVECTOR&	color, 
									   const D3DVECTOR&	position) : Light(lpD3D)
{
    light.dltType = D3DLIGHT_PARALLELPOINT;
    light.dcvColor.r = color[0];
    light.dcvColor.g = color[1];
    light.dcvColor.b = color[2];
    light.dvPosition.x = position[0];
    light.dvPosition.y = position[1];
    light.dvPosition.z = position[2];

	changed = 1;
} // end of ParallelPointLight::ParallelPointLight constructor

/*
**-----------------------------------------------------------------------------
**  Name:       Material::Material
**  Purpose:	Constructor
**-----------------------------------------------------------------------------
*/

Material::Material(LPDIRECT3D3 lpD3D, LPDIRECT3DDEVICE3 lpDev)
{
    memset(&Mat, 0, sizeof(D3DMATERIAL));
	Mat.dwSize = sizeof(D3DMATERIAL);

	// leave the default material black

    Mat.dwRampSize = 16;
	Mat.diffuse.a = 1.0f;

	lpD3D->CreateMaterial(&lpMat, NULL);
    lpMat->SetMaterial(&Mat);
    lpMat->GetHandle(lpDev, &hMat);

	changed = 1;
	Set();
} // end of Material::Material
  
/*
**-----------------------------------------------------------------------------
**  Name:       Material::~Material
**  Purpose:	Destructor
**-----------------------------------------------------------------------------
*/

Material::~Material()
{
	if (lpMat) {
		lpMat->Release();
		lpMat = NULL;
	}
} // end of Material::~Material
  

/*
**-----------------------------------------------------------------------------
**  end of File
**-----------------------------------------------------------------------------
*/


