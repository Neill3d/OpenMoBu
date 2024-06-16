
#pragma once

/**	\file	Timer.h
*
*	system timer and high peformance timer
*
*	Author Sergey Solokhin, 2010
*		e-mail to: s@neill3d.com
*/

/**
*	\page	engine timer
*	\section	Usage example
*
*	info( Timer::FormatCPUSpeed() );
*
*	\section	History
*	12.05.10 - adding UNICODE support
*
*/

#include "Types.h"

namespace CEUtils
{
	//! Timer class
	/*!
	 * support high quality timer on counters
	 * support cpu speed calculating
	 */
	class Timer : public	Singleton<Timer>
	{
	public:
		//! a constructor
		Timer(void);
		//! a desturctor
		~Timer(void);

		/*! Refresh
		 * call each program frame
		 */
		int Refresh();

		/*! getCPUSpeed
		 * \return cpu speed in Mhz
		 */
		static double			GetCPUSpeed();

		/* FormatCPUSpeed
		 * \return format string with cpu speed in Ghz
		 */
		static  const TCHAR		*FormatCPUSpeed();

		double	GetFrameTime() { return frametime;	}
		double	GetWorkTime() { return fTime; }

	private:
		int				lowshift;
		BYTE			tPerformanceTimerEnabled;

		double			fTime;			// time since windows started
		double			deltaTime;		// last recorded time
		double			frametime;		// time elapsed in the last frame
		int				frames;			// number of frames

		LARGE_INTEGER	tFrequency;
		double			tResolution;

		unsigned int	oldTime;
		int				sametimecount;	// counter for frames with the same time.
	};

};
