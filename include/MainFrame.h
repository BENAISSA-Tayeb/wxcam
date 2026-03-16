#pragma once
#include <wx/wx.h>
#include "CameraPanel.h"

class MainFrame : public wxFrame {
public:
    MainFrame();

private:
    void OnScreenshot (wxCommandEvent& evt);
    void OnArchive    (wxCommandEvent& evt);
    void OnPurge      (wxCommandEvent& evt);
    void OnFetchPosts (wxCommandEvent& evt);

    wxString      GetScreenshotsDir() const;
    wxArrayString GetPngFiles()       const;

    CameraPanel* m_cameraPanel = nullptr;

    wxDECLARE_EVENT_TABLE();
};
