
void DrawGraph( CAR *car );
void ControlCar( CAR *car);
void UpdateCarMotion( CAR *car );
void CircularMotion( CAR *car );
void BounceAgainstPlane( float *Approach, float *Plane, float *Exit );
void BounceAgainstPlane2( float *Approach, float *Plane, float *Exit );
void PhysicsCollision( CAR *car, float *Offset);
void CollisionMain( CAR *car );
float GetScalar( float *Matrix, float *In );
void WholeCollision( CAR *car );
void GoodCollision( CAR *car );

#define WORLDMETER 200

#define IDEALFRAMERATE 72

#define GRAVITY ((float)(9.81f*WORLDMETER)/IDEALFRAMERATE)

#define DRIVEWHEELRADIUS 10.0f

#define DRIVEWHEELCIRC ((float)2*3.14159f*DRIVEWHEELRADIUS)

#define MAXREVS 12000

#define GEARRATIO ((float)1/6.328f)

#define METERSPERMILE 1609.344f


#define COEFFOFFRICTION 1.0f

#define CARMASS 2.5f


#define COEFFOFDRAG 0.00038f


#define CARFRONTPOINT 55.895f
#define CARBACKPOINT -54.7f

#define CARLENGTH CARFRONTPOINT-CARBACKPOINT

#define CARFRONTWHEELPOINT 45.3f
#define CARBACKWHEELPOINT -31.35f



