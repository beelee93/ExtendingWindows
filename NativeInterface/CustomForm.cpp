// This is the main DLL file.

#include "stdafx.h"
#include "CustomForm.h"

void NativeInterface::CustomForm::NCCalcSize(Message % m)
{
	RECT* pRect;
	if (m.WParam.ToInt32()) {
		// proposed dimensions contained within NCCALCSIZE_PARAMS of lParam
		NCCALCSIZE_PARAMS* nparams = (NCCALCSIZE_PARAMS*)m.LParam.ToPointer();
		pRect = &(nparams->rgrc[0]); // contains the proposed windows dimensions
	}
	else {
		// proposed dimensions contained within RECT of lParam
		pRect = (RECT*)m.LParam.ToInt32();
	}
		
	MakeClientRect(pRect);
	m.Result = (IntPtr)0;
}

void NativeInterface::CustomForm::NCPaint(Message % m)
{
	if (m.HWnd != Handle) return;

	bool hasToFreeRgn = false;
	HRGN hrgn = (HRGN)m.WParam.ToPointer();
	HWND hwnd = (HWND)m.HWnd.ToPointer();
	HDC hdc;

	RECT windowRect;
	GetWindowRect(hwnd, &windowRect);

	// hrgn isn't always valid, so we create the region if so
	if (hrgn == 0 || hrgn == (HRGN)1) {
		// we will take the Full Window Region, subtract away the Client Region
		// so that any painting will only be limited within the frame
		hrgn = CreateRectRgnIndirect(&windowRect);

		RECT clientRect = windowRect;
		MakeClientRect(&clientRect);
		HRGN clientRegion = CreateRectRgnIndirect(&clientRect);
		CombineRgn(hrgn, hrgn, clientRegion, RGN_DIFF);

		DeleteObject(clientRegion);

		hasToFreeRgn = true;
	}
	
	// the 0x10000 flag is undocumented, but according to some literature, it just 
	// works (and it is needed, oddly)
	hdc = GetDCEx(hwnd, hrgn, DCX_WINDOW | DCX_INTERSECTRGN | 0x10000);

	if (!hdc) {
		DeleteObject(hrgn);
		return;
	}
	
	RECT rect;
	GetWindowRect(hwnd, &rect);

	// create graphics and raise event
	System::Drawing::Graphics^ graphics = System::Drawing::Graphics::FromHdc(IntPtr(hdc));
	System::Drawing::Size paintArea(windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);
	OnNonClientPaint(graphics, paintArea);

	// release resources
	delete graphics;
	ReleaseDC(hwnd, hdc);


	RedrawWindow(hwnd, &rect, hrgn, RDW_UPDATENOW);
	m.Result = (IntPtr)0;
}

void NativeInterface::CustomForm::NCHitTest(Message % m)
{
	// get the mouse coords
	System::Drawing::Point mouse(m.LParam.ToInt32());

	// transform the mouse point into window space
	RECT windowRect;
	GetWindowRect((HWND)m.HWnd.ToPointer(), &windowRect);
	mouse.X -= windowRect.left;
	mouse.Y -= windowRect.top;

	System::Drawing::Rectangle rect1(0, 0,
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);
	System::Drawing::Rectangle rect2;

	// top border
	rect2 = rect1;
	rect2.Height = 2;
	if (rect2.Contains(mouse)) {
		m.Result = (IntPtr)HTTOP;
		return;
	}

	// top-left corner
	rect2 = rect1;
	rect2.Height = 6;
	rect2.Width = 6;
	if (rect2.Contains(mouse)) {
		m.Result = (IntPtr)HTTOPLEFT;
		return;
	}

	// top-right corner
	rect2.X = rect1.Width - 6;
	if (rect2.Contains(mouse)) {
		m.Result = (IntPtr)HTTOPRIGHT;
		return;
	}

	// bottom-right corner
	rect2.Y = rect1.Height - 6;
	if (rect2.Contains(mouse)) {
		m.Result = (IntPtr)HTBOTTOMRIGHT;
		return;
	}

	// bottom-left corner
	rect2.X = 0;
	if (rect2.Contains(mouse)) {
		m.Result = (IntPtr)HTBOTTOMLEFT;
		return;
	}

	// title bar
	rect2 = rect1;
	rect2.X += 6;
	rect2.Width -= 12;
	rect2.Height = PAD_TOP;
	if (rect2.Contains(mouse)) {
		m.Result = (IntPtr)HTCAPTION;
		return;
	}

	// left border
	rect2 = rect1;
	rect2.Width = PAD_LEFT;
	if (rect2.Contains(mouse)) {
		m.Result = (IntPtr)HTLEFT;
		return;
	}

	// right border
	rect2.X = rect1.Width - PAD_RIGHT;
	rect2.Width = PAD_RIGHT;
	if (rect2.Contains(mouse)) {
		m.Result = (IntPtr)HTRIGHT;
		return;
	}

	// bottom border
	rect2 = rect1;
	rect2.Y = rect1.Height - PAD_BOTTOM;
	rect2.Height = PAD_BOTTOM;
	if (rect2.Contains(mouse)) {
		m.Result = (IntPtr)HTBOTTOM;
		return;
	}

	// we are inside
	m.Result = (IntPtr)HTCLIENT;
}

void NativeInterface::CustomForm::NCActivate(Message % m)
{
	m_active = m.WParam.ToInt32() == TRUE;
	System::Diagnostics::Debug::WriteLine(m_active);

	// force redraw of the non-client area
	RedrawWindow((HWND)m.HWnd.ToPointer(), NULL, NULL, RDW_INVALIDATE|RDW_FRAME);
}

void NativeInterface::CustomForm::WindowPaint(Message %m)
{
	HWND hwnd = (HWND)m.HWnd.ToPointer();
	HDC hdc = 0;

	RECT clientRect;
	GetWindowRect(hwnd, &clientRect);
	MakeClientRect(&clientRect);

	PAINTSTRUCT ps;
	hdc = BeginPaint(hwnd, &ps);
	System::Drawing::Graphics^ graphics = System::Drawing::Graphics::FromHdc(IntPtr(hdc));
	System::Drawing::Rectangle paintArea(0,0,clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

	// use the existing OnPaint method
	OnPaint(gcnew PaintEventArgs(graphics, paintArea));

	// release resources
	delete graphics; 
	EndPaint(hwnd, &ps);

	m.Result = (IntPtr)0;
}

void NativeInterface::CustomForm::WndProc(Message % m)
{
	switch (m.Msg) {
	case WM_NCCALCSIZE:
		NCCalcSize(m);
		break;
	case WM_NCHITTEST:
		NCHitTest(m);
		break;
	case WM_NCPAINT:
		NCPaint(m);
		break;
	case WM_NCACTIVATE:
		NCActivate(m);
		break;
	case WM_PAINT:
		WindowPaint(m);
		break;
	default:
		DefWndProc(m);
		break;
	}
}

void NativeInterface::CustomForm::OnNonClientPaint(System::Drawing::Graphics ^ g, System::Drawing::Size paintArea)
{
	g->Clear(System::Drawing::SystemColors::ActiveBorder);

	// raise the event
	NonClientPaint(this, gcnew NativeInterface::CustomPaintEventArgs(g, paintArea));
}

void NativeInterface::CustomForm::OnPaint(PaintEventArgs ^ e)
{
	Form::OnPaint(e);
	e->Graphics->Clear(System::Drawing::SystemColors::Control);
}

// helper function
System::Drawing::Rectangle NativeInterface::FromNativeRect(RECT * rect)
{
	return System::Drawing::Rectangle(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top);
}

// obtain the client rectangle dimensions given the window rectangle
void NativeInterface::MakeClientRect(RECT * windowRect)
{
	windowRect->left += PAD_LEFT;
	windowRect->right -= PAD_RIGHT;
	windowRect->top += PAD_TOP;
	windowRect->bottom -= PAD_BOTTOM;
}
