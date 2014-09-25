#include ".\handsimage.h"
#include ".\log.h"
#include "StringUtils.h"

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

#define CHARACTER_CTRL_IMAGE	"\\system\\handtool\\character.tga"
#define HANDS_CTRL_IMAGE		"\\system\\handtool\\hands.tga"
#define FINGER_CTRL_IMAGE		"\\system\\handtool\\region_drop.tga"
#define FINGER_CTRL_IMAGE_25	"\\system\\handtool\\region_drop_25.tga"
#define FINGER_CTRL_IMAGE_50	"\\system\\handtool\\region_drop_50.tga"
#define FINGER_CTRL_IMAGE_75	"\\system\\handtool\\region_drop_75.tga"
#define FINGER_CTRL_IMAGE_100	"\\system\\handtool\\region_drop_100.tga"
#define SELECTED_CTRL_IMAGE		"\\system\\handtool\\region_selected.tga"

#define CONFIG_FILENAME			"\\config\\handtool\\fingers.cfg"

LOG		g_Log;

/************************************************
 *	Load texture from TGA and put it into video memory (return texture index).
 ************************************************/
GLuint LoadTGA( char *filename ) 
{
	GLubyte		TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};	//Uncompressed TGA header
	GLubyte		TGAcompare[12];								//Used to compare TGA header
	GLubyte		header[6];									//The first six useful bytes from the header
	GLuint		bytesPerPixel;								//Holds the bpp of the TGA
	GLuint		imageSize;									//Used to store image size while in RAM
	GLuint		temp;										//Temp variable
	GLuint		type=GL_RGBA;								//Set the default OpenGL mode to RBGA (32 BPP)
	//-- texture image data
	GLuint		nWidth;										// texture width
	GLuint		nHeight;									// texture height
	GLuint		nBpp;										// color depth
	GLubyte		*pData;										// image pixel data
	GLuint		texID=0;									// generated id in memory of this texture 

	FILE* file = fopen( filename, "rb" );						// Open The TGA File

	if(file==NULL													   ||	// Does File Even Exist?
		fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0			   ||	// Does The Header Match What We Want?
		fread(header,1,sizeof(header),file)!=sizeof(header))					// If So Read Next 6 Header Bytes
		{
		if(file==NULL)									// Did The File Even Exist? *Added Jim Strong*
			{
			
			return false;
			}
		else
			{
			fclose(file);						// If anything failed, close the file
			
			return false;
			}
		}

	nWidth  = header[1] * 256 + header[0];		// Determine The TGA Width	(highbyte*256+lowbyte)
	nHeight = header[3] * 256 + header[2];		// Determine The TGA Height	(highbyte*256+lowbyte)

 	if(nWidth	<=0	||								// Is The Width Less Than Or Equal To Zero
		nHeight<=0	||								// Is The Height Less Than Or Equal To Zero
		(header[4]!=24 && header[4]!=32))				// Is The TGA 24 or 32 Bit?
		{
		fclose(file);									// If Anything Failed, Close The File

		return false;
		}

	nBpp		= header[4];						// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel		= nBpp/8;						// Divide By 8 To Get The Bytes Per Pixel
	imageSize			= nWidth * nHeight*bytesPerPixel;	// Calculate The Memory Required For The TGA Data

	pData=(GLubyte *)malloc(imageSize);		// Reserve Memory To Hold The TGA Data

	if(pData==NULL ||							// Does The Storage Memory Exist?
		fread(pData, 1, imageSize, file)!=imageSize)	// Does The Image Size Match The Memory Reserved?
		{
		if(pData!=NULL)						// Was Image Data Loaded
			free(pData);						// If So, Release The Image Data

		fclose(file);								// Close The File
		return false;								// Return False
		}

	for(GLuint i=0; i<int(imageSize); i+=bytesPerPixel)		// Loop Through The Image Data
		{										// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp=pData[i];					// Temporarily Store The Value At Image Data 'i'
		pData[i] = pData[i + 2];	// Set The 1st Byte To The Value Of The 3rd Byte
		pData[i + 2] = temp;				// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
		}

	fclose (file);								//Close the file

	// Build A Texture From The Data
	glGenTextures(1, &texID);				//Generate OpenGL texture IDs

	glBindTexture(GL_TEXTURE_2D, texID);			//Bind the texture to a texture object
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	//Filtering for if texture is bigger than should be
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//Filtering for if texture is smaller than it should be

	if(nBpp==24)							//Was the TGA 24 bpp?
		type=GL_RGB;

	glTexImage2D(GL_TEXTURE_2D, 0, type, nWidth, nHeight, 0, type, GL_UNSIGNED_BYTE, pData);

	//-- after putting texture in video memory, just free storage buffer
	if (pData != NULL)
	{
		free(pData);
	}

	return texID;
}


void CHandsImage::FillHandLeftFinger( int n, float x, float y, bool selected, FBModel *pModel )
{
	mHandLeft[n].x = x;
	mHandLeft[n].y = HANDS_IMAGE_HEIGHT - y;
	mHandLeft[n].selected = selected;
	mHandLeft[n].mModel = pModel;
}

void CHandsImage::FillHandRightFinger( int n, float x, float y, bool selected, FBModel *pModel )
{
	mHandRight[n].x = x;
	mHandRight[n].y = HANDS_IMAGE_HEIGHT - y;
	mHandRight[n].selected = selected;
	mHandRight[n].mModel = pModel;
}

/************************************************
 *	CHandsImage Constructor.
 ************************************************/
CHandsImage::CHandsImage(void)
{
	FBStringList list;
	FBString appPath = mSystem.ApplicationPath;
	SplitPath( appPath, list );

	appPath = "";
	for (int i=0; i<list.GetCount(); ++i)
	{
		appPath = appPath + list[i];
		appPath = appPath + "\\";
		if (strcmp(list[i], "bin") == 0) break;
	}

	FBString		szFullFilePath( appPath, HANDS_CTRL_IMAGE );
	FBString		szRegionDropPath( appPath, FINGER_CTRL_IMAGE );
	FBString		szRegionDropPath25( appPath, FINGER_CTRL_IMAGE_25 );
	FBString		szRegionDropPath50( appPath, FINGER_CTRL_IMAGE_50 );
	FBString		szRegionDropPath75( appPath, FINGER_CTRL_IMAGE_75 );
	FBString		szRegionDropPath100( appPath, FINGER_CTRL_IMAGE_100 );
	FBString		szRegionSelPath( appPath, SELECTED_CTRL_IMAGE );

	mHandsId = LoadTGA( szFullFilePath );
	mRegionDropId = LoadTGA( szRegionDropPath );
	mRegionDropId25		= LoadTGA( szRegionDropPath25 );
	mRegionDropId50		= LoadTGA( szRegionDropPath50 );
	mRegionDropId75		= LoadTGA( szRegionDropPath75 );
	mRegionDropId100	= LoadTGA( szRegionDropPath100 );
	mRegionSelectedId = LoadTGA( szRegionSelPath );

	mSelected = false;
	mReDraw = true;

	//-------------------
	//-- LEFT HAND
	FillHandLeftFinger( 0,	15, 95 );

	//-- index
	FillHandLeftFinger( 1,	50, 53 );
	FillHandLeftFinger( 2,	58, 43 );
	FillHandLeftFinger( 3,	68, 32 );
	//-- middle
	FillHandLeftFinger( 4,	36, 49 );
	FillHandLeftFinger( 5,	41, 35 );
	FillHandLeftFinger( 6,	45, 21 );
	//-- ring
	FillHandLeftFinger( 7,	25, 52 );
	FillHandLeftFinger( 8,	25, 38 );
	FillHandLeftFinger( 9,	25, 25 );
	//-- pinky
	FillHandLeftFinger( 10,		14, 57 );
	FillHandLeftFinger( 11,		10, 45 );
	FillHandLeftFinger( 12,		6, 34 );

	//-- thumb
	FillHandLeftFinger( 13,		58, 80 );
	FillHandLeftFinger( 14,		66, 74 );
	FillHandLeftFinger( 15,		76, 65 );

	//------------------------
	//-- RIGHT HAND
	FillHandRightFinger( 0,	15, 95 );

	//-- index
	FillHandRightFinger( 1,	50, 53 );
	FillHandRightFinger( 2,	58, 43 );
	FillHandRightFinger( 3,	68, 32 );
	//-- middle
	FillHandRightFinger( 4,	36, 49 );
	FillHandRightFinger( 5,	41, 35 );
	FillHandRightFinger( 6,	45, 21 );
	//-- ring
	FillHandRightFinger( 7,	25, 52 );
	FillHandRightFinger( 8,	25, 38 );
	FillHandRightFinger( 9,	25, 25 );
	//-- pinky
	FillHandRightFinger( 10,		14, 57 );
	FillHandRightFinger( 11,		10, 45 );
	FillHandRightFinger( 12,		6, 34 );

	//-- thumb
	FillHandRightFinger( 13,		58, 80 );
	FillHandRightFinger( 14,		66, 74 );
	FillHandRightFinger( 15,		76, 65 );

	//-- try to load finger position's from config file
	FBString		szConfigPath( appPath, CONFIG_FILENAME );
	LoadFingerConfig( szConfigPath );

	UpdateWeight();
}


/************************************************
 *	free usage objects and relation's.
 ************************************************/
void CHandsImage::CleanUP()
{
	for (int i=0; i<FINGERS_COUNT; i++)
	{
		mHandLeft[i].CleanUP();
		mHandRight[i].CleanUP();
	}
}

void CHandsImage::FillHandLeftModels( FBModel *pHandModel )
{
	if (pHandModel == NULL)
	{
		for (int i=0; i<FINGERS_COUNT; i++)
			mHandLeft[i].mModel = NULL;
	}
	else
	if ( pHandModel->Children.GetCount() != 0 )
	{
		mHandLeft[0].mModel = pHandModel;
		//-- index
		mHandLeft[1].mModel = pHandModel->Children[1];
		mHandLeft[2].mModel = pHandModel->Children[1]->Children[0];
		mHandLeft[3].mModel = pHandModel->Children[1]->Children[0]->Children[0];
		//-- middle
		mHandLeft[4].mModel = pHandModel->Children[2];
		mHandLeft[5].mModel = pHandModel->Children[2]->Children[0];
		mHandLeft[6].mModel = pHandModel->Children[2]->Children[0]->Children[0];
		//-- ring
		mHandLeft[7].mModel = pHandModel->Children[3];
		mHandLeft[8].mModel = pHandModel->Children[3]->Children[0];
		mHandLeft[9].mModel = pHandModel->Children[3]->Children[0]->Children[0];
		//-- pinky
		mHandLeft[10].mModel = pHandModel->Children[4];
		mHandLeft[11].mModel = pHandModel->Children[4]->Children[0];
		mHandLeft[12].mModel = pHandModel->Children[4]->Children[0]->Children[0];
		//-- thumb
		mHandLeft[13].mModel = pHandModel->Children[0];
		mHandLeft[14].mModel = pHandModel->Children[0]->Children[0];
		mHandLeft[15].mModel = pHandModel->Children[0]->Children[0]->Children[0];
	}
}

void CHandsImage::FillHandRightModels( FBModel *pHandModel )
{
	if (pHandModel == NULL)
	{
		for (int i=0; i<FINGERS_COUNT; i++)
			mHandRight[i].mModel = NULL;
	}
	else
	if ( pHandModel->Children.GetCount() != 0 )
	{
		mHandRight[0].mModel = pHandModel;
		//-- index
		mHandRight[1].mModel = pHandModel->Children[1];
		mHandRight[2].mModel = pHandModel->Children[1]->Children[0];
		mHandRight[3].mModel = pHandModel->Children[1]->Children[0]->Children[0];
		//-- middle
		mHandRight[4].mModel = pHandModel->Children[2];
		mHandRight[5].mModel = pHandModel->Children[2]->Children[0];
		mHandRight[6].mModel = pHandModel->Children[2]->Children[0]->Children[0];
		//-- ring
		mHandRight[7].mModel = pHandModel->Children[3];
		mHandRight[8].mModel = pHandModel->Children[3]->Children[0];
		mHandRight[9].mModel = pHandModel->Children[3]->Children[0]->Children[0];
		//-- pinky
		mHandRight[10].mModel = pHandModel->Children[4];
		mHandRight[11].mModel = pHandModel->Children[4]->Children[0];
		mHandRight[12].mModel = pHandModel->Children[4]->Children[0]->Children[0];
		//-- thumb
		mHandRight[13].mModel = pHandModel->Children[0];
		mHandRight[14].mModel = pHandModel->Children[0]->Children[0];
		mHandRight[15].mModel = pHandModel->Children[0]->Children[0]->Children[0];
	}
}

void CHandsImage::ResetSelection(void)
{
	//bool selected = false;

	//-- list for storing selection
	FBModelList	*lModelList = FBCreateModelList();

	if (lModelList)
	{
		//-- put all selected models in this list
		FBGetSelectedModels( *lModelList );

		//-- reset selection flag for each model
		for (int i=0; i<lModelList->GetCount(); i++)
		{
			lModelList->GetAt(i)->Selected = false;
		}

		/*
		for (int i=0; i<lModelList->GetCount(); i++)
		{
			FBModel *lModel = lModelList->GetAt(i);

			selected = false;
			for( int j=0; j<FINGERS_COUNT; j++)
			{
				if (mHandLeft[j].mModel == lModel)
				{
					selected = true;
					break;
				}

				if (mHandRight[j].mModel == lModel)
				{
					selected = true;
					break;
				}
			}

			lModel->Selected = selected;
		}*/


		FBDestroyModelList( lModelList );
	}
}

void CHandsImage::CheckHandsSelection(void)
{
	if (mHandLeft[0].mModel)
		for (int i=0; i<FINGERS_COUNT; i++)
			mHandLeft[i].selected = mHandLeft[i].mModel->Selected;
	if (mHandRight[0].mModel)
		for (int i=0; i<FINGERS_COUNT; i++)
			mHandRight[i].selected = mHandRight[i].mModel->Selected;

	FBView::Refresh( true );
}

/************************************************
 *	Load markers position.
 ************************************************/
int CHandsImage::LoadFingerConfig( FBString fileName )
{
	struct CFingerPos
	{
		int x;
		int y;
	} fingerPos;

	FILE *f = fopen( fileName, "r" );
	if (f == NULL)
		return false;

	fseek( f, 0,0 );

	//-- load left hand config
	for (int i=0; i<FINGERS_COUNT; i++ )
	{
		if (fread( &fingerPos, 1, sizeof(CFingerPos), f ) != sizeof(CFingerPos))
			return false;

		mHandLeft[i].x = (float) fingerPos.x;
		mHandLeft[i].y = (float) fingerPos.y;
	}

	//-- load right hand config
	for (int i=0; i<FINGERS_COUNT; i++ )
	{
		if (fread( &fingerPos, 1, sizeof(CFingerPos), f ) != sizeof(CFingerPos))
			return false;

		mHandRight[i].x = (float) fingerPos.x;
		mHandRight[i].y = (float) fingerPos.y;
	}

	fclose( f );

	return true;
}

/************************************************
 *	Save markers position.
 ************************************************/
int CHandsImage::SaveFingerConfig( FBString fileName )
{
	struct CFingerPos
	{
		int x;
		int y;
	} fingerPos;

	FILE	*f = fopen( fileName, "w+" );

	if (f == NULL) 
	{
		return false;
	}
	fseek( f, 0, 0 );

	//-- save left hand config
	for (int i=0; i<FINGERS_COUNT; i++)
	{
		fingerPos.x = (int) mHandLeft[i].x;
		fingerPos.y = (int) mHandLeft[i].y;

		if (fwrite( &fingerPos, 1, sizeof(CFingerPos), f ) != sizeof(CFingerPos) )
			return false;
	}

	//-- save right hand config
	for (int i=0; i<FINGERS_COUNT; i++)
	{
		fingerPos.x = (int) mHandRight[i].x;
		fingerPos.y = (int) mHandRight[i].y;

		if (fwrite( &fingerPos, 1, sizeof(CFingerPos), f ) != sizeof(CFingerPos) )
			return false;
	}

	fclose( f );

	return true;
}

/************************************************
 *	CFinger Destructor.
 ************************************************/
CHandsImage::~CHandsImage(void)
{
	if (mHandsId)
		glDeleteTextures( 1, &mHandsId );
	if (mRegionDropId)
		glDeleteTextures( 1, &mRegionDropId );
	if (mRegionDropId25)
		glDeleteTextures( 1, &mRegionDropId25 );
	if (mRegionDropId50)
		glDeleteTextures( 1, &mRegionDropId50 );
	if (mRegionDropId75)
		glDeleteTextures( 1, &mRegionDropId75 );
	if (mRegionDropId100)
		glDeleteTextures( 1, &mRegionDropId100 );
	if (mRegionSelectedId)
		glDeleteTextures( 1, &mRegionSelectedId );

	//-- try to save finger position's from config file
	//FBString		szConfigPath( mSystem.ApplicationPath, CONFIG_FILENAME );
	//SaveFingerConfig( szConfigPath );
}

/************************************************
 *	Refresh view.
 ************************************************/
void CHandsImage::Refresh( bool pNow )
{
	if (mReDraw)
	{
		UpdateWeight();

		FBView::Refresh( pNow );
	}
}

bool CHandsImage::IsLeftBodyPartSelected(int n)
{
	if (n==0)
	{
		for (int i=0; i<FINGERS_COUNT; i++)
			if (mHandLeft[i].selected == true)
				return true;
	}
	else
	{
		int bodypart = (n-1) / 3;
		int index = bodypart * 3 + 1;

		if (mHandLeft[index].selected == true)
			return true;
		if (mHandLeft[index+1].selected == true)
			return true;
		if (mHandLeft[index+2].selected == true)
			return true;
	}

	return false;
}

void CHandsImage::SelectLeftBodyPart(int n, bool sel)
{
	if (n==0)
	{
		for (int i=0; i<FINGERS_COUNT; i++)
			mHandLeft[i].SetSel( sel );	
	}
	else
	{
		int bodypart = (n-1) / 3;
		int index = bodypart * 3 + 1;

		mHandLeft[index	 ].SetSel( sel );
		mHandLeft[index+1].SetSel( sel );
		mHandLeft[index+2].SetSel( sel );
	}
}

bool CHandsImage::IsRightBodyPartSelected(int n)
{
	if (n==0)
	{
		for (int i=0; i<FINGERS_COUNT; i++)
			if (mHandRight[i].selected == true)
				return true;
	}
	else
	{
		int bodypart = (n-1) / 3;
		int index = bodypart * 3 + 1;

		if (mHandRight[index].selected == true)
			return true;
		if (mHandRight[index+1].selected == true)
			return true;
		if (mHandRight[index+2].selected == true)
			return true;
	}

	return false;
}

void CHandsImage::SelectRightBodyPart(int n, bool sel)
{
	if (n==0)
	{
		for (int i=0; i<FINGERS_COUNT; i++)
			mHandRight[i].SetSel( sel );	
	}
	else
	{
		int bodypart = (n-1) / 3;
		int index = bodypart * 3 + 1;

		mHandRight[index	 ].SetSel( sel );
		mHandRight[index+1].SetSel( sel );
		mHandRight[index+2].SetSel( sel );
	}
}

/************************************************
 *	Markers selection process with mouse & ctrl keys.
 ************************************************/
void CHandsImage::ViewInput(int pMouseX,int pMouseY,FBInputType pAction,int pButtonKey,int pModifier)
{
	
	switch( pAction )
	{

	case kFBButtonPress:
		{
			bool ShiftPressed = (pModifier == kFBKeyShift);
			bool CtrlPressed = (pModifier == kFBKeyCtrl);

			if (!ShiftPressed && !CtrlPressed)
			{
				//-- first of all - clear current selection
				ResetSelection();
			}

			/*
			if (pModifier == kFBKeyShift)
			{
				
				//-- left hand
				for (int i=0; i<FINGERS_COUNT; i++)
				{
					if (mHandLeft[i].selected)
					{
						mHandLeft[i].x = (float) pMouseX;
						mHandLeft[i].y = (float) (Region.Height - pMouseY);
					}
				}
				
				//-- right hand
				for (int i=0; i<FINGERS_COUNT; i++)
				{
					if (mHandRight[i].selected)
					{
						mHandRight[i].x = (float) pMouseX;
						mHandRight[i].y = (float) (Region.Height - pMouseY);
					}
				}
				break;
			}
			*/

			int InvY = Region.Height - pMouseY;
			float x1, x2;
			float y1, y2;

			//---------------------------------- LEFT HAND
			x1 = mHandLeft[0].x;
			y1 = mHandLeft[0].y;
			x2 = x1 + HAND_SIZE;
			y2 = y1 + HAND_SIZE;

			if ((pMouseX > x1) && (pMouseX < x2) && (InvY > y1) && (InvY < y2))
			{
				if (!ShiftPressed)
				{
					if (!CtrlPressed)
						mHandLeft[0].SetSel( true );
					else
						mHandLeft[0].SetSel( !mHandLeft[0].selected );
				}
				else
					if (mHandLeft[0].selected)
					{
						for (int i=0; i<FINGERS_COUNT; i++)
							mHandLeft[i].SetSel( false );
					}
					else
					{
						for (int i=0; i<FINGERS_COUNT; i++)
							mHandLeft[i].SetSel( true );
					}
			}
			else 
				if (!CtrlPressed && !ShiftPressed) 
					mHandLeft[0].SetSel( false );


			for (int i=1; i<FINGERS_COUNT; i++)
			{
				x1 = mHandLeft[i].x;
				y1 = mHandLeft[i].y;
				x2 = x1 + FINGER_SIZE;
				y2 = y1 + FINGER_SIZE;

				if ((pMouseX > x1) && (pMouseX < x2) && (InvY > y1) && (InvY < y2))
				{
					if (!ShiftPressed)
					{
						if (!CtrlPressed)
							mHandLeft[i].SetSel( true );
						else
							mHandLeft[i].SetSel( !mHandLeft[i].selected );
					}
					else
					{
						if ( IsLeftBodyPartSelected( i ) )
							SelectLeftBodyPart( i, false );
						else
							SelectLeftBodyPart( i, true );
					}
				}
				else 
					if (!CtrlPressed && !ShiftPressed) 
						mHandLeft[i].SetSel( false );
			}

			//---------------------------------- RIGHT HAND
			x1 = mHandRight[0].x;
			y1 = mHandRight[0].y;
			x2 = x1 + HAND_SIZE;
			y2 = y1 + HAND_SIZE;

			if ((pMouseX > x1) && (pMouseX < x2) && (InvY > y1) && (InvY < y2))
			{
				if (!ShiftPressed)
				{
					if (!CtrlPressed)
						mHandRight[0].SetSel( true );
					else
						mHandRight[0].SetSel( !mHandRight[0].selected );
				}
				else
					if (mHandRight[0].selected)
					{
						for (int i=0; i<FINGERS_COUNT; i++)
							mHandRight[i].SetSel( false );
					}
					else
					{
						for (int i=0; i<FINGERS_COUNT; i++)
							mHandRight[i].SetSel( true );
					}
			}
			else 
				if (!CtrlPressed && !ShiftPressed) 
					mHandRight[0].SetSel( false );


			for (int i=1; i<FINGERS_COUNT; i++)
			{
				x1 = mHandRight[i].x;
				y1 = mHandRight[i].y;
				x2 = x1 + FINGER_SIZE;
				y2 = y1 + FINGER_SIZE;

				if ((pMouseX > x1) && (pMouseX < x2) && (InvY > y1) && (InvY < y2))
				{
					if (!ShiftPressed)
					{
						if (!CtrlPressed)
							mHandRight[i].SetSel( true );
						else
							mHandRight[i].SetSel( !mHandRight[i].selected );
					}
					else
					{
						if ( IsRightBodyPartSelected( i ) )
							SelectRightBodyPart( i, false );
						else
							SelectRightBodyPart( i, true );
					}
				}
				else 
					if (!CtrlPressed && !ShiftPressed) 
						mHandRight[i].SetSel( false );
			}

			if (mHandRight[0].mModel)
			{
				if (!mHandRight[0].selected)
				{
					mReDraw = false;

					mHandRight[0].mModel->Selected = true;
					mHandRight[0].mModel->Selected = false;

					mReDraw = true;
				}
			}
		}
	}

	//-- redraw view
	FBView::Refresh( true );
}


void CFingerImage::CleanUP()
{
	mModel = NULL;
}


int CalculateWeight( int weight, int index, FBVector3d inherit )
{
	int lW = 0.0;

	switch( index )
	{
	case 0: if ( weight > 0 ) lW = 100 - abs( weight );
			else lW = 100;
			lW *= inherit[0]*0.01;
			break;
	case 1: lW = 100.0 - abs(weight);
			lW *= inherit[1]*0.01;
			break;
	case 2: if (weight < 0) lW = 100 - abs( weight );
			else lW = 100;
			lW *= inherit[2]*0.01;
			break;
	};

	return lW;
}

void CFingerImage::UpdateTexture( FBModel *pRoot, int index, GLuint drop, GLuint drop25, GLuint drop50, GLuint drop75, GLuint drop100 )
{
	if (mModel)
	{
		FBProperty		*lWeightProp = pRoot->PropertyList.Find( "weight" );
		FBProperty		*lInheritProp = pRoot->PropertyList.Find( "inherit" );
		if (lWeightProp && lInheritProp)
		{
			int value = lWeightProp->AsInt();
			FBVector3d		lInherit;
			lInheritProp->GetData( &lInherit, sizeof( FBVector3d) );

			value = CalculateWeight( (int) value, index, lInherit );

			if (value <= 10)
			{
				textureId = drop;
			}
			else
			if (value <= 35)
			{
				textureId = drop25;
			}
			else
			if (value <= 60)
			{
				textureId = drop50;
			}
			else
			if (value <= 85)
			{
				textureId = drop75;
			}
			else
				textureId = drop100;
			
		}
	} else
		textureId = drop;
}

void CFingerImage::UpdateTexture(  double weight, FBVector3d inherit, int index, 
								 GLuint drop, GLuint drop25, GLuint drop50, GLuint drop75, GLuint drop100 )
{
	if (mModel)
	{
		int value = CalculateWeight( (int) weight, index, inherit );

		if (value <= 10)
		{
			textureId = drop;
		}
		else
		if (value <= 35)
		{
			textureId = drop25;
		}
		else
		if (value <= 60)
		{
			textureId = drop50;
		}
		else
		if (value <= 85)
		{
			textureId = drop75;
		}
		else
			textureId = drop100;
	} else
		textureId = drop;
}


/************************************************
 *	Updating rendering weight in markers (choose needed texture).
 ************************************************/
void CHandsImage::UpdateWeight()
{
	int bodypart;
	int index;

	for( int i=0; i<FINGERS_COUNT; i++)
	{
		bodypart = (i-1) / 3;
		index = bodypart * 3 + 1;

		mHandLeft[i].UpdateTexture( mHandLeft[index].mModel, i-index, mRegionDropId, mRegionDropId25, mRegionDropId50, mRegionDropId75, mRegionDropId100 );
		mHandRight[i].UpdateTexture( mHandRight[index].mModel, i-index, mRegionDropId, mRegionDropId25, mRegionDropId50, mRegionDropId75, mRegionDropId100 );
	}
}


/************************************************
 *	Drawing quad from rect(x,y - x2,y2).
 ************************************************/
void CHandsImage::DrawQuadf( float x, float y, float x2, float y2 )
{
	glBegin( GL_QUADS );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2f( x, y );

		glTexCoord2f( 1.0f, 0.0f );
		glVertex2f( x2, y );

		glTexCoord2f( 1.0f, 1.0f );
		glVertex2f( x2, y2 );

		glTexCoord2f( 0.0f, 1.0f );
		glVertex2f( x, y2 );
	glEnd();
}

/************************************************
 *	Drawing quad from pos(x,y) and size(w,h).
 ************************************************/
void CHandsImage::DrawQuadWH( float x, float y, float w, float h )
{
	glBegin( GL_QUADS );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2f( x, y );

		glTexCoord2f( 1.0f, 0.0f );
		glVertex2f( x+w, y );

		glTexCoord2f( 1.0f, 1.0f );
		glVertex2f( x+w, y+h );

		glTexCoord2f( 0.0f, 1.0f );
		glVertex2f( x, y+h );
	glEnd();
}


/************************************************
 *	Hands OpenGL rendering.
 ************************************************/
void CHandsImage::ViewExpose()
{
	//-- skip redrawing if there is not any reason to do that
	if (!mReDraw) return;

	SetViewport(0,0, HANDS_IMAGE_WIDTH, HANDS_IMAGE_HEIGHT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0.0f, HANDS_IMAGE_WIDTH, 0, HANDS_IMAGE_HEIGHT );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glClearColor( .0f, .0f, .0f, 1.0f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3f( 1.0f, 1.0f, 1.0f );

	glEnable( GL_TEXTURE_2D );
	
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	//-- drawing hand background
	glBindTexture( GL_TEXTURE_2D, mHandsId );
	DrawQuadf( .0f, .0f, HANDS_IMAGE_WIDTH, HANDS_IMAGE_HEIGHT );
	

	//----------------------------------------------
	//-- HAND MARKERS
	glBindTexture( GL_TEXTURE_2D, mRegionDropId );
	//-- left hand marker
	DrawQuadWH( mHandLeft[0].x, mHandLeft[0].y, HAND_SIZE, HAND_SIZE );
	//-- right hand marker
	DrawQuadWH( mHandRight[0].x, mHandRight[0].y, HAND_SIZE, HAND_SIZE );
	//-- fingers markers
	for (int i=1; i<FINGERS_COUNT; i++)
	{
		//-- left hand markers
		glBindTexture( GL_TEXTURE_2D, mHandLeft[i].textureId );
		DrawQuadWH( mHandLeft[i].x, mHandLeft[i].y, FINGER_SIZE, FINGER_SIZE );
		//-- right hand markers
		glBindTexture( GL_TEXTURE_2D, mHandRight[i].textureId );
		DrawQuadWH( mHandRight[i].x, mHandRight[i].y, FINGER_SIZE, FINGER_SIZE );
	}

	//----------------------- render selection markers
	glBindTexture( GL_TEXTURE_2D, mRegionSelectedId );
	//-- left hand
	if (mHandLeft[0].selected)
		DrawQuadWH( mHandLeft[0].x, mHandLeft[0].y, HAND_SIZE, HAND_SIZE );
	//-- right hand
	if (mHandRight[0].selected)
		DrawQuadWH( mHandRight[0].x, mHandRight[0].y, HAND_SIZE, HAND_SIZE );

	for (int i=1; i<FINGERS_COUNT; i++)
	{
		//-- left hand selected markers
		if (mHandLeft[i].selected) 
		{
			DrawQuadWH( mHandLeft[i].x, mHandLeft[i].y, FINGER_SIZE, FINGER_SIZE );
		}
		//-- right hand selected markers
		if (mHandRight[i].selected)
		{
			DrawQuadWH( mHandRight[i].x, mHandRight[i].y, FINGER_SIZE, FINGER_SIZE );
		}
	}
	
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
}



/************************************************
 *	CCharacterImage Constructor.
 ************************************************/
CCharacterImage::CCharacterImage(void)
{
	FBString		szFullFilePath( mSystem.ApplicationPath, CHARACTER_CTRL_IMAGE );
	FBString		szRegionDropPath( mSystem.ApplicationPath, FINGER_CTRL_IMAGE );
	FBString		szRegionSelPath( mSystem.ApplicationPath, SELECTED_CTRL_IMAGE );

	mCharacterId = LoadTGA( szFullFilePath );
	mRegionDropId = LoadTGA( szRegionDropPath );
	mRegionSelectedId = LoadTGA( szRegionSelPath );
}

/************************************************
 *	CFinger Destructor.
 ************************************************/
CCharacterImage::~CCharacterImage(void)
{
	if (mCharacterId)
		glDeleteTextures( 1, &mCharacterId );
	if (mRegionDropId)
		glDeleteTextures( 1, &mRegionDropId );
	if (mRegionSelectedId)
		glDeleteTextures( 1, &mRegionSelectedId );

	//-- try to save finger position's from config file
	//FBString		szConfigPath( mSystem.ApplicationPath, CONFIG_FILENAME );
	//SaveFingerConfig( szConfigPath );
}

/************************************************
 *	Refresh view.
 ************************************************/
void CCharacterImage::Refresh( bool pNow )
{
	FBView::Refresh( pNow );
}

/************************************************
 *	Markers selection process with mouse & ctrl keys.
 ************************************************/
void CCharacterImage::ViewInput(int pMouseX,int pMouseY,int pAction,int pButtonKey,int pModifier)
{
	
	switch( pAction )
	{

	case kFBButtonDoubleClick:
		{
		}
		break;
	}

	//-- redraw view
	FBView::Refresh( true );
}

/************************************************
 *	Drawing quad from rect(x,y - x2,y2).
 ************************************************/
void CCharacterImage::DrawQuadf( float x, float y, float x2, float y2 )
{
	glBegin( GL_QUADS );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2f( x, y );

		glTexCoord2f( 1.0f, 0.0f );
		glVertex2f( x2, y );

		glTexCoord2f( 1.0f, 1.0f );
		glVertex2f( x2, y2 );

		glTexCoord2f( 0.0f, 1.0f );
		glVertex2f( x, y2 );
	glEnd();
}

/************************************************
 *	Drawing quad from pos(x,y) and size(w,h).
 ************************************************/
void CCharacterImage::DrawQuadWH( float x, float y, float w, float h )
{
	glBegin( GL_QUADS );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2f( x, y );

		glTexCoord2f( 1.0f, 0.0f );
		glVertex2f( x+w, y );

		glTexCoord2f( 1.0f, 1.0f );
		glVertex2f( x+w, y+h );

		glTexCoord2f( 0.0f, 1.0f );
		glVertex2f( x, y+h );
	glEnd();
}

/************************************************
 *	character fk rendering.
 ************************************************/
void CCharacterImage::ViewExpose()
{
	SetViewport(0,0, HANDS_IMAGE_WIDTH, HANDS_IMAGE_HEIGHT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0.0f, HANDS_IMAGE_WIDTH, 0, HANDS_IMAGE_HEIGHT );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glClearColor( .0f, .0f, .0f, 1.0f );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3f( 1.0f, 1.0f, 1.0f );

	glEnable( GL_TEXTURE_2D );
	
	//-- drawing hand background
	glBindTexture( GL_TEXTURE_2D, mCharacterId );
	DrawQuadf( .0f, .0f, HANDS_IMAGE_WIDTH, HANDS_IMAGE_HEIGHT );
	
	glDisable( GL_TEXTURE_2D );
}