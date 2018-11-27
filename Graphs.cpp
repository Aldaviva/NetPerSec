/* 
 * Draws the histogram and line graphs
 */

#include "StdAfx.h"
#include "Globals.h"
#include "Graphs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Width of bars or line segments
#define WIDTH 4


// CGraphs
CGraphs::CGraphs() {
	m_nGraphRange = 100;
	m_bBarGraph = TRUE;
}

CGraphs::~CGraphs() {}


BEGIN_MESSAGE_MAP(CGraphs, CWnd)
	//{{AFX_MSG_MAP(CGraphs)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CGraphs message handlers

BOOL CGraphs::Create(DWORD dwStyle, const RECT &rc, CWnd *pParentWnd, UINT nID, CCreateContext* /* pContext */) {
	static CString sClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);
	return CWnd::CreateEx(WS_EX_STATICEDGE,
			sClass, NULL, dwStyle,
			rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
			pParentWnd->GetSafeHwnd(),
			(HMENU)nID);
}


void CGraphs::SetGraphRange(UINT nRange) {
	m_nGraphRange = max(nRange, 1);
	RedrawGraph();
}


// For line graph
void CGraphs::SetSize(int nPoints) {
	m_GraphArray.SetSize(nPoints);
}


void CGraphs::SetStyle(BOOL nStyle) {
	m_bBarGraph = nStyle;
	int len = m_GraphArray.GetSize();
	for (int i = 0; i < len; i++)
		m_GraphArray[i] = 0;
}


void CGraphs::DrawGrid(CDC &pDC, CRect &pRect) {
	CPen pen(PS_SOLID, 1, RGB(0,128,0));
	CPen *pOldPen = pDC.SelectObject(&pen);
	pDC.MoveTo(0, pRect.Height() / 2);
	pDC.LineTo(pRect.right, pRect.Height() / 2);
	pDC.SelectObject(pOldPen);
}


void CGraphs::ClearGraph() {
	CRect rc;
	GetClientRect(rc);
	CBrush bkBrush(g_ColorBack);
	m_MemDC.FillRect(rc, &bkBrush);
	DrawGrid(m_MemDC, rc);
}


void CGraphs::RedrawGraph() {
	CClientDC dc(this);
	CRect rcClient;
	GetClientRect(rcClient);
	
	if (m_MemDC.GetSafeHdc() == NULL) {
		m_MemDC.CreateCompatibleDC(&dc);
		m_Bitmap.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
		m_MemDC.SelectObject(m_Bitmap);
		// Draw scale
		m_MemDC.SetBkColor(RGB(0,0,0));
		CBrush brush(g_ColorBack);
		m_MemDC.FillRect(rcClient, &brush);
		DrawGrid(m_MemDC, rcClient);
	}
	
	InvalidateRect(rcClient);
}


void CGraphs::SetPos(UINT nPos, COLORREF cr, int nIndex) {
	DrawGraph(nPos, cr, nIndex);
	Invalidate();
}


void CGraphs::OnPaint() {
	CPaintDC dc(this);  // Device context for painting
	
	// Do not call CWnd::OnPaint() for painting messages
	CRect rcClient;
	GetClientRect(rcClient);
	
	// draw scale
	if (m_MemDC.GetSafeHdc() != NULL)
		dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &m_MemDC, 0, 0, SRCCOPY);
}


void CGraphs::ShiftLeft() {
	CRect rcClient;
	GetClientRect(rcClient);
	
	if (m_MemDC.GetSafeHdc() != NULL) {
		CRect rcRight = rcClient;
		rcRight.left = rcRight.right - WIDTH;
		
		m_MemDC.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &m_MemDC, WIDTH, 0, SRCCOPY);
		CBrush bkBrush(g_ColorBack);
		m_MemDC.FillRect(rcRight, &bkBrush);
	}
}


void CGraphs::DrawGraph(UINT nPos, COLORREF crColor, int nLineIndex) {
	CRect rcClient;
	GetClientRect(rcClient);
	
	if (m_MemDC.GetSafeHdc() != NULL) {
		m_MemDC.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &m_MemDC, 0, 0, SRCCOPY);
		
		CRect rcTop(rcClient.right - WIDTH, 0, rcClient.right - WIDTH/2, rcClient.bottom);
		rcTop.top = rcClient.bottom - (long)(min((double)nPos / m_nGraphRange, 1.0) * rcClient.Height() + 0.5);
		
		m_MemDC.SetBkColor(RGB(0,0,0));
		
		// Draw scale
		CRect rcRight = rcClient;
		rcRight.left = rcRight.right - WIDTH;
		
		DrawGrid(m_MemDC, rcRight);
		
		// Draw a tick mark
		{
			CPen pen(PS_SOLID, 1, RGB(0,128,0));
			CPen *pOldPen = m_MemDC.SelectObject(&pen);
			m_MemDC.MoveTo(rcRight.left, rcRight.Height() / 2 - 2);
			m_MemDC.LineTo(rcRight.left, rcRight.Height() / 2 + 3);
			m_MemDC.SelectObject(pOldPen);
		}
		
		// Draw graph
		if (m_bBarGraph) {
			CBrush brush(crColor);
			m_MemDC.FillRect(rcTop, &brush);
		} else if (nLineIndex != -1 && nLineIndex < m_GraphArray.GetSize()) {
			CPen pen(PS_SOLID, 1, crColor);
			CPen *pOldPen = m_MemDC.SelectObject(&pen);
			m_MemDC.MoveTo(rcRight.left - 1, m_GraphArray.GetAt(nLineIndex));
			m_MemDC.LineTo(rcClient.right - 1, rcTop.top);
			m_GraphArray.SetAt(nLineIndex, rcTop.top);
			m_MemDC.SelectObject(pOldPen);
		}
	}
}


int CGraphs::GetTotalElements() {
	CRect rc;
	GetClientRect(rc);
	return rc.right / WIDTH + 1;
}
