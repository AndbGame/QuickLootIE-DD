#pragma once

#include <thread>

namespace QuickLootDD
{
	class SpinLock
	{
	public:
		enum
		{
			kFastSpinThreshold = 10000,
		};

		SpinLock() :
			_owningThread(0), _lockCount(0){};

		void spinLock(std::uint32_t a_pauseAttempts = 0) noexcept
		{
			std::uint32_t myThreadID = std::this_thread::get_id()._Get_underlying_id();
			_mm_lfence();
			if (_owningThread == myThreadID) {
				::_InterlockedIncrement(&_lockCount);
			} else {
				std::uint32_t attempts = 0;
#if DEBUG_SPINLOCK == 1
				std::uint32_t counter = 0;
#endif
				if (::_InterlockedCompareExchange(&_lockCount, 1, 0)) {
					do {
						++attempts;
#if DEBUG_SPINLOCK == 1
						++counter;
						if (counter >= 100) {
							SKSE::log::error("SpinLock {} attempts!", counter);
							counter = 0;
						}
#endif
						_mm_pause();
						if (attempts >= a_pauseAttempts) {
							std::uint32_t spinCount = 0;
							while (::_InterlockedCompareExchange(&_lockCount, 1, 0)) {
#if DEBUG_SPINLOCK == 1
								++counter;
								if (counter >= 100) {
									SKSE::log::error("SpinLock {} attempts!", counter);
									counter = 0;
								}
#endif
								std::this_thread::sleep_for(++spinCount < kFastSpinThreshold ? 0ms : 1ms);
							}
							break;
						}

					} while (::_InterlockedCompareExchange(&_lockCount, 1, 0));
					_mm_lfence();
				}

				_owningThread = myThreadID;
				_mm_sfence();
			}
		}

		bool trySpinLock() noexcept
		{
			std::uint32_t myThreadID = std::this_thread::get_id()._Get_underlying_id();
			_mm_lfence();
			if (_owningThread == myThreadID) {
				::_InterlockedIncrement(&_lockCount);
				return true;
			} else {
				if (::_InterlockedCompareExchange(&_lockCount, 1, 0)) {
					_mm_lfence();
					return false;
				}
				_owningThread = myThreadID;
				_mm_sfence();
				return true;
			}
		}

		void spinUnlock() noexcept
		{
			std::uint32_t myThreadID = std::this_thread::get_id()._Get_underlying_id();

			_mm_lfence();
			if (_owningThread == myThreadID) {
				if (_lockCount == 1) {
					_owningThread = 0;
					_mm_mfence();
					::_InterlockedCompareExchange(&_lockCount, 0, 1);
				} else {
					::_InterlockedDecrement(&_lockCount);
				}
			}
		}

	private:
		// members
		volatile std::uint32_t _owningThread;  // 0
		volatile long _lockCount;              // 4
	};

	static_assert(sizeof(SpinLock) == 0x8);

	class UniqueSpinLock
	{
	public:
		UniqueSpinLock(SpinLock& a_lock)
		{
			_lock = &a_lock;
			_lock->spinLock();
		}
		~UniqueSpinLock()
		{
			_lock->spinUnlock();
		}
		UniqueSpinLock() = delete;
		UniqueSpinLock(UniqueSpinLock const&) = delete;
		void operator=(UniqueSpinLock const& x) = delete;

	private:
		mutable SpinLock* _lock;
	};
}