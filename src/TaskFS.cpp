
#include <iostream>
#include <stdio.h>

#include <kernel/fs_index.h>


#include <Resources.h>

#include "Task.h"
#include "TaskFS.h"
#include "TaskColumns.h"
#include "TasksApp.h"


TaskFS::TaskFS()
{
}


TaskFS::~TaskFS()
{
	
}


status_t TaskFS::Init(void)
{
	taskList		= new BObjectList<Task>();
	categoryList	= new BObjectList<Category>();
	return PrepareFirstStart();
}


BObjectList<Task>* TaskFS::GetTasks(void)
{
	taskList->MakeEmpty();
	entry_ref ref;
	while (tasksDir.GetNextRef(&ref) == B_OK)
		taskList->AddItem(FileToTask(ref));
	return taskList;
}

BObjectList<Task>* TaskFS::GetTasks(Category forCategorie)
{
	Task				*tmpTask		= NULL;
	BObjectList<Task>	*taskCategory	= new BObjectList<Task>();

	int32	i;
	GetTasks();
	for (i = 0; i<taskList->CountItems();i++) {
		tmpTask=taskList->ItemAt(i);
		if (tmpTask->GetCategory() == forCategorie)
			taskCategory->AddItem(tmpTask);
	}
	return taskCategory;
}

	
status_t TaskFS::UpdateTasks(BObjectList<Task>*)
{
	
}


status_t TaskFS::UpdateCategories(BObjectList<Category>*)
{
	//check if they are already tehre
}

status_t TaskFS::PrepareFirstStart()
{
	status_t err = B_OK;
	//first create the task directory
	BPath homeDir;
	find_directory(B_USER_DIRECTORY, &homeDir);

	BString tasksDirString = homeDir.Path();
	tasksDirString << "/" << TASK_DIRECTORY;

	tasksDir = BDirectory(tasksDirString.String());
	//if the folder was not found create it
	if (tasksDir.InitCheck() != B_OK) {
		BDirectory homeDirectory(homeDir.Path());
		err = homeDirectory.CreateDirectory(TASK_DIRECTORY,
			&tasksDir);
		if (err != B_OK)
			printf("Failed to create tasks directory (%s)\n",
				strerror(err));
		ssize_t written = tasksDir.WriteAttr("_trk/columns_le", B_RAW_TYPE,
			0, task_columns, sizeof(task_columns));
		if (written < 0)
			printf("Failed to write column info (%s)\n", strerror(written));
	}
	
	//set the MimeType
	BMimeType mime(TASK_MIMETYPE);
	//later do better check
	bool valid = mime.IsInstalled();
	if (!valid) {
		mime.Install();
		mime.SetShortDescription(B_TRANSLATE_CONTEXT("Tasks",
			"Short mimetype description"));
		mime.SetLongDescription(B_TRANSLATE_CONTEXT("Tasks",
			"Long mimetype description"));
		//get the icon from our Ressources
		BResources* res = BApplication::AppResources();
		if (res != NULL){
			size_t size;
			const void* data = res->LoadResource(B_VECTOR_ICON_TYPE, 2, &size);
			if (data!=NULL)
				mime.SetIcon((const uint8 *)data, sizeof(data));
		}
		mime.SetPreferredApp(APP_SIG);

		// add default task fields to meta-mime type
		BMessage fields;
		for (int32 i = 0; sDefaultAttributes[i].attribute; i++) {
			fields.AddString("attr:public_name", sDefaultAttributes[i].name);
			fields.AddString("attr:name", sDefaultAttributes[i].attribute);
			fields.AddInt32("attr:type", sDefaultAttributes[i].type);
			fields.AddBool("attr:viewable", sDefaultAttributes[i].isPublic);
			fields.AddBool("attr:editable", sDefaultAttributes[i].editable);
			fields.AddInt32("attr:width", sDefaultAttributes[i].width);
			fields.AddInt32("attr:alignment", B_ALIGN_LEFT);
			fields.AddBool("attr:extra", false);
		}
		mime.SetAttrInfo(&fields);
	}

	// create indices on all volumes for the found attributes.
	int32 count = 8;
	BVolumeRoster volumeRoster;
	BVolume volume;
	while (volumeRoster.GetNextVolume(&volume) == B_OK) {
		for (int32 i = 0; i < count; i++) {
			if (sDefaultAttributes[i].isPublic == true)
				fs_create_index(volume.Device(), sDefaultAttributes[i].attribute,
					sDefaultAttributes[i].type, 0);
		}
	}
	return err;
}


status_t TaskFS::TaskToFile(Task *theTask, bool overwrite)
{
	BFile		taskFile;
	BEntry		entry;
	status_t	err;
	
	bool	completed	= theTask->IsCompleted();
	uint32	priority	= theTask->Priority();
	time_t	due			= theTask->DueTime();
	int32	id			= theTask->ID();
	
	//first check if the File already exists..
	//if not and overwrite is on check the ids..
	// and search for the correspondending file...
	if (tasksDir.FindEntry(theTask->Title(),&entry) == B_OK) {
		taskFile.SetTo((const BEntry*)&entry,B_READ_WRITE);
		err = B_OK;
	} 
	else {
		entry_ref *ref= FileForId(theTask);
		if (ref==NULL){
			tasksDir.CreateFile(theTask->Title(),&taskFile,overwrite);
			tasksDir.FindEntry(theTask->Title(),&entry);
		}
		else {
			entry.SetTo(ref);
			taskFile.SetTo((const BEntry*)ref,B_READ_WRITE);
		}
	}
	taskFile.WriteAttr("META:completed",B_BOOL_TYPE, 0, &completed, sizeof(completed));
	entry.Rename(theTask->Title());
	taskFile.WriteAttrString("META:category",new BString(theTask->GetCategory().Name()));
	taskFile.WriteAttrString("META:notes",new BString(theTask->Notes()));
	taskFile.WriteAttr("META:priority", B_UINT32_TYPE, 0, &priority, sizeof(priority));
	taskFile.WriteAttr("META:due", B_TIME_TYPE, 0, &due, sizeof(due));
	taskFile.WriteAttr("META:task_id", B_INT32_TYPE, 0, &id, sizeof(id));
	taskFile.WriteAttrString("META:task_url",new BString(theTask->URL()));
}


Task* TaskFS::FileToTask(entry_ref theEntryRef)
{
	Task*	newTask		=new Task();
	//needed for the "attribute stuff
	BFile	theFile(&theEntryRef,B_READ_ONLY);
	//needed to get out the name
	BEntry	theEntry(&theEntryRef);
	
	bool	completed;
	char	name[B_FILE_NAME_LENGTH];
	BString	category;
	BString	notes;
	uint32	priority;
	time_t	due;
	int32	id;
	BString	url;	
	
	//maby do a check if everything went ok and only if so set the values
	theFile.ReadAttr("META:completed",B_BOOL_TYPE, 0, &completed, sizeof(completed));
	theEntry.GetName(name);
	theFile.ReadAttrString("META:category",&category);
	theFile.ReadAttrString("META:notes",&notes);
	theFile.ReadAttr("META:priority", B_UINT32_TYPE, 0, &priority, sizeof(priority));
	theFile.ReadAttr("META:due", B_TIME_TYPE, 0, &due, sizeof(due));
	theFile.ReadAttr("META:task_id", B_INT32_TYPE, 0, &id, sizeof(id));
	theFile.ReadAttrString("META:task_url",&url);
	
	newTask->Complete(completed);
	newTask->SetTitle(BString(name));
	newTask->SetCategory(Category(category));
	newTask->SetNotes(notes);
	newTask->SetPriority(priority);
	newTask->SetDueTime(due);
	newTask->SetID(id);
	newTask->SetURL(url);
	return newTask;
}

entry_ref* TaskFS::FileForId(Task *theTask)
{
	//scan through all files for the ID
	int32		fileID;
	entry_ref	*ref;
	BNode		theNode;
	
	while (tasksDir.GetNextRef(ref) == B_OK){
		theNode.SetTo(ref);
		if (theNode.ReadAttr("META:task_id", B_INT32_TYPE, 0, &fileID, sizeof(fileID)) == B_OK)
			if (fileID==theTask->ID() && fileID != 0)
				return ref;
	}
	return NULL;
}