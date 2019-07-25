/* Copyright (c) 2017-2019 Hans-Kristian Arntzen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "vulkan_common.hpp"
#include "context.hpp"
#include "cookie.hpp"
#include "object_pool.hpp"

namespace Vulkan
{
class Device;

class SemaphoreHolder;
struct SemaphoreHolderDeleter
{
	void operator()(SemaphoreHolder *semaphore);
};

class SemaphoreHolder : public Util::IntrusivePtrEnabled<SemaphoreHolder, SemaphoreHolderDeleter, HandleCounter>,
                        public InternalSyncEnabled
{
public:
	friend struct SemaphoreHolderDeleter;

	~SemaphoreHolder();

	const VkSemaphore &get_semaphore() const
	{
		return semaphore;
	}

	bool is_signalled() const
	{
		return signalled;
	}

	VkSemaphore consume()
	{
		auto ret = semaphore;
		VK_ASSERT(semaphore);
		VK_ASSERT(signalled);
		semaphore = VK_NULL_HANDLE;
		signalled = false;
		return ret;
	}

	VkSemaphore release_semaphore()
	{
		auto ret = semaphore;
		semaphore = VK_NULL_HANDLE;
		signalled = false;
		return ret;
	}

	bool can_recycle() const
	{
		return !should_destroy_on_consume;
	}

	void signal_external()
	{
		VK_ASSERT(!signalled);
		VK_ASSERT(semaphore);
		signalled = true;
	}

	void destroy_on_consume()
	{
		should_destroy_on_consume = true;
	}

	void signal_pending_wait()
	{
		pending = true;
	}

	bool is_pending_wait() const
	{
		return pending;
	}

private:
	friend class Util::ObjectPool<SemaphoreHolder>;
	SemaphoreHolder(Device *device_, VkSemaphore semaphore_, bool signalled_)
	    : device(device_)
	    , semaphore(semaphore_)
	    , signalled(signalled_)
	{
	}

	Device *device;
	VkSemaphore semaphore;
	bool signalled = true;
	bool pending = false;
	bool should_destroy_on_consume = false;
};

using Semaphore = Util::IntrusivePtr<SemaphoreHolder>;
}
