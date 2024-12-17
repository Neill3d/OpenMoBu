
#pragma once

/** \file posteffect_userobject.h

Sergei <Neill3d> Solokhin 2018-2024

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE

*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include "postprocessing_helper.h"

//--- Registration define

#define POSTEFFECT_USEROBJECT__CLASSSTR	"PostEffectUserObject"

/// <summary>
/// This class will make sure that the data related to the post effect is persistent.
/// </summary>
class PostEffectUserObject : public FBUserObject 
{
    //--- FiLMBOX declaration.
	FBClassDeclare(PostEffectUserObject, FBUserObject)
	FBDeclareUserObject(PostEffectUserObject);

public:
	//! a constructor
	PostEffectUserObject(const char *pName = nullptr, HIObject pObject = nullptr);

    //--- FiLMBOX Construction/Destruction,
    virtual bool FBCreate() override;        //!< FiLMBOX Creation function.
    virtual void FBDestroy() override;       //!< FiLMBOX Destruction function.

    
	void CopyValues(PostEffectUserObject* pOther);

public: // PROPERTIES

	FBPropertyInt				UniqueClassId;
	FBPropertyAnimatableBool	Active;

	FBPropertyAction			ReloadShaders;
	FBPropertyBool				GenerateMipMaps;
	FBPropertyAction			ResetToDefault;

	FBPropertyBool				UseMasking;
	FBPropertyBaseEnum<EMaskingChannel>	MaskingChannel;
	
public:

	void DoReloadShaders();

	bool IsNeedToReloadShaders();
	void SetReloadShadersState(bool state);

protected:

	friend class ToolPostProcessing;

	FBSystem			mSystem;

    FBString			mText;
	bool				mReloadShaders{ false };
	
	void		DefaultValues();
	void		LoadFromConfig(const char *sessionFilter=nullptr);
	void		LoadFarValueFromConfig();

};
