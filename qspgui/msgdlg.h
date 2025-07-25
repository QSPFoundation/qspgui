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

#ifndef MSGDLG_H
    #define MSGDLG_H

    #include <wx/wx.h>
    #include <wx/statline.h>
    #include "textbox.h"
    #include "path_provider.h"

    enum
    {
        ID_MSG_DESC
    };

    class QSPMsgDlg : public wxDialog
    {
        const int MinWidth = 450;
        const int MaxWidth = 550;
        const int MinHeight = 100;
        const int MaxHeight = 350;

        DECLARE_CLASS(QSPMsgDlg)
        DECLARE_EVENT_TABLE()
    public:
        // C-tors / D-tor
        QSPMsgDlg(wxWindow* parent,
                  wxWindowID id,
                  const wxColour& backColor,
                  const wxColour& fontColor,
                  const wxFont& font,
                  const wxString& caption,
                  const wxString& text,
                  bool isHtml,
                  PathProvider *pathProvider);
    protected:
        // Events
        void OnInitDialog(wxInitDialogEvent& event);
        void OnLinkClicked(wxHtmlLinkEvent& event);

        // Fields
        QSPTextBox *m_desc;
    };

#endif
