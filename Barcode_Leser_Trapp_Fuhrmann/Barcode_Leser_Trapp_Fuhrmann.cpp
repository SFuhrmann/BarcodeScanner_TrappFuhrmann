
// Barcode_Leser_Trapp_Fuhrmann.cpp: Definiert das Klassenverhalten f�r die Anwendung.
//

#include "stdafx.h"
#include "Barcode_Leser_Trapp_Fuhrmann.h"
#include "Barcode_Leser_Trapp_FuhrmannDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBarcode_Leser_Trapp_FuhrmannApp

BEGIN_MESSAGE_MAP(CBarcode_Leser_Trapp_FuhrmannApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CBarcode_Leser_Trapp_FuhrmannApp-Erstellung

CBarcode_Leser_Trapp_FuhrmannApp::CBarcode_Leser_Trapp_FuhrmannApp()
{
	// TODO: Hier Code zur Konstruktion einf�gen
	// Alle wichtigen Initialisierungen in InitInstance positionieren
}


// Das einzige CBarcode_Leser_Trapp_FuhrmannApp-Objekt

CBarcode_Leser_Trapp_FuhrmannApp theApp;


// CBarcode_Leser_Trapp_FuhrmannApp-Initialisierung

BOOL CBarcode_Leser_Trapp_FuhrmannApp::InitInstance()
{
	CWinApp::InitInstance();


	// Shell-Manager erstellen, falls das Dialogfeld
	// Shellstrukturansicht- oder Shelllistenansicht-Steuerelemente enth�lt.
	CShellManager *pShellManager = new CShellManager;

	// Standardinitialisierung
	// Wenn Sie diese Features nicht verwenden und die Gr��e
	// der ausf�hrbaren Datei verringern m�chten, entfernen Sie
	// die nicht erforderlichen Initialisierungsroutinen.
	// �ndern Sie den Registrierungsschl�ssel, unter dem Ihre Einstellungen gespeichert sind.
	// TODO: �ndern Sie diese Zeichenfolge entsprechend,
	// z.B. zum Namen Ihrer Firma oder Organisation.
	SetRegistryKey(_T("Vom lokalen Anwendungs-Assistenten generierte Anwendungen"));

	CBarcode_Leser_Trapp_FuhrmannDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	//CFileDialog fOpenDlg(true,"txt", "vicon_cams_data", OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, "Camera Data Files (*.txt)|*.txt|*.dat||", this);
	//CFileDialog fOpenDlg(true, 0, 0, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, 0, this);

	if (nResponse == ID_FILE_OPEN)
	{
		// TODO: F�gen Sie hier Code ein, um das Schlie�en des
		//  Dialogfelds �ber "OK" zu steuern
	}
	else if (nResponse == IDCLOSE )
	{
		// TODO: F�gen Sie hier Code ein, um das Schlie�en des
		//  Dialogfelds �ber "Abbrechen" zu steuern
	}

	// Den oben erstellten Shell-Manager l�schen.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Da das Dialogfeld geschlossen wurde, FALSE zur�ckliefern, sodass wir die
	//  Anwendung verlassen, anstatt das Nachrichtensystem der Anwendung zu starten.
	return FALSE;
}

