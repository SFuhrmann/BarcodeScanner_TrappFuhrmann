
// Barcode_Leser_Trapp_Fuhrmann.h: Hauptheaderdatei f�r die PROJECT_NAME-Anwendung
//

#pragma once

#ifndef __AFXWIN_H__
	#error "'stdafx.h' vor dieser Datei f�r PCH einschlie�en"
#endif

#include "resource.h"		// Hauptsymbole


// CBarcode_Leser_Trapp_FuhrmannApp:
// Siehe Barcode_Leser_Trapp_Fuhrmann.cpp f�r die Implementierung dieser Klasse
//

class CBarcode_Leser_Trapp_FuhrmannApp : public CWinApp
{
public:
	CBarcode_Leser_Trapp_FuhrmannApp();

// �berschreibungen
public:
	virtual BOOL InitInstance();

// Implementierung

	DECLARE_MESSAGE_MAP()
};

extern CBarcode_Leser_Trapp_FuhrmannApp theApp;