#include <iostream>
using std::cout, std::endl;

#include <thread>
using std::thread;

#include "../timer/timer.hpp"
using az::Timer;

#include <mutex>
using std::mutex;
using lock = std::lock_guard<mutex>;

#include <queue>
#include <vector>
#include <string>
using std::string;
using std::queue;
using std::vector;
using squeue = queue<string>;
using svector = vector<string>;


class Handler
{
public:
    void operator()(squeue&, mutex&, svector&, mutex&, svector&, mutex&);
};

class Caller
{
public:
    virtual void operator()(Handler&&, squeue&, mutex&, svector&, mutex&, svector&, mutex&) = 0;
};

class CallerOneThread : public Caller
{
public:
    void operator()(Handler&&, squeue&, mutex&, svector&, mutex&, svector&, mutex&) override;
};

class CallerMultiThread : public Caller
{
public:
    void operator()(Handler&&, squeue&, mutex&, svector&, mutex&, svector&, mutex&) override;
};

class Printer
{
public:
    void operator()(svector&, svector&);
};

class Wrapper
{
public:
    void operator()(Caller&&, Handler&&, Printer&&);
};


void caller1();
void caller2();

int main()
{
    cout << "===============================================================================" << endl;
    cout << "One thread\n";
    cout << Timer::timer<Timer::ns>(Wrapper(), CallerOneThread(), Handler(), Printer()) << " ns" << endl;
    cout << "===============================================================================" << endl;
    cout << "Two thread\n";
    cout << Timer::timer<Timer::ns>(Wrapper(), CallerMultiThread(), Handler(), Printer()) << " ns" << endl;
    cout << "===============================================================================" << endl;

    return 0;
}

void Wrapper::operator()(Caller&& caller, Handler&& handler, Printer&& printer)
{
    squeue raw_text;
    svector text;
    svector comments;

    raw_text.push("Line 1");
    raw_text.push("#Line 1");
    raw_text.push("Line 2");
    raw_text.push("#Line 2");
    raw_text.push("Line 3");
    raw_text.push("Line 4");
    raw_text.push("#Line 3");
    raw_text.push("Line 5");
    raw_text.push("Line 6");
    raw_text.push("#Line 4");
    raw_text.push("Line 7");
    raw_text.push("#Line 5");

    text.reserve(7);
    comments.reserve(5);

    mutex mtx_raw_text;
    mutex mtx_text;
    mutex mtx_comments;

    caller(std::forward<Handler>(handler), raw_text, mtx_raw_text, text, mtx_text, comments, mtx_comments);
    printer(text, comments);
}

void Printer::operator()(svector& text, svector& comments)
{
#ifdef _DEBUG
    for (auto it = text.begin(); it < text.end(); ++it) cout << *it << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
    for (auto it = comments.begin(); it < comments.end(); ++it) cout << *it << endl;
    cout << "-------------------------------------------------------------------------------" << endl;
#endif
}

void CallerOneThread::operator()(Handler&& handler, squeue& raw_text, mutex& mtx_rt,
                                                    svector& text, mutex& mtx_t,
                                                    svector& comments, mutex& mtx_c)
{
    handler(raw_text, mtx_rt, text, mtx_t, comments, mtx_c);
}

void CallerMultiThread::operator()(Handler&& handler, squeue& raw_text, mutex& mtx_rt,
                                                      svector& text, mutex& mtx_t,
                                                      svector& comments, mutex& mtx_c)
{
    thread t1 (std::forward<Handler>(handler), std::ref(raw_text), std::ref(mtx_rt), std::ref(text), std::ref(mtx_t), std::ref(comments), std::ref(mtx_c));
    thread t2 (std::forward<Handler>(handler), std::ref(raw_text), std::ref(mtx_rt), std::ref(text), std::ref(mtx_t), std::ref(comments), std::ref(mtx_c));

    t1.join();
    t2.join();
}

void Handler::operator()(squeue& raw_text, mutex& mtx_rt,
                         svector& text, mutex& mtx_t,
                         svector& comments, mutex& mtx_c)
{
    while (true)
    {
        if (raw_text.empty()) break;
        string line;

        {
            lock l(mtx_rt);
            line = raw_text.front();
            raw_text.pop();
        }

        if (line.rfind('#', 0) == 0)
        {
            lock l(mtx_c);
            comments.push_back(line);
        }
        else
        {
            lock l(mtx_t);
            text.push_back(line);
        }
    }
}
