
#pragma once

/**	\file	device_facecap_hardware.h
*	Developed by Sergei <Neill3d> Solokhin 2019
*	e-mail to: neill3d@gmail.com
*	twitter: @Neill3d
*
* OpenMoBu github - https://github.com/Neill3d/OpenMoBu
*/

//--- SDK include
#include <fbsdk/fbsdk.h>


//--- Array size defines
#define	MAX_BUFFER_SIZE		2048


// incoming blendshapes
enum class EHardwareBlendshapes : uint8_t
{
	brow_inner_up,
	brow_down_left,
	brow_down_right,
	brow_outer_up_left,
	brow_outer_up_right,
	eye_look_up_left,
	eye_look_up_right,
	eye_look_down_left,
	eye_look_down_right,
	eye_look_in_left,
	eye_look_in_right,
	eye_look_out_left,
	eye_look_out_right,
	eye_blink_left,
	eye_blink_right,
	eye_squint_left,
	eye_squint_right,
	eye_wide_left,
	eye_wide_right,
	cheek_puff,
	cheek_squint_left,
	cheek_squint_right,
	nose_sneer_left,
	nose_sneer_right,
	jaw_open,
	jaw_forward,
	jaw_left,
	jaw_right,
	mouth_funnel,
	mouth_pucker,
	mouth_left,
	mouth_right,
	mouth_roll_upper,
	mouth_roll_lower,
	mouth_shrug_upper,
	mouth_shrug_lower,
	mouth_close,
	mouth_smile_left,
	mouth_smile_right,
	mouth_frown_left,
	mouth_frown_right,
	mouth_dimple_left,
	mouth_dimple_right,
	mouth_upper_up_left,
	mouth_upper_up_right,
	mouth_lower_down_left,
	mouth_lower_down_right,
	mouth_press_left,
	mouth_press_right,
	mouth_stretch_left,
	mouth_stretch_right,
	tongue_out,
	count
};

constexpr const char* blendshape_names[52] = {
	"brow inner up",
	"brow down left",
	"brow down right",
	"brow outer up left",
	"brow outer up right",
	"eye look up left",
	"eye look up right",
	"eye look down left",
	"eye look down right",
	"eye look in left",
	"eye look in right",
	"eye look out left",
	"eye look out right",
	"eye blink left",
	"eye blink right",
	"eye squint left",
	"eye squint right",
	"eye wide left",
	"eye wide right",
	"cheek puff",
	"cheek squint left",
	"cheek squint right",
	"nose sneer left",
	"nose sneer right",
	"jaw open",
	"jaw forward",
	"jaw left",
	"jaw right",
	"mouth funnel",
	"mouth pucker",
	"mouth left",
	"mouth right",
	"mouth roll upper",
	"mouth roll lower",
	"mouth shrug upper",
	"mouth shrug lower",
	"mouth close",
	"mouth smile left",
	"mouth smile right",
	"mouth frown left",
	"mouth frown right",
	"mouth dimple left",
	"mouth dimple right",
	"mouth upper up left",
	"mouth upper up right",
	"mouth lower down left",
	"mouth lower down right",
	"mouth press left",
	"mouth press right",
	"mouth stretch left",
	"mouth stretch right",
	"tongue out"
};

// forward declaration
struct tosc_message;
class CDevice_FaceCap;

//! Device hardware template
class CDevice_FaceCap_Hardware
{
public:
	CDevice_FaceCap_Hardware();							//!< Constructor.
	~CDevice_FaceCap_Hardware();							//!< Destructor.

	void		SetParent(FBDevice* pParent);

	//--- Communications
	bool		Open();										//!< Open the device.
	bool		Close();									//!< Close the device.
	int			FetchData			();						//!< Fetch a data packet.
	bool		PollData			();						//!< Poll for new data.

	//--- Attribute management
	void		SetStreaming		(bool pStreaming)		{	mStreaming = pStreaming;		}
	bool		GetStreaming		()						{	return mStreaming;				}
	void		SetCommunicationType(FBCommType pType);
	int			GetCommunicationType();
	
	void		SetNetworkPort		(int pPort)				{ 	mNetworkPort = pPort;			}
	int			GetNetworkPort		()						{ 	return mNetworkPort;			}
	
	//--- Device interaction
	bool		GetSetupInfo		();
	bool		StartStream			();
	bool		StopStream			();

	//--- Hardware abstraction of device positional information
	void		GetPosition			( double* pPos );
	void		GetRotation			( double* pRot );

	void		GetLeftEyeRotation(double* rotation);
	void		GetRightEyeRotation(double* rotation);

	const int	GetNumberOfBlendshapes() const;
	const double GetBlendshapeValue(const int index);

private:
	//--- Utility members
	FBSystem		mSystem;								//!< System interface.
	FBDevice* mParent{ nullptr };

	int			mSocket{ 0 };									//!< Socket for communication

	bool		m_Verbose{ false };

	//--- Data extraction members
								
	char	mBuffer	[ MAX_BUFFER_SIZE	];			//!< Read buffer.
	
	//--- Communications members
	
	int				mNetworkSocket{ 0 };							//!< Network socket.
	int				mNetworkPort{ 9000 };							//!< Network port number.
	
	bool			mStreaming{ true };								//!< Is device in streaming mode?

	//--- Device channel status
	double			mPosition[3] = { 0.0 };							//!< Position of input from hardware.
	double			mRotation[3] = { 0.0 };							//!< Rotation of input from hardware.

	double			m_LeftEye[2] = { 0.0 };		// left eye rotation
	double			m_RightEye[2] = { 0.0 };		// right eye rotation

	double			m_BlendShapes[static_cast<uint32_t>(EHardwareBlendshapes::count)] = { 0.0 };

protected:

	int StartServer(const int server_port);
	bool ProcessMessage(tosc_message *osc);

};

