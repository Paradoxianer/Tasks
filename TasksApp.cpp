#include <Mime.h>

#include "TasksApp.h"

int
main(int argc, const char *argv[])
{
	TaskApp app;
	app.Run();
	return 0;
}


TaskApp::TaskApp()
	:	BApplication(APP_SIG)
{
}


TaskApp::~TaskApp()
{
}


status_t TaskApp::FirstStart()
{
	return B_OK;
}

void TaskApp::MessageReceived(BMessage *message)
{
}


int32 TaskApp::EventLoop(void *data)
{
	TaskApp *app = (TaskApp *)data;
	return 0;
}