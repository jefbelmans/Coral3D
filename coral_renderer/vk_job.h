#pragma once

// STD
#include <condition_variable>
#include <iostream>

#include "vulkan/vulkan.h"

enum class JobStatus : uint8_t
{
	PENDING,
	IN_PROGRESS,
	COMPLETE
};

class vk_job
{
public:
	vk_job* pNext{ nullptr };
	virtual void execute(VkCommandBuffer command_buffer) = 0;
};

class vk_work_queue final
{
public:
	vk_job* pFirst_{ nullptr };
	vk_job* pLast_{ nullptr };
	std::mutex mutex_{};
	std::mutex log_mutex_{};

	std::condition_variable job_flag_;
	std::condition_variable job_done_flag_;

	void push(vk_job* pJob)
	{
		if (!pFirst_)
			pFirst_ = pJob;
		else
			pLast_->pNext = pJob;

		pLast_ = pJob;
	}

	vk_job* pop()
	{
		vk_job* pCurrent = pFirst_;
		if (pCurrent)
		{
			pFirst_ = pCurrent->pNext;
			if(!pFirst_)
				pLast_ = nullptr;
		}

		return pCurrent;
	}

	bool done()
	{
		return !pFirst_;
	}
};

class worker_thread final
{
public:
	bool& done_;
	vk_work_queue& work_queue_;
	unsigned int id_;

	VkCommandBuffer command_buffer_{ VK_NULL_HANDLE };

	worker_thread(bool& done, vk_work_queue& work_queue, unsigned int id, VkCommandBuffer command_buffer)
		: done_(done), work_queue_(work_queue), id_(id), command_buffer_{ command_buffer } {}

	void operator()()
	{
		work_queue_.log_mutex_.lock();
		std::cout << "Worker thread " << id_ << " started" << std::endl;
		work_queue_.log_mutex_.unlock();
		
		while (!done_)
		{
			// Wait until there is a job or until time-out
			std::unique_lock<std::mutex> lock(work_queue_.mutex_);
			bool result = work_queue_.job_flag_.wait_until(
				lock,
				std::chrono::system_clock::now() + std::chrono::milliseconds(200),
				[&] { return !work_queue_.done(); }
			);

			if (result)
			{
				vk_job* pending_job = work_queue_.pop();
				if (!pending_job) continue;

				// We've got our job, we can unlock the mutex
				lock.unlock();

				// Execute the job
				pending_job->execute(command_buffer_);
				work_queue_.log_mutex_.lock();
				std::cout << "Job finished by thread " << id_ << std::endl;
				work_queue_.log_mutex_.unlock();
				work_queue_.job_done_flag_.notify_one();
			}
		}

		work_queue_.log_mutex_.lock();
		std::cout << "Worker thread " << id_ << " stopped" << std::endl;
		work_queue_.log_mutex_.unlock();
	}
};