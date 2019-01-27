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

/**	\file	qttestwidget.cxx
*/
#include "qttestwidget.h"
#include <QtCore\QDebug>
#include <QtCore\QCoreApplication>
#include <QtWidgets\QMenu>
#include <QtGui\QMouseEvent>
#include <QtGui\QInputEvent>
#include <QtOpenGL\QGLWidget>
//#include <Qt3Support>

#include <QtTest\QTest>
#include <QtTest\qtestkeyboard.h>

#include <functional>

//#include <Windows.h>

#ifndef SPACER
#  define SPACER "    "
#endif

// replace Auto interpolation mode

bool gTrigger = false;
std::vector<FBFCurve*>		gCurves;

bool isTriggered() {
	return gTrigger;
}
void SetTrigger(bool value)
{
	gTrigger = value;
}

// new key event to assign auto interpolation

bool gSendKeys = false;

bool isSendKeys()
{
	return gSendKeys;
}
void SetSendKeys(bool value)
{
	gSendKeys = value;
}



int ComputeNumberOfFCurves(FBModelList *plist, std::function<int (int, FBFCurve*)> func_obj)
{
	const int numberOfModels = plist->GetCount();

	int curveId = 0;
	for (int i = 0; i < numberOfModels; ++i)
	{
		FBModel *pModel = plist->GetAt(i);

		FBAnimationNode *pRoot = pModel->AnimationNode;

		int numberOfNodes = pRoot->Nodes.GetCount();

		for (int j = 0; j < numberOfNodes; j++)
		{
			FBAnimationNode *pnode = pRoot->Nodes[j];

			if (nullptr != pnode)
			{
				if (pnode->Nodes.GetCount() > 0)
				{
					for (int k = 0; k < pnode->Nodes.GetCount(); ++k)
					{
						if (nullptr != pnode->Nodes[k]->FCurve)
						{
							func_obj(curveId, pnode->Nodes[k]->FCurve);
							curveId += 1;
						}
						
					}

				}
				else
				{
					if (nullptr != pnode->FCurve)
					{
						func_obj(curveId, pnode->FCurve);
						curveId += 1;
					}
					
				}
			}
		}
	}

	return curveId;
}

int ComputeNumberOfFCurves(FBModel *pModel, std::function<int(int, FBFCurve*)> func_obj)
{
	int curveId = 0;
	FBAnimationNode *pRoot = pModel->AnimationNode;

	int numberOfNodes = pRoot->Nodes.GetCount();

	for (int j = 0; j < numberOfNodes; j++)
	{
		FBAnimationNode *pnode = pRoot->Nodes[j];

		if (nullptr != pnode)
		{
			if (pnode->Nodes.GetCount() > 0)
			{
				for (int k = 0; k < pnode->Nodes.GetCount(); ++k)
				{
					if (nullptr != pnode->Nodes[k]->FCurve)
					{
						func_obj(curveId, pnode->Nodes[k]->FCurve);
						curveId += 1;
					}
				}
			}
			else
			{
				if (nullptr != pnode->FCurve)
				{
					func_obj(curveId, pnode->FCurve);
					curveId += 1;
				}
			}
		}
	}
	
	return curveId;
}

void StoreCurves()
{
	//
	FBModelList *plist = FBCreateModelList();

	if (plist)
	{
		FBGetSelectedModels(*plist);

		FreeCurves();

		//
		auto empty_fn = [](int i, FBFCurve *pcurve)->int {
			return 0;
		};

		const int numberOfCurves = ComputeNumberOfFCurves(plist, empty_fn);
		gCurves.resize(numberOfCurves, nullptr);	// tr, rot, scl

		// copy curves
		auto copy_fn = [&](int curveId, FBFCurve *pcurve)->int {

			FBFCurve *newcurve = new FBFCurve();
			newcurve->EditBegin();
			newcurve->KeyReplaceBy(*pcurve);
			newcurve->EditEnd();

			gCurves[curveId] = newcurve;
			return 1;
		};

		ComputeNumberOfFCurves(plist, copy_fn);
	}

	FBDestroyModelList(plist);
}

// return true if equal

bool CompareKeys(FBFCurveKey *key1, FBFCurveKey *key2)
{
	/*
	if (kFBTangentModeClampProgressive != key1->TangentMode)
		return false;
	if (kFBTangentModeClampProgressive != key2->TangentMode)
		return false;
	*/
	if (key1->LeftDerivative != key2->LeftDerivative)
		return false;
	if (key1->RightDerivative != key2->RightDerivative)
		return false;
	if (key1->LeftTangentWeight != key2->LeftTangentWeight)
		return false;
	if (key1->RightTangentWeight != key2->RightTangentWeight)
		return false;
	if (key1->LeftBezierTangent != key2->LeftBezierTangent)
		return false;
	if (key1->RightBezierTangent != key2->RightBezierTangent)
		return false;

	if (key1->Tension != key2->Tension)
		return false;
	if (key1->Continuity != key2->Continuity)
		return false;
	if (key1->Bias != key2->Bias)
		return false;

	if (key1->Interpolation != key2->Interpolation)
		return false;
	if (key1->TangentMode != key2->TangentMode)
		return false;
	if (key1->TangentClampMode != key2->TangentClampMode)
		return false;
	if (key1->TangentBreak != key2->TangentBreak)
		return false;
	if (key1->TangentConstantMode != key2->TangentConstantMode)
		return false;

	return true;
}

void SetAutoInterpolation()
{
	//
	FBModelList *plist = FBCreateModelList();

	if (plist)
	{
		FBGetSelectedModels(*plist);

		auto empty_fn = [](int i, FBFCurve *pcurve)->int {
			return 0;
		};

		const int numberOfCurves = ComputeNumberOfFCurves(plist, empty_fn);

		if (numberOfCurves == gCurves.size())
		{
			auto process_fn = [&](int curveId, FBFCurve *pcurve)->int {

				int lResult = 0;
				FBFCurve *oldcurve = gCurves[curveId];

				if (pcurve && oldcurve)
				{
					const int numberOfKeys = pcurve->Keys.GetCount();
					const int oldNumberOfKeys = oldcurve->Keys.GetCount();

					if (numberOfKeys == oldNumberOfKeys)
					{
						for (int k = 0; k < numberOfKeys; ++k)
						{
							FBFCurveKey key = pcurve->Keys[k];
							FBFCurveKey oldKey = oldcurve->Keys[k];

							if (false == CompareKeys(&key, &oldKey))
							{
								key.Interpolation = kFBInterpolationCubic;
								key.TangentBreak = false;
								key.TangentMode = kFBTangentModeClampProgressive;
								lResult += 1;
							}
						}
					}
				}

				return lResult;
			};
		
			ComputeNumberOfFCurves(plist, process_fn);
		}
	}

	FBDestroyModelList(plist);
}

void SetAutoInterpolationSuper(FBTime selStart, FBTime selStop)
{

//	FBPlayerControl &lPlayer = FBPlayerControl::TheOne();

	FBSystem::TheOne().Scene->CandidateEvaluationAndResolve();
	FBSystem::TheOne().Scene->Evaluate();

	selStop = selStop + FBTime(0, 0, 0, 1);

	FBTimeSpan selSpan;
	selSpan.Set(selStart, selStop);

	//
	auto process_fn = [&selSpan](int curveId, FBFCurve *pcurve)->int {

		int lResult = 0;

		if (pcurve)
		{
			const int numberOfKeys = pcurve->Keys.GetCount();

			for (int k = 0; k < numberOfKeys; ++k)
			{
				FBFCurveKey key = pcurve->Keys[k];
				FBTime keyTime = key.Time;

				if (true == (selSpan & keyTime))
				{
					key.Interpolation = kFBInterpolationCubic;
					key.TangentBreak = false;
					key.TangentMode = kFBTangentModeClampProgressive;
					lResult += 1;
				}
			}

		}

		return lResult;
	};

	//
	FBModelList *plist = FBCreateModelList();

	if (plist)
	{
		FBGetSelectedModels(*plist);
		ComputeNumberOfFCurves(plist, process_fn);
	}

	FBDestroyModelList(plist);

	// DONE: support for char bodypart and fullbody modes

	FBCharacter *pChar = FBApplication::TheOne().CurrentCharacter;

	if (nullptr != pChar)
	{
		bool activeStates[kFBLastCtrlSetPartIndex];
		pChar->GetActiveBodyPart(activeStates);

		FBCharacterKeyingMode mode = pChar->KeyingMode;

		if (kFBCharacterKeyingFullBody == mode || kFBCharacterKeyingFullBodyNoPull == mode)
		{
			for (int i = 0; i < kFBLastCtrlSetPartIndex; ++i)
				activeStates[i] = true;
		}
		
		if (kFBCharacterKeyingSelection != mode)
		{
			// effector
			for (int i = 0; i < kFBLastEffectorId; ++i)
			{
				FBEffectorId id = (FBEffectorId)i;

				FBModel *pModel = pChar->GetEffectorModel(id);
				FBBodyPartId bodyPartId = FBGetEffectorBodyPart(id);

				if (nullptr != pModel && true == activeStates[bodyPartId])
				{
					// DONE: process curves on pModel
					ComputeNumberOfFCurves(pModel, process_fn);
				}
			}

			// fk nodes
			for (int i = 0; i < kFBLastNodeId; ++i)
			{
				FBBodyNodeId id = (FBBodyNodeId)i;

				FBModel *pModel = pChar->GetCtrlRigModel(id);
				FBBodyPartId bodyPartId = FBGetBodyNodeBodyPart(id);

				if (nullptr != pModel && true == activeStates[bodyPartId])
				{
					// DONE: process curves on pModel
					ComputeNumberOfFCurves(pModel, process_fn);
				}
			}
		}
	}

}

const int GetCurvesCount()
{
	return (int)gCurves.size();
}

FBFCurve *GetCurves()
{
	if (gCurves.size() > 0)
		return gCurves[0];
	return nullptr;
}

void FreeCurves()
{
	for (auto iter = begin(gCurves); iter != end(gCurves); ++iter)
	{
		FBFCurve *pcurve = *iter;
		if (pcurve)
		{
			delete pcurve;
			pcurve = nullptr;
		}
	}
	gCurves.clear();
}

void QtTestWidget::print_qobject(QObject* o, int level)
{
	QString line;
	while ((level--) != 0) line += SPACER;

	line += o->metaObject()->className();

	/*
	QApplication *app = qobject_cast<QApplication*>(o);
	if (app)
	{
		app->installEventFilter(this);
	}
	*/
	/*
	QGLWidget *q = qobject_cast<QGLWidget*>(o);
	if (q)
	{

		q->setContextMenuPolicy(Qt::CustomContextMenu);
		connect( q, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));
	}
	*/
	if (0 == strcmp("QAction", o->metaObject()->className()))
	{
		
		/*
		QAction *pAction = (QAction*)o;
		QMenu *pMenu = pAction->menu();
		if (nullptr != pMenu)
		{
			pMenu->addAction(QString("Some test item"));
		}
		*/
	}

	if (o->objectName().length() > 0)
		line += " #" + o->objectName();

	printf("%s\n", line.toUtf8().constData());
}

void DoSendKeys()
{
	// This structure will be used to create the keyboard
	// input event.
	INPUT ip;

	// Set up a generic keyboard event.
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Press the "F" key
	ip.ki.wVk = 0x46; // virtual-key code for the "a" key
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Release the "F" key
	ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));
}

bool CheckParentTool(QWidget *pWidget)
{
	bool lStatus = false;

	if (nullptr != pWidget)
	{
		QString name = pWidget->objectName();
		if (name.length() > 0)
			lStatus = (0 == strcmp("ToolWindow_4", name.toUtf8().constData()));

		if (false == lStatus)
		{
			QWidget *parent = pWidget->parentWidget();
			lStatus = CheckParentTool(parent);
		}
	}
	
	return lStatus;
}

bool QtTestWidget::eventFilter(QObject *obj, QEvent *event){

	if (event->type() == QEvent::Show)
	{
	//	QShowEvent *mEvent = static_cast<QShowEvent*>(event);

		plastmenu = nullptr;
		const char *classname = obj->metaObject()->className();
		
		if (0 == strcmp(classname, "BasePopup"))
		{
			QMenu *pmenu = qobject_cast<QMenu*>(obj);
			
			if (pmenu)
			{
				bool interpolateMenu = false;

				foreach(QAction *action, pmenu->actions())
				{
					if (false == action->isSeparator())
					{
						QString text = action->text();
						if (text.size() > 0)
						{
							std::string stdstr = text.toUtf8().constData();
							//printf("menu item text - %s\n", stdstr.c_str());

							if (0 == strcmp("Auto", stdstr.c_str()))
							{
								interpolateMenu = true;
							}
						}
						
					}
					
				}

				if (true == interpolateMenu)
				{
					//pmenu->insertItem("Auto Smooth", this, SLOT(OnNewAction), 0, 55, 55);
					StoreCurves();
					plastmenu = pmenu;
				}
			}
		}
		
	}
	else
	if (event->type() == QEvent::KeyRelease) {
		QKeyEvent *mEvent = static_cast<QKeyEvent*>(event);

		if (mEvent->key() == Qt::Key_D)
		{
			QGLWidget *pwidget = qobject_cast<QGLWidget*>(obj);

			if (nullptr != pwidget)
			{
				if (true == CheckParentTool(pwidget))
				{
					printf("send key press events\n");
					SetSendKeys(true);
				}
			}
		}
	}
	else
	if (event->type() == QEvent::MouseButtonPress) {
		
		QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);

		if (mEvent->button() == Qt::LeftButton)
		{
			int x = mEvent->x();
			int y = mEvent->y();

			//const char *classname = obj->metaObject()->className();
			//QString str = obj->objectName();

			//printf("[Mouse Button Press] class name under cursor - %s, obj name - %s, x-%d, y-%d\n", classname, str.toUtf8().constData(), x, y);

			if (plastmenu == obj && x >= 0 && y >= 0 && x <= 70 && y <= 20)
			{
				//OnAutoInterpTrigger();
				//SetSendKeys(true);
			}
		}
	}
	return QObject::eventFilter(obj, event);
}

void QtTestWidget::inspect(QObject* o, int level)
{
	if (level == 0) print_qobject(o, level);

	level++;

	foreach(QObject* child, o->children())
	{
		print_qobject(child, level);
		inspect(child, level); //recurse
	}
}

QtTestWidget::QtTestWidget( QWidget* gParent )
: QFrame(gParent)
    
{
    // This will effectively populate QtTestWidget instance with all 
    // Ui customization specified in Ui_QtTestWidget base class.
    setupUi(this);

	//
//	bool maximized = true;
	if (gParent)
	{
		QWidget *pParent = gParent->parentWidget();
		if (pParent)
		{
			pParent = pParent->parentWidget();

			if (pParent)
			{

				//pParent->setWindowState((maximized) ? Qt::WindowFullScreen : Qt::WindowNoState);

				pParent = pParent->parentWidget();
				if (pParent)
				{
					//pParent->setWindowState((maximized) ? Qt::WindowFullScreen : Qt::WindowNoState);

					//inspect(pParent); //prints hierarchy to stderr

					//QApplication::instance()->installEventFilter(this);

				}
			}
		}
	}
	
	QApplication::instance()->installEventFilter(this);
}

QtTestWidget::~QtTestWidget()
{    
}

void QtTestWidget::on_createCubeBtn_clicked()
{    
    if ( !mHdlCube.Ok() )
    {
        rotateCubeDial->setValue( 0 );
        scaleCubeSlider->setMaximum( 100 );
        scaleCubeSlider->setMinimum( 1 );
        scaleCubeSlider->setValue( 1 );          
        mHdlCube = new FBModelCube( "My FB Cube" );
        mHdlCube->Show = true;

        createCubeBtn->setDisabled( true );
    }    
}

void QtTestWidget::on_rotateCubeDial_valueChanged( int pValue )
{
    if ( mHdlCube.Ok() )
    {
        FBVector3d lRotation( pValue, pValue, pValue );
        mHdlCube->Rotation = lRotation;
    }
}

void QtTestWidget::on_scaleCubeSlider_valueChanged( int pValue )
{
    if ( mHdlCube.Ok() )
    {
        FBVector3d lScaling( pValue, pValue, pValue );
        mHdlCube->Scaling = lScaling;
    }
}

void QtTestWidget::ShowContextMenu(const QPoint &pos)
{
	QMenu contextMenu(tr("Context menu"), this);

	QAction action1("Remove Data Point", this);
	//connect(&action1, SIGNAL(triggered()), this, SLOT(removeDataPoint()));
	contextMenu.addAction(&action1);

	QPoint mypos = { 5, 5 };
	contextMenu.exec(mapToGlobal(mypos));
}

void QtTestWidget::OnAutoInterpTrigger()
{
	SetTrigger(true);
}