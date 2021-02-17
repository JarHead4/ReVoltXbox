

#ifndef _AD_LIGHT_
#define _AD_LIGHT_

#include <jcclasses.h>
#include <d3dtypes.h>
#include <math.h>


class JCCLASS_DLL AD_LightType: public JC_ListElement
{
	public:
		AD_LightType(void);
		AD_LightType(const AD_LightType& source);
		~AD_LightType();
		
		virtual void Extract(JC_Buffer& buf);
		virtual void Insert(JC_Buffer& buf) const;
		virtual void Copy(const JC_ListElement *src_item);
		friend JC_Buffer& operator << (JC_Buffer& buf, const AD_LightType& light)	{ light.Insert(buf); return buf; }
		friend JC_Buffer& operator >> (JC_Buffer& buf, AD_LightType& light)	{ light.Extract(buf); return buf; }
			
		void SetLightRange(double X, double Y, double Z) { XRange = X; YRange = Y; ZRange = Z; }
		void GetLightRange(double &X, double &Y, double &Z) { X = XRange; Y = YRange; Z = ZRange; }

		void SetPosition(double X, double Y, double Z) { Position.x = (float)X; Position.y = (float)Y; Position.z = (float)Z; }
		void GetPosition(double &X, double &Y, double &Z) { X = Position.x; Y = Position.y; Z = Position.z; }
		void GetPosition(D3DVECTOR &NewPosition) { NewPosition = Position; }

		void SetDirection(double X, double Y, double Z) { Direction.x = (float)X; Direction.y = (float)Y; Direction.z = (float)Z; }
		void GetDirection(double &X, double &Y, double &Z) { X = Direction.x; Y = Direction.y; Z = Direction.z; }
		void GetDirection(D3DVECTOR &NewDirection) { NewDirection = Direction; }

		void SetDirectionFromModel(BOOL FromModel) { ModelForwardDirection = FromModel; }
		BOOL GetDirectionFromModel(void) { return ModelForwardDirection; }			

		void SetColour(U8 R, U8 G, U8 B) { Colour.Set(R, G, B); }
		void GetColour(U8 &R, U8 &G, U8 &B) { Colour.Get(R, G, B); }

		void SetIntensity(double NewIntensityPercentage) { Intensity = NewIntensityPercentage / 100; }
		double GetIntensity(void) { return Intensity; }	
			
		void SetDirectionalRadius(double NewDirectionalConeAngle) { DirectionalConeAngle = NewDirectionalConeAngle; }
		double GetDirectionalRadius(void) { return DirectionalConeAngle; }

		void SetSpotConeRadius(double NewSpotConeAngle) 
		{ 					
			SpotConeAngle = NewSpotConeAngle;
		}
		double GetSpotConeRadius(void) 
		{ 					
			return SpotConeAngle; 
		}

		void SetLightScale(double NewLightScale)
		{
			LightScale = NewLightScale;
		}
		double GetLightScale(void) { return LightScale; }

	public:		
		U8 LightType;

		BOOL ShowLight;
        BOOL RayIntersectionFlag;
		

	private:	
		U16 Version;
		double XRange;	
		double YRange;
		double ZRange;
					
		D3DVECTOR Position;						
		D3DVECTOR Direction;		
		BOOL ModelForwardDirection;

		double DirectionalConeAngle;
		double SpotConeAngle;

		JC_Colour Colour;
		double Intensity;
		
		double LightScale;
};
 
#endif
