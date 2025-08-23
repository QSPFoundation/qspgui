// Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org)
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "updateappdialog.h"

IMPLEMENT_CLASS(UpdateAppDialog, wxDialog)

BEGIN_EVENT_TABLE(UpdateAppDialog, wxDialog)
    EVT_TEXT_URL(wxID_ANY, UpdateAppDialog::OnTextUrl)
END_EVENT_TABLE()

UpdateAppDialog::UpdateAppDialog(wxWindow *parent, const wxString &title,
    const wxString &newVersion, const wxString& releaseNotes, const wxString &updateUrl, int style) :
    wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(400, 400), style)
{
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText *updateMessage = new wxStaticText(this, wxID_ANY,
        wxString::Format(_("A new version (%s) is available!"), newVersion.wx_str()));
    wxFont updateMessageFont = updateMessage->GetFont();
    updateMessageFont.SetWeight(wxFONTWEIGHT_BOLD);
    updateMessageFont.SetPointSize(updateMessageFont.GetPointSize() + 2);
    updateMessage->SetFont(updateMessageFont);

    wxHyperlinkCtrl *link = new wxHyperlinkCtrl(this, wxID_ANY, _("Click here to download the update"), updateUrl);

    wxStaticText *releaseNotesLabel = new wxStaticText(this, wxID_ANY, _("Release notes:"));

    wxTextCtrl *releaseNotesText = new wxTextCtrl(this, wxID_ANY,
        releaseNotes, wxDefaultPosition, wxSize(380, 200),
        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH | wxTE_AUTO_URL);

    wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton *updateBtn = new wxButton(this, wxID_OK, _("Get the latest version"));
    wxButton *laterBtn = new wxButton(this, wxID_CANCEL, _("Later"));

    buttonSizer->Add(updateBtn, 0, wxALL, 5);
    buttonSizer->Add(laterBtn, 0, wxALL, 5);

    mainSizer->Add(updateMessage, 0, wxLEFT | wxRIGHT | wxTOP | wxALIGN_CENTER, 10);
    mainSizer->Add(link, 0, wxALL | wxALIGN_CENTER, 5);
    mainSizer->Add(releaseNotesLabel, 0, wxALL, 5);
    mainSizer->Add(releaseNotesText, 1, wxLEFT | wxRIGHT | wxEXPAND, 5);
    mainSizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);

    SetSizerAndFit(mainSizer);
    SetAutoLayout(true);

    SetMinClientSize(wxSize(400, 350));
}

void UpdateAppDialog::OnTextUrl(wxTextUrlEvent &event)
{
    if (event.GetMouseEvent().LeftUp())
    {
        wxTextCtrl *textCtrl = wxStaticCast(event.GetEventObject(), wxTextCtrl);
        wxString fullText = textCtrl->GetValue();

        wxString link = fullText.Mid(event.GetURLStart(), event.GetURLEnd() - event.GetURLStart());

        if (!link.IsEmpty())
            wxLaunchDefaultBrowser(link);
    }
}
