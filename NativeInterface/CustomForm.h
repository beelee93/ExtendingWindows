// NativeInterface.h

#pragma once

#define PAD_LEFT 10
#define PAD_RIGHT 10
#define PAD_TOP 40
#define PAD_BOTTOM 20

using namespace System;
using namespace System::Windows::Forms;

namespace NativeInterface {
	// helper functions
	System::Drawing::Rectangle FromNativeRect(RECT* rect);
	void MakeClientRect(RECT* windowRect); 

	// for use in the NCPaint event handler
	public ref class CustomPaintEventArgs : EventArgs {
		System::Drawing::Graphics^ m_graphics;
		System::Drawing::Size m_paintArea;

	internal:
		CustomPaintEventArgs(System::Drawing::Graphics^ g, System::Drawing::Size paintArea) : 
			m_graphics(g), m_paintArea(paintArea) { }

	public:
		property System::Drawing::Graphics^ Graphics {
			System::Drawing::Graphics^ get() { return m_graphics; }
		}
	};

public ref class CustomForm : Form
{
public:
	event EventHandler<CustomPaintEventArgs^>^ NonClientPaint;

	property bool IsWindowActive {
		bool get() { return m_active; }
	}
private:
	void NCCalcSize(Message %m);
	void NCPaint(Message %m);
	void NCHitTest(Message %m);
	void NCActivate(Message %m);
	void WindowPaint(Message %m);

	bool m_active = true;

protected:
	virtual void WndProc(Message %m) override;

	virtual void OnNonClientPaint(System::Drawing::Graphics^ g, 
		System::Drawing::Size paintArea);

	virtual void OnPaint(PaintEventArgs^ e) override;

};

	
}
