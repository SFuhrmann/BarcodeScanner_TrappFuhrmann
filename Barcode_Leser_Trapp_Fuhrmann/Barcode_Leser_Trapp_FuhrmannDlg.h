
// Barcode_Leser_Trapp_FuhrmannDlg.h: Headerdatei
//

#pragma once
#include "afxwin.h"
#include "Barcode_Decoder.h"


// CBarcode_Leser_Trapp_FuhrmannDlg-Dialogfeld
class CBarcode_Leser_Trapp_FuhrmannDlg : public CDialogEx
{
// Konstruktion
public:
	CBarcode_Leser_Trapp_FuhrmannDlg(CWnd* pParent = NULL);	// Standardkonstruktor

// Dialogfelddaten
	enum { IDD = IDD_BARCODE_LESER_TRAPP_FUHRMANN_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung


// Implementierung
protected:
	HICON m_hIcon;
	Barcode_Decoder barcode_decoder;

	// Generierte Funktionen für die Meldungstabellen
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP();
public:
	afx_msg void OnBnClickedCopy();
	afx_msg void OnBnClickedFileOpen();
	CEdit m_Result;
	CStatic m_picture;
};
