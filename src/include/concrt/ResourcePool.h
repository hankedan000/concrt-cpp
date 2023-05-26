#pragma once

#include <ck_ring.h>
#include <limits.h>
#include <semaphore.h>
#include <stdexcept>
#include <time.h>

namespace concrt
{
	static const unsigned int UNLIMITED_TRIES = UINT_MAX;
	static const unsigned long WAIT_FOREVER = ULONG_MAX;

	static const int OK = 0;
	static const int ERR_INTERNAL = -1;
	static const int ERR_TIMEOUT = -2;

	// types of produce/consumer models
	namespace PC_Model
	{
		// single producer; single consumer
		static const unsigned int SPSC = 0;

		// multiple producers; single consumer
		static const unsigned int MPSC = 1;
		
		// single producer; multiple consumers
		static const unsigned int SPMC = 2;
		
		// multiple producers; multiple consumers
		static const unsigned int MPMC = 3;
	}

	template <typename USER_T, unsigned int PC_MODEL>
	class ResourcePool
	{
	public:
		/**
		 * Constructs a pool of (size - 1) resources.
		 * 
		 * @param[in] size
		 * a power of two greater than or equal to 4.
		 * 
		 * Note: the actual number of acquirable resources will be (size - 1).
		 */
		ResourcePool(
			unsigned int size)
		: size_(size)
		, free_ring_(nullptr)
		, free_buffer_(nullptr)
		, ready_ring_(nullptr)
		, ready_buffer_(nullptr)
		, data_block_(nullptr)
		{
			if ( ! (size >= 4 && (size & (size - 1)) == 0))
			{
				throw std::runtime_error("size must be a power of two and >= 4");
			}

			free_ring_ = new ck_ring_t;
			free_buffer_ = new ck_ring_buffer_t[size];
			ck_ring_init(free_ring_,size);

			ready_ring_ = new ck_ring_t;
			ready_buffer_ = new ck_ring_buffer_t[size];
			ck_ring_init(ready_ring_,size);

			if (sem_init(&ready_sem_, 0, 0) != 0)
			{
				throw std::runtime_error("failed to init ready semaphore");
			}

			data_block_ = new USER_T[size - 1];

			switch (PC_MODEL)
			{
				case PC_Model::SPSC:
					enqueue_func_ = &ck_ring_enqueue_spsc;
					dequeue_func_ = &ck_ring_dequeue_spsc;
					break;
				case PC_Model::MPSC:
					enqueue_func_ = &ck_ring_enqueue_mpsc;
					dequeue_func_ = &ck_ring_dequeue_mpsc;
					break;
				case PC_Model::SPMC:
					enqueue_func_ = &ck_ring_enqueue_spmc;
					dequeue_func_ = &ck_ring_dequeue_spmc;
					break;
				case PC_Model::MPMC:
					enqueue_func_ = &ck_ring_enqueue_mpmc;
					dequeue_func_ = &ck_ring_dequeue_mpmc;
					break;
				default:
					throw std::runtime_error("unsupported PC_MODEL (" + std::to_string(PC_MODEL) + ")");
					break;
			}

			for (unsigned int i=0; i<(size - 1); i++)
			{
				if ( ! enqueue_func_(free_ring_,free_buffer_,data_block_ + i)) {
					throw std::runtime_error("failed to enqueue " + std::to_string(i) + "th free resource");
				}
			}
		}

		~ResourcePool()
		{
			if (available() == capacity())
			{
				// all resources are free, so we're safe to delete memory
				delete free_ring_;
				delete[] free_buffer_;
				delete ready_ring_;
				delete[] ready_buffer_;
				delete[] data_block_;
			}
			else
			{
				// there are still acquired/consumed resources out there; application
				// shutdown is flawed. leaking memory is more favorable than crashing.
			}
		}

		/**
		 * Acquire a resource if one is available; otherwise, busily
		 * retry up to 'max_tries' before returning.
		 * 
		 * @param[out] res
		 * returned pointer to an acquired resource. value of returned pointer
		 * should not be assumed if return code is not concrt::OK
		 * 
		 * @param[in] max_tries
		 * the maximum number of times to try to dequeue a free list
		 * 
		 * @return
		 * concrt::OK - on success
		 * concrt::ERR_TIMEOUT - if max_tries was reached
		 */
		int
		acquire_busy(
			USER_T *&res,
			unsigned int max_tries = UINT_MAX)
		{
			int rc = OK;
			unsigned int dec_val = (max_tries == UNLIMITED_TRIES ? 0 : 1);
			while (max_tries > 0 && dequeue_func_(free_ring_,free_buffer_,&res) == false)
			{
				max_tries -= dec_val;
				if (max_tries == 0)
				{
					rc = ERR_TIMEOUT;
				}
			}
			return rc;
		}

		/**
		 * Mark a resource as ready for consumption
		 * 
		 * @param[in]
		 * Pointer to a previously acquired resource
		 * 
		 * @return
		 * concrt::OK - on success
		 * concrt::ERR_INTERNAL - if internal error occured
		 */
		int
		produce(
			USER_T *res)
		{
			bool okay = enqueue_func_(ready_ring_,ready_buffer_,res);
			if (okay)
			{
				sem_post(&ready_sem_);
			}
			return okay ? OK : ERR_INTERNAL;
		}

		/**
		 * Consumer a resource if one is available; otherwise, busily
		 * retry up to 'max_tries' before returning.
		 * 
		 * Note: Concurrent usage of consume_busy() and consume_wait() is not
		 * supported. The caller must exclusively use one method or the other.
		 * 
		 * @param[out] res
		 * returned pointer to a consumed resource. value of returned pointer
		 * should not be assumed if return code is not concrt::OK
		 * 
		 * @param[in] max_tries
		 * the maximum number of times to try to dequeue a readied resource
		 * 
		 * @return
		 * concrt::OK - on success
		 * concrt::ERR_TIMEOUT - if max_tries was reached
		 */
		int
		consume_busy(
			USER_T *&res,
			unsigned int max_tries = UINT_MAX)
		{
			int rc = OK;
			unsigned int dec_val = (max_tries == UNLIMITED_TRIES ? 0 : 1);
			while (max_tries > 0 && dequeue_func_(ready_ring_,ready_buffer_,&res) == false)
			{
				max_tries -= dec_val;
				if (max_tries == 0)
				{
					rc = ERR_TIMEOUT;
				}
			}
			return rc;
		}

		/**
		 * Consumer a resource if one is available; otherwise wait.
		 * The calling thread will be suspended for up to 'timeout'
		 * seconds or until a resource is produced.
		 * 
		 * Note: Concurrent usage of consume_busy() and consume_wait() is not
		 * supported. The caller must exclusively use one method or the other.
		 * 
		 * @param[out] res
		 * returned pointer to a consumed resource. value of returned pointer
		 * should not be assumed if return code is not concrt::OK
		 * 
		 * @param[in] timeout
		 * timeout duration in seconds
		 * 
		 * @return
		 * concrt::OK - on success
		 * concrt::ERR_TIMEOUT - if timeout was reached
		 * concrt::ERR_INTERNAL - if internal error occured
		 */
		int
		consume_wait(
			USER_T *&res,
			unsigned long timeout = ULONG_MAX)
		{
			int rc = OK;
			if (timeout == WAIT_FOREVER)
			{
				if (sem_wait(&ready_sem_) == 0)
				{
					dequeue_func_(ready_ring_,ready_buffer_,&res);
				}
				else
				{
					rc = ERR_INTERNAL;
				}
			}
			else
			{
				struct timespec ts;
				if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
				{
					ts.tv_sec += timeout;
					if (sem_timedwait(&ready_sem_, &ts) == 0)
					{
						dequeue_func_(ready_ring_,ready_buffer_,&res);
					}
					else if (errno == ETIMEDOUT)
					{
						rc = ERR_TIMEOUT;
					}
					else
					{
						rc = ERR_INTERNAL;
					}
				}
				else
				{
					rc = ERR_INTERNAL;
				}
			}
			return rc;
		}

		/**
		 * Mark a resource free (can be acquired again)
		 * 
		 * @param[in]
		 * Pointer to a previously acquired or consumed resource
		 * 
		 * @return
		 * concrt::OK - on success
		 * concrt::ERR_INTERNAL - if internal error occured
		 */
		int
		release(
			USER_T *res)
		{
			return enqueue_func_(free_ring_,free_buffer_,res) ? OK : ERR_INTERNAL;
		}

		/**
		 * @return
		 * The total number of resource this pool manages
		 */
		unsigned int
		capacity() const
		{
			return size_ - 1;
		}

		/**
		 * @return
		 * The number of resources that are currently free
		 */
		unsigned int
		available() const
		{
			return ck_ring_size(free_ring_);
		}

	private:
		unsigned int size_;

		// pointers to enqueu/dequeue functions based on produce/consumer model
		bool (*enqueue_func_)(struct ck_ring */*ring*/, struct ck_ring_buffer */*buffer*/, const void */*entry*/);
		bool (*dequeue_func_)(struct ck_ring */*ring*/, const struct ck_ring_buffer */*buffer*/, void */*data*/);

		// ring of free resources ready to be acquired
		ck_ring_t *free_ring_;
		ck_ring_buffer_t *free_buffer_;

		// ring of produced requeses ready to be consumed
		ck_ring_t *ready_ring_;
		ck_ring_buffer_t *ready_buffer_;

		// semaphore tracking how many resources are ready
		sem_t ready_sem_;

		// block of user data
		USER_T *data_block_;

	};

}