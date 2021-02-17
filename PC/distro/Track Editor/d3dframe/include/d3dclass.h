#ifndef D3DCLASS_H
#define D3DCLASS_H
/*
**-----------------------------------------------------------------------------
** Name:    D3DClass.h
** Purpose: Various D3D support classes
** Notes:
**
** Copyright (c) 1995 - 1997 by Microsoft, all rights reserved
**-----------------------------------------------------------------------------
*/

#define D3D_OVERLOADS
#include <d3d.h>
#include <d3dtypes.h>

extern const float pi;

/*
**-----------------------------------------------------------------------------
** Classes
**-----------------------------------------------------------------------------
*/

/*
**-----------------------------------------------------------------------------
**	Class:		Light
**	Purpose:	
**-----------------------------------------------------------------------------
*/

class Light {
protected:
public:
	D3DLIGHT2			light;		// structure defining the light
	LPDIRECT3DLIGHT		lpD3DLight;	// object pointer for the light
	int					changed;
public:
	Light(LPDIRECT3D3 lpD3D);
	~Light();

	HRESULT		AddToViewport(LPDIRECT3DVIEWPORT3 lpView);
	HRESULT		RemoveFromViewport(LPDIRECT3DVIEWPORT3 lpView);
	HRESULT		Set(void);

	void		SetColor(const D3DVECTOR& color);
	D3DVECTOR	GetColor(void) const;
	void		SetPosition(const D3DVECTOR& position);
	D3DVECTOR	GetPosition(void) const;
	void		SetDirection(const D3DVECTOR& direction);
	D3DVECTOR	GetDirection(void) const;
	void		SetAttenuation(const D3DVECTOR& attenuation);
	D3DVECTOR	GetAttenuation(void) const;
	void		SetRange(const float range);
	float		GetRange(void) const;
	void		SetFalloff(const float falloff);
	float		GetFalloff(void) const;
	void		SetUmbra(const float falloff);
	float		GetUmbra(void) const;
	void		SetPenumbra(const float falloff);
	float		GetPenumbra(void) const;
	void		SetFlags(const DWORD flags);
	DWORD		GetFlags(void) const;
}; // End Light


/*
**-----------------------------------------------------------------------------
**	Class:		PointLight
**	Purpose:	
**-----------------------------------------------------------------------------
*/

class PointLight : public Light {
	public:
		PointLight(LPDIRECT3D3 lpD3D, 
					const D3DVECTOR & color, 
					const D3DVECTOR & position);
}; // End PointLight


/*
**-----------------------------------------------------------------------------
**	Class:		SpotLight
**	Purpose:	
**-----------------------------------------------------------------------------
*/

class SpotLight : public Light {
	public:
		SpotLight(LPDIRECT3D3 lpD3D, 
				const D3DVECTOR& color, 
				const D3DVECTOR& position, 
				const D3DVECTOR& direction, 
				const float umbra_angle, 
				const float penumbra_angle);
}; // End SpotLight



/*
**-----------------------------------------------------------------------------
**	Class:		DirectionalLight
**	Purpose:	
**-----------------------------------------------------------------------------
*/

class DirectionalLight : public Light {
	public:
		DirectionalLight(LPDIRECT3D3 lpD3D, 
			const D3DVECTOR& color, const D3DVECTOR& direction);
};



/*
**-----------------------------------------------------------------------------
**	Class:		ParallelPointLight
**	Purpose:	
**-----------------------------------------------------------------------------
*/

class ParallelPointLight : public Light {
	public:
		ParallelPointLight(LPDIRECT3D3 lpD3D, const D3DVECTOR& color, const D3DVECTOR& position);
}; // End ParallelPointLight



/*
**-----------------------------------------------------------------------------
**	Class:		Material
**	Purpose:	
**-----------------------------------------------------------------------------
*/

class Material 
{
protected:
	D3DMATERIAL			Mat;
    D3DMATERIALHANDLE	hMat;
	LPDIRECT3DMATERIAL3 lpMat;
	int					changed;
public:
	Material(LPDIRECT3D3 lpD3D, LPDIRECT3DDEVICE3 lpDev);
	~Material();

	HRESULT		SetAsCurrent(LPDIRECT3DDEVICE3 lpDev);
	HRESULT		SetAsBackground(LPDIRECT3DVIEWPORT3 lpView);
	HRESULT		Set(void);

	void		SetDiffuse(const D3DVECTOR& color);
	D3DVECTOR	GetDiffuse(void) const;
	void		SetAlpha(const float& alpha);
	float		GetAlpha(void) const;
	void		SetAmbient(const D3DVECTOR& color);
	D3DVECTOR	GetAmbient(void) const;
	void		SetEmissive(const D3DVECTOR& color);
	D3DVECTOR	GetEmissive(void) const;
	void		SetSpecular(const D3DVECTOR& color);
	D3DVECTOR	GetSpecular(void) const;
	void		SetPower(const D3DVALUE& power);
	D3DVALUE	GetPower(void) const;
	void		SetRampSize(const DWORD& ramp);
	DWORD		GetRampSize(void) const;
	void		SetTextureHandle(const D3DTEXTUREHANDLE& hTexture);
	D3DTEXTUREHANDLE	GetTextureHandle(void);
}; // End Material


//
// include the inline functions for the classes
//
#include "d3dclass.inl"


/*
**-----------------------------------------------------------------------------
**	End of File
**-----------------------------------------------------------------------------
*/
#endif // D3DCLASS_H


