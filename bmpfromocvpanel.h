//Created by: William Jonathan Kusnomo
#pragma once

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <opencv2/opencv.hpp>


class wxBitmapFromOpenCVPanel : public wxScrolledCanvas
{
public:
    wxBitmapFromOpenCVPanel(wxWindow* parent);
    bool m_firstClick;
    bool drawMode = false;
    wxPoint m_firstPoint;
    std::vector<std::pair<wxString, wxPoint>> m_texts;
    std::vector<std::pair<wxPoint, wxPoint>> m_lines;
    std::vector<std::vector<cv::Point>> road_lines;
    bool SetBitmap(const wxBitmap& bitmap, const long timeGet, const long timeConvert);
    void  OnMouseClick(wxMouseEvent& event);
    void DrawLineOnImage(const wxPoint& p1, const wxPoint& p2);
    void vectorClear();
    void undo();
    std::vector<std::vector<cv::Point>> getHeatmap();
    const wxBitmap& GetBitmap() { return m_bitmap; }
    void DrawTextAtPoint(const wxString& message, const wxPoint& point);

private:
    wxBitmap m_bitmap;
    wxColour m_overlayTextColour;
    wxFont   m_overlayFont;
    long     m_timeGetCVBitmap{0};   // time to obtain bitmap from OpenCV in ms
    long     m_timeConvertBitmap{0}; // time to convert Mat to wxBitmap in ms

    wxSize DoGetBestClientSize() const override;
    void OnPaint(wxPaintEvent&);

};
