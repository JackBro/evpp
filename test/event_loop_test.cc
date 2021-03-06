
#include "test_common.h"

#include <evpp/exp.h>
#include <evpp/libevent_headers.h>
#include <evpp/libevent_watcher.h>
#include <evpp/event_loop.h>
#include <evpp/timestamp.h>

#include <thread>

namespace evloop {
static std::shared_ptr<evpp::EventLoop> loop;
static evpp::Duration delay(1.0);
static bool event_handler_called = false;
static void Handle(evpp::InvokeTimerPtr t) {
    event_handler_called = true;
    t->Cancel();
    loop->Stop();
}

static void MyEventThread() {
    LOG_INFO << "EventLoop is running ...";
    loop = std::shared_ptr<evpp::EventLoop>(new evpp::EventLoop);
    loop->Run();
    loop.reset(); // 确保析构时，是在其自身运行的线程中
}

static int periodic_run_count = 0;
static void PeriodicFunc() {
    periodic_run_count++;
    LOG_INFO << "PeriodicFunc is called , periodic_run_count=" << periodic_run_count;
}
}

TEST_UNIT(testEventLoop) {
    using namespace evloop;
    std::thread th(MyEventThread);
    usleep(delay.Microseconds());
    evpp::Timestamp start = evpp::Timestamp::Now();
    evpp::InvokeTimerPtr t = loop->RunEvery(evpp::Duration(0.3), &PeriodicFunc);
    loop->RunAfter(delay, std::bind(&Handle, t));
    th.join();
    t.reset();
    evpp::Duration cost = evpp::Timestamp::Now() - start;
    H_TEST_ASSERT(delay <= cost);
    H_TEST_ASSERT(event_handler_called);
    H_TEST_ASSERT(periodic_run_count == 3);
    H_TEST_ASSERT(evpp::GetActiveEventCount() == 0);
}


