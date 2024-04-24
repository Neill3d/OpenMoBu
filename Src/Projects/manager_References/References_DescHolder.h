#ifndef __REFERENCES_DESC_HOLDER_H__
#define __REFERENCES_DESC_HOLDER_H__

/**	\file	References_DescHolder.h

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>
#include <vector>
#include <string>

//--- Registration define
#define DESCHOLDER__CLASSNAME	DescriptionHolder
#define DESCHOLDER__CLASSSTR	"DescriptionHolder"

/////////////////////////////////////////////////////////////////////////////////////////////////////////

/**	Description Holder.
*/
class DescriptionHolder : public FBUserObject
{
	//--- FiLMBOX declaration.
	FBClassDeclare(DescriptionHolder, FBUserObject)
	FBDeclareUserObject(DescriptionHolder);

public:
	DescriptionHolder(const char *pName = NULL, HIObject pObject = NULL);

	//--- FiLMBOX Construction/Destruction,
	virtual bool FBCreate();		//!< FiLMBOX Creation function.
	virtual void FBDestroy();		//!< FiLMBOX Destruction function.

	virtual bool FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);
	virtual bool FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat);

	virtual bool PlugNotify(FBConnectionAction pAction, FBPlug* pThis, int pIndex, FBPlug* pPlug, FBConnectionType pConnectionType, FBPlug* pNewPlug) override;

	FBPropertyListObject	ReferenceModel;
	FBPropertyString		DescriptionFileName;	// xml to be stored
	FBPropertyString		TempFileName;			// extracted xml

	FBPropertyAction		Store;
	FBPropertyAction		Extract;
	FBPropertyAction		Reveal;
	
	static void DescHolderStoreAction(HIObject pMbObject, bool pValue);
	static void DescHolderExtractAction(HIObject pMbObject, bool pValue);
	static void DescHolderRevealAction(HIObject pMbObject, bool pValue);

public:

	void DoStore();
	void DoExtract();
	void DoReveal();

protected:

	FBSystem					mSystem;

	std::vector<std::string>	mRefNames;
	std::vector<char>			mData;

};


#endif /* __REFERENCES_DESC_HOLDER_H__ */
