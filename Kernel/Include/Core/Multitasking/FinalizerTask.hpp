#pragma once

namespace Multitasking {

class FinalizerTask {
public:
	static void start();

private:
	static void finalizer_task(void*);
};

}
