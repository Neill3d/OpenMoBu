
// desc_bake.cpp
// Sergei <Neill3d> Solokhin 2018

#include "desc.h"
#include "tinyxml.h"

#include <vector>
#include <set>

///////////////////////////////////////////////////// 

#define FAILED_STRUCT_TEXT	"failed to recognize xml structure"

/////////////////////////////////////////////////////


struct ParsingShader
{
	std::string		classname;
	std::string		name;
	std::string		longname;
	std::string		type;

	TiXmlElement	*pElem;

	int propCount;
	TiXmlElement	*pFirstProp;

	int attCount;
	TiXmlElement	*pFirstAttachment;

	bool			isValid;

	//! a constructor
	ParsingShader()
	{
		isValid = false;
		pElem = nullptr;
		attCount = 0;
		pFirstAttachment = nullptr;
	}

	//! an alternative constructor
	ParsingShader(TiXmlElement *_pElem)
	{
		Prep(_pElem);
	}


	const char *GetLongName() const
	{
		return longname.c_str();
	}

	const int GetDstObjectCount() const {
		return attCount;
	}

	const bool IsValid() const
	{
		return isValid;
	}

	TiXmlElement *GetFirstPropertyElem()
	{
		return pFirstProp;
	}
	TiXmlElement *GetFirstAttachmentElem()
	{
		return pFirstAttachment;
	}

	bool Prep(TiXmlElement *_pElem)
	{
		isValid = false;
		pElem = _pElem;
		int founded = 0;

		for (TiXmlAttribute *pAttrib = pElem->FirstAttribute();
			nullptr != pAttrib;
			pAttrib = pAttrib->Next())
		{
			if (0 == strcmp("ClassName", pAttrib->Name()))
			{
				classname = pAttrib->ValueStr();
				founded += 1;
			}
			else if (0 == strcmp("Name", pAttrib->Name()))
			{
				name = pAttrib->ValueStr();
				founded += 1;
			}
			else if (0 == strcmp("LongName", pAttrib->Name()))
			{
				longname = pAttrib->ValueStr();
				founded += 1;
			}
			else if (0 == strcmp("Type", pAttrib->Name()))
			{
				type = pAttrib->ValueStr();
				founded += 1;
			}
		}

		if (0 == classname.size() || 0 == longname.size())
		{
			return false;
		}

		// look for properties
		founded = 0;
		propCount = 0;

		TiXmlElement *pPropElem = pElem->FirstChildElement("Properties");
		if (nullptr != pPropElem)
		{
			for (TiXmlAttribute *pAttrib = pPropElem->FirstAttribute();
				nullptr != pAttrib;
				pAttrib = pAttrib->Next())
			{
				if (0 == strcmp("Count", pAttrib->Name()))
				{
					propCount = pAttrib->IntValue();
					founded += 1;
				}
			}

			pFirstProp = pPropElem->FirstChildElement("Property");
		}

		if (nullptr == pFirstProp || 0 == founded)
		{
			printf("> skipping one xml shader property\n");
			return false;
		}

		// look for attachments

		founded = 0;
		attCount = 0;

		pPropElem = pElem->FirstChildElement("Attachments");
		if (nullptr != pPropElem)
		{
			for (TiXmlAttribute *pAttrib = pPropElem->FirstAttribute();
				nullptr != pAttrib;
				pAttrib = pAttrib->Next())
			{
				if (0 == strcmp("Count", pAttrib->Name()))
				{
					attCount = pAttrib->IntValue();
					founded += 1;
				}
			}

			pFirstAttachment = pPropElem->FirstChildElement("Dst");
		}

		if (nullptr == pFirstAttachment || 0 == founded)
		{
			printf("> skipping one xml shader attachment\n");
			return false;
		}

		isValid = true;
		return true;
	}
};

void FreeParsingShadersVector(std::vector<ParsingShader*> &shaders)
{
	for (auto iter = begin(shaders); iter != end(shaders); ++iter)
	{
		ParsingShader *pshader = *iter;
		if (nullptr != pshader)
		{
			delete pshader;
			pshader = nullptr;
		}
	}
}

// return true if fbx shaders differ from xml shaders
bool CheckForShadersGraphChanges(FbxScene *pScene, std::vector<ParsingShader*> &shaders, std::set<FbxObject*> &fbxShaders)
{
	// check for number of shaders, longname and classname, number of connections, connection longname

	// 1 - check number of shaders
	if (shaders.size() != fbxShaders.size())
		return true;

	// 2 - check for long name and type

	for (auto xmlshaderIter = begin(shaders); xmlshaderIter != end(shaders); ++xmlshaderIter)
	{
		bool founded = false;

		for (auto fbxshaderIter = begin(fbxShaders); fbxshaderIter != end(fbxShaders); ++fbxshaderIter)
		{
			const char *fbxshaderName = (*fbxshaderIter)->GetName();
			FbxProperty subTypeProp = (*fbxshaderIter)->FindProperty("MoBuSubTypeName");
			if (false == subTypeProp.IsValid())
				break;
			FbxString subTypeStr(subTypeProp.Get<FbxString>());

			// DONE: check type (classname)
			if (0 == strcmp(fbxshaderName, (*xmlshaderIter)->GetLongName())
				&& 0 == strcmp(subTypeStr.Buffer(), (*xmlshaderIter)->classname.c_str()))
			{
				founded = true;

				// check number of connections
				const int fbxDstCount = (*fbxshaderIter)->GetDstObjectCount() - 1; // -1 I'm removing FbxScene connection
				const int xmlDstCount = (*xmlshaderIter)->GetDstObjectCount();

				// TODO: check each dst longname and type !!
				if (fbxDstCount != xmlDstCount)
				{
					return true;
				}

				break;
			}
		}

		if (false == founded)
			return true;
	}

	

	return false;
}

bool ReadXmlShader(FbxScene *pScene, FbxObject *pFbxShader, ParsingShader *pXmlShader, bool readConnections=false)
{
	bool anyChanges = false;
	bool anyConnChanges = false;
	const double eps = 0.001;

	// conns

	if (true == readConnections)
	{
		// TODO:

		for (TiXmlElement *pElem = pXmlShader->GetFirstAttachmentElem();
			nullptr != pElem;
			pElem = pElem->NextSiblingElement("Dst"))
		{
			std::string modelname;

			for (TiXmlAttribute *pAttrib = pElem->FirstAttribute(); nullptr != pAttrib; pAttrib = pAttrib->Next())
			{
				if (0 == strcmp("LongName", pAttrib->Name()))
				{
					modelname = pAttrib->ValueStr();
				}
			}

			if (modelname.size() > 0)
			{
				FbxNode *pnode = pScene->FindNodeByName(FbxString(modelname.c_str()));
				if (nullptr != pnode)
					pnode->ConnectSrcObject(pFbxShader);
			}
		}

	}

	// props

	for (TiXmlElement *pElem = pXmlShader->GetFirstPropertyElem(); 
			nullptr != pElem; 
			pElem = pElem->NextSiblingElement("Property"))
	{
		std::string name, value;
		double x, y, z, w;
		x = y = z = w = 0.0;

		for (TiXmlAttribute *pAttrib = pElem->FirstAttribute(); nullptr != pAttrib; pAttrib = pAttrib->Next())
		{
			if (0 == strcmp("Name", pAttrib->Name()))
			{
				name = pAttrib->ValueStr();
			}
			else if (0 == strcmp("Value", pAttrib->Name()))
			{
				value = pAttrib->ValueStr();
			}
			else if (0 == strcmp("X", pAttrib->Name()))
			{
				value = pAttrib->ValueStr();
				sscanf_s( value.c_str(), "%lf", &x );
			}
			else if (0 == strcmp("Y", pAttrib->Name()))
			{
				value = pAttrib->ValueStr();
				sscanf_s(value.c_str(), "%lf", &y);
			}
			else if (0 == strcmp("Z", pAttrib->Name()))
			{
				value = pAttrib->ValueStr();
				sscanf_s(value.c_str(), "%lf", &z);
			}
			else if (0 == strcmp("W", pAttrib->Name()))
			{
				value = pAttrib->ValueStr();
				sscanf_s(value.c_str(), "%lf", &w);
			}
		}

		if (0 == name.size() || 0 == value.size())
		{
			printf("> skipping one shader property\n");
			continue;
		}
		
		FbxProperty prop = pFbxShader->FindProperty(name.c_str());
		
		if (prop.IsValid())
		{
			EFbxType dataType = prop.GetPropertyDataType().GetType();

			switch (dataType)
			{
			case eFbxBool:
			{
				bool oldValue = prop.Get<FbxBool>();
				bool newValue = (0 == strcmp("True", value.c_str()));

				if (oldValue != newValue)
				{
					prop.Set<FbxBool>(newValue);
					anyChanges = true;
				}
			}break;
			case eFbxDouble:
			{
				double oldValue = prop.Get<FbxDouble>();
				double newValue = 0.0;
				sscanf_s(value.c_str(), "%lf", &newValue);

				if (abs(oldValue - newValue) > eps)
				{
					prop.Set<FbxDouble>(newValue);
					anyChanges = true;
				}

			}break;

			case eFbxEnum:
			case eFbxInt:
			{
				int oldValue = prop.Get<FbxInt>();
				int newValue = atoi(value.c_str());

				if (abs(oldValue - newValue) > 0)
				{
					prop.Set<FbxInt>(newValue);
					anyChanges = true;
				}
			}break;
			
			case eFbxDouble3:
			{
				FbxDouble3	oldValue = prop.Get<FbxDouble3>();
				
				if (abs(oldValue[0] - x) > eps || abs(oldValue[1] - y) > eps || abs(oldValue[2] - z) > eps)
				{
					prop.Set<FbxDouble3>(FbxDouble3(x, y, z));
					anyChanges = true;
				}

			}break;

			case eFbxDouble4:
			{
				FbxDouble4	oldValue = prop.Get<FbxDouble4>();

				if (abs(oldValue[0] - x) > eps || abs(oldValue[1] - y) > eps || abs(oldValue[2] - z) > eps || abs(oldValue[3]-w) > eps)
				{
					prop.Set<FbxDouble4>(FbxDouble4(x, y, z, w));
					anyChanges = true;
				}

			}break;

			case eFbxString:
			{
				FbxString oldValue = prop.Get<FbxString>();

				if (0 != strcmp(oldValue, value.c_str()))
				{
					oldValue = value.c_str();
					prop.Set<FbxString>(oldValue);
					anyChanges = true;
				}

			}break;

			}


			//
			// TODO: check connections

			int numberOfFbxConnections = prop.GetSrcObjectCount();
			int numberOfXmlConnections = 0;

			TiXmlElement *pAllConn = pElem->FirstChildElement("Connections");
			if (nullptr != pAllConn)
			{
				for (TiXmlAttribute *pAttrib = pAllConn->FirstAttribute(); nullptr != pAttrib; pAttrib = pAttrib->Next())
				{
					if (0 == strcmp("Count", pAttrib->Name()))
						numberOfXmlConnections = pAttrib->IntValue();
				}
			}
			
			prop.DisconnectAllSrcObject();
			
			if (numberOfXmlConnections > 0)
			{
				// assign connection from xml
				TiXmlElement *pConnElem = pAllConn->FirstChildElement("Source");

				for (; nullptr != pConnElem; pConnElem = pConnElem->NextSiblingElement("Source"))
				{
					
					std::string longname;

					for (TiXmlAttribute *pAttrib = pConnElem->FirstAttribute(); nullptr != pAttrib; pAttrib = pAttrib->Next())
					{
						if (0 == strcmp("LongName", pAttrib->Name()))
							longname = pAttrib->ValueStr();
					}
					
					if (longname.size() > 0)
					{
						FbxNode *pNode = pScene->FindNodeByName(longname.c_str());

						if (nullptr != pNode)
						{
							prop.ConnectSrcObject(pNode);
						}
					}
					
				}

				anyConnChanges = true;
			}
		}
	}

	if (true == anyChanges)
	{
		FbxProperty prop = pFbxShader->FindProperty("MoBuAttrBlindData");

		if (prop.IsValid())
		{
			prop.Destroy();
		}
	}
	if (true == anyConnChanges)
	{
		// TODO: do we need to remove blind conn Blod data ?!
		FbxProperty prop = pFbxShader->FindProperty("MoBuRelationBlindData");

		if (prop.IsValid())
		{
			prop.Destroy();
		}
	}

	return true;
}

FbxObject *CloneShaderFromLibrary(FbxManager *pManager, FbxScene *pScene, const char *type, std::vector<FbxObject*> &fbxShadersLibrary)
{
	for (auto iter = begin(fbxShadersLibrary); iter != end(fbxShadersLibrary); ++iter)
	{
		FbxObject *pShaderBase = *iter;

		FbxProperty subTypeProp = pShaderBase->FindProperty("MoBuSubTypeName");
		if (false == subTypeProp.IsValid())
			break;
		FbxString subTypeStr(subTypeProp.Get<FbxString>());

		if (pShaderBase->IsRuntimePlug()
			&& 0 == strcmp(type, subTypeStr.Buffer()) )
		{
			FbxClassId classid = pShaderBase->GetRuntimeClassId();

			// adding two new clones
			FbxObject *newObj = classid.Create(*pManager, "New Shader\0", pShaderBase);
			newObj->SetName("New Shader\0");
			pScene->ConnectSrcObject(newObj);

			return newObj;
		}
	}

	return nullptr;
}

bool ReadXml(const char *fname, FbxManager *pManager, FbxScene *pScene, std::set<FbxObject*> &fbxShaders, std::vector<FbxObject*> &fbxShadersLibrary)
{
	bool lStatus = true;
	TiXmlDocument	*pDoc = nullptr;
	std::vector<ParsingShader*>		xmlShaders;

	try
	{
		pDoc = new TiXmlDocument();
		if (nullptr == pDoc)
			throw std::exception("failed to allocate memory");

		if (false == pDoc->LoadFile(fname))
			throw std::exception("failed to load a xml file");

		TiXmlElement *pElem = pDoc->FirstChildElement("ShadersGraph");

		if (nullptr == pElem)
			throw std::exception(FAILED_STRUCT_TEXT);

		pElem = pElem->FirstChildElement("Shaders");
		if (nullptr == pElem)
			throw std::exception(FAILED_STRUCT_TEXT);

		int numberOfShaders = 0;

		for (TiXmlAttribute *pAttrib = pElem->FirstAttribute(); nullptr != pAttrib; pAttrib = pAttrib->Next())
		{
			if (0 == strcmp("Count", pAttrib->Name()))
			{
				numberOfShaders = pAttrib->IntValue();
			}
		}
		
		pElem = pElem->FirstChildElement("Shader");
		for (; nullptr != pElem; pElem = pElem->NextSiblingElement("Shader"))
		{
			ParsingShader	*newParsing = new ParsingShader(pElem);

			if (false == newParsing->IsValid())
			{
				delete newParsing;
				newParsing = nullptr;
				throw std::exception("failed to parse shader");
			}

			xmlShaders.push_back(newParsing);
		}

		// check if there are any changes in the scene
		bool anyChanges = CheckForShadersGraphChanges(pScene, xmlShaders, fbxShaders);

		if (true == anyChanges)
		{
			// we have to remove all fbx shaders and replace them with new one, imported from xml with new shader-model connections

			for (auto iter = begin(fbxShaders); iter != end(fbxShaders); ++iter)
			{
				(*iter)->Destroy();
			}

			for (auto parseIter = begin(xmlShaders); parseIter != end(xmlShaders); ++parseIter)
			{
				FbxObject *pNewShader = nullptr;

				// Clone a library shader with a specified type
				pNewShader = CloneShaderFromLibrary(pManager, pScene, (*parseIter)->classname.c_str(), fbxShadersLibrary);

				if (nullptr != pNewShader)
				{
					ReadXmlShader(pScene, pNewShader, *parseIter, true);
				}
			}

		}
		else
		{
			// retrive ONLY updated shader properties

			for (auto parseIter = begin(xmlShaders); parseIter != end(xmlShaders); ++parseIter)
			{
				// DONE: try to find that shader in the fbx scene
				for (auto iter = begin(fbxShaders); iter != end(fbxShaders); ++iter)
				{
					FbxObject *pObject = *iter;

					FbxProperty subTypeProp = pObject->FindProperty("MoBuSubTypeName");
					if (false == subTypeProp.IsValid())
						break;
					FbxString subTypeStr(subTypeProp.Get<FbxString>());

					if (0 == strcmp(pObject->GetName(), (*parseIter)->GetLongName()))
						// && 0 == strcmp(subTypeStr.Buffer(), type.c_str()) )
					{
						// find a shader, let's find an xml value
						ReadXmlShader(pScene, pObject, *parseIter);
					}
				}
			}
		}
		


	}
	catch (const std::exception &e)
	{
		printf("error while reading xml - %s\n", e.what());
		lStatus = false;
	}

	// free mem

	FreeParsingShadersVector(xmlShaders);

	if (pDoc)
	{
		delete pDoc;
		pDoc = nullptr;
	}
	
	return lStatus;
}