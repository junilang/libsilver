IAsyncRT_DECLARE(DummyAsync)

void DummyAsync_submit(void *this, const AsyncTask *tasks, usize tasks_size) {
	// recursively runs async tasks

	auto const end = tasks + tasks_size;

	for (; tasks < end; tasks++) {
		auto task = *tasks;
		repeat:;

		auto const task_data = AsyncTask_data(task);
		auto result = task_data->entry(DummyAsync_upcast(nullptr), task);

		switch (result.intent) {
			case AsyncIntent_SUSPEND: return;
			case AsyncIntent_FINISH: goto Bfinish;
			case AsyncIntent_PANIC: goto Bpanic;

			default:
				task = result.next;
				goto repeat;
		}

		if (0) Bfinish: {
			auto const metadata = AsyncTask_metadata(task);

			if (metadata & FLAG(AsyncTaskFlag, RESUMING)) {
				task = task_data->resume;
				goto repeat;
			} else if (!AsyncFuture_isnull(task_data->future)) {
				result = AsyncFuture_callback(task_data->future);
				switch (result.intent) {
					case AsyncIntent_SUSPEND: return;
					case AsyncIntent_PANIC: goto Bpanic;
					case AsyncIntent_FINISH:
						PANIC("DummyAsync: future shouldn't return finish intent");

					default:
						task = result.next;
						goto repeat;
				}
			} else { // task has no successor
				return;
			}
		}

		if (0) Bpanic: {
			PANIC("DummyAsync: task panicked");
		}
	}
}

IAsyncRT_GENERATE_KNOWN(DummyAsync)
