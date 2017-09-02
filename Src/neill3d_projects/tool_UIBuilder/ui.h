
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////


/**	\file	ui.h

	Author - Sergey Solohin (Neill)
	homepage - http://neill3d.com/tool-programming-in-mobu/ui-builder?langswitch_lang=en
*/

#ifndef		UI_H_
#define		UI_H_

#include <fbsdk/fbsdk.h>

///////////////////////////////// define types

#define		MAX_UI_RECTS			100
#define		MAX_CODE_LENGTH		50

enum RegionAction {raNone, raMove, raMoveX, raMoveY, raSize, raSizeW, raSizeH, raRegion};
enum RegionState {rsNone, rsSelect, rsDelete};

#define		REGION_DIFF_NAME	0x0001
#define		REGION_DIFF_TITLE	0x0010
#define		REGION_DIFF_STATE	0x0100
#define		REGION_DIFF_SIZE	0x1000

// tool properties
#define	TOOL_CELL_TITLE				0
#define	TOOL_CELL_SIZEX				1
#define	TOOL_CELL_SIZEY				2

// region properties
#define	PROP_CELL_TITLE						0
#define PROP_CELL_X								1
#define PROP_CELL_X_ATTACH_TYPE		2
#define PROP_CELL_X_ATTACH_NODE		3
#define PROP_CELL_X_MULT					4
#define PROP_CELL_Y								5
#define PROP_CELL_Y_ATTACH_TYPE		6
#define PROP_CELL_Y_ATTACH_NODE		7
#define PROP_CELL_Y_MULT					8
#define PROP_CELL_W								9
#define PROP_CELL_W_ATTACH_TYPE		10
#define PROP_CELL_W_ATTACH_NODE		11
#define PROP_CELL_W_MULT					12
#define PROP_CELL_H								13
#define PROP_CELL_H_ATTACH_TYPE		14
#define PROP_CELL_H_ATTACH_NODE		15
#define PROP_CELL_H_MULT					16

// general properties
#define	GENERAL_CELL_VISIBLE			0
#define	GENERAL_CELL_READONLY			1
#define	GENERAL_CELL_ENABLED			2
#define	GENERAL_CELL_HINT					3

// button properties
#define	BUTTON_CELL_CAPTION				4
#define BUTTON_CELL_STATE					5
#define	BUTTON_CELL_STYLE					6
#define	BUTTON_CELL_JUSTIFY				7
#define	BUTTON_CELL_LOOK					8

// label properties
#define	LABEL_CELL_CAPTION				4
#define	LABEL_CELL_STYLE					5
#define	LABEL_CELL_JUSTIFY				6
#define	LABEL_CELL_WORDWRAP				7

// edit properties
#define	EDIT_CELL_TEXT						4
#define	EDIT_CELL_PASSWORD				5

// edit number properties
#define	EDIT_NUM_CELL_VALUE				4
#define	EDIT_NUM_CELL_MIN					5
#define	EDIT_NUM_CELL_MAX					6
#define	EDIT_NUM_CELL_PRECISION		7
#define	EDIT_NUM_CELL_LARGESTEP		8
#define	EDIT_NUM_CELL_SMALLSTEP		9

// edit color properties
#define	EDIT_COLOR_CELL_MODE			4
#define	EDIT_COLOR_CELL_VALUE			5

// edit vector properties
#define	EDIT_VECTOR_CELL_VALUE		4

// list properties
#define	LIST_CELL_ITEMINDEX				4
#define	LIST_CELL_MULTISELECT			5
#define	LIST_CELL_EXTENDEDSELECT	6
#define	LIST_CELL_STYLE						7

// container properties
#define	CONTAINER_CELL_ITEMINDEX		4
#define	CONTAINER_CELL_ITEMWRAP			5
#define	CONTAINER_CELL_ICONPOSITION	6
#define	CONTAINER_CELL_ITEMWIDTH		7
#define	CONTAINER_CELL_ITEMHEIGHT		8

// spread properties
#define	SPREAD_CELL_CAPTION					4
#define	SPREAD_CELL_MULTISELECT			5

// thermometer properties
#define	THERMOMETER_CELL_MIN				4
#define	THERMOMETER_CELL_MAX				5
#define	THERMOMETER_CELL_VALUE			6

// slider properties
#define	SLIDER_CELL_MIN							4
#define	SLIDER_CELL_MAX							5
#define	SLIDER_CELL_VALUE						6
#define	SLIDER_CELL_ORIENTATION			7
#define	SLIDER_CELL_LARGESTEP				8
#define	SLIDER_CELL_SMALLSTEP				9

// tree properties
#define	TREE_CELL_MULTIDRAG							4
#define	TREE_CELL_AUTOSCROLL						5
#define	TREE_CELL_CHECKBOXES						6
#define	TREE_CELL_SHOWLINES							7
#define	TREE_CELL_ITEMHEIGHT						8
#define	TREE_CELL_MULTISELECT						9
#define	TREE_CELL_EDITNODE							10
#define	TREE_CELL_SELECTIONACTIVE				11
#define	TREE_CELL_DESELECTONCOLLAPSE		12
#define	TREE_CELL_NOSELECTONDRAG				13
#define	TREE_CELL_NOSELECTONRIGHTCLICK	14
#define	TREE_CELL_HIGHLIGHTONRIGHTCLICK	15
#define	TREE_CELL_AUTOSCROLLONEXPAND		16
#define	TREE_CELL_AUTOEXPANDONDRAGOVER	17
#define	TREE_CELL_AUTOEXPANDONDBLCLICK	18

// time code properties
#define	TIME_CODE_VALUE							4

// layout region properties
#define	LAYOUT_REGION_CELL_CAPTION				4
#define	LAYOUT_REGION_CELL_INSET					5
#define	LAYOUT_REGION_CELL_BORDER					6
#define	LAYOUT_REGION_CELL_WIDTH					7
#define	LAYOUT_REGION_CELL_SPACING				8
#define	LAYOUT_REGION_CELL_MAXANGLE				9
#define	LAYOUT_REGION_CELL_CORNERRADIUS		10

// visual component types
enum
{
	COMPONENT_CLEAR,		// it's not a component, just special flag
	// define component list
	#define COMPCLASS(name, objName) name,
	#include "components_list.h"
	#undef COMPCLASS
};

///////////////////////////////// define classes & structs

// 
struct	uielement
{
	FBLayoutRegion		element;
};


// whole tool layout
struct	uilayout
{
	FBArrayTemplate<uilayout>	layouts;
	FBArrayTemplate<uielement>	elements;

private:

	FBLayout		layout;


public:
	uilayout(FBLayout &maintool, FBLayout &root)
	{
		
		
	}
};

/////////////////////////////////////////////////////////////////////////////
// ControlElement
/////////////////////////////////////////////////////////////////////////////
struct ControlElement
{
public:
	void SetRow(int _row)
	{
		row = _row;
	}
	int GetRow() {
		return row;
	}

private:
	int row;
};

enum controltype {	
		ctString,
		ctInteger,
		ctDouble,
		ctTime,
		ctVector,
		ctColor,
		ctBoolean
};

union controlvalue
{
	bool bvalue;
	int ivalue;
	double dvalue;
	kLongLong	tvalue;	// time code
};

/////////////////////////////////////////////////////////////////////////////
// ControlParam
/////////////////////////////////////////////////////////////////////////////
struct ControlParam : public ControlElement
{
public:

	ControlParam()
	{
		SetRow(0);
	}
	ControlParam(int _row, ControlParam::controltype	_type)
	{
		SetRow(_row);
		type = _type;
	}
	void Init (char* _name, int _row, ControlParam::controltype	_type, FBString svalue)
	{
		name = _name;
		SetRow(_row);
		type = _type;
		szbuffer = svalue;
		defaultBuffer = szbuffer;
	}
	void Init (char* _name, int _row, ControlParam::controltype	_type, int ivalue, char *_enumText = 0)
	{
		name = _name;
		SetRow(_row);
		type = _type;
		if (type == ctBoolean)
			value.bvalue = (ivalue != 0);
		else
			value.ivalue = ivalue;
		defaultValue = value;
		enumText = _enumText;
	}
	void Init (char* _name, int _row, ControlParam::controltype	_type, double dvalue)
	{
		name = _name;
		SetRow(_row);
		type = _type;
		value.dvalue = dvalue;
		defaultValue.dvalue = dvalue;
	}
	void Init (char* _name, int _row, ControlParam::controltype	_type, FBVector4d vvalue)
	{
		name = _name;
		SetRow(_row);
		type = _type;
		vector = vvalue;
		defaultVector = vvalue;
	}

	void Clear()
	{
		szbuffer = defaultBuffer;
		value = defaultValue;
		vector = defaultVector;
	}

	void FillPropTable(FBSpread &proptable)
	{
		switch(type)
		{
		case ctString:	proptable.SetCell(GetRow(), 0, szbuffer); break;
		case ctInteger:	proptable.SetCell(GetRow(), 0, value.ivalue); break;
		case ctDouble:	proptable.SetCell(GetRow(), 0, value.dvalue); break;
		case ctTime:		proptable.SetCell(GetRow(), 0, value.tvalue); break;
		case ctColor:
		case ctVector:	
			proptable.SetCell(GetRow(), 0, vector[0] );				//	x
			proptable.SetCell(GetRow()+1, 0, vector[1] );			// y
			proptable.SetCell(GetRow()+2, 0, vector[2] );			//	z
			proptable.SetCell(GetRow()+3, 0, vector[3] );			//	a
			break;
		case ctBoolean:	proptable.SetCell(GetRow(), 0, value.bvalue); break;
		}
	}
	void PropTableChange(FBSpread &proptable)
	{
		switch(type)
		{
		case ctString:	
			{
				const char *lBuffer;
				proptable.GetCell(GetRow(), 0, lBuffer);
				szbuffer = lBuffer;
			}break;
		case ctInteger:	proptable.GetCell(GetRow(), 0, value.ivalue); break;
		case ctDouble:	proptable.GetCell(GetRow(), 0, value.dvalue ); break;
		case ctColor:
		case ctVector:
			proptable.GetCell(GetRow(), 0, vector[0] );				//	x
			proptable.GetCell(GetRow()+1, 0, vector[1] );			// y
			proptable.GetCell(GetRow()+2, 0, vector[2] );			//	z
			proptable.GetCell(GetRow()+3, 0, vector[3] );			//	a
			break;
		case ctTime:	proptable.GetCell(GetRow(), 0, value.tvalue ); break;
		case ctBoolean:	
			{
				int v;
				proptable.GetCell(GetRow(), 0, v ); 
				value.bvalue = (v != 0);
			}break;
		}
	}

	FBString GetString()
	{
		FBString s("empty");
		char *text = (char*)szbuffer;
		if (text)
			s = szbuffer;
		return s;
	}
	int& GetInt()
	{
		return value.ivalue;
	}
	double& GetDouble()
	{
		return value.dvalue;
	}
	FBTime GetTime()
	{
		FBTime	time;
		time.SetMilliSeconds(value.tvalue);
		return time;
	}
	FBVector4d	GetVector() const
	{
		return vector;
	}
	bool&	GetBoolean()
	{
		return value.bvalue;
	}

	//! export param value to the python format
	void ExportPython(FILE *f, FBString &regionName)
	{
		switch(type)
		{
		case ctString:
			fprintf( f, "\t%s.%s = \"%s\"\n", (char*)regionName, name, (char*)szbuffer );
			break;
		case ctInteger:
			{
				if (enumText.GetLen() > 0)
				{	
					int n=0;
					int pos=0;
					FBString s(enumText);
					while ( (pos = s.Find('~')) > 0)
					{
						if (n==value.ivalue)
						{
							s = s.Left(pos);
							fprintf( f, "\t%s.%s = %s\n", (char*)regionName, name, (char*)s );
						}
						else
							s = s.Mid(pos+1, s.GetLen() );
						n++;
					}
					if (n==value.ivalue)
						fprintf( f, "\t%s.%s = %s\n", (char*)regionName, name, (char*)s );
				}
				else	fprintf( f, "\t%s.%s = %d\n", (char*)regionName, name, value.ivalue );
			} break;
		case ctDouble:
			fprintf( f, "\t%s.%s = %f\n", (char*)regionName, name, value.dvalue );
			break;
		case ctColor:
			fprintf( f, "\t%s.%s = FBColor(%f, %f, %f, %f)\n", (char*)regionName, name, vector[0], vector[1], vector[2], vector[3]);
			break;
		case ctVector:
			fprintf( f, "\t%s.%s = FBVector3d(%f, %f, %f)\n", (char*)regionName, name, vector[0], vector[1], vector[2]);
			break;
		case ctTime:
			fprintf( f, "\t%s.%s = %d\n", (char*)regionName, name, value.tvalue );
			break;
		case ctBoolean:
			if (value.bvalue)
				fprintf( f, "\t%s.%s = True\n", (char*)regionName, name );
			else
				fprintf( f, "\t%s.%s = False\n", (char*)regionName, name );
			break;
		}
	}
	//! export param value to the c++ format
	void ExportCPP(FILE *f, FBString &regionName)
	{
		switch(type)
		{
		case ctString:
			fprintf( f, "%s.%s = \"%s\";\n", (char*)regionName, name, (char*)szbuffer );
			break;
		case ctInteger:
			{
				if (enumText.GetLen() > 0)
				{	
					int n=0;
					int pos=0;
					FBString s(enumText);
					while ( (pos = s.Find('~')) > 0)
					{
						if (n==value.ivalue)
						{
							s = s.Left(pos);
							fprintf( f, "%s.%s = %s;\n", (char*)regionName, name, (char*)s );
						}
						else
							s = s.Mid(pos+1, s.GetLen() );
						n++;
					}
					if (n==value.ivalue)
						fprintf( f, "%s.%s = %s;\n", (char*)regionName, name, (char*)s );
				}
				else	fprintf( f, "%s.%s = %d;\n", (char*)regionName, name, value.ivalue );
			} break;
		case ctDouble:
			fprintf( f, "%s.%s = %f;\n", (char*)regionName, name, value.dvalue );
			break;
		case ctColor:
			fprintf( f, "%s.%s = FBColor(%f, %f, %f, %f);\n", (char*)regionName, name, vector[0], vector[1], vector[2], vector[3] );
			break;
		case ctVector:
			fprintf( f, "%s.%s = FBVector3d(%f, %f, %f);\n", (char*)regionName, name, vector[0], vector[1], vector[2] );
			break;
		case ctTime:
			fprintf( f, "%s.%s = %d;\n", (char*)regionName, name, value.tvalue );
			break;
		case ctBoolean:
			if (value.bvalue)
				fprintf( f, "%s.%s = true;\n", (char*)regionName, name );
			else
				fprintf( f, "%s.%s = false;\n", (char*)regionName, name );
			break;
		}
	}
	//! load param value from file
	void Load(FILE *f)
	{
		int v;
		char buffer[2048];
		char	ptr[256];

		// read name & title
		memset(buffer,0,sizeof(char)*2048);
		fgets(buffer, 2048, f);
		sscanf(buffer, "%d", &v );
		type = (controltype) v;

		memset(buffer,0,sizeof(char)*2048);
		memset(ptr, 0, sizeof(char)*256);
		fgets(buffer, 2048, f);
		sscanf(buffer, "%s", ptr );
		name = ptr;

		memset(buffer,0,sizeof(char)*2048);
		memset(ptr, 0, sizeof(char)*256);
		fgets(buffer, 2048, f);
		switch(type)
		{
		case ctString:
			//sscanf(buffer, "%s", ptr );
			if (strlen(buffer) > 0)
			{
				memcpy(ptr, buffer, sizeof(char)*strlen(buffer)-1);
				szbuffer = ptr;
			}
			else
				szbuffer = "";
			break;
		case ctInteger:
			sscanf(buffer, "%d", &value.ivalue );
			memset(buffer,0,sizeof(char)*2048);
			memset(ptr, 0, sizeof(char)*256);
			fgets(buffer, 2048, f);
			sscanf(buffer, "%s", ptr );
			enumText = ptr;
			break;
		case ctDouble:
			sscanf(buffer, "%lf", &value.dvalue );
			break;
		case ctColor:
		case ctVector:
			sscanf(buffer, "%lf %lf %lf %lf", &vector[0], &vector[1], &vector[2], &vector[3] );
			break;
		case ctTime:
			sscanf(buffer, "%d", &value.tvalue );
			break;
		case ctBoolean:
			{
				int v;
				sscanf(buffer, "%d", &v );
				value.bvalue = (v != 0);
			} break;
		}
	}
	//! put param value into the file
	void Save(FILE *f)
	{
		fprintf(f, "%d\n", (int)type);
		fprintf(f, "%s\n", (char*)name);

		switch(type)
		{
		case ctString:
			fprintf( f, "%s\n", (char*)szbuffer );
			break;
		case ctInteger:
			fprintf( f, "%d\n", value.ivalue );
			if (enumText.GetLen())
				fprintf( f, "%s\n", (char*)enumText );
			else
				fprintf( f, "\n" );
			break;
		case ctDouble:
			fprintf( f, "%f\n", value.dvalue );
			break;
		case ctColor:
		case ctVector:
			fprintf( f, "%f %f %f %f\n", vector[0], vector[1], vector[2], vector[3]);
			break;
		case ctTime:
			fprintf( f, "%d\n", value.tvalue );
			break;
		case ctBoolean:
			{
				if (value.bvalue)
					fprintf( f, "1\n" );
				else
					fprintf( f, "0\n" );
			}break;
		}
	}

	FBString GetParamName() const
	{
		return name;
	}
	controltype GetParamType() const
	{
		return type;
	}
	FBString	GetParamBuffer() const
	{
		return szbuffer;
	}
	controlvalue	GetParamValue() const
	{
		return value;
	}
	FBString	GetParamEnumText() const
	{
		return enumText;
	}
	FBString GetDefaultBuffer() const
	{
		return defaultBuffer;
	}
	controlvalue GetDefaultValue() const
	{
		return defaultValue;
	}
	FBVector4d	GetDefaultVector() const
	{
		return defaultVector;
	}

	ControlParam &operator = (const ControlParam &cParam)
	{
		name = cParam.GetParamName();
		type = cParam.GetParamType();
		szbuffer = cParam.GetParamBuffer();
		value = cParam.GetParamValue();
		vector = cParam.GetVector();
		enumText = cParam.GetParamEnumText();
		defaultBuffer = cParam.GetDefaultBuffer();
		defaultValue = cParam.GetDefaultValue();
		defaultVector = cParam.GetDefaultVector();

		return *this;
	}

private:
	//! parameter name
	FBString			name;
	//! parameter type (string, int, double, boolean, time, vector)
	controltype		type;
	//! text value buffer
	FBString			szbuffer;
	//! int, double, boolean, time values
	controlvalue	value;
	//! enumuarate for values
	FBString			enumText;
	//! vector, color values
	FBVector4d		vector;

	//! values for reset state
	FBString			defaultBuffer;
	FBVector4d		defaultVector;
	controlvalue	defaultValue;
};


/////////////////////////////////////////////////////////////////////////////
// ControlEvent
/////////////////////////////////////////////////////////////////////////////
struct ControlEvent
{
public:
	FBString			eventName;
	
	int						codeLength;
	FBString			eventCode[MAX_CODE_LENGTH];

	bool					mSpecial;

	ControlEvent()
		: codeLength(0)
	{mSpecial=false;}
	ControlEvent(FBString _eventName)
		: eventName(_eventName)
		, codeLength(0)
	{mSpecial=false;}

	FBString GetName()
	{
		return eventName;
	}
	void SetName(FBString name)
	{
		eventName = name;
	}
	void Save(FILE *f)
	{
		fprintf(f, "%d\n", codeLength);
		for (int i=0; i<codeLength; i++)
		{
			char *szCode = eventCode[i];
			fprintf( f, "%s\n", szCode );
		}
	}
	void Load(FILE *f)
	{
		int v;
		char buffer[2048];
		//char	ptr[256];

		// read name & title
		memset(buffer,0,sizeof(char)*2048);
		fgets(buffer, 2048, f);
		sscanf(buffer, "%d", &v );
		codeLength = v;

		for (int i=0; i<codeLength; i++)
		{
			memset(buffer,0,sizeof(char)*2048);
			fgets(buffer, 2048, f);
			for (int j=0; j<2048; j++)
				if (buffer[j] == '\n') buffer[j] = 0;
			eventCode[i] = buffer;
		}
	}
	//! export callback function in python format
	void ExportPython(FILE *f)
	{
		if (codeLength > 0)
		{
			fprintf( f, "def %s(control, event):\n", (char*) eventName );
			for (int i=0; i<codeLength; i++)
				if (mSpecial)
					fprintf( f, "%s\n", (char*) eventCode[i] );
				else
					fprintf( f, "\t%s\n", (char*) eventCode[i] );
			fprintf( f, "\n" );
		}
	}
	//! export callback function in python format
	void ExportCPP(FILE *f)
	{
		if (codeLength > 0)
		{
			fprintf( f, "void %s( HISender pSender, HKEvent pEvent )\n", (char*) eventName );
			fprintf( f, "{\n" );
			for (int i=0; i<codeLength; i++)
				fprintf( f, "%s\n", (char*) eventCode[i] );
			fprintf( f, "}\n" );
		}
	}
	//! write each line of event code into the file
	void ExportPythonEvents(FILE *f)
	{
		if (codeLength > 0)
		{
			for (int i=0; i<codeLength; i++)
				if (mSpecial)
					fprintf( f, "%s\n", (char*) eventCode[i] );
				else
					fprintf( f, "\t%s\n", (char*) eventCode[i] );
			fprintf( f, "\n" );
		}
	}
	//! write each line of event code into the file
	void ExportCPPEvents(FILE *f)
	{
		if (codeLength > 0)
		{
			for (int i=0; i<codeLength; i++)
				fprintf( f, "%s\n", (char*) eventCode[i] );
			fprintf( f, "\n" );
		}
	}
	//! add callback to the visual component
	void ExportPythonCallback(FILE *f, FBString &componentName, FBString prefix)
	{
		fprintf( f, "\t%s.%s.Add(%sCallback%s)\n", (char*)componentName, (char*)eventName, (char*)prefix, (char*)componentName);
	}
	//! add callback to the visual component
	void ExportCPPCallback(FILE *f, FBString &componentName, FBString prefix)
	{
		fprintf( f, "%s.%s.Add( this, (FBCallback) %sEvent%s);\n", (char*)componentName, (char*)eventName, (char*)prefix, (char*)componentName);
	}

	//! get stringlist of event code
	void GetCode(FBStringList &list)
	{
		//list = eventCode;
		list.Clear();
		for (int i=0; i<codeLength; i++)
			list.Add( eventCode[i] );
	}
	//! put stringlist into the event code
	void SetCode(FBStringList *pList)
	{
		//eventCode = *pList;
		codeLength = pList->GetCount();
		if (codeLength > MAX_CODE_LENGTH)
			codeLength = MAX_CODE_LENGTH;
		for (int i=0; i<codeLength; i++)
			eventCode[i] = pList->GetAt(i);
	}

	ControlEvent &operator = (const ControlEvent &cEvent)
	{
		eventName = cEvent.eventName;
		codeLength = cEvent.codeLength;

		for (int i=0; i<codeLength; i++)
			eventCode[i] = cEvent.eventCode[i];

		return *this;
	}
};

/////////////////////////////////////////////////////////////////////////////
// ControlEventList
/////////////////////////////////////////////////////////////////////////////
struct	ControlEventList
{
public:
	//! save events to file
	void SaveEvents(FILE *f)
	{
		for (int i=0; i<events.GetCount(); i++)
			events[i].Save(f);
	}
	//! load events from file
	void LoadEvents(FILE *f)
	{
		for (int i=0; i<events.GetCount(); i++)
			events[i].Load(f);
	}
	
	//! export events in python format
	void ExportPythonEvents(FILE *f, FBString &componentName, FBString prefix)
	{
		int test = 0;
		for (int i=0; i<events.GetCount(); i++)
				test += (events[i].codeLength > 0);

		if (test) {
			
			for (int i=0; i<events.GetCount(); i++)
				if (events[i].codeLength)
				{
					// event header
					fprintf( f, "def %sCallback%s(control, event):\n", (char*) prefix, (char*) componentName );
					// event body
					events[i].ExportPythonEvents(f);
				}
			fprintf( f, "\n" );
		}
	}
	void ExportPythonEventsSpecial(FILE *f)
	{
		if (events.GetCount() != 2) return;

		// tool declaration code
		if (events[0].codeLength)
		{
			// event body
			events[0].ExportPythonEvents(f);
		}

		// tool _on show
		if (events[1].codeLength)
		{
			// event header
			fprintf( f, "def OnShow():\n" );
			// event body
			events[1].ExportPythonEvents(f);
		}
	}

	//! export events in cpp format
	void ExportCPPEvents(FILE *f, FBString &componentName, FBString prefix)
	{
		int test = 0;
		for (int i=0; i<events.GetCount(); i++)
				test += (events[i].codeLength > 0);

		if (test) {
			fprintf( f, "void %sEvent%s( HISender pSender, HKEvent pEvent )\n", (char*) prefix, (char*) componentName );
			fprintf( f, "{\n" );
			for (int i=0; i<events.GetCount(); i++)
					events[i].ExportCPPEvents(f);
			fprintf( f, "}\n" );
		}
	}
	//! export callbacks in python format
	void ExportPythonCallbacks(FILE *f, FBString &componentName, FBString prefix)
	{
		for (int i=0; i<events.GetCount(); i++)
			if (events[i].codeLength > 0)
				events[i].ExportPythonCallback(f, componentName, prefix);
	}
	//! export callbacks in cpp format
	void ExportCPPCallbacks(FILE *f, FBString &componentName, FBString prefix)
	{
		for (int i=0; i<events.GetCount(); i++)
			if (events[i].codeLength > 0)
				events[i].ExportCPPCallback(f, componentName, prefix);
	}

	//! set events from events list
	void FillEvents(FBList	*pEventsList)
	{
		if (pEventsList)
		{
			pEventsList->Items.Clear();
			for (int i=0; i<events.GetCount(); i++)
				pEventsList->Items.Add( events[i].GetName() );
		}
	}
	//! return stringlist of the eventName event
	void GetEventCode(const FBString &eventName, FBStringList &list)
	{
		for (int i=0; i<events.GetCount(); i++)
			if (eventName == events[i].GetName() )
				events[i].GetCode(list);
	}
	//! put stringlist code to the eventName event
	void SetEventCode(FBString eventName, FBStringList *pList)
	{
		for (int i=0; i<events.GetCount(); i++)
			if (eventName == events[i].GetName() )
				events[i].SetCode(pList);
	}
	//! allocate memory for count events
	void SetEventsCount(int count)
	{
		events.SetCount(count);
	}
	int GetEventsCount() const
	{
		return events.GetCount();
	}
	ControlEvent &GetEvent(int i) const
	{
		return events[i];
	}
	ControlEvent *GetEventPtr(int i)
	{
		return &events[i];
	}
	//! put name to the n event
	void SetEventName(int n, FBString name, bool special=false)
	{
		events[n].SetName(name);
		events[n].mSpecial = special;
	}
	//! has n event any code?
	bool isEventEmpty(int n) 
	{
		return (events[n].codeLength == 0);
	}
	//! clear all events code
	void ClearEvents()
	{
		FBStringList	lines;
		for (int i=0; i<events.GetCount(); i++)
			events[i].SetCode( &lines );
	}

	ControlEventList &operator = (const ControlEventList &eventList)
	{
		events.SetCount(eventList.GetEventsCount() );
		for (int i=0; i<events.GetCount(); i++)
			events[i] = eventList.GetEvent(i);

		return *this;
	}

private:
	//! events array
	FBArrayTemplate<ControlEvent>	events;
};

/////////////////////////////////////////////////////////////////////////////
// ControlManager
/////////////////////////////////////////////////////////////////////////////
struct ControlManager
{
public:

	//! get r parameter as string
	FBString GetString(int r)
	{
		return params[r].GetString();
	}
	//! get r param as interger value
	int& GetInt(int r)
	{
		return params[r].GetInt();
	}
	//! get r param as double value
	double& GetDouble(int r)
	{
		return params[r].GetDouble();
	}
	//! get r param as time value
	FBTime	GetTime(int r)
	{
		return params[r].GetTime();
	}
	FBVector4d	GetVector(int r)
	{
		return params[r].GetVector();
	}
	//! get r param as boolean value
	bool& GetBoolean(int r)
	{
		return params[r].GetBoolean();
	}

	//! update property table with values
	void ComponentCellChange(FBSpread &proptable, int c, int r) {
		for (int i=0; i<params.GetCount(); i++)
		{
			params[i].PropTableChange(proptable);
		}
	}
	//! prepare property table colomns
	void PrepareCompTable(FBSpread &proptable) {
		proptable.ColumnAdd( "Value", 0 );
		proptable.ColumnAdd( "Description", 1 );
	}
	//! update property table with values
	void FillCompTable(FBSpread &proptable) {
		for (int i=0; i<params.GetCount(); i++)
		{
			params.GetAt(i).FillPropTable(proptable);
		}
	}
	// event properties
	void EventCellChange(FBSpread &proptable, int c, int r ) {}
	void FillEventTable(FBSpread &proptable) {}

	//! preview for this visual component
	void SetControl(FBLayout *root, FBString &region) {}

	//! export params to python format
	void ExportPython(FILE *f, FBString &regionName)
	{
		for (int i=0; i<params.GetCount(); i++)
		{
			params[i].ExportPython(f, regionName);
		}
	}
	//! export params to the cpp format
	void ExportCPP(FILE *f, FBString &regionName)
	{
		for (int i=0; i<params.GetCount(); i++)
		{
			params[i].ExportCPP(f, regionName);
		}
	}

	//! clear params values
	void ClearParams()
	{
		for (int i=0; i<params.GetCount(); i++)
			params[i].Clear();
	}
	//! load params from file
	void Load(FILE *f)
	{
		for (int i=0; i<params.GetCount(); i++)
		{
			params[i].Load(f);
		}
	}
	//! save params into the file
	void Save(FILE *f)
	{
		for (int i=0; i<params.GetCount(); i++)
		{
			params[i].Save(f);
		}
	}

	void SetParamsCount(int count)
	{
		params.SetCount(count);
	}
	void SetParam(int Index, FBString Name, controltype Type, int iValue, FBString enumText)
	{
		params[Index].Init(Name, Index, Type, iValue, enumText);
	}
	void SetParam(int Index, FBString Name, controltype Type, FBString sValue, FBString enumText)
	{
		params[Index].Init(Name, Index, Type, sValue);
	}
	void SetParam(int Index, FBString Name, controltype Type, double dValue, FBString enumText)
	{
		params[Index].Init(Name, Index, Type, dValue);
	}
	void SetParam(int Index, FBString Name, controltype Type, FBVector4d	vvalue, FBString enumText)
	{
		params[Index].Init(Name, Index, Type, vvalue);
	}

	int GetParamsCount() const
	{
		return params.GetCount();
	}
	ControlParam &GetParam(int i) const
	{
		return params[i];
	}
	ControlParam *GetParamPtr(int i)
	{
		return &params[i];
	}
	ControlManager &operator = (const ControlManager &cManager)
	{
		params.SetCount( cManager.GetParamsCount() );
		for (int i=0; i<params.GetCount(); i++)
			params[i] = cManager.GetParam(i);

		return *this;
	}

private:
	//! element parameters
	FBArrayTemplate <ControlParam> params;
};

/////////////////////////////////////////////////////////////////////////////
// ControlBase
/////////////////////////////////////////////////////////////////////////////
//! abstract class with declaring functionality
struct	ControlBase : public	ControlManager, ControlEventList
{
public:
	virtual ~ControlBase()
	{}

	virtual char *GetComponentPrefix() = 0;
	virtual void Reset() = 0;
	virtual void ComponentCellChange(FBSpread &proptable, int c, int r ) = 0;
	virtual void PrepareCompTable(FBSpread &proptable) = 0;
	virtual void FillCompTable(FBSpread &proptable) = 0;
	virtual void SetControl(FBLayout	*root,	FBString &region) = 0;
	virtual void ExportPythonVar(FILE *f, FBString regionName) = 0;
	virtual void ExportCPPVar(FILE *f, FBString regionName) = 0;
	virtual void ExportPython(FILE *f, FBString regionName) = 0;
	virtual void ExportCPP(FILE *f, FBString regionName) = 0;

	ControlBase &operator = (const ControlBase &vComp)
	{
		// parameters
		SetParamsCount( vComp.GetParamsCount() );
		for (int i=0; i<GetParamsCount(); i++)
			*GetParamPtr(i) = vComp.GetParam(i);

		// events
		SetEventsCount( vComp.GetEventsCount() );
		for (int i=0; i<GetEventsCount(); i++)
			*GetEventPtr(i) = vComp.GetEvent(i);

		return *this;
	}
};


/////////////////////////////////////////////////////////////////////////////
// ControlButton
/////////////////////////////////////////////////////////////////////////////
/**	Define a parameter attributes.
*	\param	ParamIndex	index in paramerters array.
*	\param	Name		parameter call name.
*	\param	Type		param type (ctInteger, ctBoolean, ctDouble, ctTime, ctString)
* \param	Value		parameter default value
*	\param	enumText	string list to each parameter value
*/
#define DEFINE_CONTROL_PARAM( ParamIndex, ParamName, ParamType, ParamValue, enumText)   \
	SetParam(##ParamIndex, ##ParamName, ##ParamType, ##ParamValue, ##enumText)

#define	DEFINE_GENERAL_PARAMS() \
	SetParam(GENERAL_CELL_VISIBLE, "Visible", ctBoolean, true, "False~True");	\
	SetParam(GENERAL_CELL_ENABLED, "Enabled", ctBoolean, true, "False~True");	\
	SetParam(GENERAL_CELL_READONLY, "ReadOnly", ctBoolean, false, "False~True");	\
	SetParam(GENERAL_CELL_HINT, "Hint", ctString, "", "")

#define	GENERAL_CELL_CHANGES( Component ) \
	case GENERAL_CELL_VISIBLE:	##Component.Visible = GetBoolean(GENERAL_CELL_VISIBLE); break;	\
	case GENERAL_CELL_ENABLED:	##Component.Enabled = GetBoolean(GENERAL_CELL_ENABLED); break;	\
	case GENERAL_CELL_READONLY:	##Component.ReadOnly = GetBoolean(GENERAL_CELL_READONLY); break;	\
	case GENERAL_CELL_HINT:	##Component.Hint = GetString(GENERAL_CELL_HINT); break

#define	GENERAL_RAW_INSERT() \
	proptable.RowAdd( "Visible",			GENERAL_CELL_VISIBLE );	\
	proptable.RowAdd( "Enabled",			GENERAL_CELL_ENABLED );	\
	proptable.RowAdd( "ReadOnly",			GENERAL_CELL_READONLY );	\
	proptable.RowAdd( "Hint",					GENERAL_CELL_HINT );	\
	proptable.GetCell(GENERAL_CELL_VISIBLE, 0).Style = kFBCellStyleMenu;	\
	proptable.SetCell(GENERAL_CELL_VISIBLE, 0, "False~True" );	\
	proptable.GetCell(GENERAL_CELL_ENABLED, 0).Style = kFBCellStyleMenu;	\
	proptable.SetCell(GENERAL_CELL_ENABLED, 0, "False~True" );	\
	proptable.GetCell(GENERAL_CELL_READONLY, 0).Style = kFBCellStyleMenu;	\
	proptable.SetCell(GENERAL_CELL_READONLY, 0, "False~True" );	\
	proptable.GetCell(GENERAL_CELL_HINT, 0).Style = kFBCellStyleString

#define	GENERAL_SET_CONTROL( Component )	\
	Component.Visible = GetBoolean(GENERAL_CELL_VISIBLE);	\
	Component.Enabled = GetBoolean(GENERAL_CELL_ENABLED);	\
	Component.ReadOnly = GetBoolean(GENERAL_CELL_READONLY);	\
	Component.Hint = GetString(GENERAL_CELL_HINT)

#define	DEFINE_GENERAL_EVENTS() \
	SetEventName(0, "OnEnter");	\
	SetEventName(1, "OnExit")

//! class for controlling button over FBButton
struct ControlButton : public ControlBase
{
public:
	//! a constructor
	ControlButton()
	{
		SetParamsCount(9);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM(BUTTON_CELL_CAPTION, "Caption", ctString, "Button", "");
		DEFINE_CONTROL_PARAM(BUTTON_CELL_JUSTIFY, "Justify", ctInteger, 0,
			"FBTextJustify.kFBTextJustifyLeft~FBTextJustify.kFBTextJustifyRight~FBTextJustify.kFBTextJustifyCenter");
		DEFINE_CONTROL_PARAM(BUTTON_CELL_STATE, "State", ctInteger, 0, "");
		DEFINE_CONTROL_PARAM(BUTTON_CELL_STYLE, "Style", ctInteger, 0,
			"FBButtonStyle.kFBPushButton~FBButtonStyle.kFBBitmapButton~FBButtonStyle.kFBCheckbox~FBButtonStyle.kFBRadioButton~FBButtonStyle.kFB2States~FBButtonStyle.kFBBitmap2States");
		DEFINE_CONTROL_PARAM(BUTTON_CELL_LOOK, "Look", ctInteger, 0,
			"FBButtonLook.kFBLookNormal~FBButtonLook.kFBLookColorChange~FBButtonLook.kFBLookPush~FBButtonLook.kFBLookFlat~FBButtonLook.kFBLookAlphaBackground");

		SetEventsCount(4);
		DEFINE_GENERAL_EVENTS();
		SetEventName(2, "OnClick");
		SetEventName(3, "OnDragAndDrop");

		button = NULL;
	}
	
	//! a destructor
	~ControlButton()
	{}

	void DefineButton(FBButton *pButton)
	{
		button = pButton;
	}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "Btn";
	}

	//! reset component params and events
	void Reset()
	{
		ClearParams();
		ClearEvents();
	}
	//! update visual component params
	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);

		if (button)
		{
			FBButton &lButton = *button;
			switch(r)
			{
			GENERAL_CELL_CHANGES(lButton);
			case BUTTON_CELL_CAPTION:		lButton.Caption = GetString(BUTTON_CELL_CAPTION);	break;
			case BUTTON_CELL_JUSTIFY:		lButton.Justify = (FBTextJustify) GetInt(BUTTON_CELL_JUSTIFY);		break;
			case BUTTON_CELL_STATE:			lButton.State =  GetInt(BUTTON_CELL_STATE);	break;
			case BUTTON_CELL_STYLE:			lButton.Style = (FBButtonStyle) GetInt(BUTTON_CELL_STYLE);		break;
			case BUTTON_CELL_LOOK:			lButton.Look = (FBButtonLook) GetInt(BUTTON_CELL_LOOK);		break;
			}
		}
	}
	//! prepare visual component property table
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);

		GENERAL_RAW_INSERT();

		proptable.RowAdd( "Caption",			BUTTON_CELL_CAPTION );
		proptable.RowAdd( "State",				BUTTON_CELL_STATE );
		proptable.RowAdd( "Style",				BUTTON_CELL_STYLE );
		proptable.RowAdd( "Justify",			BUTTON_CELL_JUSTIFY );
		proptable.RowAdd( "Look",					BUTTON_CELL_LOOK );

		proptable.GetCell(BUTTON_CELL_CAPTION, 0).Style = kFBCellStyleString;
		proptable.GetCell(BUTTON_CELL_STATE, 0).Style = kFBCellStyleInteger;
		proptable.GetCell(BUTTON_CELL_JUSTIFY, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(BUTTON_CELL_JUSTIFY, 0, "Left~Right~Center" );
		proptable.GetCell(BUTTON_CELL_STYLE, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(BUTTON_CELL_STYLE, 0, "Push~Bitmap~Checkbox~Radio~2States~Bitmap2States" );
		proptable.GetCell(BUTTON_CELL_LOOK, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(BUTTON_CELL_LOOK, 0, "Normal~ColorChange~Push~Flat~AlphaBackground" );
	}
	//! fill data for the property table
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}
	//! set visual component (preview mode)
	void SetControl(FBLayout	*root,	FBString &region) {
		if (root->GetRegion(region) && button )
			root->SetControl(region, button);

		if (button)
		{
			FBButton &lButton = *button;
			GENERAL_SET_CONTROL(lButton);

			lButton.Caption = GetString(BUTTON_CELL_CAPTION);	
			lButton.Justify = (FBTextJustify) GetInt(BUTTON_CELL_JUSTIFY);		
			lButton.State =  GetInt(BUTTON_CELL_STATE);
			lButton.Style = (FBButtonStyle) GetInt(BUTTON_CELL_STYLE);		
			lButton.Look = (FBButtonLook) GetInt(BUTTON_CELL_LOOK);
		}
	}
	//! export component variable in python style
	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBButton()\n", (char*)regionName );
	}
	//! export component variable in cpp style
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBButton	%s;\n", (char*)regionName );
	}
	//! export python control init
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);

		ExportPythonCallbacks(f, regionName, GetComponentPrefix());
	}
	//! export control to the cpp code
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);

		ExportCPPCallbacks(f, regionName, GetComponentPrefix());
	}

	ControlButton	&operator = (const ControlButton &cbutton)
	{

		return *this;
	}

private:
	FBButton			*button;
};

/////////////////////////////////////////////////////////////////////////////
// ControlLabel
/////////////////////////////////////////////////////////////////////////////
//! class for controlling label over FBLabel
struct ControlLabel : public ControlBase
{
public:

	//! a constructor
	ControlLabel()
	{
		SetParamsCount(8);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM( LABEL_CELL_CAPTION, "Caption", ctString, "Label", "" );
		DEFINE_CONTROL_PARAM( LABEL_CELL_JUSTIFY, "Justify", ctInteger, 0,
			"FBTextJustify.kFBTextJustifyLeft~FBTextJustify.kFBTextJustifyRight~FBTextJustify.kFBTextJustifyCenter" );
		DEFINE_CONTROL_PARAM( LABEL_CELL_STYLE, "Style", ctInteger, 0,
			"FBTextStyle.kFBTextStyleNone~FBTextStyle.kFBTextStyleBold~FBTextStyle.kFBTextStyleItalic~FBTextStyle.kFBTextStyleUnderlined" );
		DEFINE_CONTROL_PARAM( LABEL_CELL_WORDWRAP, "WordWrap", ctBoolean, true,
			"True~False" );

		SetEventsCount(2);
		DEFINE_GENERAL_EVENTS();

		label = NULL;
	}

	//! a destructor
	~ControlLabel()
	{}

	void DefineLabel(FBLabel *pLabel)
	{
		label = pLabel;
	}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "Lbl";
	}

	void Reset()
	{
		ClearParams();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);
		if (label)
		{
			FBLabel &lLabel = *label;
			switch(r)
			{
			GENERAL_CELL_CHANGES(lLabel);
			case LABEL_CELL_CAPTION:		label->Caption = GetString(LABEL_CELL_CAPTION);	break;
			case LABEL_CELL_JUSTIFY:		label->Justify = (FBTextJustify) GetInt(LABEL_CELL_JUSTIFY);		break;
			case LABEL_CELL_STYLE:			label->Style = (FBTextStyle) GetInt(LABEL_CELL_STYLE);		break;
			case LABEL_CELL_WORDWRAP:		label->WordWrap = GetBoolean(LABEL_CELL_WORDWRAP);		break;
			}
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);

		GENERAL_RAW_INSERT();
		proptable.RowAdd( "Caption",			LABEL_CELL_CAPTION );
		proptable.RowAdd( "Style",				LABEL_CELL_JUSTIFY );
		proptable.RowAdd( "Justify",			LABEL_CELL_STYLE );
		proptable.RowAdd( "WordWrap",			LABEL_CELL_WORDWRAP );

		proptable.GetCell(LABEL_CELL_CAPTION, 0).Style = kFBCellStyleString;
		proptable.GetCell(LABEL_CELL_JUSTIFY, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(LABEL_CELL_JUSTIFY, 0, "Left~Right~Center" );
		proptable.GetCell(LABEL_CELL_STYLE, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(LABEL_CELL_STYLE, 0, "None~Bold~Italic~Underlined" );
		proptable.GetCell(LABEL_CELL_WORDWRAP, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(LABEL_CELL_WORDWRAP, 0, "True~False" );
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		if ( root->GetRegion(region) && label )
			root->SetControl(region, *label);

		if (label)
		{
			FBLabel		&lLabel = *label;
			GENERAL_SET_CONTROL(lLabel);
			
			FBString	caption = GetString(LABEL_CELL_CAPTION);
			label->Caption = caption;

			label->Caption = GetString(LABEL_CELL_CAPTION);	
			label->Justify = (FBTextJustify) GetInt(LABEL_CELL_JUSTIFY);		
			label->Style = (FBTextStyle) GetInt(LABEL_CELL_STYLE);		
			label->WordWrap = GetBoolean(LABEL_CELL_WORDWRAP);
			
		}
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBLabel()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBLabel	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);
	}

private:
	FBLabel			*label;
};

/////////////////////////////////////////////////////////////////////////////
// ControlThermometer
/////////////////////////////////////////////////////////////////////////////
//! class for controlling progress bar over FBThermometer
struct ControlThermometer : public ControlBase
{
public:

	//! a constructor
	ControlThermometer()
	{
		SetParamsCount(7);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM(THERMOMETER_CELL_MIN, "Min", ctInteger, 0, "");
		DEFINE_CONTROL_PARAM(THERMOMETER_CELL_MAX, "Max", ctInteger, 100, "");
		DEFINE_CONTROL_PARAM(THERMOMETER_CELL_VALUE, "Value", ctInteger, 0, "");

		SetEventsCount(2);
		DEFINE_GENERAL_EVENTS();
	}

	~ControlThermometer()
	{}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "Thr";
	}

	void Reset()
	{
		ClearParams();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);

		switch(r)
		{
		GENERAL_CELL_CHANGES(thermometer);
		case THERMOMETER_CELL_MIN:		thermometer.Min = GetInt(THERMOMETER_CELL_MIN);	break;
		case THERMOMETER_CELL_MAX:		thermometer.Max = GetInt(THERMOMETER_CELL_MAX);		break;
		case THERMOMETER_CELL_VALUE:			thermometer.Value = GetInt(THERMOMETER_CELL_VALUE);		break;
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);

		GENERAL_RAW_INSERT();

		proptable.RowAdd( "Min",			THERMOMETER_CELL_MIN );
		proptable.RowAdd( "Max",				THERMOMETER_CELL_MAX );
		proptable.RowAdd( "Value",			THERMOMETER_CELL_VALUE );

		proptable.GetCell(THERMOMETER_CELL_MIN, 0).Style = kFBCellStyleInteger;
		proptable.GetCell(THERMOMETER_CELL_MAX, 0).Style = kFBCellStyleInteger;
		proptable.GetCell(THERMOMETER_CELL_VALUE, 0).Style = kFBCellStyleInteger;
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		if ( root->GetRegion(region) )
			root->SetControl(region, thermometer);

		GENERAL_SET_CONTROL(thermometer);

		thermometer.Min = GetInt(THERMOMETER_CELL_MIN);	
		thermometer.Max =  GetInt(THERMOMETER_CELL_MAX);		
		thermometer.Value =  GetInt(THERMOMETER_CELL_VALUE);		
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBThermometer()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBThermometer	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);
	}

private:
	FBThermometer			thermometer;
};

/////////////////////////////////////////////////////////////////////////////
// ControlEdit
/////////////////////////////////////////////////////////////////////////////
//! class for controlling edit over FBEdit
struct ControlEdit : public ControlBase
{
public:
	//! a constructor
	ControlEdit()
	{
		SetParamsCount(6);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM(EDIT_CELL_TEXT, "Text", ctString, "Text", "" );
		DEFINE_CONTROL_PARAM(EDIT_CELL_PASSWORD, "PasswordMode", ctBoolean, false, "False~True" );

		SetEventsCount(4);
		DEFINE_GENERAL_EVENTS();
		SetEventName(2, "OnChange");
		SetEventName(3, "OnTransaction");

		edit = NULL;
	}

	//! a destructor
	~ControlEdit()
	{}

	void DefineEdit(FBEdit *pEdit)
	{
		edit = pEdit;
	}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "Edit";
	}

	void Reset()
	{
		ClearParams();
		ClearEvents();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);

		if (edit)
		{
			FBEdit &lEdit = *edit;
			switch(r)
			{
			GENERAL_CELL_CHANGES(lEdit);
			case EDIT_CELL_TEXT:			lEdit.Text = GetString(EDIT_CELL_TEXT);	break;
			case EDIT_CELL_PASSWORD:	lEdit.PasswordMode = GetBoolean(EDIT_CELL_PASSWORD);		break;
			}
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);
		
		GENERAL_RAW_INSERT();

		proptable.RowAdd( "Text",									EDIT_CELL_TEXT );
		proptable.RowAdd( "Password mode",				EDIT_CELL_PASSWORD );

		proptable.GetCell(EDIT_CELL_TEXT, 0).Style = kFBCellStyleString;
		proptable.GetCell(EDIT_CELL_PASSWORD, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(EDIT_CELL_PASSWORD, 0, "False~True" );
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region);

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBEdit()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBEdit	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);

		ExportPythonCallbacks(f, regionName, GetComponentPrefix());
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);

		ExportCPPCallbacks(f, regionName, GetComponentPrefix());
	}

private:
	FBEdit			*edit;
};


/////////////////////////////////////////////////////////////////////////////
// ControlEditNumber
/////////////////////////////////////////////////////////////////////////////
//! class for controlling double value edit over FBEditNumber
struct ControlEditNumber : public ControlBase
{
public:
	//! a constructor
	ControlEditNumber()
	{
		SetParamsCount(10);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM(EDIT_NUM_CELL_VALUE, "Value", ctDouble, 0.0, "" );
		DEFINE_CONTROL_PARAM(EDIT_NUM_CELL_MIN, "Min", ctDouble, 0.0, "" );
		DEFINE_CONTROL_PARAM(EDIT_NUM_CELL_MAX, "Max", ctDouble, 0.0, "" );
		DEFINE_CONTROL_PARAM(EDIT_NUM_CELL_PRECISION, "Precision", ctDouble, 0.0, "" );
		DEFINE_CONTROL_PARAM(EDIT_NUM_CELL_LARGESTEP, "LargeStep", ctDouble, 0.0, "" );
		DEFINE_CONTROL_PARAM(EDIT_NUM_CELL_SMALLSTEP, "SmallStep", ctDouble, 0.0, "" );

		SetEventsCount(1);
		//DEFINE_GENERAL_EVENTS();
		SetEventName(0, "OnChange");
	}

	~ControlEditNumber()
	{}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "EditNum";
	}

	void Reset()
	{
		ClearParams();
		ClearEvents();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);

		switch(r)
		{
		GENERAL_CELL_CHANGES(edit);
		case EDIT_NUM_CELL_VALUE:			edit.Value = GetDouble(EDIT_NUM_CELL_VALUE);	break;
		case EDIT_NUM_CELL_MIN:			edit.Min = GetDouble(EDIT_NUM_CELL_MIN);	break;
		case EDIT_NUM_CELL_MAX:			edit.Max = GetDouble(EDIT_NUM_CELL_MAX);	break;
		case EDIT_NUM_CELL_PRECISION:			edit.Precision = GetDouble(EDIT_NUM_CELL_PRECISION);	break;
		case EDIT_NUM_CELL_LARGESTEP:			edit.LargeStep = GetDouble(EDIT_NUM_CELL_LARGESTEP);	break;
		case EDIT_NUM_CELL_SMALLSTEP:			edit.SmallStep = GetDouble(EDIT_NUM_CELL_SMALLSTEP);	break;
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);
		
		GENERAL_RAW_INSERT();

		proptable.RowAdd( "Value",			EDIT_NUM_CELL_VALUE );
		proptable.RowAdd( "Min",				EDIT_NUM_CELL_MIN );
		proptable.RowAdd( "Max",				EDIT_NUM_CELL_MAX );
		proptable.RowAdd( "Precision",				EDIT_NUM_CELL_PRECISION );
		proptable.RowAdd( "Large step",				EDIT_NUM_CELL_LARGESTEP );
		proptable.RowAdd( "Small step",				EDIT_NUM_CELL_SMALLSTEP );

		proptable.GetCell(EDIT_NUM_CELL_VALUE, 0).Style = kFBCellStyleDouble;
		proptable.GetCell(EDIT_NUM_CELL_MIN, 0).Style = kFBCellStyleDouble;
		proptable.GetCell(EDIT_NUM_CELL_MAX, 0).Style = kFBCellStyleDouble;
		proptable.GetCell(EDIT_NUM_CELL_PRECISION, 0).Style = kFBCellStyleDouble;
		proptable.GetCell(EDIT_NUM_CELL_LARGESTEP, 0).Style = kFBCellStyleDouble;
		proptable.GetCell(EDIT_NUM_CELL_SMALLSTEP, 0).Style = kFBCellStyleDouble;
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		if ( root->GetRegion(region) )
			root->SetControl(region, edit);
		
		GENERAL_SET_CONTROL(edit);

		edit.Value = GetDouble(EDIT_NUM_CELL_VALUE);	
		edit.Min = GetDouble(EDIT_NUM_CELL_MIN);
		edit.Max = GetDouble(EDIT_NUM_CELL_MAX);	
		edit.Precision = GetDouble(EDIT_NUM_CELL_PRECISION);	
		edit.LargeStep = GetDouble(EDIT_NUM_CELL_LARGESTEP);	
		edit.SmallStep = GetDouble(EDIT_NUM_CELL_SMALLSTEP);
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBEditNumber()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBEditNumber	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);

		ExportPythonCallbacks(f, regionName, GetComponentPrefix());
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);

		ExportCPPCallbacks(f, regionName, GetComponentPrefix());
	}

private:
	FBEditNumber			edit;
};

/////////////////////////////////////////////////////////////////////////////
// ControlEditColor
/////////////////////////////////////////////////////////////////////////////
//! class for controlling double value edit over FBEditNumber
struct ControlEditColor : public ControlBase
{
public:

	ControlEditColor()
	{
		SetParamsCount(6);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM(EDIT_COLOR_CELL_MODE, "ColorMode", ctInteger, 0, "3~4" );
		DEFINE_CONTROL_PARAM(EDIT_COLOR_CELL_VALUE, "Value", ctColor, FBVector4d(1.0, 1.0, 1.0, 1.0), "" );

		SetEventsCount(1);
		//DEFINE_GENERAL_EVENTS();
		SetEventName(0, "OnChange");
	}

	virtual ~ControlEditColor()
	{}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "EditClr";
	}

	void Reset()
	{
		ClearParams();
		ClearEvents();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{	
		ControlManager::ComponentCellChange(proptable, c ,r);

		switch(r)
		{
		GENERAL_CELL_CHANGES(edit);
		case EDIT_COLOR_CELL_MODE:			edit.ColorMode = GetInt(EDIT_COLOR_CELL_MODE) + 3;	break;
		case EDIT_COLOR_CELL_VALUE:			
		case (EDIT_COLOR_CELL_VALUE+1):
		case (EDIT_COLOR_CELL_VALUE+2):
		case (EDIT_COLOR_CELL_VALUE+3):
			{
				FBVector4d	vector = GetVector(EDIT_COLOR_CELL_VALUE);
				edit.Value = FBColor(&vector[0]);	
			}break;
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);
		
		GENERAL_RAW_INSERT();

		proptable.RowAdd( "Color mode",			EDIT_COLOR_CELL_MODE );
		proptable.RowAdd( "R",				EDIT_COLOR_CELL_VALUE );
		proptable.RowAdd( "G",				EDIT_COLOR_CELL_VALUE+1 );
		proptable.RowAdd( "B",				EDIT_COLOR_CELL_VALUE+2 );
		proptable.RowAdd( "A",				EDIT_COLOR_CELL_VALUE+3 );

		proptable.GetCell(EDIT_COLOR_CELL_MODE, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(EDIT_COLOR_CELL_MODE, 0, "RGB~RGBA" );
		proptable.GetCell(EDIT_COLOR_CELL_VALUE, 0).Style = kFBCellStyleDouble;
		proptable.GetCell(EDIT_COLOR_CELL_VALUE+1, 0).Style = kFBCellStyleDouble;
		proptable.GetCell(EDIT_COLOR_CELL_VALUE+2, 0).Style = kFBCellStyleDouble;
		proptable.GetCell(EDIT_COLOR_CELL_VALUE+3, 0).Style = kFBCellStyleDouble;
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		if ( root->GetRegion(region) )
			root->SetControl(region, edit);
		
		GENERAL_SET_CONTROL(edit);

		edit.ColorMode = GetInt(EDIT_COLOR_CELL_MODE);
		FBVector4d	vector = GetVector(EDIT_COLOR_CELL_VALUE);
		edit.Value = FBColor(&vector[0]);	
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBEditColor()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBEditColor	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);

		ExportPythonCallbacks(f, regionName, GetComponentPrefix());
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);

		ExportCPPCallbacks(f, regionName, GetComponentPrefix());
	}

private:
	FBEditColor			edit;
};


/////////////////////////////////////////////////////////////////////////////
// ControlEditVector
/////////////////////////////////////////////////////////////////////////////
//! class for controlling vector value edit over FBEditVector

struct ControlEditVector : public ControlBase
{
public:

	ControlEditVector()
	{
		SetParamsCount(5);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM(EDIT_VECTOR_CELL_VALUE, "Value", ctVector, FBVector4d(0.0, 0.0, 0.0, 0.0), "" );

		SetEventsCount(1);
		//DEFINE_GENERAL_EVENTS();
		SetEventName(0, "OnChange");
	}

	virtual ~ControlEditVector()
	{}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "EditVec";
	}

	void Reset()
	{
		ClearParams();
		ClearEvents();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{	
		ControlManager::ComponentCellChange(proptable, c ,r);

		switch(r)
		{
		GENERAL_CELL_CHANGES(edit);
		case EDIT_VECTOR_CELL_VALUE:			
		case (EDIT_VECTOR_CELL_VALUE+1):
		case (EDIT_VECTOR_CELL_VALUE+2):
			{
				FBVector4d	vector = GetVector(EDIT_VECTOR_CELL_VALUE);
				edit.Value = FBVector3d(&vector[0]);	
			}break;
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);
		
		GENERAL_RAW_INSERT();

		proptable.RowAdd( "X",				EDIT_VECTOR_CELL_VALUE );
		proptable.RowAdd( "Y",				EDIT_VECTOR_CELL_VALUE+1 );
		proptable.RowAdd( "Z",				EDIT_VECTOR_CELL_VALUE+2 );

		proptable.GetCell(EDIT_VECTOR_CELL_VALUE, 0).Style = kFBCellStyleDouble;
		proptable.GetCell(EDIT_VECTOR_CELL_VALUE+1, 0).Style = kFBCellStyleDouble;
		proptable.GetCell(EDIT_VECTOR_CELL_VALUE+2, 0).Style = kFBCellStyleDouble;
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		if ( root->GetRegion(region) )
			root->SetControl(region, edit);
		
		GENERAL_SET_CONTROL(edit);

		FBVector4d	vector = GetVector(EDIT_VECTOR_CELL_VALUE);
		edit.Value = FBVector3d(&vector[0]);	
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBEditVector()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBEditVector	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);

		ExportPythonCallbacks(f, regionName, GetComponentPrefix());
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);

		ExportCPPCallbacks(f, regionName, GetComponentPrefix());
	}

private:
	FBEditVector			edit;
};


/////////////////////////////////////////////////////////////////////////////
// ControlSlider
/////////////////////////////////////////////////////////////////////////////
//! class for controlling slider over FBSlider
struct ControlSlider : public ControlBase
{
public:

	ControlSlider()
	{
		SetParamsCount(10);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM(SLIDER_CELL_MIN, "Min", ctInteger, 0, "" ); 
		DEFINE_CONTROL_PARAM(SLIDER_CELL_MAX, "Max", ctInteger, 100, "" );
		DEFINE_CONTROL_PARAM(SLIDER_CELL_VALUE, "Value", ctInteger, 0, "" );
		DEFINE_CONTROL_PARAM(SLIDER_CELL_ORIENTATION, "Orientation", ctInteger, 0, 
			"FBOrientation.kFBHorizontal~FBOrientation.kFBVertical" );
		DEFINE_CONTROL_PARAM(SLIDER_CELL_LARGESTEP, "LargeStep", ctDouble, 5.0, "" );
		DEFINE_CONTROL_PARAM(SLIDER_CELL_SMALLSTEP, "SmallStep", ctDouble, 1.0, "" );

		SetEventsCount(4);
		DEFINE_GENERAL_EVENTS();
		SetEventName(2, "OnChange");
		SetEventName(3, "OnTransaction");
	}

	virtual ~ControlSlider()
	{}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "Sld";
	}

	void Reset()
	{
		ClearParams();
		ClearEvents();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);

		switch(r)
		{
		GENERAL_CELL_CHANGES(slider);
		case SLIDER_CELL_MIN:		slider.Min = GetInt(SLIDER_CELL_MIN);	break;
		case SLIDER_CELL_MAX:		slider.Max = GetInt(SLIDER_CELL_MIN);		break;
		case SLIDER_CELL_VALUE:	slider.Value = GetInt(SLIDER_CELL_VALUE);		break;
		case SLIDER_CELL_ORIENTATION:		slider.Orientation = (FBOrientation)GetInt(SLIDER_CELL_ORIENTATION);	break;
		case SLIDER_CELL_LARGESTEP:		slider.LargeStep = GetDouble(SLIDER_CELL_LARGESTEP);		break;
		case SLIDER_CELL_SMALLSTEP:	slider.SmallStep = GetDouble(SLIDER_CELL_SMALLSTEP);		break;
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);

		GENERAL_RAW_INSERT();

		proptable.RowAdd( "Min",				SLIDER_CELL_MIN );
		proptable.RowAdd( "Max",				SLIDER_CELL_MAX );
		proptable.RowAdd( "Value",				SLIDER_CELL_VALUE );
		proptable.RowAdd( "Orientation",	SLIDER_CELL_ORIENTATION );
		proptable.RowAdd( "Large step",		SLIDER_CELL_LARGESTEP );
		proptable.RowAdd( "Small step",		SLIDER_CELL_SMALLSTEP );

		proptable.GetCell(SLIDER_CELL_MIN, 0).Style = kFBCellStyleInteger;
		proptable.GetCell(SLIDER_CELL_MAX, 0).Style = kFBCellStyleInteger;
		proptable.GetCell(SLIDER_CELL_VALUE, 0).Style = kFBCellStyleInteger;
		proptable.GetCell(SLIDER_CELL_LARGESTEP, 0).Style = kFBCellStyleDouble;
		proptable.GetCell(SLIDER_CELL_SMALLSTEP, 0).Style = kFBCellStyleDouble;

		proptable.GetCell(SLIDER_CELL_ORIENTATION, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(SLIDER_CELL_ORIENTATION, 0, "Horizontal~Vertical" );
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		if ( root->GetRegion(region) )
			root->SetControl(region, slider);

		GENERAL_SET_CONTROL(slider);

		slider.Min = GetInt(SLIDER_CELL_MIN);	
		slider.Max = GetInt(SLIDER_CELL_MAX);	
		slider.Value = GetInt(SLIDER_CELL_VALUE);	
		slider.Orientation = (FBOrientation)GetInt(SLIDER_CELL_ORIENTATION);
		slider.LargeStep = GetInt(SLIDER_CELL_LARGESTEP);
		slider.SmallStep = GetInt(SLIDER_CELL_SMALLSTEP);
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBSlider()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBSlider	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);

		ExportPythonCallbacks(f, regionName, GetComponentPrefix());
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);

		ExportCPPCallbacks(f, regionName, GetComponentPrefix());
	}

private:
	FBSlider			slider;
};

/////////////////////////////////////////////////////////////////////////////
// ControlList
/////////////////////////////////////////////////////////////////////////////
//! class for controlling listbox over FBList
struct ControlList : public ControlBase
{
public:

	ControlList()
	{
		SetParamsCount(8);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM(LIST_CELL_ITEMINDEX, "ItemIndex", ctInteger, -1, "" );
		DEFINE_CONTROL_PARAM(LIST_CELL_MULTISELECT, "MultiSelect", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(LIST_CELL_EXTENDEDSELECT, "ExtendedSelect", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(LIST_CELL_STYLE, "Style", ctInteger, 0, 
			"FBListStyle.kFBDropDownList~FBListStyle.kFBVerticalList" );

		SetEventsCount(4);
		DEFINE_GENERAL_EVENTS();
		SetEventName(2, "OnChange");
		SetEventName(3, "OnDragAndDrop");
	}

	~ControlList()
	{}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "List";
	}

	void Reset()
	{
		ClearParams();
		ClearEvents();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);

		switch(r)
		{
		GENERAL_CELL_CHANGES(list);
		case LIST_CELL_ITEMINDEX:			list.ItemIndex = GetInt(LIST_CELL_ITEMINDEX);	break;
		case LIST_CELL_MULTISELECT:		list.MultiSelect = GetBoolean(LIST_CELL_MULTISELECT);		break;
		case LIST_CELL_EXTENDEDSELECT:list.ExtendedSelect = GetBoolean(LIST_CELL_EXTENDEDSELECT);		break;
		case LIST_CELL_STYLE:					list.Style = (FBListStyle) GetInt(LIST_CELL_STYLE);		break;
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);

		GENERAL_RAW_INSERT();

		proptable.RowAdd( "Item Index",					LIST_CELL_ITEMINDEX );
		proptable.RowAdd( "Multi select",				LIST_CELL_MULTISELECT );
		proptable.RowAdd( "Extended select",		LIST_CELL_EXTENDEDSELECT );
		proptable.RowAdd( "Style",							LIST_CELL_STYLE );

		proptable.GetCell(LIST_CELL_ITEMINDEX, 0).Style = kFBCellStyleInteger;
		proptable.SetCell(LIST_CELL_ITEMINDEX, 0, 0);
		proptable.GetCell(LIST_CELL_MULTISELECT, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(LIST_CELL_MULTISELECT, 0, "True~False" );
		proptable.GetCell(LIST_CELL_EXTENDEDSELECT, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(LIST_CELL_EXTENDEDSELECT, 0, "True~False" );
		proptable.GetCell(LIST_CELL_STYLE, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(LIST_CELL_STYLE, 0, "Drop down~Vertical" );
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		if ( root->GetRegion(region) )
			root->SetControl(region, list);

		GENERAL_SET_CONTROL(list);

		list.ItemIndex = GetInt(LIST_CELL_ITEMINDEX);	
		list.MultiSelect = GetBoolean(LIST_CELL_MULTISELECT);		
		list.ExtendedSelect = GetBoolean(LIST_CELL_EXTENDEDSELECT);		
		list.Style = (FBListStyle) GetInt(LIST_CELL_STYLE);
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBList()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBList	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);

		ExportPythonCallbacks(f, regionName, GetComponentPrefix());
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);

		ExportCPPCallbacks(f, regionName, GetComponentPrefix());
	}

private:
	FBList			list;
};

/////////////////////////////////////////////////////////////////////////////
// ControlView
/////////////////////////////////////////////////////////////////////////////
//! class for controlling view over FBView
struct ControlView : public ControlBase
{
public:

	//! a constructor
	ControlView()
	{
		SetParamsCount(4);
		DEFINE_GENERAL_PARAMS();
		SetEventsCount(2);
		DEFINE_GENERAL_EVENTS();
	}

	~ControlView()
	{}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "View";
	}

	void Reset()
	{
		ClearParams();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);
		switch(r)
		{
		GENERAL_CELL_CHANGES(view);
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);
		GENERAL_RAW_INSERT();
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		root->SetView(region, view);

		GENERAL_SET_CONTROL(view);
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBView()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBView	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetView(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetView(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);
	}

private:
	FBView			view;
};

/////////////////////////////////////////////////////////////////////////////
// ControlContainer
/////////////////////////////////////////////////////////////////////////////
//! class for controlling container over FBContainer
struct ControlContainer : public ControlBase
{
public:

	ControlContainer()
	{
		SetParamsCount(9);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM(CONTAINER_CELL_ITEMINDEX, "ItemIndex", ctInteger, -1, "" );
		DEFINE_CONTROL_PARAM(CONTAINER_CELL_ITEMWRAP, "ItemWrap", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(CONTAINER_CELL_ICONPOSITION, "IconPosition", ctInteger, 0, 
			"FBIconPosition.kFBIconLeft~FBIconPosition.kFBIconTop" );
		DEFINE_CONTROL_PARAM(CONTAINER_CELL_ITEMWIDTH, "ItemWidth", ctInteger, 32, "" );
		DEFINE_CONTROL_PARAM(CONTAINER_CELL_ITEMHEIGHT, "ItemHeight", ctInteger, 32, "" );

		SetEventsCount(5);
		DEFINE_GENERAL_EVENTS();
		SetEventName(2, "OnChange");
		SetEventName(3, "OnDblClick");
		SetEventName(4, "OnDragAndDrop");
	}

	virtual ~ControlContainer()
	{}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "Cnt";
	}

	void Reset()
	{
		ClearParams();
		ClearEvents();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);

		switch(r)
		{
		GENERAL_CELL_CHANGES(container);
		case CONTAINER_CELL_ITEMINDEX:			container.ItemIndex = GetInt(CONTAINER_CELL_ITEMINDEX);	break;
		case CONTAINER_CELL_ITEMWRAP:				container.ItemWrap = GetBoolean(CONTAINER_CELL_ITEMWRAP);		break;
		case CONTAINER_CELL_ICONPOSITION:		container.IconPosition= (FBIconPosition)GetInt(CONTAINER_CELL_ICONPOSITION);		break;
		case CONTAINER_CELL_ITEMWIDTH:			container.ItemWidth = GetInt(CONTAINER_CELL_ITEMWIDTH);		break;
		case CONTAINER_CELL_ITEMHEIGHT:			container.ItemHeight = GetInt(CONTAINER_CELL_ITEMHEIGHT);		break;
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);

		GENERAL_RAW_INSERT();

		proptable.RowAdd( "Item Index",				CONTAINER_CELL_ITEMINDEX );
		proptable.RowAdd( "Item wrap",				CONTAINER_CELL_ITEMWRAP );
		proptable.RowAdd( "Icon position",		CONTAINER_CELL_ICONPOSITION );
		proptable.RowAdd( "Item width",				CONTAINER_CELL_ITEMHEIGHT );
		proptable.RowAdd( "Item height",			CONTAINER_CELL_ITEMWIDTH );

		proptable.GetCell(CONTAINER_CELL_ITEMINDEX, 0).Style = kFBCellStyleInteger;
		proptable.SetCell(CONTAINER_CELL_ITEMINDEX, 0, 0);
		proptable.GetCell(CONTAINER_CELL_ITEMWRAP, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(CONTAINER_CELL_ITEMWRAP, 0, "True~False" );
		proptable.GetCell(CONTAINER_CELL_ICONPOSITION, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(CONTAINER_CELL_ICONPOSITION, 0, "Left~Top" );
		proptable.GetCell(CONTAINER_CELL_ITEMHEIGHT, 0).Style = kFBCellStyleInteger;
		proptable.SetCell(CONTAINER_CELL_ITEMHEIGHT, 0, 32);
		proptable.GetCell(CONTAINER_CELL_ITEMWIDTH, 0).Style = kFBCellStyleInteger;
		proptable.SetCell(CONTAINER_CELL_ITEMWIDTH, 0, 32);
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		if ( root->GetRegion(region) )
			root->SetControl(region, container);

		GENERAL_SET_CONTROL(container);

		container.ItemIndex = GetInt(CONTAINER_CELL_ITEMINDEX);	
		container.ItemWrap = GetBoolean(CONTAINER_CELL_ITEMWRAP);		
		container.IconPosition = (FBIconPosition)GetInt(CONTAINER_CELL_ICONPOSITION);		
		container.ItemWidth = GetInt(CONTAINER_CELL_ITEMWIDTH);	
		container.ItemHeight = GetInt(CONTAINER_CELL_ITEMHEIGHT);	
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBContainer()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBContainer	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);

		ExportPythonCallbacks(f, regionName, GetComponentPrefix());
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);

		ExportCPPCallbacks(f, regionName, GetComponentPrefix());
	}

private:
	FBVisualContainer			container;
};


/////////////////////////////////////////////////////////////////////////////
// ControlSpread
/////////////////////////////////////////////////////////////////////////////
//! class for controlling listbox over FBList
struct ControlSpread : public ControlBase
{
public:

	ControlSpread()
	{
		SetParamsCount(6);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM( SPREAD_CELL_CAPTION, "Caption", ctString, "Text", "" );
		DEFINE_CONTROL_PARAM( SPREAD_CELL_MULTISELECT, "MultiSelect", ctBoolean, false, "False~True" );

		SetEventsCount(6);
		DEFINE_GENERAL_EVENTS();
		SetEventName(2, "OnCellChange");
		SetEventName(3, "OnRowClick");
		SetEventName(4, "OnColumnClick");
		SetEventName(5, "OnDragAndDrop");
	}

	virtual ~ControlSpread()
	{}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "Spr";
	}

	void Reset()
	{
		ClearParams();
		ClearEvents();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);

		switch(r)
		{
		GENERAL_CELL_CHANGES(spread);
		case SPREAD_CELL_CAPTION:			spread.Caption = GetString(SPREAD_CELL_CAPTION);	break;
		case SPREAD_CELL_MULTISELECT:		spread.MultiSelect = GetBoolean(SPREAD_CELL_MULTISELECT);		break;
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);

		GENERAL_RAW_INSERT();

		proptable.RowAdd( "Caption",						SPREAD_CELL_CAPTION );
		proptable.RowAdd( "Multi select",				SPREAD_CELL_MULTISELECT );

		proptable.GetCell(SPREAD_CELL_CAPTION, 0).Style = kFBCellStyleString;
		proptable.SetCell(SPREAD_CELL_CAPTION, 0, "Spread" );
		proptable.GetCell(SPREAD_CELL_MULTISELECT, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(SPREAD_CELL_MULTISELECT, 0, "True~False" );
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		if ( root->GetRegion(region) )
			root->SetControl(region, spread);

		GENERAL_SET_CONTROL(spread);

		spread.Caption = GetString(SPREAD_CELL_CAPTION);	
		spread.MultiSelect = GetBoolean(SPREAD_CELL_MULTISELECT);		
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBSpread()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBSpread	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);

		ExportPythonCallbacks(f, regionName, GetComponentPrefix());
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);

		ExportCPPCallbacks(f, regionName, GetComponentPrefix());
	}

private:
	FBSpread			spread;
};

/////////////////////////////////////////////////////////////////////////////
// ControlTree
/////////////////////////////////////////////////////////////////////////////
//! class for controlling tree over FBTree
struct ControlTree : public ControlBase
{
public:

	ControlTree()
	{
		SetParamsCount(19);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM(TREE_CELL_ITEMHEIGHT, "ItemHeight", ctInteger, 12, "" );
		DEFINE_CONTROL_PARAM(TREE_CELL_MULTIDRAG, "MultiDrag", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_AUTOSCROLL, "AutoScroll", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_CHECKBOXES, "CheckBoxes", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_SHOWLINES, "ShowLines", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_MULTISELECT, "MultiSelect", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_EDITNODE, "EditNodeOn2Select", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_SELECTIONACTIVE, "SelectionActive", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_DESELECTONCOLLAPSE, "DeselectOnCollapse", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_NOSELECTONDRAG, "NoSelectOnDrag", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_NOSELECTONRIGHTCLICK, "NoSelectOnRightClick", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_HIGHLIGHTONRIGHTCLICK, "HighlightOnRightClick", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_AUTOSCROLLONEXPAND, "AutoScrollExpand", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_AUTOEXPANDONDRAGOVER, "AutoExpandOnDragOver", ctBoolean, false, "False~True" );
		DEFINE_CONTROL_PARAM(TREE_CELL_AUTOEXPANDONDBLCLICK, "AutoExpandOnDblClick", ctBoolean, false, "False~True" );


		SetEventsCount(12);
		DEFINE_GENERAL_EVENTS();
		SetEventName(2, "OnSelect");
		SetEventName(3, "OnClick");
		SetEventName(4, "OnDblClick");
		SetEventName(5, "OnDeselect");
		SetEventName(6, "OnExpanded");
		SetEventName(7, "OnExpanding");
		SetEventName(8, "OnCollapsing");
		SetEventName(9, "OnChange");
		SetEventName(10, "OnDragAndDrop");
		SetEventName(11, "OnEditNode");
	}

	virtual ~ControlTree()
	{}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "Tree";
	}

	void Reset()
	{
		ClearParams();
		ClearEvents();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);

		switch(r)
		{
		GENERAL_CELL_CHANGES(tree);
		case TREE_CELL_MULTIDRAG:			tree.MultiDrag = GetBoolean(TREE_CELL_MULTIDRAG);	break;
		case TREE_CELL_AUTOSCROLL:		tree.AutoScroll = GetBoolean(TREE_CELL_AUTOSCROLL);		break;
		case TREE_CELL_CHECKBOXES:		tree.CheckBoxes= GetBoolean(TREE_CELL_CHECKBOXES);		break;
		case TREE_CELL_SHOWLINES:			tree.ShowLines = GetBoolean(TREE_CELL_SHOWLINES);		break;
		case TREE_CELL_ITEMHEIGHT:		tree.ItemHeight = GetInt(TREE_CELL_ITEMHEIGHT);		break;
		case TREE_CELL_MULTISELECT:			tree.MultiDrag = GetBoolean(TREE_CELL_MULTISELECT);	break;
		case TREE_CELL_EDITNODE:		tree.AutoScroll = GetBoolean(TREE_CELL_EDITNODE);		break;
		case TREE_CELL_SELECTIONACTIVE:		tree.CheckBoxes= GetBoolean(TREE_CELL_SELECTIONACTIVE);		break;
		case TREE_CELL_DESELECTONCOLLAPSE:			tree.ShowLines = GetBoolean(TREE_CELL_DESELECTONCOLLAPSE);		break;
		case TREE_CELL_NOSELECTONDRAG:			tree.MultiDrag = GetBoolean(TREE_CELL_NOSELECTONDRAG);	break;
		case TREE_CELL_NOSELECTONRIGHTCLICK:		tree.AutoScroll = GetBoolean(TREE_CELL_NOSELECTONRIGHTCLICK);		break;
		case TREE_CELL_HIGHLIGHTONRIGHTCLICK:		tree.CheckBoxes= GetBoolean(TREE_CELL_HIGHLIGHTONRIGHTCLICK);		break;
		case TREE_CELL_AUTOSCROLLONEXPAND:			tree.ShowLines = GetBoolean(TREE_CELL_AUTOSCROLLONEXPAND);		break;
		case TREE_CELL_AUTOEXPANDONDRAGOVER:			tree.MultiDrag = GetBoolean(TREE_CELL_AUTOEXPANDONDRAGOVER);	break;
		case TREE_CELL_AUTOEXPANDONDBLCLICK:		tree.AutoScroll = GetBoolean(TREE_CELL_AUTOEXPANDONDBLCLICK);		break;
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);

		GENERAL_RAW_INSERT();

		proptable.RowAdd( "Multi drag",				TREE_CELL_MULTIDRAG );
		proptable.RowAdd( "Auto scroll",				TREE_CELL_AUTOSCROLL );
		proptable.RowAdd( "Show checkboxes",		TREE_CELL_CHECKBOXES );
		proptable.RowAdd( "Show lines",				TREE_CELL_SHOWLINES );
		proptable.RowAdd( "Item height",			TREE_CELL_ITEMHEIGHT );
		proptable.RowAdd( "Multi select",			TREE_CELL_MULTISELECT );
		proptable.RowAdd( "Edit node on 2 select",			TREE_CELL_EDITNODE );
		proptable.RowAdd( "Selection active",			TREE_CELL_SELECTIONACTIVE );
		proptable.RowAdd( "Deselect on collapse",			TREE_CELL_DESELECTONCOLLAPSE);
		proptable.RowAdd( "No select on drag",			TREE_CELL_NOSELECTONDRAG );
		proptable.RowAdd( "No select on right click",			TREE_CELL_NOSELECTONRIGHTCLICK);
		proptable.RowAdd( "Hightlight on right click",			TREE_CELL_HIGHLIGHTONRIGHTCLICK );
		proptable.RowAdd( "Auto scroll on expand",			TREE_CELL_AUTOSCROLLONEXPAND );
		proptable.RowAdd( "Auto expand on drag over",			TREE_CELL_AUTOEXPANDONDRAGOVER );
		proptable.RowAdd( "Auto expand on dbl click",			TREE_CELL_AUTOEXPANDONDBLCLICK );

		proptable.GetCell(TREE_CELL_MULTIDRAG, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_MULTIDRAG, 0, "True~False" );
		proptable.GetCell(TREE_CELL_AUTOSCROLL, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_AUTOSCROLL, 0, "True~False" );
		proptable.GetCell(TREE_CELL_CHECKBOXES, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_CHECKBOXES, 0, "True~False" );
		proptable.GetCell(TREE_CELL_SHOWLINES, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_SHOWLINES, 0, "True~False" );
		proptable.GetCell(TREE_CELL_ITEMHEIGHT, 0).Style = kFBCellStyleInteger;
		proptable.SetCell(TREE_CELL_ITEMHEIGHT, 0, 12 );
		proptable.GetCell(TREE_CELL_MULTISELECT, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_MULTISELECT, 0, "True~False" );
		proptable.GetCell(TREE_CELL_EDITNODE, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_EDITNODE, 0, "True~False" );
		proptable.GetCell(TREE_CELL_SELECTIONACTIVE, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_SELECTIONACTIVE, 0, "True~False" );
		proptable.GetCell(TREE_CELL_DESELECTONCOLLAPSE, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_DESELECTONCOLLAPSE, 0, "True~False" );
		proptable.GetCell(TREE_CELL_NOSELECTONDRAG, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_NOSELECTONDRAG, 0, "True~False" );
		proptable.GetCell(TREE_CELL_NOSELECTONRIGHTCLICK, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_NOSELECTONRIGHTCLICK, 0, "True~False" );
		proptable.GetCell(TREE_CELL_HIGHLIGHTONRIGHTCLICK, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_HIGHLIGHTONRIGHTCLICK, 0, "True~False" );
		proptable.GetCell(TREE_CELL_AUTOSCROLLONEXPAND, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_AUTOSCROLLONEXPAND, 0, "True~False" );
		proptable.GetCell(TREE_CELL_AUTOEXPANDONDRAGOVER, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_AUTOEXPANDONDRAGOVER, 0, "True~False" );
		proptable.GetCell(TREE_CELL_AUTOEXPANDONDBLCLICK, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(TREE_CELL_AUTOEXPANDONDBLCLICK, 0, "True~False" );
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		if ( root->GetRegion(region) )
			root->SetControl(region, tree);

		GENERAL_SET_CONTROL(tree);

		tree.MultiDrag = GetBoolean(TREE_CELL_MULTIDRAG);
		tree.AutoScroll = GetBoolean(TREE_CELL_AUTOSCROLL);		
		tree.CheckBoxes= GetBoolean(TREE_CELL_CHECKBOXES);		
		tree.ShowLines = GetBoolean(TREE_CELL_SHOWLINES);		
		tree.ItemHeight = GetInt(TREE_CELL_ITEMHEIGHT);	
		tree.MultiDrag = GetBoolean(TREE_CELL_MULTISELECT);	
		tree.AutoScroll = GetBoolean(TREE_CELL_EDITNODE);	
		tree.CheckBoxes= GetBoolean(TREE_CELL_SELECTIONACTIVE);
		tree.ShowLines = GetBoolean(TREE_CELL_DESELECTONCOLLAPSE);
		tree.MultiDrag = GetBoolean(TREE_CELL_NOSELECTONDRAG);
		tree.AutoScroll = GetBoolean(TREE_CELL_NOSELECTONRIGHTCLICK);	
		tree.CheckBoxes= GetBoolean(TREE_CELL_HIGHLIGHTONRIGHTCLICK);		
		tree.ShowLines = GetBoolean(TREE_CELL_AUTOSCROLLONEXPAND);	
		tree.MultiDrag = GetBoolean(TREE_CELL_AUTOEXPANDONDRAGOVER);
		tree.AutoScroll = GetBoolean(TREE_CELL_AUTOEXPANDONDBLCLICK);
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBTree()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBTree	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);

		ExportPythonCallbacks(f, regionName, GetComponentPrefix());
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);

		ExportCPPCallbacks(f, regionName, GetComponentPrefix());
	}

private:
	FBTree			tree;
};


/////////////////////////////////////////////////////////////////////////////
// ControlTimeCode
/////////////////////////////////////////////////////////////////////////////
//! class for controlling time value edit over FBTimeCode
struct ControlTimeCode : public ControlBase
{
public:

	ControlTimeCode()
	{
		SetParamsCount(5);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM(TIME_CODE_VALUE, "Value", ctTime, 0.0, "" );

		SetEventsCount(1);
		//DEFINE_GENERAL_EVENTS();
		SetEventName(0, "OnChange");
	}

	virtual ~ControlTimeCode()
	{}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "TC";
	}

	void Reset()
	{
		ClearParams();
		ClearEvents();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);

		switch(r)
		{
		GENERAL_CELL_CHANGES(timecode);
		case TIME_CODE_VALUE:			timecode.Value = GetTime(TIME_CODE_VALUE);	break;
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);
		
		GENERAL_RAW_INSERT();

		proptable.RowAdd( "Value",			TIME_CODE_VALUE );
		proptable.GetCell(TIME_CODE_VALUE, 0).Style = kFBCellStyleTime;
	}
	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		if ( root->GetRegion(region) )
			root->SetControl(region, timecode);
		
		GENERAL_SET_CONTROL(timecode);

		timecode.Value = GetTime(TIME_CODE_VALUE);	
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBTimeCode()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBTimeCode	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);

		ExportPythonCallbacks(f, regionName, GetComponentPrefix());
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);

		ExportCPPCallbacks(f, regionName, GetComponentPrefix());
	}

private:
	FBEditTimeCode			timecode;
};


/////////////////////////////////////////////////////////////////////////////
// ControlLayoutRegion
/////////////////////////////////////////////////////////////////////////////
//! class for controlling region group over FBLayoutRegion
struct ControlLayoutRegion : public ControlBase
{
public:

	ControlLayoutRegion()
	{
		mRoot = NULL;
		mRegionName = "";

		SetParamsCount(11);
		DEFINE_GENERAL_PARAMS();
		DEFINE_CONTROL_PARAM(LAYOUT_REGION_CELL_CAPTION, "Title", ctBoolean, true, "True~False" ); 
		DEFINE_CONTROL_PARAM(LAYOUT_REGION_CELL_INSET, "InSet", ctBoolean, true, "True~False" );
		DEFINE_CONTROL_PARAM(LAYOUT_REGION_CELL_BORDER, "BorderStyle", ctInteger, 0, 
			"kFBNoBorder~kFBStandardBorder~kFBEmbossBorder~kFBEmbossSmoothBorder~kFBEmbossEdgeSmoothBorder\
			~kFBEmbossSmoothEdgeBorder~kFBStandardSmoothBorder~kFBStandardEdgeSmoothBorder~kFBStandardSmoothEdgeBorder\
			~kFBHighlightBorder~kFBPickingBorder" );
		DEFINE_CONTROL_PARAM(LAYOUT_REGION_CELL_WIDTH, "Width", ctInteger, 1, "" );
		DEFINE_CONTROL_PARAM(LAYOUT_REGION_CELL_SPACING, "Spacing", ctInteger, 0, "" );
		DEFINE_CONTROL_PARAM(LAYOUT_REGION_CELL_MAXANGLE, "MaxAngle", ctDouble, 90.0, "" );
		DEFINE_CONTROL_PARAM(LAYOUT_REGION_CELL_CORNERRADIUS, "CornerRadius", ctInteger, 0, "" );

		SetEventsCount(2);
		DEFINE_GENERAL_EVENTS();
	}

	//! return component indentificator
	char *GetComponentPrefix()
	{
		return "LR";
	}

	void Reset()
	{
		ClearParams();
		ClearEvents();
	}

	void ComponentCellChange(FBSpread &proptable, int c, int r ) 
	{
		ControlManager::ComponentCellChange(proptable, c ,r);

		switch(r)
		{
		GENERAL_CELL_CHANGES(layoutregion);
		}
		if (mRoot && (mRegionName.GetLen() ))
		{
			mRoot->SetBorder(mRegionName, (FBBorderStyle) GetInt(LAYOUT_REGION_CELL_BORDER),
				GetBoolean(LAYOUT_REGION_CELL_CAPTION),
				GetBoolean(LAYOUT_REGION_CELL_INSET),
				GetInt(LAYOUT_REGION_CELL_WIDTH),
				GetInt(LAYOUT_REGION_CELL_SPACING),
				(float)GetDouble(LAYOUT_REGION_CELL_MAXANGLE),
				GetInt(LAYOUT_REGION_CELL_CORNERRADIUS)
				);
		}
	}
	void PrepareCompTable(FBSpread &proptable)
	{
		ControlManager::PrepareCompTable(proptable);

		GENERAL_RAW_INSERT();

		proptable.RowAdd( "Show Title",				LAYOUT_REGION_CELL_CAPTION );
		proptable.RowAdd( "Inset",				LAYOUT_REGION_CELL_INSET );
		proptable.RowAdd( "Border",				LAYOUT_REGION_CELL_BORDER );
		proptable.RowAdd( "Width",	LAYOUT_REGION_CELL_WIDTH );
		proptable.RowAdd( "Spacing",		LAYOUT_REGION_CELL_SPACING );
		proptable.RowAdd( "Max Angle",		LAYOUT_REGION_CELL_MAXANGLE );
		proptable.RowAdd( "Corner radius",		LAYOUT_REGION_CELL_CORNERRADIUS );

		proptable.GetCell(LAYOUT_REGION_CELL_WIDTH, 0).Style = kFBCellStyleInteger;
		proptable.GetCell(LAYOUT_REGION_CELL_SPACING, 0).Style = kFBCellStyleInteger;
		proptable.GetCell(LAYOUT_REGION_CELL_CORNERRADIUS, 0).Style = kFBCellStyleInteger;
		proptable.GetCell(LAYOUT_REGION_CELL_MAXANGLE, 0).Style = kFBCellStyleDouble;

		proptable.GetCell(LAYOUT_REGION_CELL_INSET, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(LAYOUT_REGION_CELL_INSET, 0, "True~False" );
		proptable.GetCell(LAYOUT_REGION_CELL_CAPTION, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(LAYOUT_REGION_CELL_CAPTION, 0, "True~False" );

		proptable.GetCell(LAYOUT_REGION_CELL_BORDER, 0).Style = kFBCellStyleMenu;
		proptable.SetCell(LAYOUT_REGION_CELL_BORDER, 0, "No border~Standard~Embossed~Smooth~Edged smooth~Smoothed edges~Standard smooth~Standard edged smooth~Standard smoothed edges~Highlight~Picking" );
	}

	void FillCompTable(FBSpread &proptable)
	{
		ControlManager::FillCompTable(proptable);
	}

	void SetControl(FBLayout	*root,	FBString &region) {
		mRoot = root;
		mRegionName = region;

		if ( root->GetRegion(region) )
			root->SetControl(region, layoutregion);

		GENERAL_SET_CONTROL(layoutregion);

		if (mRoot && (mRegionName.GetLen() ))
		{
			mRoot->SetBorder(mRegionName, (FBBorderStyle) GetInt(LAYOUT_REGION_CELL_BORDER),
				GetBoolean(LAYOUT_REGION_CELL_CAPTION),
				GetBoolean(LAYOUT_REGION_CELL_INSET),
				GetInt(LAYOUT_REGION_CELL_WIDTH),
				GetInt(LAYOUT_REGION_CELL_SPACING),
				(float)GetDouble(LAYOUT_REGION_CELL_MAXANGLE),
				GetInt(LAYOUT_REGION_CELL_CORNERRADIUS)
				);
		}
	}

	void ExportPythonVar(FILE *f, FBString regionName)
	{
		fprintf( f, "%s = FBLayoutRegion()\n", (char*)regionName );
	}
	void ExportCPPVar(FILE *f, FBString regionName)
	{
		fprintf( f, "FBLayoutRegion	%s;\n", (char*)regionName );
	}
	void ExportPython(FILE *f, FBString regionName)
	{
		fprintf( f, "\tt.SetControl(\"%s\", %s)\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportPython(f, regionName);

		ExportPythonCallbacks(f, regionName, GetComponentPrefix());
	}
	void ExportCPP(FILE *f, FBString regionName)
	{
		fprintf( f, "SetControl(\"%s\", %s);\n", (char*)regionName, (char*)regionName );
		ControlManager::ExportCPP(f, regionName);

		ExportCPPCallbacks(f, regionName, GetComponentPrefix());
	}

private:
	FBLayout						*mRoot;
	FBString						mRegionName;
	FBLayoutRegion			layoutregion;
};



/////////////////////////////////////////////////////////////
// each ui rect
//
struct	uirect
{
	int id;		//!> index in static array
	FBLayout		*root;
	FBLayoutRegion	element;
	FBLayoutRegion	sel;	//!> display selection state

	int visualType;
	ControlBase	*currComp;

	//! components objects
	#define	COMPCLASS(name, objName) Control##objName	v##objName;
	#include "components_list.h"
	#undef COMPCLASS

	FBTreeNode		*treeNode;	//!> navigator tree node

	RegionState		state;	//!> is the region selected?

	// region properties
	FBString		name;
	FBString		title;
	
	struct	regionprop
	{
		int			value;
		FBString	attachTo;
		uirect		*attachNode;
		int			attachType;
		int			holdValue;

		regionprop()
		{
			value = 0;
			holdValue = 0;
			attachTo = "";
			attachNode = NULL;
			attachType = (int) kFBAttachNone;
		}

		regionprop& operator = (const regionprop& item)
		{
			value = item.value;
			attachTo = item.attachTo;
			attachNode = item.attachNode;
			attachType = item.attachType;

			return *this;
		}

		bool operator != (const regionprop& item)
		{
			return (value != item.value);
		}
		void Hold()
		{
			holdValue = value;
		}
		void Fetch()
		{
			value = holdValue;
		}
		void UpdateAttachTo()
		{
			if (attachNode)
				attachTo = attachNode->name;
		}
	};
	regionprop	regionx;
	regionprop	regiony;
	regionprop	regionw;
	regionprop	regionh;

	//
	// visual component
	//
	bool				visual;
	bool				previewMode;

	uirect();

	//
	void HoldPosition()
	{
		regionx.Hold();
		regiony.Hold();
		regionw.Hold();
		regionh.Hold();
	}
	void FetchPosition()
	{
		regionx.Fetch();
		regiony.Fetch();
		regionw.Fetch();
		regionh.Fetch();
		root->MoveRegion(name, regionx.value, regiony.value);
		root->SizeRegion(name, regionw.value, regionh.value);
	}

	// copy data from the array
	void Assign(uirect &item);
	int Diff(uirect &item);

	bool IsSelected();
	void Select(bool sel=true);
	void Rename(const char *newName);
	bool IsDeleted() { return (state==rsDelete); }
	void Delete();
	void Clear();

	void UpdateComponentPointer();
	void attachVisualControl();
	void SetControl( int type );
	void ClearControl();

	void PreviewEnter();
	void PreviewClose();

	void ExportEvents(FILE *f, bool pythonformat=true)
	{
		if (visual)
			if (currComp)
			{
				char *prefix = currComp->GetComponentPrefix();
				(pythonformat) ?	currComp->ExportPythonEvents(f, title, prefix) : currComp->ExportCPPEvents(f, title, prefix);
			}	
	}

	void FillEvents(FBList	*pEventList)
	{
		if (visual && currComp)
			currComp->FillEvents(pEventList);
	}
	void GetEventCode(const FBString &eventName, FBStringList	&list)
	{
		if (visual && currComp)
			currComp->GetEventCode(eventName, list);
	}
	void SetEventCode(FBString eventName, FBStringList *pList)
	{
		if (visual && currComp)
			currComp->SetEventCode(eventName, pList);
	}

	FBString convertAttachToString(FBAttachType type);

	void Load(FILE *f);
	void Save(FILE *f);

	void ExportCPP(FILE *f);
	void ExportCPPVisual(FILE *f);
	void ExportPython(FILE *f);
	void ExportVar(FILE *f, bool pythonformat=true);

	void CreateSelElement();
	void RemoveSelElement();

	void Create();
	void ReBuild();
	void RePosition();
	void Init(FBLayout	&_root, FBString _name, FBString _title);

	RegionAction testMouseCursor(int mX, int mY);

	void FillPropTable(FBSpread &proptable);
	void PrepareCompTable(FBSpread &proptable);
	void FillCompTable(FBSpread &proptable);

	void ComponentCellChange(FBSpread &proptable, int c, int r);

	void Move(int *pX=0, int *pY=0, bool relative=false, int snapstep=0);
	void Size(int *pW=0, int *pH=0, bool relative=false, int snapstep=0);
	void ChangeAttachType( int *tX=0, int *tY=0, int *tW=0, int *tH=0 );
	void ChangeAttachNode( uirect *pX=0, uirect *pY=0, uirect *pW=0, uirect *pH=0 );
	void UpdateAttachNode( uirect *pX=0, uirect *pY=0, uirect *pW=0, uirect *pH=0 );
	void UpdateAttachTo();
};



////////////////////////////////////////////////////////////////////////
// main ui control class
//
struct uimain : public ControlEventList
{
	int			ndx;		// loop through the max_ui_rects
	int			ndx2;		// loop through the count
	int			ndx_2;		// loop through the max_ui_rects
	int			ndx_22;		// loop through the count
	int			count;
	uirect		rects[MAX_UI_RECTS];

	int			mSnapStep;

	bool		makeChanges;
	int			storeCount;
	uirect		store[MAX_UI_RECTS];	// undo / redo buffer

	FBTree		*pNavigator;

	FBList		*pEventsList;

	struct	uitool
	{
		FBString		name;
		int					sizex;
		int					sizey;

		uitool()
			: name("Tool")
			, sizex(400)
			, sizey(400)
		{}
		void Clear()
		{
			name = "Tool";
			sizex = 400;
			sizey = 400;
		}
	} mTool;

	//uilayout	main;	// main layout for the build tool

	uimain();

	// undo / redo buffer
	void Hold();		// store data into the array
	void Fetch();		// restore data from the array
	void Swap();

	// io
	void New();
	void Load(char *filename, FBLayout &mFormView, FBSpread &mPropTable);
	void Save(char *filename);
	void ExportCPP(char *fileName);
	void ExportPython(char *fileName);
	int ExportXml(const char *fileName);
	//! export events code in python format
	void ExportPythonEvents(FILE *f)
	{
		FBString toolName("Tool");
		ControlEventList::ExportPythonEventsSpecial(f);

		uirect	*iter = first();
		{while (iter != NULL)
		{
			iter->ExportEvents(f);		
			iter = next();
		}}
	}
	//! export events code in cpp format
	void ExportCPPEvents(FILE *f)
	{
		FBString toolName("Tool");
		ControlEventList::ExportCPPEvents(f, toolName, "");

		uirect	*iter = first();
		{while (iter != NULL)
		{
			iter->ExportEvents(f, false);		
			iter = next();
		}}
	}

	void ClearPropTable(FBSpread &proptable);
	void FillPropTable(FBSpread &proptable);

	void FillCompTable(FBSpread &proptable);

	void FillEvents(FBList	*pEventList)
	{
		ControlEventList::FillEvents(pEventList);

		uirect	*iter = first();
		{while (iter != NULL)
		{
			if (iter->IsSelected())
			{
				iter->FillEvents(pEventList);		
			}
			iter = next();
		}}
	}
	void GetEventCode(const FBString &eventName, FBStringList	&list)
	{
		ControlEventList::GetEventCode(eventName, list);

		uirect	*iter = first();
		{while (iter != NULL)
		{
			if (iter->IsSelected() && iter->visual)
			{
				iter->GetEventCode(eventName, list);
			}
			iter = next();
		}}
	}
	void SetEventCode(FBString eventName, FBStringList *pList)
	{
		ControlEventList::SetEventCode(eventName, pList);

		uirect	*iter = first();
		{while (iter != NULL)
		{
			if (iter->IsSelected())
			{
				iter->SetEventCode(eventName, pList);
			}
			iter = next();
		}}
	}

	void Rename(const char *newName);

	void Move(int *pX=0, int *pY=0, bool relative=false, bool hold=true);
	void Size(int *pW=0, int *pH=0, bool relative=false, bool hold=true);
	void SnapRegions();
	void ChangeAttachType( int *tX=0, int *tY=0, int *tW=0, int *tH=0 );

	uirect	*FindNode(char *lName);

	void ChangeAttachNode( int *tX=0, int *tY=0, int *tW=0, int *tH=0 );
	void UpdateAttachNodes();

	uirect&		AddElement();

	uirect*		first(bool secondloop=false);
	uirect*		last(bool secondloop=false);
	uirect*		next(bool secondloop=false);
	uirect*		prev(bool secondloop=false);

	void SetCount(int _count) { count = _count; }
	int GetCount() { return count; }
	uirect& GetAt(int i);
	uirect&	operator [] (int i) {
		return GetAt(i);
	}
	void SelectAll();
	void SelectInvert();
	void SelectNone();
	int		GetSelectionCount();

	void DeleteSel();

	void GenerateUniqueName(char *name, char *title);

	// visual components
	void AssignComponent(int type);

	void PreviewEnter();
	void PreviewClose();

	void ComponentCellChange(FBSpread &proptable, int c, int r);
};


#endif