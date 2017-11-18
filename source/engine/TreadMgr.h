#pragma once
#include "cube.h"
#define TASKELEMENT threadmgr::task
#define FOCESTOPTASK if(forcestopped) return !forcestop(); //quick check to see if we need to stop
class tasksorter //maybe come back and make uint mask a template, but this may cause serious problems.
{

public:
	tasksorter(uint);//arg1 is the starting mask.
	void setmask(uint, bool = false, uint = 10000); //changes mask to allow for new opperations, if the 2nd arg is true it will force stop all curent tasks, if task dont come back in arg3 mms then all tasks will be force killed.
	bool cansetmask(uint); //checks to see if any curent tasks are being ran that need to finish before mask can change (if mask matches all curent tasks then returns 0 if there is a conflict this will return an estimate of how many seconds till the opperation will change (based on data gathered)). This will allow you to gauge when to change the mask.
	virtual bool canprocesstask(taskmask *); //given the tasks mask it will determind whether the object should be added to the task pool or to the task box. (This is for assimulating task in the middle of the process frame).
private:
	taskmask *mask;
};

class taskmask //virtual for the task level. The goal is to take any number of variables and quantify them to a simple int level score then find the best option.
{
	friend tasksorter;
	virtual bool operator== (tasksorter *) = 0; //compare 2 tasksorters to see if to operators have the same exact mask
	virtual bool operator|= (tasksorter *) = 0; //compare 2 tasksorters to see if they can be ran on the same cycle
	virtual bool operator<  (tasksorter *) = 0; //compare 2 tasks to see which one has a higher priority (this can use a multi varible approach, Say "time till due" and priority.
	virtual bool operator>  (tasksorter *) = 0; //same as above, keep in mind that we dont care if 2 tasks have the same priority, if this happens we dont care which order they run in.
	virtual void operator++  ()			   = 0;	//way to add priority to the element quickly. This should be used to push opperations to the front of the stack,
	virtual void operator--  ()			   = 0;	//way to remove prioity to the element. This whould push this element to the back of the stack.
	virtual		 operator int()			   = 0;	//convert my values directly to a number for quick calulation. This will make sorting and other opperations much easier, and allow for a quanification for debuging.
};
namespace threadmgr
{
	#pragma region Task system

	class tasker
	{
		friend tasksorter;
	public:
		tasker(uint = 0, uchar = 0); //arg 1 is the starting mask, and arg2 is the number of threads to initalize.
		void init();//if we have a sorter, and workers we will delete all objects and resign them using the restart fuction, if none exist we will just init
		void restart(); //kills all workers tasks and tasks sorters and rebuilds them, this should be only used if there is an error. Uses shut down followed by a init();
		void shutdown(); //deletes all workers tasks, and sorters (including threads), This will be used at program close.
		void getnewtask(); //this is our main loop, this will call when ever a process no longer has a task. This will also assimulate new tasks from the batch and orgainize them properly before being assigned a new task. If no task in task box then tread will sleep till mask is changed.
		void newtask(task *); //gets a new task and adds it to the task list. This task will not be added to the pool till the next task pull by any thread.
	//variables
	private:
		taskmask *mask; //declares the curent state we are in we will just ask what we should do with each element.
		vector<workers *> workers; //curent workers (which contains the threads and tasks curently being worked on)
		vector<task *> taskbatch; //list of all tasks that are un prioritized (aka new tasks) are sorted on every getnewtask(). This batch list should be empty most of the time.
		vector<task *> taskpool; //list of all tasks, that are not being worked on or in the task box. New object that are assimilated will be added to the taskpool unless they are high priority and they match the curent mask.
		vector<task *> taskbox; //curent set of tasks, organized by priority. (This priority is determind per mask change), tasks can be manually forced to the top using forcetask. This will bring it to the top of the list (but this will only run if all mulox arent being used by the process.) 
	};
	class task //virtual class that can be inherited from
	{
	public:
		virtual void init(uint, str); //contructor requires a mask and a name.
		virtual bool canrun();// can this task start (this will look to see if it can get to essential function and variables). this will test various mutex and will also chack other stats to guarentee that the function runs with no interuptions
		virtual bool _starttask() final; //this is the start point for all tasks. This is the header, this code runs automatically for all tasks before the task is ran.
		virtual bool _forcestop() final; //this is a singal passed to this task to force its self to stop. This shoulc be checked during any loop or any opperation that may not returned.
		virtual 
			taskmask *_getmask() final;
	protected:
		virtual bool starttask(); //this is the forced virtaul where the acutal code gets ran from
		virtual bool forcestop(); //user defined code in the case of a force stop to all proper exitiing of code;
	private:
		//varables
		taskmask *mask; //use a refernce for ease of use
		bool forcestoped; //should be checked to make sure that we arent running over our timing or if the tasker wants to stop us; always returns true; so that we know that the opperation was never completed. If this returns false then task will be deleted from the tasker so it can not be revisited
	};

	class workers //holds the processor and is the main worker be and interface between the tasker and task.
	{
	public:
		bool forcestop(); //stops the curent task, this will require the task to be complient.
		bool killtask(); //stops the curent task, and kills the thread, and will reboot the thread and get a new task. This should only be used in extreme cases. Also may lead to memory leaks or other nasty things. This will delete the task, and then kill the thread. Then create a new thread to replace the deleted thread.

	};
	#pragma endregion "contains the task system and sub elements which determind which tasks to fire and when"
#pragma region Mulox mgr

#pragma endregion Contains the mgr in charge of locking and unlocking tasks. This will check the locks and will allow task to run if locks are valid.

}