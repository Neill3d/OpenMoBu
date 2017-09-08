
#ifdef USE_QT
#include <QtGui/QWidget>

static QWidget *gParent = nullptr;

QWidget* CreateQtTestWidget( QWidget* pParent )
{
    // pParent is passed to us by the runtime and it will be a QWidget 
    //QtTestWidget* lWidget = new QtTestWidget( pParent );

	//FBMessageBox( "test", "Try to mazimize", "Ok" );

	gParent = pParent;
	pParent->setWindowState(Qt::WindowMaximized);

    // Return the newly created widget.
    //return lWidget;
	return nullptr;
}


void ToggleMaximize(const bool maximized)
{
	QWidget *pParent = gParent->parentWidget();
	if (pParent)
	{
		pParent = pParent->parentWidget();
			
		if (pParent)
		{
			
#ifdef MOBU2015
			pParent->setWindowState( (maximized) ? Qt::WindowFullScreen: Qt::WindowNoState);	
#else
			pParent = pParent->parentWidget();
			if (pParent)
			{
				pParent->setWindowState( (maximized) ? Qt::WindowFullScreen: Qt::WindowNoState);	
			}
#endif		
		}
	}
}

#else

#include <QtGui/QWidget>

QWidget* CreateQtTestWidget( QWidget* pParent )
{
	return nullptr;
}

void ToggleMaximize(const bool maximized)
{
}

#endif