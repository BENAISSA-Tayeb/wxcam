#pragma once
#include <wx/wx.h>
#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>
#include <mutex>

class CameraPanel : public wxPanel {
public:
    CameraPanel(wxWindow* parent);
    ~CameraPanel();

    wxImage GetCurrentFrame();
    bool    IsCameraOpen() const;

private:
    void StartCapture();
    void StopCapture();
    void CaptureLoop();           // background thread

    void OnPaint(wxPaintEvent& evt);
    void OnTimer(wxTimerEvent& evt);
    void OnSize (wxSizeEvent&  evt);

    cv::VideoCapture  m_cap;
    cv::Mat           m_frame;        // latest raw BGR frame
    std::mutex        m_mutex;
    std::thread       m_thread;
    std::atomic<bool> m_running{false};
    wxTimer           m_timer;

    wxDECLARE_EVENT_TABLE();
};