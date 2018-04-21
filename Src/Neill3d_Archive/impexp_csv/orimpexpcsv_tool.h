#ifndef __ORIMPEXP_TOOL_TOOL_H__
#define __ORIMPEXP_TOOL_TOOL_H__
/***************************************************************************************
 Autodesk(R) Open Reality(R) Samples
 
 (C) 2006 Autodesk, Inc. and/or its licensors
 All rights reserved.
 
 AUTODESK SOFTWARE LICENSE AGREEMENT
 Autodesk, Inc. licenses this Software to you only upon the condition that 
 you accept all of the terms contained in the Software License Agreement ("Agreement") 
 that is embedded in or that is delivered with this Software. By selecting 
 the "I ACCEPT" button at the end of the Agreement or by copying, installing, 
 uploading, accessing or using all or any portion of the Software you agree 
 to enter into the Agreement. A contract is then formed between Autodesk and 
 either you personally, if you acquire the Software for yourself, or the company 
 or other legal entity for which you are acquiring the software.
 
 AUTODESK, INC., MAKES NO WARRANTY, EITHER EXPRESS OR IMPLIED, INCLUDING BUT 
 NOT LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR 
 PURPOSE REGARDING THESE MATERIALS, AND MAKES SUCH MATERIALS AVAILABLE SOLELY ON AN 
 "AS-IS" BASIS.
 
 IN NO EVENT SHALL AUTODESK, INC., BE LIABLE TO ANYONE FOR SPECIAL, COLLATERAL, 
 INCIDENTAL, OR CONSEQUENTIAL DAMAGES IN CONNECTION WITH OR ARISING OUT OF PURCHASE 
 OR USE OF THESE MATERIALS. THE SOLE AND EXCLUSIVE LIABILITY TO AUTODESK, INC., 
 REGARDLESS OF THE FORM OF ACTION, SHALL NOT EXCEED THE PURCHASE PRICE OF THE 
 MATERIALS DESCRIBED HEREIN.
 
 Autodesk, Inc., reserves the right to revise and improve its products as it sees fit.
 
 Autodesk and Open Reality are registered trademarks or trademarks of Autodesk, Inc., 
 in the U.S.A. and/or other countries. All other brand names, product names, or 
 trademarks belong to their respective holders. 
 
 GOVERNMENT USE
 Use, duplication, or disclosure by the U.S. Government is subject to restrictions as 
 set forth in FAR 12.212 (Commercial Computer Software-Restricted Rights) and 
 DFAR 227.7202 (Rights in Technical Data and Computer Software), as applicable. 
 Manufacturer is Autodesk, Inc., 10 Duke Street, Montreal, Quebec, Canada, H3C 2L7.
***************************************************************************************/

/**	\file	orimpexptool_tool.h
*	Declaration for an import/export tool class.
*	Contains the declaration for an import/export tool class
*	ORImportExportTool.
*/

//--- Class declaration
#include "orimpexptool_menu.h"
#include "orimpexptool_engine.h"

//--- Registration defines
#define	ORTOOLIMPORTEXPORT__CLASSNAME	ORToolImportExport
#define ORTOOLIMPORTEXPORT__CLASSSTR	"ORToolImportExport"

/**	Import/Export Tool.
*	Tool with the correct buttons to import/export scenes & information.
*/
class ORToolImportExport :	public FBTool
{
	//--- FiLMBOX Tool declaration.
	FBToolDeclare( ORToolImportExport, FBTool );

public:
	//--- FiLMBOX constructor & destructor
	virtual bool FBCreate();		//!< FiLMBOX Constructor.
	virtual void FBDestroy();		//!< FiLMBOX Destructor.

	// UI Management
	void	UICreate	();
	void	UIConfigure	();
	void		UIConfigureMenu();

	// UI Callbacks
	void	EventButtonBrowseClick			( HISender pSender, HKEvent pEvent );
	void	EventButtonImportSceneClick		( HISender pSender, HKEvent pEvent );
	void	EventButtonImportOpticalClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonImportAnimationClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonExportSelectedClick	( HISender pSender, HKEvent pEvent );
	void	EventButtonExportVoiceClick		( HISender pSender, HKEvent pEvent );

private:
	FBLabel		mLabelImport;						//!< Import label.
	FBLabel		mLabelExport;						//!< Export label.
	FBButton	mButtonBrowse;						//!< Browse button.
	FBButton	mButtonImportScene;					//!< Import scene button.
	FBButton	mButtonImportOptical;				//!< Import optical button.
	FBButton	mButtonImportAnimation;				//!< Import animation button.
	FBButton	mButtonExportSelected;				//!< Export selected button.
	FBButton	mButtonExportVoice;					//!< Export voice button.

    ORImportExportEngine    mEngine;
};

#endif /* __ORIMPEXP_TOOL_TOOL_H__ */
