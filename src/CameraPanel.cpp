#include "CameraPanel.h"
#include <wx/dcbuffer.h>
#include <iostream>

static const int ID_CAMERA_TIMER = wxID_HIGHEST + 1;

wxBEGIN_EVENT_TABLE(CameraPanel, wxPanel)
    EVT_PAINT(CameraPanel::OnPaint)
    EVT_TIMER(ID_CAMERA_TIMER, CameraPanel::OnTimer)
    EVT_SIZE (CameraPanel::OnSize)
wxEND_EVENT_TABLE()


CameraPanel::CameraPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY),
      m_timer(this, ID_CAMERA_TIMER)
{

    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(*wxBLACK);

    StartCapture();
}

CameraPanel::~CameraPanel()
{
    StopCapture();
}

//
void CameraPanel::StartCapture()
{
    std::cout << "[Camera] Opening device 0..." << std::endl;

#ifdef __linux__
    m_cap.open(0, cv::CAP_V4L2);
#else
    m_cap.open(0);
#endif

    if (!m_cap.isOpened()) {
        std::cerr << "[Camera] ERROR: Cannot open camera 0.\n"
                  << "         Check: ls /dev/video* and try another index.\n";
        wxLogError("Cannot open camera.\nCheck that no other app is using it.");
        return;
    }

    m_cap.set(cv::CAP_PROP_FRAME_WIDTH,  1280);
    m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    m_cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    double w = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double h = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    std::cout << "[Camera] Opened at " << w << "x" << h << std::endl;

    m_running = true;

    m_thread = std::thread(&CameraPanel::CaptureLoop, this);


    m_timer.Start(33);
    std::cout << "[Camera] Timer started (33 ms / ~30 fps)" << std::endl;
}

void CameraPanel::StopCapture()
{
    std::cout << "[Camera] Stopping..." << std::endl;
    m_timer.Stop();
    m_running = false;
    if (m_thread.joinable())
        m_thread.join();
    if (m_cap.isOpened())
        m_cap.release();
    std::cout << "[Camera] Stopped." << std::endl;
}



void CameraPanel::CaptureLoop()
{
    std::cout << "[CaptureThread] Running." << std::endl;
    cv::Mat tmp;

    while (m_running) {
        if (!m_cap.read(tmp) || tmp.empty()) {

            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            continue;
        }
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_frame = tmp.clone();   // clone : on possède la mémoire
        }

    }

    std::cout << "[CaptureThread] Exited." << std::endl;
}



void CameraPanel::OnTimer(wxTimerEvent& /*evt*/)
{
    Refresh(false);  
}

void CameraPanel::OnSize(wxSizeEvent& evt)
{
    Refresh(false);
    evt.Skip();
}


void CameraPanel::OnPaint(wxPaintEvent& /*evt*/)
{
    wxAutoBufferedPaintDC dc(this);
    dc.SetBackground(*wxBLACK_BRUSH);
    dc.Clear();

    cv::Mat frame;
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (m_frame.empty()) return;
        frame = m_frame.clone();
    }

    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);

    wxSize sz = GetClientSize();
    if (sz.x <= 0 || sz.y <= 0) return;

    double scale = std::min((double)sz.x / rgb.cols,
                            (double)sz.y / rgb.rows);
    int newW = (int)(rgb.cols * scale);
    int newH = (int)(rgb.rows * scale);
    int offX = (sz.x - newW) / 2;
    int offY = (sz.y - newH) / 2;

    cv::Mat resized;
    cv::resize(rgb, resized, cv::Size(newW, newH), 0, 0, cv::INTER_LINEAR);

    unsigned char* buf = (unsigned char*)malloc(resized.total() * 3);
    std::memcpy(buf, resized.data, resized.total() * 3);
    wxImage img(newW, newH, buf, /*static_data=*/false);

    dc.DrawBitmap(wxBitmap(img), offX, offY, false);
}

// ──────
wxImage CameraPanel::GetCurrentFrame()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_frame.empty()) return wxImage();

    cv::Mat rgb;
    cv::cvtColor(m_frame, rgb, cv::COLOR_BGR2RGB);

    unsigned char* buf = (unsigned char*)malloc(rgb.total() * 3);
    std::memcpy(buf, rgb.data, rgb.total() * 3);
    return wxImage(rgb.cols, rgb.rows, buf, /*static_data=*/false);
}

bool CameraPanel::IsCameraOpen() const
{
    return m_cap.isOpened();
}