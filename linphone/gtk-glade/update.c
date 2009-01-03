/*
linphone, gtk-glade interface.
Copyright (C) 2008  Simon MORLAT (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "linphone.h"

#ifdef WIN32

#include <wininet.h>

static int linphone_gtk_get_new_version(const char *version_url, char *version, size_t size){
	DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer;
    BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL, 
               hConnect = NULL;
    int ret=-1;

	hSession=InternetOpen("Linphone",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0);

	if (hSession==NULL) return -1;
	
	hConnect=InternetOpenUrl(hSession,version_url,NULL,0,0,0);

	if (hConnect==NULL) {
		InternetCloseHandle(hSession);
		return -1;
	}

	if (InternetReadFile(hConnect,version,size,&dwDownloaded)){
		version[dwDownloaded]=0;
        ms_message("Got response: %s", version);
        ret=0;
	}

    
    // Close any open handles.
    if (hConnect) InternetCloseHandle(hConnect);
    if (hSession) InternetCloseHandle(hSession);
    return ret;
}

#else

static int linphone_gtk_get_new_version(char *version, size_t size){
	return -1;
}

#endif

static void new_version_response(GtkWidget *dialog, int response_id, gpointer download_site){
	if (response_id==GTK_RESPONSE_YES){
		linphone_gtk_open_browser((const char*)download_site);
	}
	gtk_widget_destroy(dialog);
}

static void popup_new_version(const char *download_site){
	GtkWidget *dialog;
	/* draw a question box. link to dialog_click callback */
	dialog = gtk_message_dialog_new (
				NULL,
                GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION,
                GTK_BUTTONS_YES_NO,
                _("A more recent version is availalble from %s.\nWould you like to open a browser to download it ?"),
				download_site);
	g_signal_connect(G_OBJECT (dialog), "response",
            G_CALLBACK (new_version_response),
        	(gpointer)download_site);
	/* actually show the box */
	gtk_widget_show(dialog);
}

static int version_compare(const char *v1, const char *v2){
	return strcmp(v1,v2);
}

static void *check_for_new_version(void *d){
	const char *version_url=(const char *)d;
	char version[256];
	if (linphone_gtk_get_new_version(version_url,version,sizeof(version))==0){
		if (version_compare(version,LINPHONE_VERSION)>0){
			const char *download_site=linphone_gtk_get_ui_config("download_site",NULL);
			if (download_site) popup_new_version(download_site);
		}
	}
}

void linphone_gtk_check_for_new_version(void){
	ortp_thread_t thread;
	static gboolean done=FALSE;
	const char *version_url;
	if (done) return;
	done=TRUE;
	version_url=linphone_gtk_get_ui_config("update_url",NULL);
	if (version_url==NULL) return ;
	ortp_thread_create(&thread,NULL,check_for_new_version,(void*)version_url);
}