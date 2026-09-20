/* Open With
 *
 * Copyright (c) 2026 ablyss
 * See the The MIT License included in this folder *
 * Some AI was used to help make this possibe.
 *
 * Lists the apps the user has symlinked into their personal
 * "Open With" folder (~/config/settings/testing/open with/) as a
 * pop-up menu, then launches whichever one they pick with the
 * selected file(s) as arguments.
 */
#include "testing.h"
#include <Alert.h>
#include <AppDefs.h>
#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <MenuItem.h>
#include <Messenger.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <Roster.h>
#include <String.h>

#include <cstring>
#include <vector>

extern "C" {

static const char *kOpenWithSubDir = "testing/open with";


// Resolves ~/config/settings/testing/open with/, creating it (and any
// missing parents) if it doesn't already exist.
static status_t
get_open_with_folder(BPath &outPath)
{
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &outPath);
	if (status != B_OK)
		return status;

	status = outPath.Append(kOpenWithSubDir);
	if (status != B_OK)
		return status;

	return create_directory(outPath.Path(), 0777);
}


// Asks Tracker to open the given folder in a new window, so a first-time
// user can immediately drop app symlinks into it.
static void
open_folder_in_tracker(const BPath &path)
{
	entry_ref ref;
	if (get_ref_for_path(path.Path(), &ref) != B_OK)
		return;

	BMessage openMsg(B_REFS_RECEIVED);
	openMsg.AddRef("refs", &ref);

	BMessenger tracker("application/x-vnd.Be-TRAK");
	if (tracker.IsValid())
		tracker.SendMessage(&openMsg);
}


void
process_refs(entry_ref dir_ref, BMessage *msg, void *reserved)
{
	// 1. Make sure the folder the user drops app symlinks into exists.
	BPath openWithPath;
	if (get_open_with_folder(openWithPath) != B_OK) {
		BAlert *alert = new BAlert("Error",
			"Could not find or create the \"Open With\" settings folder.",
			"OK");
		alert->Go();
		return;
	}

	// 2. Collect the file(s) the user selected in Tracker; these are
	// what get handed to whichever app is picked below.
	std::vector<entry_ref> selectedRefs;
	entry_ref fileRef;
	for (int32 i = 0; msg->FindRef("refs", i, &fileRef) == B_OK; i++)
		selectedRefs.push_back(fileRef);

	if (selectedRefs.empty()) {
		BAlert *alert = new BAlert("Open With", "No files were selected.",
			"OK");
		alert->Go();
		return;
	}

	// 3. Build a menu item for every app symlinked into the Open With
	// folder.
	BDirectory openWithDir(openWithPath.Path());
	if (openWithDir.InitCheck() != B_OK) {
		BAlert *alert = new BAlert("Error",
			"Could not open the \"Open With\" settings folder.", "OK");
		alert->Go();
		return;
	}

	BPopUpMenu *menu = new BPopUpMenu("Open With", false, false);

	std::vector<entry_ref> appRefs;
	BEntry entry;
	while (openWithDir.GetNextEntry(&entry, false) == B_OK) {
		if (entry.IsDirectory())
			continue;

		char name[B_FILE_NAME_LENGTH];
		if (entry.GetName(name) != B_OK)
			continue;

		entry_ref linkRef;
		if (entry.GetRef(&linkRef) != B_OK)
			continue;

		// Follow the symlink (if it is one) to find the real app to
		// launch, but keep the link's own name as the menu label so the
		// user sees whatever they called it.
		BEntry targetEntry(&linkRef, true);
		if (targetEntry.InitCheck() != B_OK || !targetEntry.Exists())
			continue;

		entry_ref appRef;
		if (targetEntry.GetRef(&appRef) != B_OK)
			continue;

		appRefs.push_back(appRef);
		menu->AddItem(new BMenuItem(name, NULL));
	}

	if (appRefs.empty()) {
		delete menu;

		BAlert *alert = new BAlert("Open With",
			"No apps are configured yet.\n\n"
			"Symlink your favorite apps into the \"Open With\" folder and "
			"they will show up here.",
			"Open Folder", "Cancel");
		if (alert->Go() == 0)
			open_folder_in_tracker(openWithPath);
		return;
	}

	// 4. Show the menu and see which app (if any) got picked.
	BPoint where(100, 100);
	BMenuItem *picked = menu->Go(where, true, true, false);

	int32 index = picked != NULL ? menu->IndexOf(picked) : -1;
	delete menu;

	if (index < 0 || index >= (int32)appRefs.size())
		return;

	entry_ref appRef = appRefs[index];

	// 5. Launch the chosen app with the selected file(s) as command line
	// arguments, the same way a terminal invocation would, so it works
	// for GTK ports and other non-native apps as well as BeOS-native ones.
	BPath appPath(&appRef);
	if (appPath.InitCheck() != B_OK)
		return;

	std::vector<BPath> filePaths;
	for (size_t i = 0; i < selectedRefs.size(); i++) {
		BPath filePath(&selectedRefs[i]);
		if (filePath.InitCheck() == B_OK)
			filePaths.push_back(filePath);
	}

	std::vector<const char *> args;
	args.push_back(appPath.Path());
	for (size_t i = 0; i < filePaths.size(); i++)
		args.push_back(filePaths[i].Path());
	args.push_back(NULL);

	status_t status = be_roster->Launch(&appRef, (int32)args.size() - 1,
		&args[0]);

	if (status != B_OK && status != B_ALREADY_RUNNING) {
		BString errorMsg("Failed to launch app: ");
		errorMsg << strerror(status);
		BAlert *alert = new BAlert("Error", errorMsg.String(), "OK");
		alert->Go();
	}
}

}
