//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
// Special Tasks which require more arguments than possible
// with STL functions...
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////

#ifndef __OTSERV_TASKS_H__
#define __OTSERV_TASKS_H__

#include "definitions.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>

const int DISPATCHER_TASK_EXPIRATION = 2000;

class Task
{
public:
	// DO NOT allocate this class on the stack
	Task(const uint32_t& ms, const boost::function<void (void)>& f);
	Task(const boost::function<void (void)>& f);
	virtual ~Task();

	void operator()();

	void setDontExpire();
	bool hasExpired() const;

protected:
	// Expiration has another meaning for scheduler tasks,
	// then it is the time the task should be added to the
	// dispatcher
	boost::system_time m_expiration;
	boost::function<void (void)> m_f;
};

inline Task* createTask(boost::function<void (void)> f)
{
	return new Task(f);
}

inline Task* createTask(uint32_t expiration, boost::function<void (void)> f)
{
	return new Task(expiration, f);
}

enum DispatcherState
{
	STATE_RUNNING,
	STATE_CLOSING,
	STATE_TERMINATED
};

class Dispatcher
{
public:
	Dispatcher();

	void addTask(Task* task, bool push_front = false);

	void start();
	void stop();
	void shutdown();

	enum DispatcherState
	{
		STATE_RUNNING,
		STATE_CLOSING,
		STATE_TERMINATED
	};

protected:

	static void dispatcherThread(void* p);

	void flush();

	boost::mutex m_taskLock;
	boost::condition_variable m_taskSignal;

	std::list<Task*> m_taskList;
	DispatcherState m_threadState;
};

extern Dispatcher g_dispatcher;

#endif
