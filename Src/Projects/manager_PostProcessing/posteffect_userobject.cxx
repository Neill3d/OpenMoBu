
/** \file posteffect_userobject.cxx

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

// Class declaration
#include "posteffect_userobject.h"
#include "posteffect_shader_userobject.h"
#include "postprocessing_ini.h"
#include <vector>
#include <limits>
#include <ctime>

#include "FileUtils.h"
#include "mobu_logging.h"

#include "postpersistentdata.h"

// custom assets inserting

/** Element Class implementation. (Asset system)
*	This should be placed in the source code file for a class.
*/
#define PostEffectFBElementClassImplementation(ClassName,AssetName,IconFileName)\
	HIObject RegisterElement##ClassName##Create(HIObject /*pOwner*/, const char* pName, void* /*pData*/){\
	ClassName* Class = new ClassName(pName); \
	Class->mAllocated = true; \
if (Class->FBCreate()){\
	__FBRemoveModelFromScene(Class->GetHIObject()); /* Hack in MoBu2013, we shouldn't add object to the scene/entity automatically*/\
	return Class->GetHIObject(); \
}\
else {\
	delete Class; \
	return NULL;\
}\
	}\
		FBLibraryModule(ClassName##Element){\
		FBRegisterObject(ClassName##R2, "Browsing/Templates/Shading Elements", AssetName, "", RegisterElement##ClassName##Create, true, IconFileName); \
		}

//--- FiLMBOX Registration & Implementation.

FBClassImplementation(PostEffectUserObject);
FBUserObjectImplement(PostEffectUserObject,
	"User Object used to store a persistance data for an effect",
	"cam_switcher_toggle.png");                                          //Register UserObject class
PostEffectFBElementClassImplementation(PostEffectUserObject, "Post Effect", "cam_switcher_toggle.png");                  //Register to the asset system

////////////////////////////////////////////////////////////////////////////////
//


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UserEffect

bool UserEffect::IsActive() const
{
	return mUserObject->Active;
}

const char* UserEffect::GetName() const
{
	return mUserObject->Name;
}

int UserEffect::GetNumberOfBufferShaders() const
{
	return (mBufferShader) ? 1 : 0;
}

PostEffectBufferShader* UserEffect::GetBufferShaderPtr(const int bufferShaderIndex)
{
	return mBufferShader;
}

const PostEffectBufferShader* UserEffect::GetBufferShaderPtr(const int bufferShaderIndex) const
{
	return mBufferShader;
}

void UserEffect::SetBufferShader(PostEffectBufferShader* bufferShader)
{
	mBufferShader = bufferShader;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PostEffectUserObject


/************************************************
 *  Constructor.
 ************************************************/
PostEffectUserObject::PostEffectUserObject(const char* pName, HIObject pObject)
	: FBUserObject(pName, pObject)
	, mText("")
{
	FBClassInit;

	mReloadShaders = false;
}

void PostEffectUserObject::ActionReloadShaders(HIObject pObject, bool value)
{
	PostEffectUserObject* p = FBCast<PostEffectUserObject>(pObject);
	if (p && value)
	{
		p->DoReloadShaders();
	}
}

/************************************************
 *  FiLMBOX Constructor.
 ************************************************/
bool PostEffectUserObject::FBCreate()
{
	// modify system behavoiur
	DisableObjectFlags(kFBFlagClonable);

	FBPropertyPublish(this, UniqueClassId, "UniqueClassId", nullptr, nullptr);
	FBPropertyPublish(this, Active, "Active", nullptr, nullptr);

	//FBPropertyPublish(this, RenderToTexture, "Render To Texture", nullptr, nullptr);
	FBPropertyPublish(this, OutputVideo, "Output Video", nullptr, nullptr);

	FBPropertyPublish(this, BufferShaders, "Shaders", nullptr, nullptr);
	FBPropertyPublish(this, ReloadShaders, "Reload Shader", nullptr, ActionReloadShaders);

	FBPropertyPublish(this, UseMasking, "Use Masking", nullptr, nullptr);
	FBPropertyPublish(this, MaskingChannel, "Masking Channel", nullptr, nullptr);

	//
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagHideProperty, true);
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagNotSavable, true);
	UniqueClassId.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
	UniqueClassId = 57;

	mUserEffect.reset(new UserEffect(this));
	return true;
}

/************************************************
 *  FiLMBOX Destructor.
 ************************************************/
void PostEffectUserObject::FBDestroy()
{
	mUserEffect.reset(nullptr);
}

bool PostEffectUserObject::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	return ParentClass::FbxStore(pFbxObject, pStoreWhat);
}
bool PostEffectUserObject::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if (pStoreWhat == kFbxObjectStore::kCleanup)
	{
		RefreshEffectConnections();
		if (IsReadyAndActive())
		{
			DoReloadShaders();
		}
	}

	return ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);
}

bool PostEffectUserObject::PlugNotify(FBConnectionAction pAction, FBPlug* pThis, int pIndex, FBPlug* pPlug, FBConnectionType pConnectionType, FBPlug* pNewPlug)
{
	if (pThis == &BufferShaders)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);
			RefreshEffectConnections();
		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
			RefreshEffectConnections();
		}
	}
	
	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}

void PostEffectUserObject::RefreshEffectConnections()
{
	PostEffectBufferShader* bufferShader = nullptr;
	for (int i = 0; i < BufferShaders.GetCount(); ++i)
	{
		if (FBIS(BufferShaders[i], EffectShaderUserObject))
		{
			if (EffectShaderUserObject* UserObject = FBCast<EffectShaderUserObject>(BufferShaders[i]))
			{
				bufferShader = UserObject->GetUserShaderPtr();
				break;
			}
		}
	}
	mUserEffect->SetBufferShader(bufferShader);
}

bool PostEffectUserObject::IsReadyAndActive() const
{
	if (!Active)
		return false;

	if (!GetUserEffectPtr())
		return false;

	if (!GetUserEffectPtr()->IsActive())
		return false;

	return true;
}

void PostEffectUserObject::DoReloadShaders()
{
	for (int i = 0; i < BufferShaders.GetCount(); ++i)
	{
		if (FBIS(BufferShaders[i], EffectShaderUserObject))
		{
			if (EffectShaderUserObject* UserObject = FBCast<EffectShaderUserObject>(BufferShaders[i]))
			{
				if (!UserObject->DoReloadShaders())
				{
					// TODO: stop reloading of next shaders if something failed
					break;
				}
			}
		}
	}
}

bool PostEffectUserObject::IsDepthSamplerUsed() const
{
	return mUserEffect->IsDepthSamplerUsed();
}

bool PostEffectUserObject::IsLinearDepthSamplerUsed() const
{
	return mUserEffect->IsLinearDepthSamplerUsed();
}