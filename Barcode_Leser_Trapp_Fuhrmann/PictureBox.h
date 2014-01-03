#pragma once

#include "afxwin.h"
   
   // CPictureBox
   
   class CPictureBox : public CStatic
   {
    DECLARE_DYNAMIC(CPictureBox)
   
   public:
    CPictureBox();
    virtual ~CPictureBox();
    void SetBitmap(CString strBitmap);
   
   protected:
    DECLARE_MESSAGE_MAP()
    void ShowBitmap(CPaintDC *pDC);
   
    CString m_sBitmap;
    CBitmap m_bmpBitmap;
    BITMAP bm;
   public:
    afx_msg void OnPaint();
   };