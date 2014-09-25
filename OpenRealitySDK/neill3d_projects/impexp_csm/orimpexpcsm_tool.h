#ifndef __ORIMPEXP_TOOL_TOOL_H__
#define __ORIMPEXP_TOOL_TOOL_H__


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

/**	\file	orimpexptool_tool.h
*	Declaration for an import/export tool class.
*	Contains the declaration for an import/export tool class
*	ORImportExportTool.
*/

//--- Class declaration
#include "orimpexpcsm_menu.h"
#include "orimpexpcsm_engine.h"

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

    CSMEngine    mEngine;
};

#endif /* __ORIMPEXP_TOOL_TOOL_H__ */
