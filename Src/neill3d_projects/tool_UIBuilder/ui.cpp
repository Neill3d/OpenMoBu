
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


/**	\file	ui.cpp

	Author - Sergey Solohin (Neill)
	homepage - http://neill3d.com/tool-programming-in-mobu/ui-builder?langswitch_lang=en
*/

#include "ui.h"
#include "tinyxml.h"

FBLabel		UI_Labels[MAX_UI_RECTS];
FBButton	UI_Buttons[MAX_UI_RECTS];
FBEdit		UI_Edits[MAX_UI_RECTS];



void ControlEdit::SetControl(FBLayout	*root,	FBString &region) {
	if ( root->GetRegion(region) && edit )
	{
		if (root->GetControl(region))
			root->ClearControl(region);
		root->SetControl(region, edit);
	}

	if (edit)
	{
		FBEdit &lEdit = *edit;
		GENERAL_SET_CONTROL(lEdit);

		lEdit.Text = GetString(EDIT_CELL_TEXT);	
		lEdit.PasswordMode = GetBoolean(EDIT_CELL_PASSWORD);		
	}
}


//////////////////////////////////////////////////////////////////
// uirect
//

uirect::uirect()
{
	name = "region";
	title = "region";

	regionx.value = 10;
	regionx.attachNode = NULL;
	regionx.attachType = (int)kFBAttachNone;
	regiony.value = 10;
	regiony.attachNode = NULL;
	regionx.attachType = (int)kFBAttachNone;
	regionw.value = 0;
	regionw.attachNode = NULL;
	regionx.attachType = (int)kFBAttachNone;
	regionh.value = 0;
	regionh.attachNode = NULL;
	regionx.attachType = (int)kFBAttachNone;

	state = rsNone;
	visual = false;
	previewMode = false;

	currComp = NULL;
}

// copy data from the array
void uirect::Assign(uirect &item)
{
	root = item.root;
	name = item.name;
	title = item.title;
	treeNode = item.treeNode;
	state = item.state;
	
	regionx = item.regionx;
	regiony = item.regiony;
	regionw = item.regionw;
	regionh = item.regionh;
	
	// visual component
	visual = item.visual;
	visualType = item.visualType;
	UpdateComponentPointer();
	previewMode = item.previewMode;

	if (item.visual)
	{
		*currComp = *item.currComp;
	}
}



int uirect::Diff(uirect &item)
{
	int res = 0;

	if (name != item.name)
		res |= REGION_DIFF_NAME;
	if (title != item.title)
		res |= REGION_DIFF_TITLE;
	if (state != item.state)
		res |= REGION_DIFF_STATE;
	if ( (regionx != item.regionx) || (regiony != item.regiony) || (regionw != item.regionw)
		|| (regionh != item.regionh) )
		res |= REGION_DIFF_SIZE;
	return res;
}

bool uirect::IsSelected() { return (state==rsSelect); }
void uirect::Select(bool sel) { 
	if (sel)
	{
		state = rsSelect;
		CreateSelElement();
	}
	else
	{
		state = rsNone;
		RemoveSelElement();
	}
}

void uirect::Rename(const char *newName)
{
	title = newName;
	root->SetRegionTitle(name, newName);
}

void uirect::Clear()
{
	visual = false;
	previewMode = false;

	// clear visual control elements
	#define COMPCLASS(name, objName) v##objName##.Reset();
	#include "components_list.h"
	#undef COMPCLASS

	name = "region";
	title = "region";

	regionx.value = 0;
	regionx.attachTo = "";
	regionx.attachNode = NULL;
	regionx.attachType = (int)kFBAttachNone;
	regiony.value = 0;
	regiony.attachTo = "";
	regiony.attachNode = NULL;
	regiony.attachType = (int)kFBAttachNone;
	regionw.value = 0;
	regionw.attachTo = "";
	regionw.attachNode = NULL;
	regionw.attachType = (int)kFBAttachNone;
	regionh.value = 0;
	regionh.attachTo = "";
	regionh.attachNode = NULL;
	regionh.attachType = (int)kFBAttachNone;
}

void uirect::Delete() { 
	// remove selection rect
	state = rsDelete; 
	RemoveSelElement();
	// remove visual element
	PreviewClose();
	ClearControl();
	visual = false;
	previewMode = false;
	// remove rect element
	root->ClearControl(name);
	if ( root->GetRegion(name) )
		root->RemoveRegion(name);

	// clear visual control elements
	#define COMPCLASS(name, objName) v##objName##.Reset();
	#include "components_list.h"
	#undef COMPCLASS

	name = "region";
	title = "region";

	regionx.value = 0;
	regionx.attachNode = NULL;
	regionx.attachType = (int)kFBAttachNone;
	regiony.value = 0;
	regiony.attachNode = NULL;
	regiony.attachType = (int)kFBAttachNone;
	regionw.value = 0;
	regionw.attachNode = NULL;
	regionw.attachType = (int)kFBAttachNone;
	regionh.value = 0;
	regionh.attachNode = NULL;
	regionh.attachType = (int)kFBAttachNone;
}

FBString uirect::convertAttachToString(FBAttachType type)
{
	switch (type)
	{
	case kFBAttachLeft:
		return "kFBAttachLeft";
	case kFBAttachRight:
		return "kFBAttachRight";
	case kFBAttachTop:
		return "kFBAttachTop";
	case kFBAttachBottom:
		return "kFBAttachBottom";
	case kFBAttachCenter:
		return "kFBAttachCenter";
	case kFBAttachWidth:
		return "kFBAttachWidth";
	case kFBAttachHeight:
		return "kFBAttachHeight";
	default:
		return "kFBAttachNone";
	}
}

void uirect::Load(FILE *f)
{
	char buffer[2048];
	char	ptr[256];

	// read name & title
	memset(buffer,0,sizeof(char)*2048);
	memset(ptr, 0, sizeof(char)*256);
	fgets(buffer, 2048, f);
	sscanf(buffer, "%s", ptr );
	name = ptr;

	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	sscanf(buffer, "%s", ptr);
	title = ptr;

	// regionx
	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	sscanf( buffer, "%d", &regionx.value );

	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	sscanf( buffer, "%d", &regionx.attachType );

	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	if (buffer[0] == 10)
		regionx.attachTo = "";
	else {
		sscanf(buffer, "%s", ptr);
		regionx.attachTo = ptr;
	}

	// regiony
	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	sscanf( buffer, "%d", &regiony.value );

	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	sscanf( buffer, "%d", &regiony.attachType );

	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	if (buffer[0] == 10)
		regiony.attachTo = "";
	else {
		sscanf(buffer, "%s", ptr);
		regiony.attachTo = ptr;
	}

	// regionw
	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	sscanf( buffer, "%d", &regionw.value );

	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	sscanf( buffer, "%d", &regionw.attachType );

	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	if (buffer[0] == 10)
		regionw.attachTo = "";
	else {
		sscanf(buffer, "%s", ptr);
		regionw.attachTo = ptr;
	}

	// regionh
	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	sscanf( buffer, "%d", &regionh.value );

	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	sscanf( buffer, "%d", &regionh.attachType );

	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	if (buffer[0] == 10)
		regionh.attachTo = "";
	else {
		sscanf(buffer, "%s", ptr);
		regionh.attachTo = ptr;
	}

	// visual component
	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);

	if (strstr(buffer, "visual") != NULL)
	{
		visual = true;
	
		memset(buffer,0,sizeof(char)*2048);
		fgets(buffer, 2048, f);
		sscanf( buffer, "%d", &visualType );
	
		UpdateComponentPointer();

		if (currComp)
		{
			currComp->Load(f);
			currComp->LoadEvents(f);
		}
	}
	else visual = false;
	previewMode = false;
}

void uirect::Save(FILE *f)
{
	// save all region properties
	fprintf(f, "%s\n", (char*)name);
	fprintf(f, "%s\n", (char*)title);

	fprintf(f, "%d\n", regionx.value);
	fprintf(f, "%d\n", regionx.attachType);
	fprintf(f, "%s\n", regionx.attachTo);

	fprintf(f, "%d\n", regiony.value);
	fprintf(f, "%d\n", regiony.attachType);
	fprintf(f, "%s\n", regiony.attachTo);

	fprintf(f, "%d\n", regionw.value);
	fprintf(f, "%d\n", regionw.attachType);
	fprintf(f, "%s\n", regionw.attachTo);

	fprintf(f, "%d\n", regionh.value);
	fprintf(f, "%d\n", regionh.attachType);
	fprintf(f, "%s\n", regionh.attachTo);

	if (visual) {
		fprintf(f, "visual\n");
		fprintf(f, "%d\n", visualType);

		if (currComp)
		{
			currComp->Save(f);
			currComp->SaveEvents(f);
		}
	}
	else
		fprintf(f, "none\n" );
}

void uirect::ExportCPP(FILE *f)
{
	FBString strAttachType;

	fprintf( f, "AddRegion( \"%s\",\t\"%s\",\n", (char*)title, (char*)title );
	//x
	strAttachType = convertAttachToString( (FBAttachType)regionx.attachType);
	fprintf( f, "\t\t%d,\t\t%s,\t\t\"%s\",\t\t1.0,\n", regionx.value, (char*)strAttachType, (char*)((regionx.attachNode) ? regionx.attachNode->title : "") );
	//y
	strAttachType = convertAttachToString( (FBAttachType)regiony.attachType);
	fprintf( f, "\t\t%d,\t\t%s,\t\t\"%s\",\t\t1.0,\n", regiony.value, (char*)strAttachType, (char*)((regiony.attachNode) ? regiony.attachNode->title : "") );
	//w
	strAttachType = convertAttachToString( (FBAttachType)regionw.attachType);
	fprintf( f, "\t\t%d,\t\t%s,\t\t\"%s\",\t\t1.0,\n", regionw.value, (char*)strAttachType, (char*)((regionw.attachNode) ? regionw.attachNode->title : "") );
	//h
	strAttachType = convertAttachToString( (FBAttachType)regionh.attachType);
	fprintf( f, "\t\t%d,\t\t%s,\t\t\"%s\",\t\t1.0 );\n", regionh.value, (char*)strAttachType, (char*)((regionh.attachNode) ? regionh.attachNode->title : "") );
}

void uirect::ExportCPPVisual(FILE *f)
{
	if (visual && currComp) {
		currComp->ExportCPP(f, title);
	}
	fprintf( f, "\n" );
}

void uirect::ExportPython(FILE *f)
{
	FBString strAttachType;

	//x
	strAttachType = convertAttachToString( (FBAttachType)regionx.attachType);
	fprintf( f, "\tx = FBAddRegionParam(%d,FBAttachType.%s,\"%s\")\n", regionx.value, (char*)strAttachType, (char*)regionx.attachTo );
	//y
	strAttachType = convertAttachToString( (FBAttachType)regiony.attachType);
	fprintf( f, "\ty = FBAddRegionParam(%d,FBAttachType.%s,\"%s\")\n", regiony.value, (char*)strAttachType, (char*)regiony.attachTo );
	//w
	strAttachType = convertAttachToString( (FBAttachType)regionw.attachType);
	fprintf( f, "\tw = FBAddRegionParam(%d,FBAttachType.%s,\"%s\")\n", regionw.value, (char*)strAttachType, (char*)regionw.attachTo );
	//h
	strAttachType = convertAttachToString( (FBAttachType)regionh.attachType);
	fprintf( f, "\th = FBAddRegionParam(%d,FBAttachType.%s,\"%s\")\n", regionh.value, (char*)strAttachType, (char*)regionh.attachTo );

	fprintf( f, "\tt.AddRegion(\"%s\",\"%s\", x, y, w, h)\n\n", (char*)title, (char*)title );

	if (visual && currComp) {
		currComp->ExportPython(f, title);
	}
	fprintf( f, "\t\n" );
}

void uirect::ExportVar(FILE *f, bool pythonformat)
{
	if (visual && currComp) {
		(pythonformat) ? currComp->ExportPythonVar(f, title) : currComp->ExportCPPVar(f, title);
	}
}

void uirect::FillCompTable(FBSpread &proptable)
{
	if (visual && currComp)
	{
		currComp->PrepareCompTable(proptable);
		currComp->FillCompTable(proptable);
	}
}

void uirect::ComponentCellChange(FBSpread &proptable, int c, int r )
{
	if (!visual) return;

	if (currComp)
	{
		currComp->ComponentCellChange(proptable, c, r);
	}
}


void uirect::FillPropTable(FBSpread &proptable)
{
	int x, y, w, h;
	root->GetRegionPositions( name, false, &x, &y, &w, &h );

	proptable.SetCell(PROP_CELL_TITLE, 0, title);
	//x
	proptable.SetCell(PROP_CELL_X, 0, x );
	proptable.SetCell(PROP_CELL_X_ATTACH_TYPE, 0, regionx.attachType );
	if (regionx.attachNode)
		proptable.SetCell(PROP_CELL_X_ATTACH_NODE, 0, regionx.attachNode->id );
	else
		proptable.SetCell(PROP_CELL_X_ATTACH_NODE, 0, 0 );
	//y
	proptable.SetCell(PROP_CELL_Y, 0, y );
	proptable.SetCell(PROP_CELL_Y_ATTACH_TYPE, 0, regiony.attachType );
	if (regiony.attachNode)
		proptable.SetCell(PROP_CELL_Y_ATTACH_NODE, 0, regiony.attachNode->id );
	else
		proptable.SetCell(PROP_CELL_Y_ATTACH_NODE, 0, 0 );
	//w
	proptable.SetCell(PROP_CELL_W, 0, w );
	proptable.SetCell(PROP_CELL_W_ATTACH_TYPE, 0, regionw.attachType );
	if (regionw.attachNode)
		proptable.SetCell(PROP_CELL_W_ATTACH_NODE, 0, regionw.attachNode->id );
	else
		proptable.SetCell(PROP_CELL_W_ATTACH_NODE, 0, 0 );
	//h
	proptable.SetCell(PROP_CELL_H, 0, h );
	proptable.SetCell(PROP_CELL_H_ATTACH_TYPE, 0, regionh.attachType );
	if (regionh.attachNode)
		proptable.SetCell(PROP_CELL_H_ATTACH_NODE, 0, regionh.attachNode->id );
	else
		proptable.SetCell(PROP_CELL_H_ATTACH_NODE, 0, 0 );
}

void uirect::Create()
{
	root->AddRegion( name, title, 
		regionx.value,	(FBAttachType)regionx.attachType, regionx.attachTo,	1.0	,
		regiony.value,	(FBAttachType)regiony.attachType, regiony.attachTo,	1.0,
		regionw.value,	(FBAttachType)regionw.attachType, regionw.attachTo,	1.0,
		regionh.value,	(FBAttachType)regionh.attachType, regionh.attachTo,	1.0 );
	root->SetControl( name, element );
	root->SetBorder ( name, kFBHighlightBorder, true,true, 1, 0,90,0);

	//root->MoveRegion( name, regionx.value, regiony.value );
	//root->SizeRegion( name, regionw.value, regionh.value );
}

void uirect::ReBuild()
{
	Create();
	if (state == rsSelect)
		CreateSelElement();
	if (visual)
		SetControl(visualType);
}

void uirect::RePosition()
{
	root->MoveRegion(name, regionx.value, regiony.value);
	root->SizeRegion(name, regionw.value, regionh.value);

	if (state == rsSelect)
	{
		FBString selName(name);
		selName = selName + "_sel";

		root->MoveRegion(selName, regionx.value, regiony.value);
		root->SizeRegion(selName, regionw.value, regionh.value);
	}
	if (visual)
	{
		FBString selName(name);
		selName = selName + "_vis";

		root->MoveRegion(selName, regionx.value, regiony.value);
		root->SizeRegion(selName, regionw.value, regionh.value);
	}
}

void uirect::CreateSelElement()
{
	FBString selName(name);
	selName = selName + "_sel";

	root->AddRegion( selName, "", 
		regionx.value,	(FBAttachType)regionx.attachType, regionx.attachTo,	1.0	,
		regiony.value,	(FBAttachType)regiony.attachType, regiony.attachTo,	1.0,
		regionw.value,	(FBAttachType)regionw.attachType, regionw.attachTo,	1.0,
		regionh.value,	(FBAttachType)regionh.attachType, regionh.attachTo,	1.0 );
	root->SetControl( selName, sel);
	root->SetBorder ( selName, kFBPickingBorder, true,true, 1, 0,90,0);
}

void uirect::RemoveSelElement()
{
	FBString selName(name);
	selName = selName + "_sel";

	root->ClearControl(selName);
	if ( root->GetRegion(selName) )
		root->RemoveRegion(selName);
}

void uirect::Init(FBLayout	&_root, FBString _name, FBString _title)
{
	name = _name;
	title = _title;

	root = &_root;
	Create();		
}

RegionAction uirect::testMouseCursor(int mX, int mY)
{
	int x, y, w, h;
	root->GetRegionPositions( name, true, &x, &y, &w, &h );

	if ( (mX > x) && (mX < (w)) && (mY > y) && (mY < (h)) )
	{
		if ( (mX < (x+10)) && (mY < (y+10)) )
			return raMove;
		else
		if (mX < (x+10))
			return raMoveX;
		else
		if (mY < (y+10))
			return raMoveY;
		else
		if ( (mX > (w-10)) && (mY > (h-10)) )
			return raSize;
		else
		if (mX > (w-10))
			return raSizeW;
		else
		if (mY > (h-10))
			return raSizeH;

		return raRegion;
	}
	return raNone;
}

void uirect::Move(int *pX, int *pY, bool relative, int snapstep)
{
	int x, y;
	root->GetRegionPositions( name, false, &x, &y );
	if (pX) 
		if (relative) x += *pX;
		else	x = *pX;
	if (pY)
		if (relative) y += *pY;
		else	y = *pY;

	if (snapstep)
	{
		int step = x / snapstep;
		x = step * snapstep;

		step = y / snapstep;
		y = step * snapstep;
	}

	root->MoveRegion( name, x, y );
	if (state == rsSelect)
	{
		FBString selName;
		selName = name + "_sel";
		root->MoveRegion( selName, x, y );
	}
	if (visual)
	{
		FBString selName;
		selName = name + "_vis";
		root->MoveRegion( selName, x, y );
	}
	regionx.value = x;
	regiony.value = y;
}

void uirect::Size(int *pW, int *pH, bool relative, int snapstep)
{
	int x, y, w, h;
	root->GetRegionPositions( name, false, &x, &y, &w, &h );

	if (pW)
		if (relative) w += *pW;
		else w = *pW;
	if (pH)
		if (relative) h += *pH;
		else h = *pH;
	
	if (snapstep)
	{
		int step = w / snapstep;
		w = step * snapstep;

		step = h / snapstep;
		h = step * snapstep;
	}

	root->SizeRegion( name, w, h );
	if (state == rsSelect)
	{
		FBString selName;
		selName = name + "_sel";
		root->SizeRegion( selName, w, h );
	}
	if (visual)
	{
		FBString selName;
		selName = name + "_vis";
		root->SizeRegion( selName, w, h );
	}
	regionw.value = w;
	regionh.value = h;
}

void uirect::UpdateComponentPointer()
{
	currComp = NULL;

	switch(visualType)
	{
		// case component
		#define COMPCLASS(name, objName) case name: \
			currComp = &v##objName; break;
		#include "components_list.h"
		#undef COMPCLASS
	}
}

void uirect::attachVisualControl()
{
	ClearControl();

	FBString visName(name);
	visName = visName + "_vis";

	if (root->GetRegion(visName) )
	{
		if (root->GetControl(visName) )
			root->ClearControl(visName);
		root->RemoveRegion(visName);
	}

	root->AddRegion( visName, "", 
		regionx.value,	(FBAttachType)regionx.attachType, regionx.attachTo,	1.0	,
		regiony.value,	(FBAttachType)regiony.attachType, regiony.attachTo,	1.0,
		regionw.value,	(FBAttachType)regionw.attachType, regionw.attachTo,	1.0,
		regionh.value,	(FBAttachType)regionh.attachType, regionh.attachTo,	1.0 );

	UpdateComponentPointer();

	visual = true;
	// go back to preview mode
	if (previewMode)
		PreviewEnter();
}

void uirect::SetControl(int type)
{
	if (visual)
		ClearControl();

	//visual = true;
	visualType = type;

	attachVisualControl();
}

void uirect::ClearControl()
{
	if (visual)
	{
		FBString visName(name);
		visName = visName + "_vis";

		//root->ClearControl( visName );
		if ( root->GetRegion(visName) )
		{
			if (root->GetControl(visName))
				root->ClearControl(visName);
			if (!root->RemoveRegion( visName ) )
				FBMessageBox( "Clear control", "remove region failed!", "Ok" );
		}
		visual = false;
		//previewMode = false;
		currComp = NULL;
		//Rename(title);
	}
}

void uirect::PreviewEnter()
{
	if (visual && currComp) {
		FBString visName(name);
		visName = visName + "_vis";

		if ( root->GetRegion(visName) )
		{
			if (root->GetControl(visName))
				root->ClearControl(visName);
			currComp->SetControl( root, visName );
		}
		else
			FBMessageBox("Preview mode", "entering failed!", "Ok" );
	}
	previewMode = true;
}

void uirect::PreviewClose()
{
	if (visual)
	{
		FBString visName(name);
		visName = visName + "_vis";

		root->ClearControl( visName );
	}
	previewMode = false;
}

void uirect::ChangeAttachType( int *tX, int *tY, int *tW, int *tH )
{
	int oldRegionX, oldRegionY, oldRegionW, oldRegionH;
	int tempX, tempY, tempW, tempH;
	bool	isPreview;

	if (tX) regionx.attachType = *tX;
	if (tY) regiony.attachType = *tY;
	if (tW) regionw.attachType = *tW;
	if (tH) regionh.attachType = *tH;

	
	root->GetRegionPositions( name, true, &oldRegionX, &oldRegionY, &oldRegionW, &oldRegionH );
	
	regionx.value = regiony.value = 0;
	regionw.value = regionh.value = 0;

	if (state == rsSelect)
		RemoveSelElement();
	isPreview = previewMode;
	if (visual)
		PreviewClose();

	root->ClearControl(name);
	if ( root->GetRegion(name) )
		root->RemoveRegion(name);

	Create();
	// update region position
	root->GetRegionPositions( name, true, &tempX, &tempY, &tempW, &tempH );
	regionx.value = oldRegionX - tempX;
	regiony.value = oldRegionY - tempY;
	root->MoveRegion( name, regionx.value, regiony.value );
	// update region size
	if (regionw.attachType == (int)kFBAttachNone)
		regionw.value = oldRegionW - oldRegionX;
	else
		regionw.value = oldRegionW - tempW;
	
	if (regionh.attachType == (int)kFBAttachNone)
		regionh.value = oldRegionH - oldRegionY;
	else
		regionh.value = oldRegionH - tempH;

	root->SizeRegion( name, regionw.value, regionh.value );

	// select rect & visual component
	if (visual) {
		SetControl(visualType);
		if (isPreview)
			PreviewEnter();
	}
	if (state == rsSelect)
		CreateSelElement();
}

void uirect::ChangeAttachNode( uirect *pX, uirect *pY, uirect *pW, uirect *pH )
{
	int oldRegionX, oldRegionY, oldRegionW, oldRegionH;
	int tempX, tempY, tempW, tempH;
	bool isPreview;

	regionx.attachNode = pX;
	if (pX) regionx.attachTo = pX->name;
	else	regionx.attachTo = "";
	regiony.attachNode = pY;
	if (pY) regiony.attachTo = pY->name;
	else regiony.attachTo = "";
	regionw.attachNode = pW;
	if (pW) regionw.attachTo = pW->name;
	else regionw.attachTo = "";
	regionh.attachNode = pH;
	if (pH) regionh.attachTo = pH->name;
	else regionh.attachTo = "";

	root->GetRegionPositions( name, true, &oldRegionX, &oldRegionY, &oldRegionW, &oldRegionH );
	
	regionx.value = regiony.value = 0;
	regionw.value = regionh.value = 0;

	if (state == rsSelect)
		RemoveSelElement();
	isPreview = previewMode;
	if (visual)
		PreviewClose();

	root->ClearControl(name);
	if ( root->GetRegion(name) )
		root->RemoveRegion(name);

	Create();
	// update region position
	root->GetRegionPositions( name, true, &tempX, &tempY, &tempW, &tempH );
	regionx.value = oldRegionX - tempX;
	regiony.value = oldRegionY - tempY;
	root->MoveRegion( name, regionx.value, regiony.value );
	// update region size
	if (regionw.attachType == (int)kFBAttachNone)
		regionw.value = oldRegionW - oldRegionX;
	else
		regionw.value = oldRegionW - tempW;
	
	if (regionh.attachType == (int)kFBAttachNone)
		regionh.value = oldRegionH - oldRegionY;
	else
		regionh.value = oldRegionH - tempH;

	root->SizeRegion( name, regionw.value, regionh.value );
	// select rect and visual component
	if (visual) {
		SetControl(visualType);
		if (isPreview)
			PreviewEnter();
	}
	if (state == rsSelect)
		CreateSelElement();
}

void uirect::UpdateAttachNode( uirect *pX, uirect *pY, uirect *pW, uirect *pH )
{
	regionx.attachNode = pX;
	regiony.attachNode = pY;
	regionw.attachNode = pW;
	regionh.attachNode = pH;
}

void uirect::UpdateAttachTo()
{
	regionx.UpdateAttachTo();
	regiony.UpdateAttachTo();
	regionw.UpdateAttachTo();
	regionh.UpdateAttachTo();
}

//
/////////////////////////////////////////////////////////// uimain
//

uimain::uimain()
{
	// define visual components
	for (int i=0; i<MAX_UI_RECTS; i++)
	{
		rects[i].vButton.DefineButton( &UI_Buttons[i] );
		store[i].vButton.DefineButton( &UI_Buttons[i] );

		rects[i].vLabel.DefineLabel( &UI_Labels[i] );
		store[i].vLabel.DefineLabel( &UI_Labels[i] );

		rects[i].vEdit.DefineEdit( &UI_Edits[i] );
		store[i].vEdit.DefineEdit( &UI_Edits[i] );
	}

	makeChanges = false;

	ndx=0;
	ndx2=0;
	count = 0;
	mSnapStep = 0;

	for (int i=0; i<MAX_UI_RECTS; i++)
		rects[i].state = rsDelete;

	pEventsList = NULL;
	SetEventsCount(2);
	SetEventName(0, "OnDeclare", true);
	SetEventName(1, "OnShow");
}


void uimain::Hold()
{
	storeCount = count;
	
	for (int i=0; i<MAX_UI_RECTS; i++)
		store[i].Assign(rects[i]);

	
	makeChanges = true;
}

void uimain::Fetch()
{
	bool selected = false;
	count = storeCount;

	for (int i=0; i<MAX_UI_RECTS; i++)
	{
		if (!rects[i].IsDeleted() )
		{
			rects[i].Delete();
		}
		
		selected = rects[i].IsSelected();

		rects[i].Assign(store[i]);
		if (!rects[i].IsDeleted() )
		{
			if (selected )
					rects[i].RemoveSelElement();
			rects[i].ReBuild();
		}
	}

	uirect *iter = first();
	while (iter != NULL)
	{
		iter->RePosition();
		iter = next();
	}

	makeChanges = false;
}

void uimain::Swap()
{
	// temp buffer
	int temp = count;
	uirect	tempBuffer[MAX_UI_RECTS];

	for (int i=0; i<MAX_UI_RECTS; i++)
		tempBuffer[i].Assign(rects[i]);

	// put the data from the store buffer
	Fetch();

	// fill the store buffer with the temp
	storeCount = temp;
	for (int i=0; i<MAX_UI_RECTS; i++)
		store[i].Assign(tempBuffer[i]);

	makeChanges = false;
}

void uimain::ClearPropTable(FBSpread &proptable)
{
	proptable.SetCell(PROP_CELL_TITLE, 0, "");
	//x
	proptable.SetCell(PROP_CELL_X, 0, 0 );
	proptable.SetCell(PROP_CELL_X_ATTACH_TYPE, 0, (int) kFBAttachNone );
	proptable.SetCell(PROP_CELL_X_ATTACH_NODE, 0, 0 );
	//y
	proptable.SetCell(PROP_CELL_Y, 0, 0 );
	proptable.SetCell(PROP_CELL_Y_ATTACH_TYPE, 0, (int) kFBAttachNone );
	proptable.SetCell(PROP_CELL_Y_ATTACH_NODE, 0, 0 );
	//w
	proptable.SetCell(PROP_CELL_W, 0, 0 );
	proptable.SetCell(PROP_CELL_W_ATTACH_TYPE, 0, (int) kFBAttachNone );
	proptable.SetCell(PROP_CELL_W_ATTACH_NODE, 0, 0 );
	//h
	proptable.SetCell(PROP_CELL_H, 0, 0 );
	proptable.SetCell(PROP_CELL_H_ATTACH_TYPE, 0, (int) kFBAttachNone );
	proptable.SetCell(PROP_CELL_H_ATTACH_NODE, 0, 0 );
}

void uimain::New()
{
	mTool.Clear();

	for (int i=0; i<MAX_UI_RECTS; i++)
	{
		if ( !rects[i].IsDeleted() )
			rects[i].Delete();
		rects[i].Clear();

		rects[i].vLabel.DefineLabel( &UI_Labels[i] );
		store[i].vLabel.DefineLabel( &UI_Labels[i] );
	}
	ndx = 0;
	ndx2 = 0;
	count = 0;

	ClearEvents();
}

void uimain::Save(char *filename)
{
	FILE	*f = fopen(filename, "w");
	if (f == NULL) return;
	fseek(f, 0, 0);

	// this tool events
	SaveEvents(f);

	// number of elements
	fprintf( f, "%d\n", count );

	uirect *iter = first();
	while (iter != NULL)
	{
		iter->Save(f);
		iter = next();
	}

	fclose(f);
}



void uimain::Load(char *filename, FBLayout &mFormView, FBSpread &mPropTable)
{
	// load from *.ui file
	FILE	*f = fopen(filename, "r");
	if (f == NULL) return;

	// clear scene before loading new
	New();

	fseek(f, 0, 0);

	int n;
	//FBString name, title, attachTo;

	char buffer[2048];
	//char ptr[80];

	LoadEvents(f);

	// number of elements
	memset(buffer,0,sizeof(char)*2048);
	fgets(buffer, 2048, f);
	sscanf( buffer, "%d", &n );

	// allocate all elements
	for (int i=0; i<n; i++)
	{
		// create new region
		uirect	&rect = AddElement();

		// read parameters
		rect.Load(f);

		rect.Init(mFormView, rect.name, rect.title);
	}

	// update attach node pointers
	for (int i=0; i<n; i++)
	{
		uirect *pX, *pY, *pW, *pH;
		pX = FindNode( (char*)rects[i].regionx.attachTo );
		pY = FindNode( (char*)rects[i].regiony.attachTo );
		pW = FindNode( (char*)rects[i].regionw.attachTo );
		pH = FindNode( (char*)rects[i].regionh.attachTo );
		rects[i].UpdateAttachNode(pX, pY, pW, pH);

		// update all visual part
		if (rects[i].visual)
			rects[i].attachVisualControl();
	}

	fclose(f);
}

void uimain::ExportCPP(char *fileName)
{
	FILE	*f = fopen(fileName, "w");
	if (f == NULL) return;
	fseek(f, 0,0);

	fprintf( f, "\n// c++ - OpenReality SDK source code\n\n" );

	//
	// export all variables
	//
	fprintf( f, "// all variables for the visual controls\n" );
	uirect *iter = first();
	while (iter != NULL)
	{
		iter->ExportVar(f, false);
		iter = next();
	}
	fprintf( f, "\n" );

	//
	// export all events functions
	//
	fprintf( f, "// all events for the visual controls\n" );
	ExportCPPEvents(f);
	fprintf( f, "\n" );

	//
	// export all regions
	//
	fprintf( f, "void UICreate()\n{\n" );
	fprintf( f, "// add regions\n" );
	
	iter = first();
	while (iter != NULL)
	{
		iter->ExportCPP(f);
		iter = next();
	}
	fprintf( f, "}\n\n" );

	//
	// config visual components for each region
	//
	fprintf( f, "void UIConfig()\n{\n" );
	fprintf( f, "// add controls\n" );
	
	iter = first();
	while (iter != NULL)
	{
		iter->ExportCPPVisual(f);
		iter = next();
	}
	fprintf( f, "}\n" );

	fclose(f);
}


void uimain::ExportPython(char *fileName)
{
	FILE	*f = fopen(fileName, "w");
	if (f == NULL) return;
	fseek(f, 0,0);

/*
def CreateTool():
    # Tool creation will serve as the hub for all other controls
    t = CreateUniqueTool("Renamer Tool")
    t.StartSizeX = 650
    t.StartSizeY = 500  
    PopulateLayout(t) 
    ShowTool(t)

CreateTool()
*/

	// output Python source code
	fprintf( f, "from pyfbsdk import *\n" );
	fprintf( f, "from pyfbsdk_additions import *\n\n" );

	// export all variables
	uirect *iter = first();
	while (iter != NULL)
	{
		iter->ExportVar(f);
		iter = next();
	}
	fprintf( f, "\n" );

	// export all events functions
	ExportPythonEvents(f);

	fprintf( f, "def PopulateTool(t):\n" );
	fprintf( f, "\t#populate regions here\n\n" );

	// output layout configuration (regions and visual components)
	bool isEmpty = true;
	iter = first();
	while (iter != NULL)
	{
		iter->ExportPython(f);
		iter = next();
		isEmpty = false;
	}

	if (isEmpty)
	{
		fprintf( f, "\tpass\n\n" );
	}

	// output create tool function
	fprintf( f, "def CreateTool():\n" );
	fprintf( f, "\tt = FBCreateUniqueTool(\"%s\")\n", mTool.name );
	fprintf( f, "\tt.StartSizeX = %d\n", mTool.sizex );
	fprintf( f, "\tt.StartSizeY = %d\n", mTool.sizey );
	fprintf( f, "\tPopulateTool(t)\n" );
	fprintf( f, "\tShowTool(t)\n" );
	if ( !isEventEmpty(1) )
		fprintf( f, "\tOnShow()\n" );
/*
	if ( !isEventEmpty(0) )
		fprintf( f, "OnCreate()\n" );
*/
	fprintf( f, "CreateTool()\n" );

	fclose(f);
}

int uimain::ExportXml(const char *fileName)
{
	TiXmlDocument doc;

	TiXmlElement	head("layout");
	head.SetAttribute( "name", mTool.name );
	head.SetAttribute( "x", 0 );
	head.SetAttribute( "y", 0 );
	head.SetAttribute( "width", mTool.sizex );
	head.SetAttribute( "height", mTool.sizey );

	uirect *iter = first();
	while (iter != NULL)
	{
		TiXmlElement	regItem("region");
		regItem.SetAttribute( "name", iter->name );
		regItem.SetAttribute( "x", iter->regionx.value );
		regItem.SetAttribute( "y", mTool.sizey - iter->regiony.value - iter->regionh.value );
		regItem.SetAttribute( "w", iter->regionw.value );
		regItem.SetAttribute( "h", iter->regionh.value );
		
		if (iter->visual)
		{
			TiXmlElement cItem( "component" );
			switch(iter->visualType)
			{
				// case component
				#define COMPCLASS(name, objName) case name: \
					cItem.SetAttribute( "class", ""#objName ); break;
				#include "components_list.h"
				#undef COMPCLASS
			}
			regItem.InsertEndChild( cItem );
		}
		head.InsertEndChild( regItem );
		iter = next();
	}

	doc.InsertEndChild( head );
	doc.SaveFile(fileName);

	if (doc.Error() )
	{
		printf( doc.ErrorDesc() );
		return 0;
	}

	return 1;
}

void uimain::FillPropTable(FBSpread &proptable)
{
	/*
	uirect *iter = last();
	if (iter)
		iter->FillPropTable(proptable);
	else
	{
		ClearPropTable(proptable);
	}
	*/

	uirect	*iter = first();
	{while (iter != NULL)
	{
		if (iter->IsSelected())
			iter->FillPropTable(proptable);
		iter = next();
	}}
}

void uimain::FillCompTable( FBSpread &proptable)
{
	proptable.Clear();
/*
	uirect *iter = last();
	if (iter)
		iter->FillCompTable(proptable);
		*/
	uirect	*iter = first();
	{while (iter != NULL)
	{
		if (iter->IsSelected())
			iter->FillCompTable(proptable);
		iter = next();
	}}
}

void uimain::Rename(const char *newName)
{
	Hold();

	uirect	*iter = first();
	{while (iter != NULL)
	{
		if (iter->IsSelected())
			iter->Rename(newName);
		iter = next();
	}}

	// update new name in nodes
	iter = first();
	{while (iter != NULL)
	{
		iter->UpdateAttachTo();
		iter = next();
	}}
}

void uimain::Move(int *pX, int *pY, bool relative, bool hold)
{
	if (hold) Hold();

	uirect	*iter = first();
	{while (iter != NULL)
	{
		if (iter->IsSelected())
			iter->Move(pX, pY, relative, 0);
		iter = next();
	}}
}
void uimain::Size(int *pW, int *pH, bool relative, bool hold)
{
	if (hold) Hold();

	uirect	*iter = first();
	{while (iter != NULL)
	{
		if (iter->IsSelected())
			iter->Size(pW, pH, relative, 0);
		iter = next();
	}}
}
void uimain::SnapRegions()
{
	uirect	*iter = first();
	{while (iter != NULL)
	{
		if (iter->IsSelected())
		{
			iter->Move(0, 0, true, mSnapStep);
			iter->Size(0, 0, true, mSnapStep);
		}
		iter = next();
	}}
}

void uimain::ChangeAttachType( int *tX, int *tY, int *tW, int *tH )
{
	Hold();

	int count = GetSelectionCount();

	for (int i=0; i<count; i++)
	{
		uirect	*iter = first();
		{while (iter != NULL)
		{
			iter->HoldPosition();
			iter = next();
		}}

		int n=0;
		iter = first();
		{while (iter != NULL)
		{
			if (iter->IsSelected())
			{
				if (n==i)
				{
					iter->ChangeAttachType(tX, tY, tW, tH);
					break;
				}
				n++;
			}
			iter = next();
		}}

		// restore positions of connected nodes
		n=0;
		iter = first();
		{while (iter != NULL)
		{
			if (!iter->IsSelected())
				iter->FetchPosition();
			else
			{
				if (n!=i)
					iter->FetchPosition();
				n++;
			}
			iter = next();
		}}
	}
}

uirect	*uimain::FindNode(char *lName)
{
	if (strlen(lName)==0) return NULL;

	uirect *iter = first();
	{while(iter!=NULL)
	{
		if ( strstr((char*)iter->name, lName) != NULL)
			return iter;
		iter = next();
	}}
	return NULL;
}

void uimain::ChangeAttachNode( int *tX, int *tY, int *tW, int *tH )
{
	Hold();

	uirect	*pX = NULL;
	uirect	*pY = NULL;
	uirect	*pW = NULL;
	uirect	*pH = NULL;

	int count = GetSelectionCount();

	for (int i=0; i<count; i++)
	{
		uirect *iter = first();
		{while(iter!=NULL)
		{
			if ( tX && (iter->id == *tX) )
				pX = iter;
			if ( tY && (iter->id == *tY) )
				pY = iter;
			if ( tW && (iter->id == *tW) )
				pW = iter;
			if ( tH && (iter->id == *tH) )
				pH = iter;
			iter->HoldPosition();
			iter = next();
		}}

		int n = 0;
		iter = first();
		{while (iter != NULL)
		{
			if (iter->IsSelected())
			{
				if (n==i) {
					iter->ChangeAttachNode(pX, pY, pW, pH);
					break;
				}
				n++;
			}
			iter = next();
		}}

		// fetch all rects positions
		n=0;
		iter = first();
		{while (iter != NULL)
		{
			if (!iter->IsSelected())
				iter->FetchPosition();
			else
			{
				if (n!=i)
					iter->FetchPosition();
				n++;
			}
			iter = next();
		}}
	}
}

void uimain::UpdateAttachNodes()
{
	uirect *iter = first();
	{while(iter!=NULL)
	{
		// for X
		if (iter->regionx.attachTo != "")
		{
			uirect	*pNode = first(true);
			{while(iter!=NULL)
			{
				if (iter->regionx.attachTo == pNode->name)
				{
					iter->regionx.attachNode = pNode;
					break;
				}
				next(true);
			}}
		}
		// for Y
		if (iter->regiony.attachTo != "")
		{
			uirect	*pNode = first(true);
			{while(iter!=NULL)
			{
				if (iter->regiony.attachTo == pNode->name)
				{
					iter->regiony.attachNode = pNode;
					break;
				}
				next(true);
			}}
		}
		// for W
		if (iter->regionw.attachTo != "")
		{
			uirect	*pNode = first(true);
			{while(iter!=NULL)
			{
				if (iter->regionw.attachTo == pNode->name)
				{
					iter->regionw.attachNode = pNode;
					break;
				}
				next(true);
			}}
		}
		// for H
		if (iter->regionh.attachTo != "")
		{
			uirect	*pNode = first(true);
			{while(iter!=NULL)
			{
				if (iter->regionh.attachTo == pNode->name)
				{
					iter->regionh.attachNode = pNode;
					break;
				}
				next(true);
			}}
		}

		iter = next();
	}}
}

uirect&		uimain::AddElement()
{
	Hold();

	int i=0;
	ndx2=0;
	while (i<MAX_UI_RECTS)
	{
		if (rects[i].IsDeleted() )
		{
			rects[i].state = rsNone;
			rects[i].id = ndx2 + 1;
			count++;
			return rects[i];
		}
		else
		{
			if (ndx2 >= count)
			{
				rects[i].state = rsNone;
				rects[i].id = ndx2 + 1;
				count++;
				return rects[i];
			}
			ndx2++;
		}

		i++;
	}
	return rects[0];
}

uirect*		uimain::first(bool secondloop)
{
	if (!count) return NULL;
	
	if (secondloop)
	{
		ndx_2 = 0;
		ndx_22 = 0;
	}
	else
	{
		ndx=0;
		ndx2=0;
	}

	int i=0;
	while (i<MAX_UI_RECTS)
	{
		if (!rects[i].IsDeleted() )
		{
			if (secondloop)
			{
				ndx_2 = i;
				ndx_22=1;
			}
			else
			{
				ndx = i;
				ndx2=1;
			}
			return &rects[i];
		}
		i++;
	}
	return NULL;
}
uirect*		uimain::last(bool secondloop)
{
	if (!count) return NULL;

	if (secondloop)
	{
		ndx_2=MAX_UI_RECTS-1;
		ndx_22=count;
	}
	else
	{
		ndx=MAX_UI_RECTS-1;
		ndx2=count;
	}

	int i=MAX_UI_RECTS-1;
	while (i>=0)
	{
		if (!rects[i].IsDeleted() )
		{
			if (secondloop)
			{
				ndx_2 = i;
				ndx_22 = count-1;
			}
			else
			{
				ndx = i;
				ndx2 = count-1;
			}
			return &rects[i];
		}
		i--;
	}
	return NULL;
}
uirect*		uimain::next(bool secondloop)
{
	if (!count) return NULL;
	if (secondloop)
		if (ndx_22 >= count) return NULL;
	else
		if (ndx2 >= count) return NULL;

	int i;
	(secondloop) ? (i=ndx_2+1) : (i=ndx+1);
	while (i<MAX_UI_RECTS)
	{
		if (!rects[i].IsDeleted() )
		{
			if (secondloop)
			{
				ndx_2 = i;
				ndx_22++;
			}
			else
			{
				ndx = i;
				ndx2++;
			}
			return &rects[i];
		}
		i++;
	}
	return NULL;
}
uirect*		uimain::prev(bool secondloop)
{
	if (!count) return NULL;
	if (secondloop)
		if (ndx_22 < 0) return NULL;
	else
		if (ndx2 < 0) return NULL;

	int i;
	(secondloop) ? (i=ndx_2-1) : (i=ndx-1);
	while (i>=0)
	{
		if (!rects[i].IsDeleted() )
		{
			if (secondloop)
			{
				ndx_2 = i;
				ndx_22--;
			}
			else
			{
				ndx = i;
				ndx2--;
			}
			return &rects[i];
		}
		i--;
	}
	return NULL;
}

uirect& uimain::GetAt(int i) {

	uirect	*iter = first();
	while (iter != NULL)
	{
		if (i == ndx2) return rects[ndx];
		iter = next();
	}
	return rects[0];
}

void uimain::SelectAll()
{
	Hold();

	uirect	*iter = first();
	while (iter != NULL)
	{
		iter->Select();
		iter = next();
	}
}

void uimain::SelectInvert()
{
	Hold();

	uirect	*iter = first();
	while (iter != NULL)
	{
		if ( iter->IsSelected() )
			iter->Select(false);
		else
			iter->Select();
		iter = next();
	}
}

void uimain::SelectNone()
{
	Hold();

	uirect	*iter = first();
	while (iter != NULL)
	{
		iter->Select(false);
		iter = next();
	}
}

int uimain::GetSelectionCount()
{
	int selCount = 0;

	uirect	*iter = first();
	while (iter != NULL)
	{
		if (iter->IsSelected())
			selCount++;
		iter = next();
	}
	return selCount;
}

void uimain::DeleteSel()
{
	Hold();

	uirect*		iter = last();
	while (iter != NULL)
	{
		if (iter->IsSelected() )
		{
			iter->Delete();
			count--;
		}
		iter = prev();
	}
}

void uimain::GenerateUniqueName(char *name, char *title)
{
	bool unique = false;

	int i=0;
	char	str[256];
	while (!unique)
	{
		i++;
		memset( &str[0], 0, sizeof(char)*256 );
		sprintf( (char*)str, "region%d", i );

		unique = true;
		uirect	*iter = first();
		while (iter != NULL)
		{
			if ( strstr( (char*)iter->name, str) != NULL )
			{
				unique = false;
				break;
			}
			iter = next();
		}			
	}
	//strset(name, 0);
	strcpy( name, (char*)str);
	//strset(title, 0);
	strcpy( title, (char*)str);
}

void uimain::AssignComponent(int type)
{
	//if (hold) Hold();

	uirect	*iter = first();
	{while (iter != NULL)
	{
		if (iter->IsSelected())
		{
			switch(type)
			{
			case COMPONENT_CLEAR:
				if (iter->visual) 
					iter->ClearControl();
				break;
			default:
				iter->SetControl(type);
				break;
			}
		}
		iter = next();
	}}	
}

void uimain::PreviewEnter()
{
	uirect	*iter = first();
	{while (iter != NULL)
	{
		iter->PreviewEnter();
		iter = next();
	}}	
}

void uimain::PreviewClose()
{
	uirect	*iter = first();
	{while (iter != NULL)
	{
		iter->PreviewClose();
		iter = next();
	}}	
}

void uimain::ComponentCellChange(FBSpread &proptable, int c, int r )
{
	uirect	*iter = first();
	{while (iter != NULL)
	{
		if (iter->IsSelected())
		{
			iter->ComponentCellChange(proptable, c,r);
		}
		iter = next();
	}}	
}