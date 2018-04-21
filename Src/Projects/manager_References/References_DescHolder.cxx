

/**	\file	references_DescHolder.cxx

Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- Class declaration
#include "References_DescHolder.h"

#include <Windows.h>
#include <shellapi.h>

#include <iostream>
#include <fstream>
#include <string>

FBClassImplementation(DescriptionHolder)
FBUserObjectImplement(DescriptionHolder, "Shaders Graph Description Holder", FB_DEFAULT_SDK_ICON);	//Register UserObject class
FBElementClassImplementation(DescriptionHolder, FB_DEFAULT_SDK_ICON);					//Register to the asset system


bool fileExists(const char * file)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE handle = FindFirstFile(file, &FindFileData);
	bool found = (handle != INVALID_HANDLE_VALUE);
	if (found)
	{
		//FindClose(&handle); this will crash
		FindClose(handle);
	}
	return found;
}

void DescriptionHolder::DescHolderStoreAction(HIObject pMbObject, bool pValue)
{
	DescriptionHolder* pFbObject = FBCast<DescriptionHolder>(pMbObject);
	if (pValue && pFbObject)
		pFbObject->DoStore();
}

void DescriptionHolder::DescHolderExtractAction(HIObject pMbObject, bool pValue)
{
	DescriptionHolder* pFbObject = FBCast<DescriptionHolder>(pMbObject);
	if (pValue && pFbObject)
		pFbObject->DoExtract();
}

void DescriptionHolder::DescHolderRevealAction(HIObject pMbObject, bool pValue)
{
	DescriptionHolder* pFbObject = FBCast<DescriptionHolder>(pMbObject);
	if (pValue && pFbObject)
		pFbObject->DoReveal();
}

/************************************************
 *	Constructor.
 ************************************************/
DescriptionHolder::DescriptionHolder(const char* pName, HIObject pObject) 
: FBUserObject(pName, pObject)
{
	FBClassInit;

	FBPropertyPublish(this, ReferenceModel, "Reference Model", nullptr, nullptr);
	FBPropertyPublish(this, DescriptionFileName, "Description FileName", nullptr, nullptr);
	FBPropertyPublish(this, TempFileName, "Temp FileName", nullptr, nullptr);

	FBPropertyPublish(this, Store, "Store", nullptr, DescHolderStoreAction);
	FBPropertyPublish(this, Extract, "Extract", nullptr, DescHolderExtractAction);
	FBPropertyPublish(this, Reveal, "Reveal", nullptr, DescHolderRevealAction);

	ReferenceModel.SetSingleConnect(false);
	ReferenceModel.SetFilter(FBFileReference::GetInternalClassId());

	TempFileName.ModifyPropertyFlag(kFBPropertyFlagNotSavable, true);
	TempFileName.ModifyPropertyFlag(kFBPropertyFlagReadOnly, true);
}

/************************************************
 *	FiLMBOX Constructor.
 ************************************************/
bool DescriptionHolder::FBCreate()
{
	return true;
}

/************************************************
 *	FiLMBOX Destructor.
 ************************************************/
void DescriptionHolder::FBDestroy()
{
	FBString temp(TempFileName);

	if (temp.GetLen() > 0)
	{
		if (true == fileExists(temp))
		{
			remove(temp);
		}
	}
}

bool DescriptionHolder::FbxStore(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{

	if (pStoreWhat == kAttributes)
	{
		//pFbxObject->FieldWriteBlockBegin(); // FBString(LongName, "_XML_BLOCK"));

		pFbxObject->FieldWriteBegin(FBString(Name, "_REF_BLOCK"));

		pFbxObject->FieldWriteI(ReferenceModel.GetCount());

		for (int i = 0, count = ReferenceModel.GetCount(); i < count; ++i)
		{
			pFbxObject->FieldWriteC(ReferenceModel.GetAt(i)->LongName);
		}

		pFbxObject->FieldWriteEnd();

		//
		//

		pFbxObject->FieldWriteBegin(FBString(Name, "_DESC_BLOCK"));
		pFbxObject->FieldWriteI((int)mData.size());
		
		if (mData.size() > 0)
		{
			const int size = (int) mData.size();
			const int blockSize = 7 * 1024;	// 7 kb block of text
			char buffer[blockSize];
			//int blocks = size / blockSize;

			pFbxObject->FieldWriteI(blockSize);

			//int pos = 0;
			for (int i = 0; i < size; i += blockSize)
			{
				int copySize = blockSize;
				if (copySize >(size - i))
					copySize = size - i;

				memcpy_s(buffer, sizeof(char)*blockSize, mData.data() + i, blockSize);
				pFbxObject->FieldWriteC(buffer);
			}
			//pFbxObject->FieldWriteBegin(FBString(LongName, "_DATA"));
			//pFbxObject->FieldWriteC(mData.data());
			//pFbxObject->FieldWriteEnd();
		}

		pFbxObject->FieldWriteEnd();

		//pFbxObject->FieldWriteBlockEnd();
	}
	return ParentClass::FbxStore(pFbxObject, pStoreWhat);
}

bool DescriptionHolder::FbxRetrieve(FBFbxObject* pFbxObject, kFbxObjectStore pStoreWhat)
{
	if (kAttributes & pStoreWhat)
	{
		// read connections
		
		if (true == pFbxObject->FieldReadBegin(FBString(Name, "_REF_BLOCK")))
		{
			int count = pFbxObject->FieldReadI();
			mRefNames.resize(count);

			for (int i = 0; i < count; ++i)
			{
				const char *longname = pFbxObject->FieldReadC();
				mRefNames.push_back(std::string(longname));
			}

			pFbxObject->FieldWriteEnd();
		}
		else
		{
			mRefNames.clear();
		}

		// read store data

		if (pFbxObject->FieldReadBegin(FBString(Name, "_DESC_BLOCK")))
		{
		
			int size = pFbxObject->FieldReadI();
			
			if (size > 0)
			{
				int blockSize = pFbxObject->FieldReadI();

				mData.resize(size, 0);
				char *ptr = mData.data();
				//int blocks = size / blockSize;

				for (int i = 0; i < size; i+=blockSize, ptr+=blockSize)
				{
					const char *data = pFbxObject->FieldReadC();
					int copySize = blockSize;
					if (copySize >(size - i))
						copySize = size - i;

					memcpy(ptr, data, sizeof(char)*copySize);
				}
			}

		}
	}
	else if (kCleanup & pStoreWhat)
	{
		ReferenceModel.Clear();
		FBScene *pScene = mSystem.Scene;
		FBString descPath(DescriptionFileName);

		if (descPath.GetLen() > 4)
		{
			FBString fbxPath(descPath.Left(descPath.GetLen() - 3), "fbx");

			if (mRefNames.size() > 0)
			{
				for (auto iter = begin(mRefNames); iter != end(mRefNames); ++iter)
				{
					const char *longname = iter->c_str();

					for (int i = 0, count = pScene->Namespaces.GetCount(); i < count; ++i)
					{
						if (FBIS(pScene->Namespaces[i], FBFileReference))
						{
							FBFileReference *pref = (FBFileReference*)pScene->Namespaces[i];
							FBString prefpath(pref->ReferenceFilePath);
							if (0 == strcmp(longname, pref->Name)
								&& 0 == _stricmp(prefpath, fbxPath))
							{
								ReferenceModel.Add(pref);
							}
						}
					}
				}

				mRefNames.clear();
			}
			else
			{
				// try to connect references by orig xml name (reference file path)

				FBString fbxPath(descPath.Left(descPath.GetLen() - 3), "fbx");

				for (int i = 0, count = pScene->Namespaces.GetCount(); i < count; ++i)
				{
					if (FBIS(pScene->Namespaces[i], FBFileReference))
					{
						FBFileReference *pref = (FBFileReference*)pScene->Namespaces[i];
						FBString prefpath(pref->ReferenceFilePath);

						if (0 == _stricmp(prefpath, fbxPath))
						{
							ReferenceModel.Add(pref);
						}
					}
				}

			}
		}
	}

	return ParentClass::FbxRetrieve(pFbxObject, pStoreWhat);
}



void DescriptionHolder::DoStore()
{
	FBString xmlname(DescriptionFileName);

	FBString lastTemp(TempFileName);

	if (lastTemp.GetLen() > 0)
	{
		if (true == fileExists(lastTemp))
		{
			remove(lastTemp);
		}
	}
	TempFileName = "";

	// DONE: if file exist, store xml in object
	if (true == fileExists(xmlname))
	{
		mData.clear();

		std::ifstream inf(xmlname);
		if (true == inf.is_open() )
		{
			std::filebuf *pbuf = inf.rdbuf();

			std::streampos fsize = 0;
			fsize = inf.tellg();
			inf.seekg(0, std::ios::end);
			fsize = inf.tellg() - fsize;

			size_t dataLen = fsize;
			mData.resize(dataLen+1, 0);
			inf.seekg(0, std::ios::beg);

			pbuf->sgetn(mData.data(), fsize);

			inf.close();
		}
	}
}

void DescriptionHolder::DoExtract()
{
	// DONE: if there is a stored object, save it to temp name and path

	FBString lastTemp(TempFileName);

	if (lastTemp.GetLen() > 0)
	{
		if (true == fileExists(lastTemp))
		{
			remove(lastTemp);
		}
	}
	TempFileName = "";

	//
	//

	if (mData.size() > 0)
	{
		// generate temp path and name
		DWORD dwRetVal = 0;
		UINT uRetVal = 0;

		TCHAR szTempFileName[MAX_PATH];
		TCHAR lpTempPathBuffer[MAX_PATH];

		dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);

		if (0 == dwRetVal)
			return;

		uRetVal = GetTempFileName(lpTempPathBuffer, TEXT("ShadersGraph"), 0, szTempFileName);

		if (0 == uRetVal)
			return;

		TempFileName = szTempFileName;

		std::ofstream outf(szTempFileName);

		outf << mData.data();
		//outf.write(mData.data(), mData.size()-1);

		outf.close();
	}
}

void DescriptionHolder::DoReveal()
{
	FBString cmd("explorer ", TempFileName);
	system(cmd);
	
}

bool DescriptionHolder::PlugNotify(FBConnectionAction pAction, FBPlug* pThis, int pIndex, FBPlug* pPlug, FBConnectionType pConnectionType, FBPlug* pNewPlug)
{
	if (pThis == &ReferenceModel)
	{
		if (pAction == kFBConnectedSrc)
		{
			ConnectSrc(pPlug);

			// update description filename
			if (FBIS(pPlug, FBFileReference))
			{
				FBFileReference *pref = (FBFileReference*)pPlug;
				std::string filename = pref->ReferenceFilePath;

				size_t len = filename.size();
				if (len > 3)
				{
					if ('x' == tolower(filename[len - 1]) && 'b' == tolower(filename[len - 2]) && 'f' == tolower(filename[len - 3]))
					{
						filename[len - 3] = 'x';
						filename[len - 2] = 'm';
						filename[len - 1] = 'l';

						DescriptionFileName = filename.c_str();
					}
				}
			}

		}
		else if (pAction == kFBDisconnectedSrc)
		{
			DisconnectSrc(pPlug);
		}
	}
	
	return ParentClass::PlugNotify(pAction, pThis, pIndex, pPlug, pConnectionType, pNewPlug);
}