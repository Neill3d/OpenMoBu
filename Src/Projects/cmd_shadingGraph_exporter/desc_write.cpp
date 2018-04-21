
// desc_write.cpp
// Sergei <Neill3d> Solokhin 2018

#include "desc.h"
#include "tinyxml.h"

TiXmlDocument	*gDoc = nullptr;
TiXmlElement	*gShaders = nullptr;
TiXmlElement	*gModels = nullptr;
int				gNumberOfShaders = 0;
int				gNumberOfModels = 0;

/////////////////////////////////////////////////////////////////////////////////////////////
// XmlBeginExport

void XmlBeginExport(const char *fbxFilename)
{
	gDoc = new TiXmlDocument();

	TiXmlDeclaration	*decl = new TiXmlDeclaration("1.0", "", "");
	gDoc->LinkEndChild(decl);

	TiXmlElement *pShadersGraph = new TiXmlElement("ShadersGraph");
	pShadersGraph->SetAttribute("FileName", fbxFilename);
	// DONE: add attributes with file size and date

	__int64	fileSize;
	TCHAR buffer[256] = { 0 };
	FileSizeAndDate(fbxFilename, fileSize, buffer, 256);

	pShadersGraph->SetAttribute("FileSize", (int)fileSize);
	pShadersGraph->SetAttribute("LastWrite", buffer);

	gDoc->LinkEndChild(pShadersGraph);

	gShaders = new TiXmlElement("Shaders");
	gNumberOfShaders = 0;

	gModels = new TiXmlElement("Models");
	gNumberOfModels = 0;

	pShadersGraph->LinkEndChild(gShaders);
	pShadersGraph->LinkEndChild(gModels);
}

bool XmlWriteShader(FbxObject *pShader)
{
	if (nullptr == gDoc || nullptr == gShaders)
		return false;

	FbxProperty subTypeProp = pShader->FindProperty("MoBuSubTypeName");

	if (false == subTypeProp.IsValid())
		return false;

	FbxString subTypeStr(subTypeProp.Get<FbxString>());

	TiXmlElement *shaderelem = new TiXmlElement("Shader");
	shaderelem->SetAttribute("ClassName", subTypeStr.Buffer());
	shaderelem->SetAttribute("Name", pShader->GetNameOnly());
	shaderelem->SetAttribute("LongName", pShader->GetName());

	//
	FbxString type("Unknown");
	shaderelem->SetAttribute("Type", type.Buffer());

	bool isSystem = pShader->GetObjectFlags(FbxObject::eSystem);
	if (0 == strcmp("Default Shader", pShader->GetName()))
		isSystem = true;

	shaderelem->SetAttribute("System", (isSystem) ? "True" : "False");

	// write shader attachments
	int dstCount = pShader->GetDstObjectCount();
	int realDstCount = 0;

	TiXmlElement *attachelem = new TiXmlElement("Attachments");
	shaderelem->LinkEndChild(attachelem);

	for (int i = 0; i < dstCount; ++i)
	{
		FbxObject *pObject = pShader->GetDstObject(i);

		FbxString className(pObject->GetClassId().GetName());
		FbxProperty objSubTypeProp = pObject->FindProperty("MoBuTypeName");

		if (objSubTypeProp.IsValid())
			className = objSubTypeProp.Get<FbxString>();

		if (0 == strcmp("FbxScene", className.Buffer()))
			continue;

		TiXmlElement *dstelem = new TiXmlElement("Dst");
		attachelem->LinkEndChild(dstelem);

		dstelem->SetAttribute("ClassName", className);

		dstelem->SetAttribute("Name", pObject->GetNameOnly());
		dstelem->SetAttribute("LongName", pObject->GetName());

		realDstCount += 1;
	}

	attachelem->SetAttribute("Count", realDstCount);

	// write properties
	TiXmlElement *propselem = new TiXmlElement("Properties");
	shaderelem->LinkEndChild(propselem);

	int numberOfExportedProps = 0;

	int ivalue;
	double dvalue;
	//double v[4];
	FbxString text;
	FbxDouble2 v2;
	FbxDouble3 v3;
	FbxDouble4 v4;

	FbxProperty prop = pShader->GetFirstProperty();

	for (; prop.IsValid(); prop = pShader->GetNextProperty(prop))
	{
		const char *propName = prop.GetNameAsCStr();

		const char *strDataType = prop.GetPropertyDataType().GetName();
		EFbxType dataType = prop.GetPropertyDataType().GetType();

		// skip system, hidden or reference properties
		if (true == prop.GetFlag(FbxPropertyFlags::eHidden))
			continue;

		// skip mobu internal types
		if (0 == strcmp("MoBuTypeName", propName) || 0 == strcmp("MoBuSubTypeName", propName)
			|| 0 == strcmp("MoBuObjectFullName", propName) || 0 == strcmp("MoBuAttrBlindData", propName) || 0 == strcmp("MoBuRelationBlindData", propName))
		{
			continue;
		}

		TiXmlElement *pelem = new TiXmlElement("Property");
		propselem->LinkEndChild(pelem);

		pelem->SetAttribute("Name", propName);
		pelem->SetAttribute("Type", strDataType);

		switch (dataType)
		{
		case eFbxBool:
			pelem->SetAttribute("Value", (true == prop.Get<FbxBool>()) ? "True" : "False");
			break;

		case eFbxInt:
			ivalue = prop.Get<FbxInt>();
			pelem->SetAttribute("Value", ivalue);
			break;

		case eFbxEnum:
			ivalue = prop.Get<FbxEnum>();
			pelem->SetAttribute("Value", ivalue);
			break;

		case eFbxDouble:
			dvalue = prop.Get<FbxDouble>();
			if (dvalue == dvalue)	// check for NAN
				pelem->SetDoubleAttribute("Value", dvalue);
			else
				pelem->SetDoubleAttribute("Value", 0.0);
			break;

		case eFbxDouble2:
			v2 = prop.Get<FbxDouble2>();
			pelem->SetDoubleAttribute("X", v2[0]);
			pelem->SetDoubleAttribute("Y", v2[1]);
			break;

		case eFbxDouble3:
			v3 = prop.Get<FbxDouble3>();
			pelem->SetDoubleAttribute("X", v3[0]);
			pelem->SetDoubleAttribute("Y", v3[1]);
			pelem->SetDoubleAttribute("Z", v3[2]);
			break;

		case eFbxDouble4:
			v4 = prop.Get<FbxDouble4>();
			pelem->SetDoubleAttribute("X", v4[0]);
			pelem->SetDoubleAttribute("Y", v4[1]);
			pelem->SetDoubleAttribute("Z", v4[2]);
			pelem->SetDoubleAttribute("W", v4[3]);
			break;

		case eFbxString:
			text = prop.Get<FbxString>();
			pelem->SetAttribute("Text", text.Buffer());
			break;
		}

		// write property connectors (models, textures, etc. like FBPropertyObjectList)

		const int srcCount = prop.GetSrcObjectCount();

		if (srcCount > 0)
		{
			TiXmlElement *connelem = new TiXmlElement("Connections");
			
			connelem->SetAttribute("Count", srcCount);
			pelem->LinkEndChild(connelem);

			for (int j = 0; j < srcCount; ++j)
			{
				FbxObject *pConn = prop.GetSrcObject(j);
				
				FbxString className(pConn->GetClassId().GetName());
				FbxProperty objSubTypeProp = pConn->FindProperty("MoBuTypeName");

				if (objSubTypeProp.IsValid())
					className = objSubTypeProp.Get<FbxString>();

				TiXmlElement *srcelem = new TiXmlElement("Source");
				connelem->LinkEndChild(srcelem);

				srcelem->SetAttribute("ClassName", className);

				srcelem->SetAttribute("Name", pConn->GetNameOnly());
				srcelem->SetAttribute("LongName", pConn->GetName());
			}
		}

		numberOfExportedProps += 1;
	}

	propselem->SetAttribute("Count", numberOfExportedProps);

	if (nullptr != shaderelem)
	{
		gShaders->LinkEndChild(shaderelem);
		gNumberOfShaders += 1;
	}

	return true;
}

bool XmlWriteModel(FbxNode *pModel)
{
	if (nullptr == gDoc || nullptr == gModels)
		return false;

	bool lShow = pModel->Show.Get();
	bool lVis = (1.0 == pModel->Visibility.Get());

	FbxString className(pModel->GetClassId().GetName());
	FbxProperty objSubTypeProp = pModel->FindProperty("MoBuTypeName");

	if (objSubTypeProp.IsValid())
		className = objSubTypeProp.Get<FbxString>();

	TiXmlElement *modelelem = new TiXmlElement("Model");
	modelelem->SetAttribute("ClassName", className.Buffer());
	modelelem->SetAttribute("Name", pModel->GetNameOnly());
	modelelem->SetAttribute("LongName", pModel->GetName());
	modelelem->SetAttribute("Show", (lShow) ? "True" : "False");
	modelelem->SetAttribute("Visibility", (lVis) ? "True" : "False");

	// TODO: store local transform for a future feature of backing edit back to the ref scene

	gModels->LinkEndChild(modelelem);
	gNumberOfModels += 1;

	return true;
}

void XmlEndExport(const char *filename)
{
	if (nullptr != gDoc)
	{
		gShaders->SetAttribute("Count", gNumberOfShaders);
		gModels->SetAttribute("Count", gNumberOfModels);

		gDoc->SaveFile(filename);

		delete gDoc;
		gDoc = nullptr;
		gShaders = nullptr;
	}
}