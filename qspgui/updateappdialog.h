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

#ifndef UPDATE_APP_DIALOG_H
    #define UPDATE_APP_DIALOG_H

    #include <wx/wx.h>
    #include <wx/hyperlink.h>

    class UpdateAppDialog : public wxDialog
    {
        DECLARE_CLASS(UpdateAppDialog)
        DECLARE_EVENT_TABLE()

        public:
            UpdateAppDialog(wxWindow *parent, const wxString &title,
                const wxString& newVersion, const wxString& releaseNotes, const wxString& updateUrl,
                int style = wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER);

        private:
            void OnTextUrl(wxTextUrlEvent &event);
    };

#endif
