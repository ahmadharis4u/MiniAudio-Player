
// MiniAudioPlayerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "MiniAudioPlayer.h"
#include "MiniAudioPlayerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <string>


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMiniAudioPlayerDlg dialog



CMiniAudioPlayerDlg::CMiniAudioPlayerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MINIAUDIOPLAYER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
#ifdef USE_AUDIO_ENGINE
	m_audioEngine = AudioEngine();
#endif
}

void CMiniAudioPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMiniAudioPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON4, &CMiniAudioPlayerDlg::OnBnRecord)
	ON_BN_CLICKED(IDC_BUTTON2, &CMiniAudioPlayerDlg::OnBnBack10s)
	ON_BN_CLICKED(IDC_BUTTON1, &CMiniAudioPlayerDlg::OnBnPlayPause)
	ON_BN_CLICKED(IDC_BUTTON5, &CMiniAudioPlayerDlg::OnBnStop)
	ON_BN_CLICKED(IDC_BUTTON3, &CMiniAudioPlayerDlg::OnBnForward10s)
	ON_BN_CLICKED(IDC_BUTTON6, &CMiniAudioPlayerDlg::OnBnPlayRecording)
END_MESSAGE_MAP()


// CMiniAudioPlayerDlg message handlers

BOOL CMiniAudioPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMiniAudioPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMiniAudioPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMiniAudioPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMiniAudioPlayerDlg::OnBnRecord()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON6)->EnableWindow(FALSE);

	std::wstring title = L"Recording...";
	GetDlgItem(IDC_STATIC)->SetWindowTextW(title.c_str());

#ifndef USE_AUDIO_ENGINE
	m_audioHelper.SimpleCapture();
#else
	m_audioEngine.StartRecording();
#endif
}


void CMiniAudioPlayerDlg::OnBnBack10s()
{
	// TODO: Add your control notification handler code here
#ifdef USE_AUDIO_ENGINE
	m_audioEngine.Backward10s();
#else
	m_audioHelper.Backward10s();
#endif
}


void CMiniAudioPlayerDlg::OnBnPlayPause()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON6)->EnableWindow(FALSE);

	int ret = 0;
	std::wstring title;
	if (!m_bPlay)
	{
#ifdef USE_AUDIO_ENGINE
		ret = m_audioEngine.PlayAudios(2);
#else
		ret = m_audioHelper.SimplePlayback();
#endif
		title = L"Playing : TestAudio-X.mp3";
	}
	else
	{
#ifdef USE_AUDIO_ENGINE
		ret = m_audioEngine.PausePlayback();
#else
		ret = m_audioHelper.PausePlayback();
#endif
		title = L"Paused.";
	}

	if (ret)
		OnBnStop();
	else
	{
		GetDlgItem(IDC_STATIC)->SetWindowTextW(title.c_str());
		m_bPlay = !m_bPlay;
	}

}


void CMiniAudioPlayerDlg::OnBnStop()
{
#ifdef USE_AUDIO_ENGINE
	m_audioEngine.Quit();
#else
	m_audioHelper.Quit();
#endif
	m_bPlay = false;

	std::wstring title = L"Stopped.";
	GetDlgItem(IDC_STATIC)->SetWindowTextW(title.c_str());

	GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON6)->EnableWindow(TRUE);
}


void CMiniAudioPlayerDlg::OnBnForward10s()
{
#ifdef USE_AUDIO_ENGINE
	m_audioEngine.Forward10s();
#else
	m_audioHelper.Forward10s();
#endif
}


void CMiniAudioPlayerDlg::OnBnPlayRecording()
{
	GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);

#ifdef USE_AUDIO_ENGINE
	if (!m_audioEngine.PlayRecording())
#else
	if (!m_audioHelper.PlayRecording())
#endif
	{
		std::wstring title = L"Playing : TestRecording.wav";
		GetDlgItem(IDC_STATIC)->SetWindowTextW(title.c_str());
	}
}
