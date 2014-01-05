
// Barcode_Leser_Trapp_FuhrmannDlg.cpp: Implementierungsdatei
//


#include "stdafx.h"
#include "Barcode_Leser_Trapp_Fuhrmann.h"
#include "Barcode_Leser_Trapp_FuhrmannDlg.h"
#include "Barcode_Decoder.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DEFINE_GUID(ImageFormatBMP, 0xb96b3cab,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);


// CBarcode_Leser_Trapp_FuhrmannDlg-Dialogfeld
CBarcode_Leser_Trapp_FuhrmannDlg::CBarcode_Leser_Trapp_FuhrmannDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBarcode_Leser_Trapp_FuhrmannDlg::IDD, pParent), barcode_decoder()
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBarcode_Leser_Trapp_FuhrmannDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, ID_RESULT, m_Result);
	DDX_Control(pDX, ID_PICCTRL, m_picture);
}

BEGIN_MESSAGE_MAP(CBarcode_Leser_Trapp_FuhrmannDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCOPY, &CBarcode_Leser_Trapp_FuhrmannDlg::OnBnClickedCopy)
	ON_BN_CLICKED(ID_FILE_OPEN, &CBarcode_Leser_Trapp_FuhrmannDlg::OnBnClickedFileOpen)
END_MESSAGE_MAP()


// CBarcode_Leser_Trapp_FuhrmannDlg-Meldungshandler

BOOL CBarcode_Leser_Trapp_FuhrmannDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	//  wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	SetIcon(m_hIcon, TRUE);			// Großes Symbol verwenden
	SetIcon(m_hIcon, FALSE);		// Kleines Symbol verwenden
	m_Result.SetReadOnly();
	barcode_decoder.initMap();
	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

// Wenn Sie dem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie
//  den nachstehenden Code, um das Symbol zu zeichnen. Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch ausgeführt.

void CBarcode_Leser_Trapp_FuhrmannDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // Gerätekontext zum Zeichnen

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Symbol in Clientrechteck zentrieren
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Symbol zeichnen
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Die System ruft diese Funktion auf, um den Cursor abzufragen, der angezeigt wird, während der Benutzer
//  das minimierte Fenster mit der Maus zieht.
HCURSOR CBarcode_Leser_Trapp_FuhrmannDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBarcode_Leser_Trapp_FuhrmannDlg::OnBnClickedFileOpen()
{
	// szFilters is a text string that includes two file name filters:
	TCHAR szFilters[]= _T("Picture Files|*.png; *.bmp; *.jpg; *.tif; *.gif; *.jpeg|All Files (*.*)|*.*||");

	// Create an Open dialog
	CFileDialog fOpenDlg(true, 0, 0, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilters, this);

	// Display the file dialog. When user clicks OK, fileDlg.DoModal() returns IDOK.
	if(fOpenDlg.DoModal() == IDOK)
	{
		CString pathName = fOpenDlg.GetPathName();

		CImage img;
		img.Load(pathName);
		img.Save(pathName, ImageFormatBMP);

		//Resize Bitmap, source: stackoverflow.com/questions/2339702/setting-resized-bitmap-file-to-an-mfc-picture-control
		CDC *screenDC = GetDC();
		CDC mDC;
		mDC.CreateCompatibleDC(screenDC);
		CBitmap bmp;
		bmp.CreateCompatibleBitmap(screenDC, 400, 300);

		CBitmap *pob = mDC.SelectObject(&bmp);
		mDC.SetStretchBltMode(HALFTONE);
		img.StretchBlt(mDC.m_hDC, 0, 0, 400, 300, 0, 0, img.GetWidth(), img.GetHeight(), SRCCOPY);
		mDC.SelectObject(pob);

		m_picture.SetBitmap((HBITMAP)bmp.Detach());
		ReleaseDC(screenDC);
		//end of external source code

		m_Result.SetWindowText(barcode_decoder.getBarcodeString(pathName));

		m_Result.Copy();

		Invalidate();
		UpdateWindow();
	}
}

	void CBarcode_Leser_Trapp_FuhrmannDlg::OnBnClickedCopy()
{
	//copy the Window Text of ID_RESULT to the clipboard
	CString strData;
	m_Result.GetWindowText(strData);

	if (OpenClipboard())
	{
		EmptyClipboard();
		HGLOBAL hClipboardData;
		hClipboardData = GlobalAlloc(GMEM_DDESHARE, 
							strData.GetLength()+1);

		char * pchData;
		pchData = (char*)GlobalLock(hClipboardData);

		strcpy(pchData, LPCSTR(strData));
		  
		GlobalUnlock(hClipboardData);
		  
		SetClipboardData(CF_TEXT,hClipboardData);

		CloseClipboard();
	}
}