
// MiniAudioPlayerDlg.h : header file
//

#pragma once

#include "AudioEngineConfig.h"

#ifdef USE_AUDIO_ENGINE
#include "AudioEngine.h"
#else
#include "AudioHelper.h"
#endif

// CMiniAudioPlayerDlg dialog
class CMiniAudioPlayerDlg : public CDialogEx
{
// Construction
public:
	CMiniAudioPlayerDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MINIAUDIOPLAYER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnRecord();

private:
#ifdef USE_AUDIO_ENGINE
	CAudioEngine m_audioEngine;
#else
	CAudioHelper m_audioHelper;
#endif
	bool m_bPlay = false;
public:
	afx_msg void OnBnBack10s();
	afx_msg void OnBnPlayPause();
	afx_msg void OnBnStop();
	afx_msg void OnBnForward10s();
	afx_msg void OnBnPlayRecording();
};
