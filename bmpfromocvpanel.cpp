//Created by: William Jonathan Kusnomo
#pragma once
#include <wx/wx.h>
#include <wx/colordlg.h>
#include <wx/dcbuffer.h>
#include <wx/fontdlg.h>
#include <vector>
#include "bmpfromocvpanel.h"

wxBitmapFromOpenCVPanel::wxBitmapFromOpenCVPanel(wxWindow* parent)
    : wxScrolledCanvas(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
    m_firstClick(true)
{
    m_overlayTextColour = *wxRED;
    m_overlayFont = GetFont();

    SetBackgroundColour(*wxBLACK);
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    SetScrollRate(FromDIP(8), FromDIP(8));
   
    EnableScrolling(false, false);
    Bind(wxEVT_LEFT_DOWN, &wxBitmapFromOpenCVPanel::OnMouseClick, this);
    Bind(wxEVT_PAINT, &wxBitmapFromOpenCVPanel::OnPaint, this);

}

bool wxBitmapFromOpenCVPanel::SetBitmap(const wxBitmap& bitmap, const long timeGet, const long timeConvert)
{
    m_bitmap = bitmap;
    if ( m_bitmap.IsOk() )
    {
        if ( m_bitmap.GetSize() != GetVirtualSize() )
        {
            InvalidateBestSize();
            SetVirtualSize(m_bitmap.GetSize());
        }
    }
    else
    {
        InvalidateBestSize();
        SetVirtualSize(1, 1);
    }
    m_timeGetCVBitmap = timeGet;
    m_timeConvertBitmap = timeConvert;

    Refresh(); Update();
    return true;
}

wxSize wxBitmapFromOpenCVPanel::DoGetBestClientSize() const
{
    if ( !m_bitmap.IsOk() )
        return FromDIP(wxSize(64, 48)); // completely arbitrary

    return m_bitmap.GetSize();
}

void wxBitmapFromOpenCVPanel::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);

    dc.Clear();

    if ( !m_bitmap.IsOk() )
        return;

    const wxSize clientSize = GetClientSize();
    wxPoint      offset = GetViewStart();
    int          pixelsPerUnitX = 0, pixelsPerUnitY = 0;
    wxStopWatch  stopWatch;

    stopWatch.Start();

    DoPrepareDC(dc);

    dc.DrawBitmap(m_bitmap, 0, 0, false);

    GetScrollPixelsPerUnit(&pixelsPerUnitX, &pixelsPerUnitY);
    offset.x *= pixelsPerUnitX; offset.y *= pixelsPerUnitY;

    // Draw info "overlay", always at the top left corner of the window
    // regardless of how the bitmap is scrolled.
    const long            drawTime = stopWatch.Time();
    wxDCTextColourChanger textColourChanger(dc, m_overlayTextColour);
    wxDCFontChanger       fontChanger(dc, m_overlayFont);

    dc.DrawText(wxString::Format("GetCVBitmap: %ld ms\nConvertCVtoWXBitmap: %ld ms\nDrawWXBitmap: %ld ms\n",
        m_timeGetCVBitmap, m_timeConvertBitmap, drawTime),
        offset);
    for (const auto& line : m_lines)// for heatmap lines
    {
        dc.SetPen(wxPen(*wxGREEN, 2));
        dc.DrawLine(line.first, line.second);
    }
    for (const auto & text : m_texts)// for velocity acceleration calculation
    {
        dc.DrawText(text.first, text.second);
    }
}

void wxBitmapFromOpenCVPanel::OnMouseClick(wxMouseEvent& event)// choosing 2 points to form a line for heatmap
{
    if (!drawMode)return;
    int x = event.GetX();
    int y = event.GetY();

    if (m_firstClick)
    {
        m_firstPoint = wxPoint(x, y); // Store the first point
        m_firstClick = false; // Next click will be the second point
    }
    else
    {
        wxPoint secondPoint(x, y); // Store the second point

        // Draw the line using wxDC
        DrawLineOnImage(m_firstPoint, secondPoint);

        // Reset for the next line drawing
        m_firstClick = true;
    }
}

void wxBitmapFromOpenCVPanel::DrawLineOnImage(const wxPoint& p1, const wxPoint& p2)
{
    // Store the line points for redrawing during the OnPaint event
    m_lines.push_back({ p1, p2 });
    cv::Point cvPoint1(p1.x, p1.y);
    cv::Point cvPoint2(p2.x, p2.y);
    road_lines.push_back({ cvPoint1,cvPoint2});

    // Trigger a repaint to draw the new line
    Refresh();
}

void wxBitmapFromOpenCVPanel::vectorClear() {
    m_lines.clear();
    road_lines.clear();
    m_texts.clear();
    Refresh();
}

void wxBitmapFromOpenCVPanel::undo()
{
    if (!m_lines.size() ) return;
    m_lines.pop_back();
    road_lines.pop_back();
    Refresh(); Update();
}
void wxBitmapFromOpenCVPanel::DrawTextAtPoint(const wxString& message, const wxPoint& point)
{
    m_texts.push_back({ message, point });
    Refresh();
}
std::vector<std::vector<cv::Point>> wxBitmapFromOpenCVPanel::getHeatmap()
{
    return road_lines;
}
