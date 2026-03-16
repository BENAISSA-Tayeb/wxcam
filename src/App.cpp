#include "App.h"
#include "MainFrame.h"
#include <wx/image.h>
#include <iostream>

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
    if (!wxApp::OnInit()) return false;

    // Nécessaire pour sauvegarder les screenshots en PNG
    wxImage::AddHandler(new wxPNGHandler());

    std::cout << "Démarrage WxCam...\n";
    std::cout << "Screenshots → ~/Pictures/wxcam-screenshots/\n";
    std::cout << "Vérifiez que la webcam est bien branchée (index 0).\n";

    MainFrame* fenetre = new MainFrame();
    fenetre->Show();
    return true;
}