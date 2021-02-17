	 
char CameraLook( float *CameraPos, float *LookAt, float *FMatrix);
void FRotation( float  X, float Y, float Z, float *FMatrix, float *result );
void FInvRotation( float  X, float  Y, float Z, float *FMatrix, float *result );
void FBuildMatrix( float x, float  y, float z, float *FMatrix );
void FMakeMatrixObjectCamera(float *cam, float *FMatrix, float *result);
void HomeCamera(float *cpos, float *cmatrix, float *opos, float *omatrix);
void CarCamera(float *cpos, float *cmatrix, float *opos, float *omatrix);
char SillyGame(void);
char UpdateGame(void);
void MakeLightSource( float *LightPos, float *LookAt, float *FMatrix, float *Dir );
