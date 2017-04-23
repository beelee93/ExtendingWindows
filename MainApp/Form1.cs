using NativeInterface;
using System;
using System.Windows.Forms;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace MainApp
{
public partial class Form1 : CustomForm
{
    public Form1()
    {
        InitializeComponent();
    }
        
    protected override void OnNonClientPaint(Graphics g, Size paintArea)
    {
        base.OnNonClientPaint(g, paintArea);

        // points to be used in linear gradient brush
        Point p1 = new Point(0, 0);
        Point p2 = new Point(paintArea.Width, 0);

        Color c1 = IsWindowActive ? Color.Blue : Color.SkyBlue;
        Color c2 = IsWindowActive ? Color.LightBlue : Color.LightSkyBlue;

        using (LinearGradientBrush brush = new LinearGradientBrush(p1, p2, c1, c2))
        {
            g.FillRectangle(brush, 0, 0, paintArea.Width, paintArea.Height);
        }

        TextRenderer.DrawText(g, "A Custom Frame", SystemFonts.CaptionFont, new Rectangle(10, 0, 256, 40), 
            Color.White, TextFormatFlags.Left| TextFormatFlags.VerticalCenter);
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        base.OnPaint(e);
        e.Graphics.FillRectangle(Brushes.DarkBlue, 0, 0, e.ClipRectangle.Width, e.ClipRectangle.Height);
    }

}
}
