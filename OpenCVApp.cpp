//Created by: William Jonathan Kusnomo
#include <wx/wx.h>

#include "OpenCVFrame.h"

class OpenCVApp : public wxApp
{
public:
    bool OnInit() override
    {
        SetVendorName("PB");
        SetAppName("wxOpenCVTest");

        (new OpenCVFrame)->Show();
        return true;
    }
}; wxIMPLEMENT_APP(OpenCVApp);