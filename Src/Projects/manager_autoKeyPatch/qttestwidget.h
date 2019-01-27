/***************************************************************************************
 Autodesk(R) Open Reality(R) Samples
 
 (C) 2009 Autodesk, Inc. and/or its licensors
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

#ifndef FB_QT_TEST_WIDGET
#define FB_QT_TEST_WIDGET

#pragma warning( disable : 4127 )
#pragma warning( disable : 4100 )

// important to include all Qt file BEFORE sdk file. This will remove lots
// of useless warnings/errors.
#include "ui_qttestwidget.h"
#include <QtWidgets/QFrame>

#include <fbsdk/fbsdk.h>

bool isSendKeys();
void SetSendKeys(bool value);
void DoSendKeys();

bool isTriggered();
void SetTrigger(bool value);
void StoreCurves();
const int GetCurvesCount();
FBFCurve *GetCurves();

void SetAutoInterpolation();
void SetAutoInterpolationSuper(FBTime selStart, FBTime selStop);

void FreeCurves();

/**
    This class shows how to create a custom QWidget using a 
    Ui class generated from a ui file (obtain from Qt Designer).

    By convention, UI generated with Qt will automatically look for correctly
    named slots in the Custom Widget and connect them to relevant signal into the Ui file.

    A correctly named slot:

    on_<widget name in Ui file>_<signal name>( <signal signature> )
*/
class QtTestWidget : public QFrame
                   , private Ui_QtTestWidget
{
    Q_OBJECT
public:
    QtTestWidget( QWidget* pParent = 0 );
    virtual ~QtTestWidget();

public slots:
    // Automatically connect to the QPushButton named createCubeBtn into Ui_QtTestWidget class
    void on_createCubeBtn_clicked();

    // Automatically connect to the QDial named rotateCubeDial into Ui_QtTestWidget class
    void on_rotateCubeDial_valueChanged( int );

    // Automatically connect to the QSlider named scaleCubeSlider into Ui_QtTestWidget class
    void on_scaleCubeSlider_valueChanged( int );

	void ShowContextMenu(const QPoint &pos);

	void OnAutoInterpTrigger();

	

private:
    HdlFBPlugTemplate<FBModel> mHdlCube;

	
	QObject		*plastmenu;

	void print_qobject(QObject* o, int level);
	void inspect(QObject* o, int level = 0);

	bool eventFilter(QObject *obj, QEvent *event);

};

#endif