#ifndef GOOGLE_CONNECT_H
#define GOOGLE_CONNECT_H

#include <Handler.h>
#include "TaskSync.h"
#include "Task.h"
#include "TaskList.h"


#define CLIENT_ID "318709342848-0h9712v3kbpcv1r7oc8krdrfu22ohlld.apps.googleusercontent.com"
#define CLIENT_SECRET "WyyNzE2JO-HUQqL5RG2VYzz2"
#define REDIRECT_URI "urn:ietf:wg:oauth:2.0:oob"


class TaskGoogle : public TaskSync{
	public:
									TaskGoogle(void);
									~TaskGoogle(void);
		/*
		 * Init calls LoadToken wich trys to load the refrech token
		 * from the KeyManager, if this fails we try force the user to 
		 * google 
		 */
		status_t					Init(void);
		
		// Load Refreshtoken from the Haiku KeyManager
		status_t					LoadToken(void);
		
		/*
		 * Called if we need Request the Acces String from Google
		 * This opens the Login Dialog and opens the Website 
		 * wich should show the AccesString
		 * TODO make it non blockable:
		 * The Dialog Sends a Message with the AccessString
		 * to this App wich is forwarded to This Handler Message Recived
		 */
		char*						RequestAccessString(void);
		
		/* This is called to Request a real AccesToken from Google
		 * if a AccesString is handed over we use it for the the first
		 * Request 
		 * else we try to use the refresh_token wich was hopefully loaded
		 * before. If not jus return an error
		 *
		 */
		status_t					RequestToken(char* accesString);
		
		/*
		 * Login and donwload all TaskLists and Tasks
		 */
		status_t					Login(BString token);
		
		/*
		 * This Methode saves the refresh_token once we recives it
		 */ 
		status_t					SaveToken(void);
		
		/*
		 * This will be called if we recognize the refresh_token dont work
		 * or short bevore we request a new AccesString
		 */
		status_t					RemoveToken(void);
		
		status_t					Load(void);
		status_t					LoadTaskLists(void);
		status_t					LoadTasks(TaskList *cat);
		

		/* =========== Handel all Tasks related operations ========*/
		virtual	BObjectList<Task>*		GetTasks(){return taskList;}
		virtual	BObjectList<Task>*		GetTasks(TaskList ctgr);
	
		virtual	Task*					GetTask(BString id);
	
		virtual status_t				AddTask(Task *tsk);
	
		virtual status_t				UpdateTask(BString id,Task *tsk);
	
		virtual status_t				RemoveTask(BString id);


		/* ========= Handle all TaskList related operations ======*/

		virtual	BObjectList<TaskList>*	GetTaskLists(void){return tLists;};
		virtual	TaskList*				GetTaskList(BString id);

		virtual status_t				AddTaskList(TaskList *ctgr);
	
		virtual status_t				UpdateTaskList(BString id,TaskList *ctgr);

		//what does google do if you call a delete on a TaskList?
		virtual status_t				RemoveTaskList(BString id);	
	

private:
	
		time_t							RFC3339ToTime(const char* timeString);
		
		char*							TimeToRFC3339(time_t timeT);
		TaskList*						TaskListBuyID(const char* forID);
		
		BString							token;
		BString							refreshToken;
		BObjectList<Task>*				taskList;
		BObjectList<TaskList>*			tLists;
		
};


#endif
