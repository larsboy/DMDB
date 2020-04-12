/*!
* \file TaskProgress.h
* \author Lars Thomas Boye 2020
*
* Classes for keeping track of the status/progress during a process.
* We use them to track progress and errors when processing wads.
*/

#ifndef TASKPROGRESS_H
#define TASKPROGRESS_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <vector>
#include "../gui/GuiProgress.h"

/*! How many times to update a progress indicater in UI. */
const int MAX_DIALOG_STEPS = 100;

/*!
* TaskProgress is used to keep track of a process which may take some time
* and/or potentially fail. The task can be named, for the benefit of the
* user if the task is presented in the user interface. If the process can
* take significant time and consists of multiple operations, use the
* progress counter so that a progress bar can be displayed: startCount,
* incrCount and completeCount. If the process encounters an error which
* prevents the process from completing, call fatalError, setting the task
* in a failed state. Non-fatal errors can be reported with warnError.
*
* A process can be broken up into sub-tasks, with child TaskProgress objects
* created for sub-tasks, with reference to the parent TaskProgress. The
* intended usage of the class is as an argument for a method running a
* process which should be tracked. The caller creates a TaskProgress with
* the wanted name, as passes it to the method. The method calls the Count
* methods if it can track progress numerically, and the Error methods for
* errors. Upon completion, the caller can check hasFailed. If the process
* call another process method, it can create a TaskProgress for the sub-task
* to pass to this method, or use the original task object if this should
* not count as a sub-task. The sub TaskProgress calls its parent when it
* is created, and it passes any error messages on to its parent when it is
* destroyed. The lifetime of the object should match the process, so there
* should never be more than one direct child at a time (which itself can
* have a child). If the sub-task has failed, the failed state can be passed
* on to the parent, otherwise it is just logged as a warning. The parent
* is also called when a child task changes its progress counter, and if
* the parent doesn't have a count of its own, its progress counter will
* be the same as the child.
*/
class TaskProgress
{
	public:
		/*!
		* The task is given a name, to identify it in the user interface
		* (can be empty string, to not show). Unless it is the top-level
		* task, a pointer to the parent task should also be given.
		*/
		TaskProgress(wxString nam, TaskProgress* parnt);

		/*!
		* The TaskProgress must be destroyed when the task is done. Any
		* parent will be informed.
		*/
		virtual ~TaskProgress();

		/*!
		* Start tracking progress of this task with a numerical value.
		* It starts at 0 and goes up to the given target. Call incrCount
		* to increment the progress count.
		*/
		virtual void startCount(int target);

		/*!
		* Increment the progress counter by delta.
		*/
		virtual void incrCount(int delta=1);

		/*!
		* Complete the progress counter, setting it to its target
		* value. After this the task no longer has a progress count.
		*/
		virtual void completeCount();

		/*!
		* Returns the current value of progress counter.
		*/
		int getCurrentCount() { return count; }

		/*!
		* Signals a non-fatal error with the current task, to be logged.
		*/
		void warnError(wxString errorMsg);

		/*!
		* Signals failure of the task, with an error message. hasFailed will
		* from now on return true, and the task should not continue.
		*/
		void fatalError(wxString errorMsg);

		/*!
		* true once fatalError has been called. All task code should check this,
		* and terminate as soon as possible if it is true, as it means the
		* task can't be completed. If affectParent is true and the task has
		* failed, the fatal error will be transferred to any parent task, so
		* it will be in the failed state.
		*/
		bool hasFailed(bool affectParent=false);

		/*!
		* Called by a child task to inform its parent that it has a sub-task.
		*/
		virtual void childStarted(wxString cname);

		/*!
		* Called by a child task to inform its parent of a change in the
		* progress counter. If the task doesn't do its own progress count,
		* its count is set to match the child.
		*/
		virtual void childCount(int current, int target);

		/*!
		* Called by a child task, when the child is destroyed. The error
		* status is passed on to the parent, with failed state and error
		* text. Notw that this won't set the task to failed.
		*/
		virtual void childDone(wxString cname, bool cfailed, wxString cerror);

		/*!
		* Returns the current error text. If the task failed, this is the
		* error message for the failure, otherwise it is a concatenation of
		* any warning messages.
		*/
		wxString getError() { return error; }

	protected:
		wxString name; // Name of task, to identify it in the user interface
		bool ownCount; //true when counting progress of this task
		int total; //Total units of progress to complete task
		int count; //Current units of progress, going from 0 to total
		bool failed; //Failed state
		wxString error;
		TaskProgress* parent;
};

/*!
* WadProgress is a subclass of TaskProgress, for representing the top object
* for tracking a process where we might want to display the information in
* a user interface. It can be connected to a GuiProgress dialog, and will
* show its progress count there as a progress bar along with the namne of
* the currently active subtask.
*/
class WadProgress :  public TaskProgress
{
	public:
		WadProgress(wxString nam);
		virtual ~WadProgress();

		/*!
		* Set a user interface dialog to show the TaskProgress state to
		* the user. It will be updated to reflect changes in state.
		* Set to NULL to remove the reference.
		*/
		void setDialog(GuiProgress* gp);

		virtual void startCount(int target);

		virtual void incrCount(int delta=1);

		virtual void completeCount();

		virtual void childStarted(wxString cname);

		virtual void childCount(int current, int target);

		virtual void childDone(wxString cname, bool cfailed, wxString cerror);

	private:
		wxString currentTask; //Name of currently active child
		vector<wxString> taskLog; //Completed child tasks
		int unitsPerStep; //How often to update dialog
		GuiProgress* dialog;
};

#endif // TASKPROGRESS_H
