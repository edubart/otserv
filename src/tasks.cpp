//////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
//////////////////////////////////////////////////////////////////////
//
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
#include "otpch.h"

#include "exception.h"
#include "tasks.h"
#include "outputmessage.h"
#include "game.h"

extern Game g_game;

// DO NOT allocate this class on the stack
Task::Task(const uint32_t& ms, const boost::function<void (void)>& f)
	: m_f(f)
{
	m_expiration = boost::get_system_time() + boost::posix_time::milliseconds(ms);
}

Task::Task(const boost::function<void (void)>& f)
	: m_expiration(boost::date_time::not_a_date_time)
	, m_f(f)
{}

Task::~Task()
{
	// Virtual Destructor
}

void Task::operator()()
{
	m_f();
}

void Task::setDontExpire()
{
	m_expiration = boost::date_time::not_a_date_time;
}

bool Task::hasExpired() const
{
	if (m_expiration == boost::date_time::not_a_date_time)
	{
		return false;
	}

	return m_expiration < boost::get_system_time();
}

Dispatcher::Dispatcher()
{
	m_taskList.clear();
	m_threadState = STATE_TERMINATED;
}

void Dispatcher::start()
{
	m_threadState = STATE_RUNNING;
	boost::thread(boost::bind(&Dispatcher::dispatcherThread, (void*)this));
}

void Dispatcher::dispatcherThread(void* p)
{
	Dispatcher* dispatcher = static_cast<Dispatcher*>(p);
	ExceptionHandler dispatcherExceptionHandler;
	dispatcherExceptionHandler.InstallHandler();
#ifdef __DEBUG_SCHEDULER__
	std::cout << "Starting Dispatcher" << std::endl;
#endif
	OutputMessagePool* outputPool;
	// NOTE: second argument defer_lock is to prevent from immediate locking
	boost::unique_lock<boost::mutex> taskLockUnique(dispatcher->m_taskLock, boost::defer_lock);

	while (dispatcher->m_threadState != STATE_TERMINATED)
	{
		Task* task = NULL;
		// check if there are tasks waiting
		taskLockUnique.lock();

		if (dispatcher->m_taskList.empty())
		{
			//if the list is empty wait for signal
#ifdef __DEBUG_SCHEDULER__
			std::cout << "Dispatcher: Waiting for task" << std::endl;
#endif
			dispatcher->m_taskSignal.wait(taskLockUnique);
		}

#ifdef __DEBUG_SCHEDULER__
		std::cout << "Dispatcher: Signalled" << std::endl;
#endif

		if (!dispatcher->m_taskList.empty() && (dispatcher->m_threadState != STATE_TERMINATED))
		{
			// take the first task
			task = dispatcher->m_taskList.front();
			dispatcher->m_taskList.pop_front();
		}

		taskLockUnique.unlock();

		// finally execute the task...
		if (task)
		{
			if (!task->hasExpired())
			{
				OutputMessagePool::getInstance()->startExecutionFrame();
				(*task)();
				outputPool = OutputMessagePool::getInstance();

				if (outputPool)
				{
					outputPool->sendAll();
				}

				g_game.clearSpectatorCache();
			}

			delete task;
#ifdef __DEBUG_SCHEDULER__
			std::cout << "Dispatcher: Executing task" << std::endl;
#endif
		}
	}

	dispatcherExceptionHandler.RemoveHandler();
}

void Dispatcher::addTask(Task* task, bool push_front /*= false*/)
{
	bool do_signal = false;
	m_taskLock.lock();

	if (m_threadState == STATE_RUNNING)
	{
		do_signal = m_taskList.empty();

		if (push_front)
		{
			m_taskList.push_front(task);
		}
		else
		{
			m_taskList.push_back(task);
		}

#ifdef __DEBUG_SCHEDULER__
		std::cout << "Dispatcher: Added task" << std::endl;
#endif
	}

#ifdef __DEBUG_SCHEDULER__
	else
	{
		std::cout << "Error: [Dispatcher::addTask] Dispatcher thread is terminated." << std::endl;
	}

#endif
	m_taskLock.unlock();

	// send a signal if the list was empty
	if (do_signal)
	{
		m_taskSignal.notify_one();
	}
}

void Dispatcher::flush()
{
	Task* task = NULL;

	while (!m_taskList.empty())
	{
		task = m_taskList.front();
		m_taskList.pop_front();
		(*task)();
		delete task;
		OutputMessagePool* outputPool = OutputMessagePool::getInstance();

		if (outputPool)
		{
			outputPool->sendAll();
		}

		g_game.clearSpectatorCache();
	}

#ifdef __DEBUG_SCHEDULER__
	std::cout << "Flushing Dispatcher" << std::endl;
#endif
}

void Dispatcher::stop()
{
	m_taskLock.lock();
	m_threadState = STATE_CLOSING;
	m_taskLock.unlock();
#ifdef __DEBUG_SCHEDULER__
	std::cout << "Stopping Dispatcher" << std::endl;
#endif
}

void Dispatcher::shutdown()
{
	m_taskLock.lock();
	m_threadState = STATE_TERMINATED;
	flush();
	m_taskLock.unlock();
#ifdef __DEBUG_SCHEDULER__
	std::cout << "Shutdown Dispatcher" << std::endl;
#endif
}
