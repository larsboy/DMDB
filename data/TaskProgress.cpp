#include "TaskProgress.h"

//**************************************************************
//************************ TaskProgress ************************
//**************************************************************

TaskProgress::TaskProgress(wxString nam, TaskProgress* parnt)
: name(nam), ownCount(false), total(0), count(0), failed(false), error(""), parent(parnt)
{
	if (parent != NULL)
		parent->childStarted(nam);
}

TaskProgress::~TaskProgress()
{
	if (parent != NULL)
		parent->childDone(name, failed, error);
}

void TaskProgress::startCount(int target)
{
	total = target;
	count = 0;
	ownCount = true;
	if (parent != NULL)
		parent->childCount(count, total);
}

void TaskProgress::incrCount(int delta)
{
	count += delta;
	if (count>total) count=total;
	if (parent != NULL)
		parent->childCount(count, total);
}

void TaskProgress::completeCount()
{
	count = total;
	if (parent != NULL)
		parent->childCount(count, total);
	ownCount = false;
}

void TaskProgress::warnError(wxString errorMsg)
{
	if (error.Length()==0) error = errorMsg;
	else error = error + ", " + errorMsg;
}

void TaskProgress::fatalError(wxString errorMsg)
{
	error = errorMsg;
	failed = true;
}

bool TaskProgress::hasFailed(bool affectParent)
{
	if (affectParent && failed) {
		parent->fatalError(error);
		error = "";
	}
	return failed;
}

void TaskProgress::childStarted(wxString cname)
{
	if (parent!=NULL)
		parent->childStarted(cname);
}

void TaskProgress::childCount(int current, int target)
{
	if (!ownCount) {
		count = current;
		total = target;
		if (parent != NULL)
			parent->childCount(count, total);
	}
}

void TaskProgress::childDone(wxString cname, bool failed, wxString error)
{
	//TODO: Add error?
	if (error.Length() > 0)
		warnError(error);
}

//*************************************************************
//************************ WadProgress ************************
//*************************************************************

WadProgress::WadProgress(wxString nam)
: TaskProgress(nam, NULL), currentTask(nam), unitsPerStep(0), dialog(NULL)
{
}

WadProgress::~WadProgress()
{
}

void WadProgress::setDialog(GuiProgress* gp)
{
	dialog = gp;
	if (dialog != NULL) {
		dialog->setLabel(currentTask);
		dialog->setRange(total, count);
	}
}

void WadProgress::startCount(int target)
{
	TaskProgress::startCount(target);
	if (target<=MAX_DIALOG_STEPS)
		unitsPerStep = 1;
	else
		unitsPerStep = target/MAX_DIALOG_STEPS;
	if (dialog != NULL)
		dialog->setRange(total, count);
}

void WadProgress::incrCount(int delta)
{
	int oldSteps = count/unitsPerStep;
	count += delta;
	if (count>total) count=total;
	if (dialog != NULL) {
		if ((count/unitsPerStep) > oldSteps)
			dialog->setProgress(count);
	}
}

void WadProgress::completeCount()
{
	TaskProgress::completeCount();
	if (dialog != NULL)
		dialog->setProgress(count);
}

void WadProgress::childStarted(wxString cname)
{
	if (cname.Length() > 0) {
		currentTask = cname;
		if (dialog != NULL)
			dialog->setLabel(currentTask);
	}
}

void WadProgress::childCount(int current, int target)
{
	if (!ownCount) {
		if (target<=MAX_DIALOG_STEPS)
			unitsPerStep = 1;
		else
			unitsPerStep = target/MAX_DIALOG_STEPS;
		int oldSteps = count/unitsPerStep;
		count = current;
		total = target;
		if (dialog != NULL) {
			if ((count==0) || (count==total))
				dialog->setRange(total, count);
			else if ((count/unitsPerStep) > oldSteps)
				dialog->setRange(total, count);
		}

	}
}

void WadProgress::childDone(wxString cname, bool cfailed, wxString cerror)
{
	//Add line to log
	wxString line;
	if (cfailed) {
		line = cname + " [FAILED: " + cerror + "]";
	} else if (cerror.Length() > 0) {
		line = cname + " [DONE: " + cerror + "]";
	} else {
		line = cname + " [DONE]";
	}
	taskLog.push_back(line);
	if (dialog != NULL)
		dialog->logLine(line);
}
