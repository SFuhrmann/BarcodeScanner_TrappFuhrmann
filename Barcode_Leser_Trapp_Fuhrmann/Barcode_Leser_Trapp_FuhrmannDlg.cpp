
// Barcode_Leser_Trapp_FuhrmannDlg.cpp: Implementierungsdatei
//


#include "stdafx.h"
#include "Barcode_Leser_Trapp_Fuhrmann.h"
#include "Barcode_Leser_Trapp_FuhrmannDlg.h"
#include "afxdialogex.h"
#include <iostream>
#include "opencv2/opencv.hpp"
#include <hash_map>
#include <sstream>
#include <string>
#include <algorithm>
using namespace cv;
using namespace stdext;
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SPACE 255
#define BAR 0
DEFINE_GUID(ImageFormatBMP, 0xb96b3cab,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);


// CBarcode_Leser_Trapp_FuhrmannDlg-Dialogfeld




CBarcode_Leser_Trapp_FuhrmannDlg::CBarcode_Leser_Trapp_FuhrmannDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBarcode_Leser_Trapp_FuhrmannDlg::IDD, pParent)
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

//-------------//
//OpenCV Code: //
//-------------//

Mat imgCpy;
hash_map<string, int> digitsMap;

void initMap(){
	//Bar = positive digits
	//Space = negative digits
	//example of first entry: "-32-11" = -3,2,-1,1 = 0001101 = 0
	
	digitsMap["-32-11"] = 0;
	digitsMap["-22-21"] = 1;
	digitsMap["-21-22"] = 2;
	digitsMap["-14-11"] = 3;
	digitsMap["-11-32"] = 4;
	digitsMap["-12-31"] = 5;
	digitsMap["-11-14"] = 6;
	digitsMap["-13-12"] = 7;
	digitsMap["-12-13"] = 8;
	digitsMap["-31-12"] = 9;	
}


//helper for rounding numbers => simulates Math.round()
float round(float num)
{
	return num < 0.0 ? ceil(num - 0.5) : floor(num + 0.5);
}

// reads a single digit starting from the current position of pos Point
// returns an int digit on succes
// returns -1 when pos.x is out of bounds
int read_digit(const Mat& img, cv::Point& pos, int direction, int unit_width)
{
  int pattern[4] = {0, 0, 0, 0};
  stringstream result;

   for (int i = 0; i < 4; i++)
   {
		int startPos = img.at<uchar>(pos);
		int add = 0;
	
		if(startPos == BAR) // When reading BAR the result should be incremented
		{ 
			add = 1;
		}
		else                // When reading SPACE the result should be decremented
		{
			add = -1;
		}

		while(img.at<uchar>(pos) == startPos){
			pattern[i] += add;
			pos.x++;
			if(pos.x >= img.cols -1) // out of bounds
			{
				return -1;
			}
		}
  
		int number = round((float)pattern[i] / unit_width); // image data / barcode is often distorted, so a rounding for approximation is needed  
		result << number * direction;	// flip digit accoring to reading direction => left block = number * 1; right block = number * -1;
 }

  string str = result.str(); 
  return digitsMap[str];
}


// will skip SPACE pixels and find the first BAR pixel.
// returns true when at least 10 SPACE pixels where skipped before hitting a BAR pixel.
// returns false when less then 10 SPACE pixels where skipped before hitting a BAR pixel or pos.x is out of bounds.
bool traversSpace(const Mat& img, cv::Point& pos)
{
  int spaceCounter = 0;
  while (img.at<uchar>(pos.y,pos.x) == SPACE){
	  if(pos.x >= img.cols -1) // out of bounds
	  {
		return false;
	  }
	  pos.x++;	 
	  spaceCounter++;
  }
  
  if(spaceCounter > 9) // Space must be at least 10 pixels wide 
  { 
	 return true;
  }
  else
  {
	 return false;
  }
}


// unit width is needed as a refernce for reading digits of the two data blocks
// returns the mean value of a single bar when starting pattern was found
// returns -1 when no pattern was found or pos.x is out of bounds
int getUnitWidth(const Mat& img, cv::Point& pos) 
{
  int pattern[3] = { BAR, SPACE, BAR };
  float widths[3] = { 0, 0, 0 };
  for (int i = 0; i < 3; i++)
  {
    while (img.at<uchar>(pos) == pattern[i])
    {
      if(pos.x >= img.cols - 1) // out of bounds
	  { 
		return -1;
	  }

	  pos.x++;
      widths[i]++;
    }
  }
  
  float max = std::max(widths[0],std::max(widths[1],widths[2]));  // determin maxmium value
  float min = std::min(widths[0],std::min(widths[1],widths[2]));  // determin minimum value

  if(min/max >= 0.75)   // when the relation between min and max is beyond a certain threshold (perfect = 1, adequate >= 0.75)
  {
	 return round(float(widths[0] + widths[1] + widths[2]) / 3);	// return mean value
  }
  return -1;
}



// traverses the pattern which appears after the first data block 
bool traversMidCode(const Mat& img, cv::Point& pos) 
{
  int pattern[5] = { SPACE, BAR, SPACE, BAR, SPACE };
  for (int i = 0; i < 5; i++){
	  while (img.at<uchar>(pos) == pattern[i]){
		  pos.x++;
		  if(pos.x >= img.cols - 1) // out of bounds
		  { 
			return false;
		  }
	  }
   }
   return true;
}

// reads a block -> 6 digits from the barcode
// receives a vector<int> from getBarcode()
// This Method is called two times when attempting to read a barcode ->
  // -> First time the digits vector is empty, direction is indicating to read the left block (direction = 1)
  // -> Second time the vector is already filled with 6 barcode digits, this time the direction indicates to read the right block (direction = -1)
// returns a vector<int> when all digits could be read without error
// returns an empty vector<int> when an error occurs
vector<int> readBlock(const Mat& img, cv::Point& pos, int direction, int unit_width, vector<int>& digits)  
{ 
	for (int i = 0; i < 6; i++)
	{
		int d = read_digit(img, pos, direction, unit_width); 
							
		if(d < 0) // error 
		{ 
			vector<int> null;
			return null;     // return null when digit is broken
		}
		digits.push_back(d);	// add digit to array
	}

	return digits;
}

// checks if block is valid
// receives a vector<int> from readBlock()
// returns true when block is valid
// returns false when block is invalid
// An invalid block could be an empty vector or a vector consisting of to many zeroes.
bool isValidBlock(vector<int>& block) 
{
	int zeroCounter = 0;
	int blockSize = block.size();


	if(blockSize < 6) 
	{
		return false;
	}

	for(int i = 0; i < blockSize; i++){
		if(block[i] == 0)
		{
			zeroCounter++;
		}
	}

	if(zeroCounter >= 5){
		return false;
	} 

	return true;
}


// traversing every pixel of the image and calling necessary functions to read a barcode.
// will return the barcode as a vector<int> of digits.
// will return and empty vector<int> when no barcode was found.
vector<int> getBarcode(const Mat& img, cv::Point& startDraw, cv::Point& endDraw)
{
	cv::Point pos(0,0);
	for(pos.y = 0; pos.y < img.rows; pos.y++)
	{		
		for(pos.x = 0; pos.x < img.cols; pos.x++)
		{	
			if(traversSpace(img, pos)) // skip white pixels (function will return 0 when less then 10 pixels are skipped)
			{  
				int unitWidth = getUnitWidth(img, pos);
				if(unitWidth > 0)									// when appropriate start of barcode was found
				{
					startDraw = pos;
					vector<int> digits;
					if(isValidBlock(readBlock(img, pos, 1, unitWidth, digits))) // read left Block
					{ 
						if(traversMidCode(img, pos))								//travers non data block
						{	
							if(isValidBlock(readBlock(img, pos, -1, unitWidth, digits))) // read right Block
							{ 	
								endDraw = pos;
								return digits;						// send digits back to calling function 
							}
						}
					}
				}
			}
		}
	}
	vector<int> null;		
	return null;			// return null when no barcode was found
}

// Draws a line where the barcode was found
void drawLine(const Mat& img, cv::Point& startPoint, cv::Point& endPoint)
{
	Mat tmp;
	img.copyTo(tmp);
	
	line(tmp,startPoint,endPoint,Scalar(0,255,0),2,8); // 2, 8
	imshow("image", tmp );
}

CString getBarcodeString(CString filePath)
{
  Mat img = cv::imread((LPCTSTR) filePath, CV_LOAD_IMAGE_COLOR);

  
  initMap();
  Mat rgbCpy;
  img.copyTo(rgbCpy);
  cvtColor(img, img, CV_BGR2GRAY);
  cv::threshold(img, img, 128, 255, cv::THRESH_BINARY); // 128 - 255

  cv::Point startDraw(0,0);
  cv::Point endDraw(0,0);
  vector<int> barcode = getBarcode(img, startDraw, endDraw);	// get barcode
  CString result, result1;
  
  if(barcode.size() > 0) // Display data when barcode found
  {
	for (int i = 0; i < 12; i++){
		CString a;
		a.Format("%d", barcode[i]);
		result = result + a;
	}
  }
  return result;


}

//------------------//
//OpenCVCode -- END //
//------------------//

void CBarcode_Leser_Trapp_FuhrmannDlg::OnBnClickedFileOpen()
{
	// szFilters is a text string that includes two file name filters:
	TCHAR szFilters[]= _T("Picture Files|*.png; *.bmp; *.jpg; *.tif; *.gif; *.jpeg|All Files (*.*)|*.*||");

	// Create an Open dialog; the default file name extension is ".my".
	CFileDialog fOpenDlg(true, 0, 0, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilters, this);

	// Display the file dialog. When user clicks OK, fileDlg.DoModal() 
	// returns IDOK.
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

		//open HBITMAP
		//HBITMAP bitmap = (HBITMAP)::LoadImage(NULL, pathName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		m_picture.SetBitmap((HBITMAP)bmp.Detach());
		ReleaseDC(screenDC);
		//end of external source code

		m_Result.SetWindowText(getBarcodeString(pathName));

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