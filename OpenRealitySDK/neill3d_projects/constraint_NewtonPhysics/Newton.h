//********************************************************************
// Newton Game dynamics 
// copyright 2000-2004
// By Julio Jerez
// VC: 6.0
// One and only header file.
//********************************************************************

#ifndef NEWTON_H__736495_6495_076__INCLUDED_
#define NEWTON_H__736495_6495_076__INCLUDED_

// force all not visual studio platform to be in library form
#ifndef _MSC_VER
	#ifndef _NEWTON_USE_LIB
		#define _NEWTON_USE_LIB
	#endif
#endif

#ifdef _NEWTON_USE_LIB
	#define NEWTON_API
#else
	#ifdef _NEWTON_BUILD_DLL
		#define NEWTON_API __declspec(dllexport)
	#else
		#define NEWTON_API __declspec(dllimport)
	#endif
#endif

#define __USE_DOUBLE_PRECISION__
#ifdef __USE_DOUBLE_PRECISION__
	typedef double dFloat;
#else
	typedef float dFloat;
#endif



#ifdef __cplusplus 
extern "C" {
#endif

	typedef struct NewtonBody{} NewtonBody;
	typedef struct NewtonWorld{} NewtonWorld;
	typedef struct NewtonJoint{} NewtonJoint;
	typedef struct NewtonContact{} NewtonContact;
	typedef struct NewtonMaterial{} NewtonMaterial;
	typedef struct NewtonCollision{} NewtonCollision;

	typedef struct NewtonRagDoll{} NewtonRagDoll;
	typedef struct NewtonRagDollBone{} NewtonRagDollBone;
	
	typedef struct NewtonUserMeshCollisionCollideDescTag
	{
		dFloat m_boxP0[4];						// lower bounding box of intersection query in local space
		dFloat m_boxP1[4];						// upper bounding box of intersection query in local space
		void* m_userData;                       // user data passed to the collision geometry at creation time
		int	  m_faceCount;                      // the application should set here how many polygons intersect the query box
		dFloat* m_vertex;                       // the application should the pointer to the vertex array. 
		int m_vertexStrideInBytes;              // the application should set here the size of each vertex
	 	int* m_userAttribute;                   // the application should set here the pointer to the user data, one for each face
		int* m_faceIndexCount;                  // the application should set here the pointer to the vertex count of each face.
		int* m_faceVertexIndex;                 // the application should set here the pointer index array for each vertex on a face.
		NewtonBody* m_objBody;                  // pointer to the colliding body
		NewtonBody* m_polySoupBody;             // pointer to the rigid body owner of this collision tree 
	} NewtonUserMeshCollisionCollideDesc;

	typedef struct NewtonUserMeshCollisionRayHitDescTag
	{
		dFloat m_p0[4];							// ray origin in collision local space
		dFloat m_p1[4];                         // ray destination in collision local space   
		dFloat m_normalOut[4];					// copy here the normal at the rat intersection
		int m_userIdOut;                        // copy here a user defined id for further feedback  
		void* m_userData;                       // user data passed to the collision geometry at creation time
	} NewtonUserMeshCollisionRayHitDesc;

	typedef struct NewtonHingeSliderUpdateDescTag
	{
		dFloat m_accel;
		dFloat m_minFriction;
		dFloat m_maxFriction;
		dFloat m_timestep;
	} NewtonHingeSliderUpdateDesc;


	// Newton callback functions
	typedef void* (*NewtonAllocMemory) (int sizeInBytes);
	typedef void (*NewtonFreeMemory) (void *ptr, int sizeInBytes);

	typedef void (*NewtonSerialize) (void* serializeHandle, const void* buffer, size_t size);
	typedef void (*NewtonDeserialize) (void* serializeHandle, void* buffer, size_t size);
	

	typedef void (*NewtonUserMeshCollisionCollideCallback) (NewtonUserMeshCollisionCollideDesc* collideDescData);
	typedef dFloat (*NewtonUserMeshCollisionRayHitCallback) (NewtonUserMeshCollisionRayHitDesc* lineDescData);
	typedef void (*NewtonUserMeshCollisionDestroyCallback) (void* descData);
	typedef void (*NewtonTreeCollisionCallback) (const NewtonBody* bodyWithTreeCollision, const NewtonBody* body,
		const dFloat* vertex, int vertexstrideInBytes, 
		int indexCount, const int* indexArray); 

	typedef void (*NewtonBodyDestructor) (const NewtonBody* body);
	typedef void (*NewtonApplyForceAndTorque) (const NewtonBody* body);
	typedef void (*NewtonBodyActivationState) (const NewtonBody* body, unsigned state);
	typedef void (*NewtonSetTransform) (const NewtonBody* body, const dFloat* matrix);
	typedef void (*NewtonSetRagDollTransform) (const NewtonRagDollBone* bone);
	typedef int (*NewtonGetBuoyancyPlane) (const int collisionID, void *context, const dFloat* globalSpaceMatrix, dFloat* globalSpacePlane);

	typedef void (*NewtonVehicleTireUpdate) (const NewtonJoint* vehicle);
	

	typedef dFloat (*NewtonWorldRayFilterCallback)(const NewtonBody* body, const dFloat* hitNormal, int collisionID, void* userData, dFloat intersetParam);
	typedef void (*NewtonBodyLeaveWorld) (const NewtonBody* body);

	typedef int  (*NewtonContactBegin) (const NewtonMaterial* material, const NewtonBody* body0, const NewtonBody* body1);
	typedef int  (*NewtonContactProcess) (const NewtonMaterial* material, const NewtonContact* contact);
	typedef void (*NewtonContactEnd) (const NewtonMaterial* material);


	typedef void (*NewtonBodyIterator) (const NewtonBody* body);
	typedef void (*NewtonCollisionIterator) (const NewtonBody* body, int vertexCount, const dFloat* FaceArray, int faceId);


	typedef void (*NewtonBallCallBack) (const NewtonJoint* ball);
	typedef unsigned (*NewtonHingeCallBack) (const NewtonJoint* hinge, NewtonHingeSliderUpdateDesc* desc);
	typedef unsigned (*NewtonSliderCallBack) (const NewtonJoint* slider, NewtonHingeSliderUpdateDesc* desc);
	typedef unsigned (*NewtonUniversalCallBack) (const NewtonJoint* universal, NewtonHingeSliderUpdateDesc* desc);
	typedef unsigned (*NewtonCorkscrewCallBack) (const NewtonJoint* corkscrew, NewtonHingeSliderUpdateDesc* desc);
	typedef void (*NewtonUserBilateralCallBack) (const NewtonJoint* userJoint);

	typedef void (*NewtonConstraintDestructor) (const NewtonJoint* me);

	// **********************************************************************************************
	//
	// world control functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonWorld* NewtonCreate (NewtonAllocMemory malloc, NewtonFreeMemory mfree);
	NEWTON_API void NewtonDestroy (const NewtonWorld* newtonWorld);
	NEWTON_API void NewtonDestroyAllBodies (const NewtonWorld* newtonWorld);


	NEWTON_API void NewtonUpdate (const NewtonWorld* newtonWorld, dFloat timestep);
	NEWTON_API void NewtonSetPlatformArchitecture (const NewtonWorld* newtonWorld, int mode);
	NEWTON_API void NewtonSetSolverModel (const NewtonWorld* newtonWorld, int model);
	NEWTON_API void NewtonSetFrictionModel (const NewtonWorld* newtonWorld, int model);
	NEWTON_API dFloat NewtonGetTimeStep (const NewtonWorld* newtonWorld);
	NEWTON_API void NewtonSetMinimumFrameRate (const NewtonWorld* newtonWorld, dFloat frameRate);
	NEWTON_API void NewtonSetBodyLeaveWorldEvent (const NewtonWorld* newtonWorld, NewtonBodyLeaveWorld callback); 
	NEWTON_API void NewtonSetWorldSize (const NewtonWorld* newtonWorld, const dFloat* minPoint, const dFloat* maxPoint); 
	
	NEWTON_API void NewtonWorldFreezeBody (const NewtonWorld* newtonWorld, const NewtonBody* body);
	NEWTON_API void NewtonWorldUnfreezeBody (const NewtonWorld* newtonWorld, const NewtonBody* body);

	NEWTON_API void NewtonWorldForEachBodyDo (const NewtonWorld* newtonWorld, NewtonBodyIterator callback);

	NEWTON_API void NewtonWorldSetUserData (const NewtonWorld* newtonWorld, void* userData);
	NEWTON_API void* NewtonWorldGetUserData (const NewtonWorld* newtonWorld);
	NEWTON_API int NewtonWorldGetVersion (const NewtonWorld* newtonWorld);

	NEWTON_API void NewtonWorldRayCast (const NewtonWorld* newtonWorld, const dFloat* p0, const dFloat* p1, NewtonWorldRayFilterCallback filter, void* userData);

	// world utility functions
	// NEWTON_API int NewtonGetBodiesCount();
	// NEWTON_API int NewtonGetActiveBodiesCount();
	// NEWTON_API int NewtonGetActiveConstraintsCount();
	// NEWTON_API dFloat NewtonGetGlobalScale (const NewtonWorld* newtonWorld);

	// **********************************************************************************************
	//
	// Physics Material Section
	//
	// **********************************************************************************************
	NEWTON_API int NewtonMaterialGetDefaultGroupID(const NewtonWorld* newtonWorld);
	NEWTON_API int NewtonMaterialCreateGroupID(const NewtonWorld* newtonWorld);
	NEWTON_API void NewtonMaterialDestroyAllGroupID(const NewtonWorld* newtonWorld);

	

	NEWTON_API void NewtonMaterialSetDefaultSoftness (const NewtonWorld* newtonWorld, int id0, int id1, dFloat value);
	NEWTON_API void NewtonMaterialSetDefaultElasticity (const NewtonWorld* newtonWorld, int id0, int id1, dFloat elasticCoef);
	NEWTON_API void NewtonMaterialSetDefaultCollidable (const NewtonWorld* newtonWorld, int id0, int id1, int state);
	NEWTON_API void NewtonMaterialSetContinuousCollisionMode (const NewtonWorld* newtonWorld, int id0, int id1, int state);
	NEWTON_API void NewtonMaterialSetDefaultFriction (const NewtonWorld* newtonWorld, int id0, int id1, 
		dFloat staticFriction, dFloat kineticFriction);

	NEWTON_API void NewtonMaterialSetCollisionCallback (const NewtonWorld* newtonWorld, int id0, int id1, void* userData,
		NewtonContactBegin begin, NewtonContactProcess process, NewtonContactEnd end);

	NEWTON_API void* NewtonMaterialGetUserData (const NewtonWorld* newtonWorld, int id0, int id1);
		


	// **********************************************************************************************
	//
	// Physics Contact control functions
	//
	// **********************************************************************************************
	NEWTON_API void NewtonMaterialDisableContact (const NewtonMaterial* material);
	NEWTON_API dFloat NewtonMaterialGetCurrentTimestep (const NewtonMaterial* material);
	NEWTON_API void *NewtonMaterialGetMaterialPairUserData (const NewtonMaterial* material);
	NEWTON_API unsigned NewtonMaterialGetContactFaceAttribute (const NewtonMaterial* material);
	NEWTON_API unsigned NewtonMaterialGetBodyCollisionID (const NewtonMaterial* material, const NewtonBody* body);
	NEWTON_API dFloat NewtonMaterialGetContactNormalSpeed (const NewtonMaterial* material, const NewtonContact* contactlHandle);
	NEWTON_API void NewtonMaterialGetContactForce (const NewtonMaterial* material, dFloat* force);
	NEWTON_API void NewtonMaterialGetContactPositionAndNormal (const NewtonMaterial* material, dFloat* posit, dFloat* normal);
	NEWTON_API void NewtonMaterialGetContactTangentDirections (const NewtonMaterial* material, dFloat* dir0, dFloat* dir);
	NEWTON_API dFloat NewtonMaterialGetContactTangentSpeed (const NewtonMaterial* material, const NewtonContact* contactlHandle, int index);
	
	NEWTON_API void NewtonMaterialSetContactSoftness (const NewtonMaterial* material, dFloat softness);
	NEWTON_API void NewtonMaterialSetContactElasticity (const NewtonMaterial* material, dFloat restitution);
	NEWTON_API void NewtonMaterialSetContactFrictionState (const NewtonMaterial* material, int state, int index);
	NEWTON_API void NewtonMaterialSetContactStaticFrictionCoef (const NewtonMaterial* material, dFloat coef, int index);
	NEWTON_API void NewtonMaterialSetContactKineticFrictionCoef (const NewtonMaterial* material, dFloat coef, int index);
	
	NEWTON_API void NewtonMaterialSetContactNormalAcceleration (const NewtonMaterial* material, dFloat accel);
	NEWTON_API void NewtonMaterialSetContactNormalDirection (const NewtonMaterial* material, const dFloat* directionVector);

	NEWTON_API void NewtonMaterialSetContactTangentAcceleration (const NewtonMaterial* material, dFloat accel, int index);
	NEWTON_API void NewtonMaterialContactRotateTangentDirections (const NewtonMaterial* material, const dFloat* directionVector);


	// **********************************************************************************************
	//
	// convex collision primitives creation functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonCollision* NewtonCreateNull (const NewtonWorld* newtonWorld);
	NEWTON_API NewtonCollision* NewtonCreateSphere (const NewtonWorld* newtonWorld, dFloat radiusX, dFloat radiusY, dFloat radiusZ, const dFloat *offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateBox (const NewtonWorld* newtonWorld, dFloat dx, dFloat dy, dFloat dz, const dFloat *offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateCone (const NewtonWorld* newtonWorld, dFloat radius, dFloat height, const dFloat *offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateCapsule (const NewtonWorld* newtonWorld, dFloat radius, dFloat height, const dFloat *offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateCylinder (const NewtonWorld* newtonWorld, dFloat radius, dFloat height, const dFloat *offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateChamferCylinder (const NewtonWorld* newtonWorld, dFloat radius, dFloat height, const dFloat *offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateConvexHull (const NewtonWorld* newtonWorld, int count, dFloat* vertexCloud, int strideInBytes, dFloat *offsetMatrix);
	NEWTON_API NewtonCollision* NewtonCreateConvexHullModifier (const NewtonWorld* newtonWorld, const NewtonCollision* convexHullCollision);
	NEWTON_API void NewtonConvexHullModifierGetMatrix (const NewtonCollision* convexHullCollision, dFloat* matrix);
	NEWTON_API void NewtonConvexHullModifierSetMatrix (const NewtonCollision* convexHullCollision, const dFloat* matrix);


	

	NEWTON_API void NewtonConvexCollisionSetUserID (const NewtonCollision* convexCollision, unsigned id);
	NEWTON_API unsigned  NewtonConvexCollisionGetUserID (const NewtonCollision* convexCollision);

	NEWTON_API dFloat NewtonConvexCollisionCalculateVolume (const NewtonCollision* convexCollision);
	NEWTON_API void NewtonConvexCollisionCalculateInertialMatrix (const NewtonCollision* convexCollision, dFloat* inertia, dFloat* origin);	

	NEWTON_API void NewtonCollisionMakeUnique (const NewtonWorld* newtonWorld, const NewtonCollision* collision);
	NEWTON_API void NewtonReleaseCollision (const NewtonWorld* newtonWorld, const NewtonCollision* collision);
	// **********************************************************************************************
	//
	// complex collision primitives creation functions
	// note: can only be used with static bodies (bodies with infinite mass)
	//
	// **********************************************************************************************
	NEWTON_API NewtonCollision* NewtonCreateCompoundCollision (const NewtonWorld* newtonWorld, int count, 
		NewtonCollision* const collisionPrimitiveArray[]);

	NEWTON_API NewtonCollision* NewtonCreateUserMeshCollision (const NewtonWorld* newtonWorld, const dFloat *minBox, 
		const dFloat *maxBox, void *userData, NewtonUserMeshCollisionCollideCallback collideCallback, 
		NewtonUserMeshCollisionRayHitCallback rayHitCallback, NewtonUserMeshCollisionDestroyCallback destroyCallback);

	// **********************************************************************************************
	//
	// CollisionTree Utility functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonCollision* NewtonCreateTreeCollision (const NewtonWorld* newtonWorld, NewtonTreeCollisionCallback userCallback);
	NEWTON_API void NewtonTreeCollisionBeginBuild (const NewtonCollision* treeCollision);
	NEWTON_API void NewtonTreeCollisionAddFace (const NewtonCollision* treeCollision, int vertexCount, const dFloat* vertexPtr, 
		int strideInBytes, int faceAttribute);
	NEWTON_API void NewtonTreeCollisionEndBuild (const NewtonCollision* treeCollision, int optimize);

	NEWTON_API void NewtonTreeCollisionSerialize (const NewtonCollision* treeCollision, NewtonSerialize serializeFunction, 
		void* serializeHandle);

	NEWTON_API NewtonCollision* NewtonCreateTreeCollisionFromSerialization (const NewtonWorld* newtonWorld, 
		NewtonTreeCollisionCallback userCallback, NewtonDeserialize deserializeFunction, void* serializeHandle);

	NEWTON_API int NewtonTreeCollisionGetFaceAtribute (const NewtonCollision* treeCollision, const int* faceIndexArray); 
	NEWTON_API void NewtonTreeCollisionSetFaceAtribute (const NewtonCollision* treeCollision, const int* faceIndexArray, 
		int attribute); 

	
	// **********************************************************************************************
	//
	// General purpose collision library functions
	//
	// **********************************************************************************************
	NEWTON_API int NewtonCollisionPointDistance (const NewtonWorld* newtonWorld, const float *point,
		const NewtonCollision* collsion, const dFloat* matrix, dFloat* contact, dFloat* normal);

	NEWTON_API int NewtonCollisionClosestPoint (const NewtonWorld* newtonWorld, 
		const NewtonCollision* collsionA, const dFloat* matrixA, const NewtonCollision* collsionB, const dFloat* matrixB,
		dFloat* contactA, dFloat* contactB, dFloat* normalAB);
	
	NEWTON_API int NewtonCollisionCollide (const NewtonWorld* newtonWorld, int maxSize,
		const NewtonCollision* collsionA, const dFloat* matrixA, const NewtonCollision* collsionB, const dFloat* matrixB,
		dFloat* contacts, dFloat* normals, dFloat* penetration);

	NEWTON_API int NewtonCollisionCollideContinue (const NewtonWorld* newtonWorld, int maxSize, const dFloat timestap, 
		const NewtonCollision* collsionA, const dFloat* matrixA, const dFloat* velocA, const dFloat* omegaA, 
		const NewtonCollision* collsionB, const dFloat* matrixB, const dFloat* velocB, const dFloat* omegaB, 
		dFloat* timeOfImpact, dFloat* contacts, dFloat* normals, dFloat* penetration);

	NEWTON_API dFloat NewtonCollisionRayCast (const NewtonCollision* collision, const dFloat* p0, const dFloat* p1, dFloat* normals, int* attribute);
	NEWTON_API void NewtonCollisionCalculateAABB (const NewtonCollision* collision, const dFloat *matrix, dFloat* p0, dFloat* p1);
	

	// **********************************************************************************************
	//
	// transforms utility functions
	//
	// **********************************************************************************************
	NEWTON_API void NewtonGetEulerAngle (const dFloat* matrix, dFloat* eulersAngles);
	NEWTON_API void NewtonSetEulerAngle (const dFloat* eulersAngles, dFloat* matrix);


	// **********************************************************************************************
	//
	// body manipulation functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonBody* NewtonCreateBody (const NewtonWorld* newtonWorld, const NewtonCollision* collision);
	NEWTON_API void  NewtonDestroyBody(const NewtonWorld* newtonWorld, const NewtonBody* body);

	NEWTON_API void  NewtonBodyAddForce (const NewtonBody* body, const dFloat* force);
	NEWTON_API void  NewtonBodyAddTorque (const NewtonBody* body, const dFloat* torque);

	NEWTON_API void  NewtonBodySetMatrix (const NewtonBody* body, const dFloat* matrix);
	NEWTON_API void  NewtonBodySetMatrixRecursive (const NewtonBody* body, const dFloat* matrix);
	NEWTON_API void  NewtonBodySetMassMatrix (const NewtonBody* body, dFloat mass, dFloat Ixx, dFloat Iyy, dFloat Izz);
	NEWTON_API void  NewtonBodySetMaterialGroupID (const NewtonBody* body, int id);
	NEWTON_API void  NewtonBodySetContinuousCollisionMode (const NewtonBody* body, unsigned state);
	NEWTON_API void  NewtonBodySetJointRecursiveCollision (const NewtonBody* body, unsigned state);
	NEWTON_API void  NewtonBodySetOmega (const NewtonBody* body, const dFloat* omega);
	NEWTON_API void  NewtonBodySetVelocity (const NewtonBody* body, const dFloat* velocity);
	NEWTON_API void  NewtonBodySetForce (const NewtonBody* body, const dFloat* force);
	NEWTON_API void  NewtonBodySetTorque (const NewtonBody* body, const dFloat* torque);

	NEWTON_API void  NewtonBodySetCentreOfMass  (const NewtonBody* body, const dFloat* com);
	NEWTON_API void  NewtonBodySetLinearDamping (const NewtonBody* body, dFloat linearDamp);
	NEWTON_API void  NewtonBodySetAngularDamping (const NewtonBody* body, const dFloat* angularDamp);
	NEWTON_API void  NewtonBodySetUserData (const NewtonBody* body, void* userData);
	NEWTON_API void  NewtonBodyCoriolisForcesMode (const NewtonBody* body, int mode);
	NEWTON_API void  NewtonBodySetCollision (const NewtonBody* body, const NewtonCollision* collision);
	NEWTON_API void  NewtonBodySetAutoFreeze (const NewtonBody* body, int state);
	NEWTON_API void  NewtonBodySetFreezeTreshold (const NewtonBody* body, dFloat freezeSpeed2, dFloat freezeOmega2, int framesCount);
	
	NEWTON_API void  NewtonBodySetTransformCallback (const NewtonBody* body, NewtonSetTransform callback);
	NEWTON_API void  NewtonBodySetDestructorCallback (const NewtonBody* body, NewtonBodyDestructor callback);
	NEWTON_API void  NewtonBodySetAutoactiveCallback (const NewtonBody* body, NewtonBodyActivationState callback);
	NEWTON_API void  NewtonBodySetForceAndTorqueCallback (const NewtonBody* body, NewtonApplyForceAndTorque callback);
	NEWTON_API NewtonApplyForceAndTorque NewtonBodyGetForceAndTorqueCallback (const NewtonBody* body);
	
	NEWTON_API void* NewtonBodyGetUserData (const NewtonBody* body);
	NEWTON_API NewtonWorld* NewtonBodyGetWorld (const NewtonBody* body);
	NEWTON_API NewtonCollision* NewtonBodyGetCollision (const NewtonBody* body);
	NEWTON_API int   NewtonBodyGetMaterialGroupID (const NewtonBody* body);
	NEWTON_API int   NewtonBodyGetContinuousCollisionMode (const NewtonBody* body);
	NEWTON_API int   NewtonBodyGetJointRecursiveCollision (const NewtonBody* body);

	NEWTON_API void  NewtonBodyGetMatrix(const NewtonBody* body, dFloat* matrix);
	NEWTON_API void  NewtonBodyGetMassMatrix (const NewtonBody* body, dFloat* mass, dFloat* Ixx, dFloat* Iyy, dFloat* Izz);
	NEWTON_API void  NewtonBodyGetInvMass(const NewtonBody* body, dFloat* invMass, dFloat* invIxx, dFloat* invIyy, dFloat* invIzz);
	NEWTON_API void  NewtonBodyGetOmega(const NewtonBody* body, dFloat* vector);
	NEWTON_API void  NewtonBodyGetVelocity(const NewtonBody* body, dFloat* vector);
	NEWTON_API void  NewtonBodyGetForce(const NewtonBody* body, dFloat* vector);
	NEWTON_API void  NewtonBodyGetTorque(const NewtonBody* body, dFloat* vector);
	NEWTON_API void  NewtonBodyGetCentreOfMass (const NewtonBody* body, dFloat* com);

	NEWTON_API int   NewtonBodyGetSleepingState(const NewtonBody* body);
	NEWTON_API int   NewtonBodyGetAutoFreeze(const NewtonBody* body);
	NEWTON_API dFloat NewtonBodyGetLinearDamping (const NewtonBody* body);
	NEWTON_API void  NewtonBodyGetAngularDamping (const NewtonBody* body, dFloat* vector);
	NEWTON_API void  NewtonBodyGetAABB (const NewtonBody* body, dFloat* p0, dFloat* p1);	
	NEWTON_API void  NewtonBodyGetFreezeTreshold (const NewtonBody* body, dFloat* freezeSpeed2, dFloat* freezeOmega2);

	
	NEWTON_API void  NewtonBodyAddBuoyancyForce (const NewtonBody* body, dFloat fluidDensity, 
		dFloat fluidLinearViscosity, dFloat fluidAngularViscosity, 
		const dFloat* gravityVector, NewtonGetBuoyancyPlane buoyancyPlane, void *context);

	NEWTON_API void NewtonBodyForEachPolygonDo (const NewtonBody* body, NewtonCollisionIterator callback);
	NEWTON_API void NewtonAddBodyImpulse (const NewtonBody* body, const dFloat* pointDeltaVeloc, const dFloat* pointPosit);
	

	// **********************************************************************************************
	//
	// Common joint functions
	//
	// **********************************************************************************************
	NEWTON_API void* NewtonJointGetUserData (const NewtonJoint* joint);
	NEWTON_API void NewtonJointSetUserData (const NewtonJoint* joint, void* userData);

	NEWTON_API int NewtonJointGetCollisionState (const NewtonJoint* joint);
	NEWTON_API void NewtonJointSetCollisionState (const NewtonJoint* joint, int state);

	NEWTON_API dFloat NewtonJointGetStiffness (const NewtonJoint* joint);
	NEWTON_API void NewtonJointSetStiffness (const NewtonJoint* joint, dFloat state);

	NEWTON_API void NewtonDestroyJoint(const NewtonWorld* newtonWorld, const NewtonJoint* joint);
	NEWTON_API void NewtonJointSetDestructor (const NewtonJoint* joint, NewtonConstraintDestructor destructor);


	// **********************************************************************************************
	//
	// Ball and Socket joint functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonJoint* NewtonConstraintCreateBall (const NewtonWorld* newtonWorld, const dFloat* pivotPoint, 
		const NewtonBody* childBody, const NewtonBody* parentBody);
	NEWTON_API void NewtonBallSetUserCallback (const NewtonJoint* ball, NewtonBallCallBack callback);
	NEWTON_API void NewtonBallGetJointAngle (const NewtonJoint* ball, dFloat* angle);
	NEWTON_API void NewtonBallGetJointOmega (const NewtonJoint* ball, dFloat* omega);
	NEWTON_API void NewtonBallGetJointForce (const NewtonJoint* ball, dFloat* force);
	NEWTON_API void NewtonBallSetConeLimits (const NewtonJoint* ball, const dFloat* pin, dFloat maxConeAngle, dFloat maxTwistAngle);

	// **********************************************************************************************
	//
	// Hinge joint functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonJoint* NewtonConstraintCreateHinge (const NewtonWorld* newtonWorld, 
		const dFloat* pivotPoint, const dFloat* pinDir, 
		const NewtonBody* childBody, const NewtonBody* parentBody);

	NEWTON_API void NewtonHingeSetUserCallback (const NewtonJoint* hinge, NewtonHingeCallBack callback);
	NEWTON_API dFloat NewtonHingeGetJointAngle (const NewtonJoint* hinge);
	NEWTON_API dFloat NewtonHingeGetJointOmega (const NewtonJoint* hinge);
	NEWTON_API void NewtonHingeGetJointForce (const NewtonJoint* hinge, dFloat* force);
	NEWTON_API dFloat NewtonHingeCalculateStopAlpha (const NewtonJoint* hinge, const NewtonHingeSliderUpdateDesc* desc, dFloat angle);

	// **********************************************************************************************
	//
	// Slider joint functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonJoint* NewtonConstraintCreateSlider (const NewtonWorld* newtonWorld, 
		const dFloat* pivotPoint, const dFloat* pinDir, 
		const NewtonBody* childBody, const NewtonBody* parentBody);
	NEWTON_API void NewtonSliderSetUserCallback (const NewtonJoint* slider, NewtonSliderCallBack callback);
	NEWTON_API dFloat NewtonSliderGetJointPosit (const NewtonJoint* slider);
	NEWTON_API dFloat NewtonSliderGetJointVeloc (const NewtonJoint* slider);
	NEWTON_API void NewtonSliderGetJointForce (const NewtonJoint* slider, dFloat* force);
	NEWTON_API dFloat NewtonSliderCalculateStopAccel (const NewtonJoint* slider, const NewtonHingeSliderUpdateDesc* desc, dFloat position);


	// **********************************************************************************************
	//
	// Corkscrew joint functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonJoint* NewtonConstraintCreateCorkscrew (const NewtonWorld* newtonWorld, 
		const dFloat* pivotPoint, const dFloat* pinDir, 
		const NewtonBody* childBody, const NewtonBody* parentBody);
	NEWTON_API void NewtonCorkscrewSetUserCallback (const NewtonJoint* corkscrew, NewtonCorkscrewCallBack callback);
	NEWTON_API dFloat NewtonCorkscrewGetJointPosit (const NewtonJoint* corkscrew);
	NEWTON_API dFloat NewtonCorkscrewGetJointAngle (const NewtonJoint* corkscrew);
	NEWTON_API dFloat NewtonCorkscrewGetJointVeloc (const NewtonJoint* corkscrew);
	NEWTON_API dFloat NewtonCorkscrewGetJointOmega (const NewtonJoint* corkscrew);
	NEWTON_API void NewtonCorkscrewGetJointForce (const NewtonJoint* corkscrew, dFloat* force);
	NEWTON_API dFloat NewtonCorkscrewCalculateStopAlpha (const NewtonJoint* corkscrew, const NewtonHingeSliderUpdateDesc* desc, dFloat angle);
	NEWTON_API dFloat NewtonCorkscrewCalculateStopAccel (const NewtonJoint* corkscrew, const NewtonHingeSliderUpdateDesc* desc, dFloat position);


	// **********************************************************************************************
	//
	// Universal joint functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonJoint* NewtonConstraintCreateUniversal (const NewtonWorld* newtonWorld, 
		const dFloat* pivotPoint, const dFloat* pinDir0, const dFloat* pinDir1, 
		const NewtonBody* childBody, const NewtonBody* parentBody);
	NEWTON_API void NewtonUniversalSetUserCallback (const NewtonJoint* universal, NewtonUniversalCallBack callback);
	NEWTON_API dFloat NewtonUniversalGetJointAngle0 (const NewtonJoint* universal);
	NEWTON_API dFloat NewtonUniversalGetJointAngle1 (const NewtonJoint* universal);
	NEWTON_API dFloat NewtonUniversalGetJointOmega0 (const NewtonJoint* universal);
	NEWTON_API dFloat NewtonUniversalGetJointOmega1 (const NewtonJoint* universal);
	NEWTON_API void NewtonUniversalGetJointForce (const NewtonJoint* universal, dFloat* force);
	NEWTON_API dFloat NewtonUniversalCalculateStopAlpha0 (const NewtonJoint* universal, const NewtonHingeSliderUpdateDesc* desc, dFloat angle);
	NEWTON_API dFloat NewtonUniversalCalculateStopAlpha1 (const NewtonJoint* universal, const NewtonHingeSliderUpdateDesc* desc, dFloat angle);


	// **********************************************************************************************
	//
	// Up vector joint functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonJoint* NewtonConstraintCreateUpVector (const NewtonWorld* newtonWorld, const dFloat* pinDir, const NewtonBody* body); 
	NEWTON_API void NewtonUpVectorGetPin (const NewtonJoint* upVector, dFloat *pin);
	NEWTON_API void NewtonUpVectorSetPin (const NewtonJoint* upVector, const dFloat *pin);


	// **********************************************************************************************
	//
	// User defined bilateral Joint
	//
	// **********************************************************************************************
	NEWTON_API NewtonJoint* NewtonConstraintCreateUserJoint (const NewtonWorld* newtonWorld, int maxDOF, NewtonUserBilateralCallBack callback,
															 const NewtonBody* childBody, const NewtonBody* parentBody); 

	NEWTON_API void NewtonUserJointAddLinearRow (const NewtonJoint* joint, const dFloat *pivot0, const dFloat *pivot1, const dFloat *dir);
	NEWTON_API void NewtonUserJointAddAngularRow (const NewtonJoint* joint, dFloat relativeAngle, const dFloat *dir);
	NEWTON_API void NewtonUserJointAddGeneralRow (const NewtonJoint* joint, const dFloat *jacobian0, const dFloat *jacobian1);
	NEWTON_API void NewtonUserJointSetRowMinimumFriction (const NewtonJoint* joint, dFloat friction);
	NEWTON_API void NewtonUserJointSetRowMaximumFriction (const NewtonJoint* joint, dFloat friction);
	NEWTON_API void NewtonUserJointSetRowAcceleration (const NewtonJoint* joint, dFloat acceleration);
	NEWTON_API void NewtonUserJointSetRowSpringDamperAcceleration (const NewtonJoint* joint, dFloat springK, dFloat springD);
	NEWTON_API void NewtonUserJointSetRowStiffness (const NewtonJoint* joint, dFloat stiffness);
	NEWTON_API dFloat NewtonUserJointGetRowForce (const NewtonJoint* joint, int row);

	

	// **********************************************************************************************
	//
	// Rag doll joint container functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonRagDoll* NewtonCreateRagDoll (const NewtonWorld* newtonWorld);
	NEWTON_API void NewtonDestroyRagDoll (const NewtonWorld* newtonWorld, const NewtonRagDoll* ragDoll);

	NEWTON_API void NewtonRagDollBegin (const NewtonRagDoll* ragDoll);
	NEWTON_API void NewtonRagDollEnd (const NewtonRagDoll* ragDoll);


//	NEWTON_API void NewtonRagDollSetFriction (const NewtonRagDoll* ragDoll, dFloat friction);

	NEWTON_API NewtonRagDollBone* NewtonRagDollFindBone (const NewtonRagDoll* ragDoll, int id);
//	NEWTON_API NewtonRagDollBone* NewtonRagDollGetRootBone (const NewtonRagDoll* ragDoll);

	NEWTON_API void NewtonRagDollSetForceAndTorqueCallback (const NewtonRagDoll* ragDoll, NewtonApplyForceAndTorque callback);
	NEWTON_API void NewtonRagDollSetTransformCallback (const NewtonRagDoll* ragDoll, NewtonSetRagDollTransform callback);
	NEWTON_API NewtonRagDollBone* NewtonRagDollAddBone (const NewtonRagDoll* ragDoll, const NewtonRagDollBone* parent, 
		                                                void *userData, dFloat mass, const dFloat* matrix, 
														const NewtonCollision* boneCollision, const dFloat* size);

	NEWTON_API void* NewtonRagDollBoneGetUserData (const NewtonRagDollBone* bone);
	NEWTON_API NewtonBody* NewtonRagDollBoneGetBody (const NewtonRagDollBone* bone);
	NEWTON_API void NewtonRagDollBoneSetID (const NewtonRagDollBone* bone, int id);


	NEWTON_API void NewtonRagDollBoneSetLimits (const NewtonRagDollBone* bone, 
		                                        const dFloat* coneDir, dFloat minConeAngle, dFloat maxConeAngle, dFloat maxTwistAngle,
												const dFloat* bilateralConeDir, dFloat negativeBilateralConeAngle, dFloat positiveBilateralConeAngle);
	
//	NEWTON_API NewtonRagDollBone* NewtonRagDollBoneGetChild (const NewtonRagDollBone* bone);
//	NEWTON_API NewtonRagDollBone* NewtonRagDollBoneGetSibling (const NewtonRagDollBone* bone);
//	NEWTON_API NewtonRagDollBone* NewtonRagDollBoneGetParent (const NewtonRagDollBone* bone);
//	NEWTON_API void NewtonRagDollBoneSetLocalMatrix (const NewtonRagDollBone* bone, dFloat* matrix);
//	NEWTON_API void NewtonRagDollBoneSetGlobalMatrix (const NewtonRagDollBone* bone, dFloat* matrix);

	NEWTON_API void NewtonRagDollBoneGetLocalMatrix (const NewtonRagDollBone* bone, dFloat* matrix);
	NEWTON_API void NewtonRagDollBoneGetGlobalMatrix (const NewtonRagDollBone* bone, dFloat* matrix);



	// **********************************************************************************************
	//
	// Vehicle joint functions
	//
	// **********************************************************************************************
	NEWTON_API NewtonJoint* NewtonConstraintCreateVehicle (const NewtonWorld* newtonWorld, const dFloat* upDir, const NewtonBody* body); 
	NEWTON_API void NewtonVehicleReset (const NewtonJoint* vehicle); 
	NEWTON_API void NewtonVehicleSetTireCallback (const NewtonJoint* vehicle, NewtonVehicleTireUpdate update);
	NEWTON_API void* NewtonVehicleAddTire (const NewtonJoint* vehicle, const dFloat* localMatrix, const dFloat* pin, dFloat mass, dFloat width, dFloat radius, 
		       dFloat suspesionShock, dFloat suspesionSpring, dFloat suspesionLength, void* userData, int collisionID);
	NEWTON_API void NewtonVehicleRemoveTire (const NewtonJoint* vehicle, void* tireId);

	NEWTON_API void* NewtonVehicleGetFirstTireID (const NewtonJoint* vehicle);
	NEWTON_API void* NewtonVehicleGetNextTireID (const NewtonJoint* vehicle, void* tireId);

	NEWTON_API int NewtonVehicleTireIsAirBorne (const NewtonJoint* vehicle, void* tireId);
	NEWTON_API int NewtonVehicleTireLostSideGrip (const NewtonJoint* vehicle, void* tireId);
	NEWTON_API int NewtonVehicleTireLostTraction (const NewtonJoint* vehicle, void* tireId);

	NEWTON_API void* NewtonVehicleGetTireUserData (const NewtonJoint* vehicle, void* tireId);
	NEWTON_API dFloat NewtonVehicleGetTireOmega (const NewtonJoint* vehicle, void* tireId);
	NEWTON_API dFloat NewtonVehicleGetTireNormalLoad (const NewtonJoint* vehicle, void* tireId);
	NEWTON_API dFloat NewtonVehicleGetTireSteerAngle (const NewtonJoint* vehicle, void* tireId);
	NEWTON_API dFloat NewtonVehicleGetTireLateralSpeed (const NewtonJoint* vehicle, void* tireId);
	NEWTON_API dFloat NewtonVehicleGetTireLongitudinalSpeed (const NewtonJoint* vehicle, void* tireId);
	NEWTON_API void NewtonVehicleGetTireMatrix (const NewtonJoint* vehicle, void* tireId, dFloat* matrix);


	NEWTON_API void NewtonVehicleSetTireTorque (const NewtonJoint* vehicle, void* tireId, dFloat torque);
	NEWTON_API void NewtonVehicleSetTireSteerAngle (const NewtonJoint* vehicle, void* tireId, dFloat angle);
	
	NEWTON_API void NewtonVehicleSetTireMaxSideSleepSpeed (const NewtonJoint* vehicle, void* tireId, dFloat speed);
	NEWTON_API void NewtonVehicleSetTireSideSleepCoeficient (const NewtonJoint* vehicle, void* tireId, dFloat coeficient);
	NEWTON_API void NewtonVehicleSetTireMaxLongitudinalSlideSpeed (const NewtonJoint* vehicle, void* tireId, dFloat speed);
	NEWTON_API void NewtonVehicleSetTireLongitudinalSlideCoeficient (const NewtonJoint* vehicle, void* tireId, dFloat coeficient);

	NEWTON_API dFloat NewtonVehicleTireCalculateMaxBrakeAcceleration (const NewtonJoint* vehicle, void* tireId);
	NEWTON_API void NewtonVehicleTireSetBrakeAcceleration (const NewtonJoint* vehicle, void* tireId, dFloat accelaration, dFloat torqueLimit);

#ifdef __cplusplus 
}
#endif
#endif


