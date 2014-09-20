

/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://code.google.com/p/motioncodelibrary/wiki/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

#include "GraphTools.h"
#include <fbsdk-opengl.h>
#include <math.h>

///////////////////////////////////////////////////////
//	ManipGraphSlider

void		ManipGraphSlider::MouseDown(int x, int y, int modifier)
{
	if ( !GetGraphPtr() || (modifier > 0) ) return;
	FBTimeSpan	&lTimeSpan = GetGraphPtr()->GetTimeSpan();
	FBTime		currTime = FBSystem().LocalTime;
	if (lTimeSpan & currTime)
	{
		currTime = currTime - lTimeSpan.GetStart();
		int val = (currTime.GetFrame() +1) * GetGraphPtr()->GetWidth() / lTimeSpan.GetDuration().GetFrame();
		SetModal( (x > (val-2)) && (x < (val+2)) );
	}
}

//! mouse move processing
void		ManipGraphSlider::MouseMove(int x, int y, int modifier)
{
	if (IsModal() && GetGraphPtr())
	{
		FBTimeSpan		&lTimeSpan	= GetGraphPtr()->GetTimeSpan();

		double mult;
		/*
		mult = lTimeSpan.GetDuration().GetFrame(true);
		mult /= GetGraphPtr()->GetWidth();
		FBTime	shift( 0, 0, 0, (x - GetGraphPtr()->GetLastX() )*mult );
*/
		mult = x * lTimeSpan.GetDuration().GetFrame() / GetGraphPtr()->GetWidth();
		FBTime	shift( 0, 0, 0, (int)mult );

		FBTime	currTime = FBSystem().LocalTime;
		currTime = lTimeSpan.GetStart() + shift;
		FBPlayerControl().Goto(currTime);
	}
}
//! mouse up processing
void		ManipGraphSlider::MouseUp(int x, int y, int modifier)
{
	SetModal(false);
}

double	CalcLargeStep(double value)
{
	double step = 1.0;
	
	if (value > 1)
	{
		while (step < (value * 0.1) )
		{
			step *= 10.0;
		}
	}
	else
	if (value < 1)
	{
		while (step > value)
		{
			step *= 0.1;
		}
	}

	return step;
}

void		ManipGraphSlider::PreDraw()
{
	if ( !GetGraphPtr() ) return;
	FBTimeSpan	&lTimeSpan = GetGraphPtr()->GetTimeSpan();
	FBVector2d	&lValueSpan	= GetGraphPtr()->GetValueSpan();

	double pX, pX2, pY, pY2, temp;

	timeDuration	= lTimeSpan.GetDuration().GetFrame();
	timeLargeStep = CalcLargeStep( (double) timeDuration );
	timeSmallStep = timeLargeStep * 0.2;

	pX = floor(lTimeSpan.GetStart().GetFrame() / timeLargeStep);
	pX = (pX-1) * timeLargeStep;
	temp = pX;
	pX2 = floor(lTimeSpan.GetStop().GetFrame() / timeLargeStep);
	pX2 = (pX2+1) * timeLargeStep;
	pY = (lValueSpan[1] - lValueSpan[0]) * 0.05;
	pY2 = lValueSpan[1] - lValueSpan[0];

	glPushMatrix();
	glTranslatef( -lTimeSpan.GetStart().GetFrame(), 0.0, 0.0 );
	
	// x grid lines
	pY = (lValueSpan[1] - lValueSpan[0]) * 0.05;

	glBegin(GL_LINES);
	pX = temp;
	while(pX < pX2)
	{
		if (fmod(pX, timeLargeStep) == 0.0)
			glColor3f(0.0f, 0.0f, 0.0f);
		else
			glColor3f(0.3f, 0.3f, 0.3f);

		glVertex2i( pX, pY );
		glVertex2i( pX, pY2 );

		pX += timeSmallStep;
	}
	glEnd();

	glPopMatrix();


	valueDuration = lValueSpan[1] - lValueSpan[0];
	valueLargeStep = CalcLargeStep( valueDuration );
	valueSmallStep = valueLargeStep * 0.2;

	pY = floor( lValueSpan[0] / valueLargeStep);
	pY = (pY-1) * valueLargeStep;
	temp = pY;
	pY2 = floor( lValueSpan[1] / valueLargeStep);
	pY2 = (pY2+1) * valueLargeStep;
	pX = timeDuration * 0.05;
	pX2 = timeDuration;

	glPushMatrix();
	glTranslatef( 0.0, -lValueSpan[0], 0.0 );

	// y grid lines
	pX = timeDuration * 0.05;
	pY = temp;
	glBegin(GL_LINES);
	while(pY < pY2)
	{
		if (fmod(pY, valueLargeStep) == 0)
			glColor3f(0.0f, 0.0f, 0.0f);
		else
			glColor3f(0.3f, 0.3f, 0.3f);

		glVertex2i( pX, pY );
		glVertex2i( pX2, pY );

		pY += valueSmallStep;
	}
	glEnd();

	glPopMatrix();
}

void ManipGraphSlider::PostDraw()
{
	if ( !GetGraphPtr() ) return;
	FBTimeSpan	&lTimeSpan = GetGraphPtr()->GetTimeSpan();
	FBVector2d	&lValueSpan	= GetGraphPtr()->GetValueSpan();

	double	pX, pX2, pY, pY2, temp;
	char	text[32];

	//
	// X
	//
	pX = floor(lTimeSpan.GetStart().GetFrame() / timeLargeStep);
	pX = (pX-1) * timeLargeStep;
	temp = pX;
	pX2 = floor(lTimeSpan.GetStop().GetFrame() / timeLargeStep);
	pX2 = (pX2+1) * timeLargeStep;
	pY = (lValueSpan[1] - lValueSpan[0]) * 0.05;
	pY2 = lValueSpan[1] - lValueSpan[0];

	glPushMatrix();
	glTranslatef( -lTimeSpan.GetStart().GetFrame(), 0.0, 0.0 );

	// draw x axes panel
	glColor3f(0.7f, 0.7f, 0.7f);
	glBegin(GL_QUADS);

		glVertex2i( pX, pY );
		glVertex2i( pX2, pY );
		glVertex2i( pX2, 0 );
		glVertex2i( pX, 0 );

	glEnd();

	// x text values
	pY = (lValueSpan[1] - lValueSpan[0]) * 0.01;
	glColor3f(0.0f, 0.0f, 0.0f);
	while (pX < pX2)
	{
		sprintf( text, "%-10.0lf", pX );
		//DrawString( pX, pY, text );
		GetGraphPtr()->DrawString( text, pX, pY );
		pX += timeLargeStep;
	}

	glPopMatrix();

	//
	// Y
	//
	pY = floor( lValueSpan[0] / valueLargeStep);
	pY = (pY-1) * valueLargeStep;
	temp = pY;
	pY2 = floor( lValueSpan[1] / valueLargeStep);
	pY2 = (pY2+1) * valueLargeStep;
	pX = timeDuration * 0.05;
	pX2 = timeDuration;

	glPushMatrix();
	glTranslatef( 0.0, -lValueSpan[0], 0.0 );

	// draw y axes panel
	glColor3f(0.7f, 0.7f, 0.7f);
	glBegin(GL_QUADS);

		glVertex2i( pX, pY );
		glVertex2i( 0, pY );
		glVertex2i( 0, pY2 );
		glVertex2i( pX, pY2 );

	glEnd();

	// y axes values
	pX = timeDuration * 0.005;
	glColor3f(0.0f, 0.0f, 0.0f);
	while (pY < pY2)
	{
		sprintf( text, "%-10.2lf", pY );
		//DrawString( pX, pY, text );
		GetGraphPtr()->DrawString( text, pX, pY );

		pY += valueLargeStep;
	}

	glPopMatrix();

	// display time slider
	FBTime		currTime = FBSystem().LocalTime;
	int n1 = currTime.GetFrame();
	if (lTimeSpan & currTime)
	{
		n1 = lTimeSpan.GetStart().GetFrame();
		currTime = currTime - lTimeSpan.GetStart();
		n1 = currTime.GetFrame();
		pY = 0;
		pY2 = valueDuration;
		pX = currTime.GetFrame() + 1;
		//pX = (currTime.GetSecondDouble() - lTimeSpan.GetStart().GetSecondDouble() ) * 100;

		glColor3f(0.0f, 0.0f, 0.0f);
		glLineWidth(2.0);
		glBegin(GL_LINES);

			glVertex2i(pX, pY);
			glVertex2i(pX, pY2);

		glEnd();
	}
}

/////////////////////////////////////////////////////////
// ManipGraphPan

//! mouse down processing
void		ManipGraphPan::MouseDown(int x, int y, int modifier) 
{
}

//! mouse move processing
void		ManipGraphPan::MouseMove(int x, int y, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	FBTimeSpan		&lTimeSpan	= GetGraphPtr()->GetTimeSpan();
	FBVector2d		&lValueSpan = GetGraphPtr()->GetValueSpan();

	double mult;
	// x translation
	FBTime start, stop, shift;
	start = lTimeSpan.GetStart();
	stop = lTimeSpan.GetStop();
	//mult = lTimeSpan.GetDuration().GetSecondDouble();
	//mult /= GetGraphPtr()->GetWidth();
	mult = GetGraphPtr()->CalcXSens();
	mult = mult * (GetGraphPtr()->GetLastX() - x);
	shift.SetSecondDouble( mult );

	start = start + shift;
	stop = stop + shift;
	lTimeSpan.Set( start, stop );

	// y translation
	//mult = lValueSpan[1] - lValueSpan[0];
	//mult /= GetGraphPtr()->GetHeight();
	mult = GetGraphPtr()->CalcYSens();
	double yShift = y - GetGraphPtr()->GetLastY();
	yShift *= mult;
	lValueSpan[0] += yShift;
	lValueSpan[1] += yShift;
}

//! mouse up processing
void		ManipGraphPan::MouseUp(int x, int y, int modifier)
{
}


/////////////////////////////////////////////////////////
// ManipGraphZoom

//! mouse down processing
void		ManipGraphZoom::MouseDown(int x, int y, int modifier) 
{
	if ( !GetGraphPtr() ) return;
	mZoomCenterX = x;
	mZoomCenterY = y;

	mTimeSpan	= GetGraphPtr()->GetTimeSpan();
	mValueSpan = GetGraphPtr()->GetValueSpan();

	GetGraphPtr()->ScreenToWorld( x, y, wx, wy );
}

//! mouse move processing
void		ManipGraphZoom::MouseMove(int x, int y, int modifier) 
{
	if ( !GetGraphPtr() ) return;

	FBTimeSpan		&lTimeSpan	= GetGraphPtr()->GetTimeSpan();
	FBVector2d		&lValueSpan = GetGraphPtr()->GetValueSpan();

	FBTime start, stop, shift;
	double	scaleFactorX, scaleFactorY;
	scaleFactorX = 1.0;
	scaleFactorY = 1.0;
	
/*
	// x translation
	start = mTimeSpan.GetStart();
	stop = mTimeSpan.GetStop();
	
	mult = GetGraphPtr()->CalcXSens();
	shift.SetSecondDouble( (x - mZoomCenterX)*mult );
	start = start + shift;
	stop = stop - shift;
	lTimeSpan.Set( start, stop );

	// y translation
	
	mult = GetGraphPtr()->CalcYSens();
	double yShift = y - mZoomCenterY;
	yShift *= mult;
	lValueSpan[0] = mValueSpan[0] + yShift;
	lValueSpan[1] = mValueSpan[1] - yShift;
	if (lValueSpan[0] > lValueSpan[1])
	{
		printf( "span 0 is greater that 1\n" );
		mult = lValueSpan[0];
		lValueSpan[0] = lValueSpan[1];
		lValueSpan[1] = mult;
	}
*/

	double deltaX = 0.01 * (GetGraphPtr()->GetLastX() - x);
  double deltaY = 0.01 * (GetGraphPtr()->GetLastY() - y);

	if ( (modifier & VK_SHIFT) == VK_SHIFT )
	{
		start = lTimeSpan.GetStart();
		stop = lTimeSpan.GetStop();
		shift.SetSecondDouble( deltaX );
		start = start - shift;
		stop = stop + shift;
		lTimeSpan.Set( start, stop );
	}
	else
	{
		start = lTimeSpan.GetStart();
		stop = lTimeSpan.GetStop();

		scaleFactorX += deltaX;
    if (scaleFactorX <= 0.0)
        scaleFactorX = 1.0;
    scaleFactorY -= deltaY;
    if (scaleFactorY <= 0.0)
        scaleFactorY = 1.0;

		start.SetSecondDouble( (start.GetSecondDouble() - wx) * scaleFactorX + wx );
		stop.SetSecondDouble( (stop.GetSecondDouble() - wx) * scaleFactorX + wx );
		lTimeSpan.Set( start, stop );

		lValueSpan[0] = (lValueSpan[0] - wy) * scaleFactorY + wy;
		lValueSpan[1] = (lValueSpan[1] - wy) * scaleFactorY + wy;
	}
}

//! mouse up processing
void		ManipGraphZoom::MouseUp(int x, int y, int modifier)
{
}