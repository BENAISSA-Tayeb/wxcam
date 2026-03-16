#include "MainFrame.h"
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/ffile.h>
#include <wx/textfile.h>
#include <wx/datetime.h>
#include <iostream>

enum {
    ID_BTN_SCREENSHOT = wxID_HIGHEST + 100,
    ID_BTN_ARCHIVE,
    ID_BTN_PURGE,
    ID_BTN_FETCH,
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_BUTTON(ID_BTN_SCREENSHOT, MainFrame::OnScreenshot)
    EVT_BUTTON(ID_BTN_ARCHIVE,    MainFrame::OnArchive)
    EVT_BUTTON(ID_BTN_PURGE,      MainFrame::OnPurge)
    EVT_BUTTON(ID_BTN_FETCH,      MainFrame::OnFetchPosts)
wxEND_EVENT_TABLE()

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "WxCam",
              wxDefaultPosition, wxSize(960, 600),
              wxDEFAULT_FRAME_STYLE)
{
    Centre();

    wxBoxSizer* vSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);

    auto makeBtn = [&](const wxString& label, int id) {
        wxButton* b = new wxButton(this, id, label);
        b->SetMinSize(wxSize(130, 36));
        btnSizer->Add(b, 0, wxALL, 6);
    };

    makeBtn("Screenshot",  ID_BTN_SCREENSHOT);
    makeBtn("Archive",     ID_BTN_ARCHIVE);
    makeBtn("Purge",       ID_BTN_PURGE);
    makeBtn("Fetch posts", ID_BTN_FETCH);

    vSizer->Add(btnSizer, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 8);

    m_cameraPanel = new CameraPanel(this);
    vSizer->Add(m_cameraPanel, 1, wxEXPAND | wxALL, 8);

    SetSizer(vSizer);
    CreateStatusBar();

    if (m_cameraPanel->IsCameraOpen())
        SetStatusText("Camera ready");
    else
        SetStatusText("No camera found — check terminal for details");
}

// ~/Pictures/wxcam-screenshots/   (créé automatiquement)
wxString MainFrame::GetScreenshotsDir() const
{
    wxString path = wxGetHomeDir()
                  + wxFILE_SEP_PATH + "Pictures"
                  + wxFILE_SEP_PATH + "wxcam-screenshots";
    if (!wxDirExists(path)) {
        wxFileName::Mkdir(path, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
        std::cout << "[Dir] Created: " << path << std::endl;
    }
    return path;
}

wxArrayString MainFrame::GetPngFiles() const
{
    wxArrayString files;
    wxDir::GetAllFiles(GetScreenshotsDir(), &files, "*.png", wxDIR_FILES);
    return files;
}

void MainFrame::OnScreenshot(wxCommandEvent& /*evt*/)
{
    wxImage img = m_cameraPanel->GetCurrentFrame();
    if (!img.IsOk()) {
        std::cerr << "[Screenshot] No frame available yet.\n";
        wxMessageBox("No frame available.\nWait for the camera to start.",
                     "Screenshot", wxICON_WARNING | wxOK, this);
        return;
    }

    wxString ts   = wxDateTime::Now().Format("%Y%m%d_%H%M%S");
    wxString path = GetScreenshotsDir() + wxFILE_SEP_PATH + "shot_" + ts + ".png";

    if (img.SaveFile(path, wxBITMAP_TYPE_PNG)) {
        std::cout << "[Screenshot] Saved: " << path << std::endl;
        SetStatusText("Saved: " + path);
    } else {
        std::cerr << "[Screenshot] Failed to save: " << path << std::endl;
        wxMessageBox("Failed to save screenshot.", "Screenshot", wxICON_ERROR | wxOK, this);
    }
}

void MainFrame::OnArchive(wxCommandEvent& /*evt*/)
{
    wxArrayString files = GetPngFiles();
    if (files.IsEmpty()) {
        std::cout << "[Archive] No PNG files found in screenshots folder.\n";
        wxMessageBox("No screenshots to archive.", "Archive",
                     wxICON_INFORMATION | wxOK, this);
        return;
    }

    std::cout << "[Archive] Zipping " << files.GetCount() << " file(s)...\n";

    wxString tmpZip = wxFileName::CreateTempFileName("wxcam_archive");
    {
        wxFFileOutputStream out(tmpZip);
        if (!out.IsOk()) {
            std::cerr << "[Archive] Cannot create temp file: " << tmpZip << "\n";
            wxMessageBox("Cannot create temp file.", "Archive", wxICON_ERROR | wxOK, this);
            return;
        }
        wxZipOutputStream zip(out);
        for (const wxString& fp : files) {
            wxFFileInputStream fileIn(fp);
            if (!fileIn.IsOk()) { std::cerr << "[Archive] Skip: " << fp << "\n"; continue; }
            zip.PutNextEntry(wxFileName(fp).GetFullName());
            zip.Write(fileIn);
        }
        zip.Close();
        out.Close();
    }

    wxFileDialog dlg(this, "Save archive as", wxEmptyString, "screenshots.zip",
                     "ZIP files (*.zip)|*.zip", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() != wxID_OK) {
        wxRemoveFile(tmpZip);
        std::cout << "[Archive] Cancelled.\n";
        return;
    }

    wxString dest = dlg.GetPath();
    if (!wxCopyFile(tmpZip, dest)) {
        std::cerr << "[Archive] Failed to copy to: " << dest << "\n";
        wxMessageBox("Failed to copy archive.", "Archive", wxICON_ERROR | wxOK, this);
    } else {
        std::cout << "[Archive] Saved: " << dest << "\n";
        SetStatusText("Archive saved: " + dest);
    }
    wxRemoveFile(tmpZip);
}

void MainFrame::OnPurge(wxCommandEvent& /*evt*/)
{
    wxArrayString files = GetPngFiles();
    if (files.IsEmpty()) {
        std::cout << "[Purge] Nothing to delete.\n";
        wxMessageBox("No screenshots to delete.", "Purge",
                     wxICON_INFORMATION | wxOK, this);
        return;
    }

    int answer = wxMessageBox(
        wxString::Format("Delete %zu screenshot(s)?", files.GetCount()),
        "Purge", wxYES_NO | wxICON_WARNING, this);
    if (answer != wxYES) { std::cout << "[Purge] Cancelled.\n"; return; }

    int deleted = 0;
    for (const wxString& fp : files)
        if (wxRemoveFile(fp)) { ++deleted; std::cout << "[Purge] Deleted: " << fp << "\n"; }

    std::cout << "[Purge] Done — " << deleted << " file(s) removed.\n";
    SetStatusText(wxString::Format("Purged %d file(s).", deleted));
}

void MainFrame::OnFetchPosts(wxCommandEvent& /*evt*/)
{
    std::cout << "[Fetch] Starting Bluesky fetch...\n";
    SetStatusText("Fetching posts...");

    const wxString apiUrl =
        "https://public.api.bsky.app/xrpc/app.bsky.feed.getFeed"
        "?feed=at://did:plc:z72i7hdynmk6r22z27h6tvur"
        "/app.bsky.feed.generator/whats-hot"
        "&limit=20";

    wxString tmpJson = wxFileName::CreateTempFileName("wxcam_bsky");

    // -s : silencieux  -L : suit les redirections  --max-time : timeout 10s
    wxString cmd = wxString::Format(
        "curl -s -L --max-time 10 -o \"%s\" \"%s\"",
        tmpJson, apiUrl);

    std::cout << "[Fetch] Running: " << cmd << "\n";

    long ret = wxExecute(cmd, wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE);
    if (ret != 0) {
        wxRemoveFile(tmpJson);
        std::cerr << "[Fetch] curl failed with exit code " << ret << ".\n"
                  << "        Install curl: sudo apt install curl\n";
        wxMessageBox(
            wxString::Format("curl failed (exit %ld).\n"
                             "Install it: sudo apt install curl", ret),
            "Fetch posts", wxICON_ERROR | wxOK, this);
        return;
    }

    // Lecture
    wxFFile jf(tmpJson, "rb");
    wxString json;
    if (!jf.IsOpened() || !jf.ReadAll(&json)) {
        wxRemoveFile(tmpJson);
        std::cerr << "[Fetch] Cannot read temp file.\n";
        wxMessageBox("Could not read API response.", "Fetch posts", wxICON_ERROR | wxOK, this);
        return;
    }
    jf.Close();
    wxRemoveFile(tmpJson);

    std::cout << "[Fetch] Response size: " << json.Length() << " chars\n";

    // ── Extraction
    wxArrayString posts;
    size_t pos = 0;

    while (posts.GetCount() < 20) {
        size_t found = json.find("\"text\":\"", pos);
        if (found == wxString::npos) break;

        size_t start = found + 8;   // juste après "text":"
        size_t end   = start;
        bool   esc   = false;

        while (end < json.Length()) {
            wxUniChar ch = json[end];
            if (esc)        { esc = false; ++end; continue; }
            if (ch == '\\') { esc = true;  ++end; continue; }
            if (ch == '"')  break;
            ++end;
        }

        wxString text = json.Mid(start, end - start);
        text.Replace("\\n", " ");
        text.Replace("\\r", "");
        text.Trim(true).Trim(false);

        if (!text.IsEmpty()) {
            posts.Add(text.Left(200));  // troncature à 200 chars
            std::cout << "[Fetch] Post " << posts.GetCount()
                      << ": " << text.Left(60) << "...\n";
        }
        pos = end + 1;
    }

    if (posts.IsEmpty()) {
        std::cerr << "[Fetch] No posts extracted — API format may have changed.\n"
                  << "        Check: https://docs.bsky.app\n";
        wxMessageBox("No posts extracted.\nThe API format may have changed.",
                     "Fetch posts", wxICON_WARNING | wxOK, this);
        return;
    }

    std::cout << "[Fetch] Extracted " << posts.GetCount() << " post(s).\n";

    //
    wxString tmpTxt = wxFileName::CreateTempFileName("wxcam_posts");
    {
        wxFFile tf(tmpTxt, "w");
        if (!tf.IsOpened()) {
            std::cerr << "[Fetch] Cannot open temp file: " << tmpTxt << "\n";
            wxMessageBox("Cannot write temp file.", "Fetch posts", wxICON_ERROR | wxOK, this);
            return;
        }
        tf.Write(wxString::Format(
            "== Bluesky - %zu latest posts (%s) ==\n\n",
            posts.GetCount(),
            wxDateTime::Now().Format("%Y-%m-%d %H:%M:%S")));
        for (size_t i = 0; i < posts.GetCount(); ++i)
            tf.Write(wxString::Format("%02zu. %s\n", i + 1, posts[i]));
        tf.Close();
    }

    wxDirDialog dlg(this, "Choose destination folder",
                    wxStandardPaths::Get().GetDocumentsDir(),
                    wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dlg.ShowModal() != wxID_OK) {
        wxRemoveFile(tmpTxt);
        std::cout << "[Fetch] Cancelled.\n";
        return;
    }

    wxString dest = dlg.GetPath() + wxFILE_SEP_PATH + "bsky_posts.txt";
    if (!wxCopyFile(tmpTxt, dest)) {
        std::cerr << "[Fetch] Failed to copy to: " << dest << "\n";
        wxMessageBox("Failed to save file.", "Fetch posts", wxICON_ERROR | wxOK, this);
    } else {
        std::cout << "[Fetch] Saved: " << dest << "\n";
        SetStatusText("Posts saved: " + dest);
    }
    wxRemoveFile(tmpTxt);
}
