
// MiniAudioPlayer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMiniAudioPlayerApp:
// See MiniAudioPlayer.cpp for the implementation of this class
//

class CMiniAudioPlayerApp : public CWinApp
{
public:
	CMiniAudioPlayerApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMiniAudioPlayerApp theApp;
