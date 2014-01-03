// PictureBox.cpp : implementation file
   //
   
   #include "stdafx.h"
   #include "PictureBox.h"
   #include ".\picturebox.h"
   
   
   // CPictureBox
   
   IMPLEMENT_DYNAMIC(CPictureBox, CStatic)
   CPictureBox::CPictureBox()
   {
   }
   
   CPictureBox::~CPictureBox()
   {
   }
   
   
   BEGIN_MESSAGE_MAP(CPictureBox, CStatic)
    ON_WM_PAINT()
   END_MESSAGE_MAP()
   
   
   
   // CPictureBox message handlers
   
   void CPictureBox::ShowBitmap(CPaintDC *pdc)
   {
    //Create a device context to load the bitmap into
    CDC dcMem;
    dcMem.CreateCompatibleDC(pdc);
   
    //Get the Display area available
    CRect lRect;
    GetClientRect(lRect);
    lRect.NormalizeRect();
   
    //select the bitmap into compatible device context
    CBitmap* pOldBitmap = (CBitmap*)dcMem.SelectObject(&m_bmpBitmap);
    //m_bmpBitmap.SetBitmapDimension(lRect.Width(),lRect.Height());
   
    //copy & resize the window to the dialog window
    pdc->StretchBlt(0,0,lRect.Width(),lRect.Height(),&dcMem,0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);
   }
   void CPictureBox::OnPaint()
   {
    CPaintDC dc(this); // device context for painting
   
    RECT rect;
    GetClientRect(&rect);
    dc.FillSolidRect(&rect, RGB(255,255,255));
   
    if(m_sBitmap!="")
     ShowBitmap(&dc);
   }
   void CPictureBox::SetBitmap(CString strBitmap)
   {
    m_sBitmap = strBitmap;
   
    HBITMAP hBitmap = (HBITMAP) ::LoadImage(AfxGetInstanceHandle(),
    m_sBitmap, IMAGE_BITMAP, 0, 0,
    LR_LOADFROMFILE | LR_CREATEDIBSECTION);
   
    // Do we have a valid handle for the loaded image?
    if (hBitmap)
    {
     // Delete the current bitmap
     if (m_bmpBitmap.DeleteObject())
        m_bmpBitmap.Detach(); // If there was a bitmap, detach it
     // Attach the currently loaded bitmap to the bitmap object
     m_bmpBitmap.Attach(hBitmap);
    }
    m_bmpBitmap.GetBitmap(&bm);  //Get Bitmap Structure
    Invalidate();
   }