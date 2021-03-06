#ifndef TASK_APP_H
#define TASK_APP_H

#include <Application.h>
#include <Locale.h>
#include <Catalog.h>
#include <Debug.h>

#include "Task.h"
#include "TaskFS.h"
#include "TaskGoogle.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TaskApp"


static const char* tasksKeyring = "Tasks";


class TaskApp : public BApplication {
public:
						TaskApp();
						~TaskApp();

	/*creates a Settings File and a Settings Folder
	  */
		status_t		FirstStart(void);
	
	/*called on StartUp scan through all files
	  if google sync is on download the latest list and
	  sync it with the google list if google sync is on 
	  start the Sync Loop wich monitors all nodes...
	*/
		status_t		StartUp(void);
	
		void			MessageReceived(BMessage *message);
		void			ReadyToRun();

		void			BroadCast(BMessage *message);
	
static	int32			EventLoop(void *data);

private:
		Task*			MergeTasks(Task* firstTask, Task *secondTask);	
		TaskFS			*fsSync;
		TaskGoogle		*gSync;
		sem_id			fEventLock;
		BList			fEvents;
		BList			fAllEvents;
		sem_id			fNotify;
		thread_id		fEventLoop;
		bool			fQuiting;
		
		
};

#endif
