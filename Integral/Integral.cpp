#include <algorithm>
#include <chrono>
#include <cmath>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#define COUNT_TASKS_TO_MOVE_IN_GLOBAL 20
#define PRICISION_FOR_RESULT 11

struct IntegralTask {
  bool isTask = true;
  double Start;
  double End;
  double PartOfIntegral;

  IntegralTask() {}
  IntegralTask(bool isTask) : isTask(isTask) {}
  IntegralTask(double Start, double End, double PartOfIntegral)
      : Start(Start), End(End), PartOfIntegral(PartOfIntegral) {}
};

constexpr double FunctionToIntegrate(double X) {
  return std::cos(1. / (X - 5));
};

double integrateTasks(unsigned CountThreads, double Epsilon,
                      std::mutex &TasksMtx, std::mutex &isThereTasks,
                      std::vector<IntegralTask> &Tasks, unsigned &ThreadsExists,
                      unsigned &MaxThreads);

int main(int Argc, const char **Argv) {
  try {
    if (Argc < 2)
      throw std::logic_error("Second argument must be number of threads!");
    if (Argc < 3)
      throw std::logic_error("Third argument must be Epsilon!");

    unsigned CountThreads = std::atoll(Argv[1]);
    double Epsilon = std::atof(Argv[2]);
    unsigned long HWThreads = std::thread::hardware_concurrency();

    if (HWThreads != CountThreads)
      std::cout << "Warning! Hardware threads: " << HWThreads
                << ", but you choose count threads: " << CountThreads << "\n\n";

    //------------------------------Init_First_Task--------------------------------------
    std::vector<IntegralTask> Tasks;
    constexpr double Start = 0.005;
    constexpr double End = 4.995;
    constexpr double PartOfIntegral =
        (FunctionToIntegrate(End) + FunctionToIntegrate(Start)) *
        (End - Start) / 2;
    Tasks.emplace_back(Start, End, PartOfIntegral);

    std::vector<std::future<double>> Results(CountThreads);
    std::mutex TasksMtx;
    std::mutex isThereTasks;

    unsigned ThreadsExists = 0;
    unsigned MaxThreads = 0;
    //------------------------------Start_Integrate--------------------------------------

    auto StartTime = std::chrono::high_resolution_clock::now();
    for (auto Rank = 0; Rank < CountThreads; ++Rank) {
      std::packaged_task<double(unsigned, double, std::mutex &, std::mutex &,
                                std::vector<IntegralTask> &, unsigned &,
                                unsigned &)>
          Task{integrateTasks};
      Results[Rank] = Task.get_future();
      std::thread Thread{move(Task),
                         CountThreads,
                         Epsilon,
                         std::ref(TasksMtx),
                         std::ref(isThereTasks),
                         std::ref(Tasks),
                         std::ref(ThreadsExists),
                         std::ref(MaxThreads)};
      Thread.detach();
    }
    double Result = 0;
    std::for_each(Results.begin(), Results.end(),
                  [&Result](auto &Elem) { Result += Elem.get(); });
    auto StopTime = std::chrono::high_resolution_clock::now();

    //-------------------------------Stop_Integrate--------------------------------------

    auto Time = std::chrono::duration_cast<std::chrono::milliseconds>(StopTime -
                                                                      StartTime)
                    .count();

    std::cout << "Number of using threads: " << MaxThreads << ". Time: " << Time
              << " millisec.\n";
    std::cout << "Integral of a function on an interval [" << Start << ", "
              << End << "]: " << std::setprecision(PRICISION_FOR_RESULT)
              << Result << "\n";
  } catch (const std::exception &Ex) {
    std::cerr << Ex.what() << "\n";
  }
  return 0;
}

double integrateTasks(unsigned CountThreads, double Epsilon,
                      std::mutex &TasksMtx, std::mutex &isThereTasks,
                      std::vector<IntegralTask> &Tasks, unsigned &ThreadsExists,
                      unsigned &MaxThreads) {
  double CurrResult = 0;
  IntegralTask Task;
  while (true) {
    // Wait until the task appears.
    isThereTasks.lock();
    {
      std::lock_guard<std::mutex> LockMtx{TasksMtx};

      Task = Tasks.back();
      Tasks.pop_back();
      if (!Tasks.empty())
        isThereTasks.unlock();
      // Finalizing task.
      if (!Task.isTask)
        break;
      ++ThreadsExists;
      if (ThreadsExists > MaxThreads)
        MaxThreads = ThreadsExists;
    }

    double Result = 0;
    std::vector<IntegralTask> MyTasks;
    while (true) {
      double Center = (Task.Start + Task.End) / 2;
      double FunctionInCenter = FunctionToIntegrate(Center);

      double PartOfIntegralStartCenter =
          (FunctionInCenter + FunctionToIntegrate(Task.Start)) *
          (Center - Task.Start) / 2;
      double PartOfIntegralCenterEnd =
          (FunctionToIntegrate(Task.End) + FunctionInCenter) *
          (Task.End - Center) / 2;
      double BetterPartOfIntegral =
          PartOfIntegralStartCenter + PartOfIntegralCenterEnd;

      // If we have not achieved the required accuracy,
      // we will divide the task into two.
      if (std::abs((BetterPartOfIntegral - Task.PartOfIntegral) /
                   BetterPartOfIntegral) >= Epsilon) {
        MyTasks.emplace_back(Task.Start, Center, PartOfIntegralStartCenter);
        Task.Start = Center;
        Task.PartOfIntegral = PartOfIntegralCenterEnd;
      } else {
        Result += BetterPartOfIntegral;

        if (MyTasks.empty())
          break;
        Task = MyTasks.back();
        MyTasks.pop_back();
      }

      // If all the threads have calculated their parts,
      // then we will transfer part of the calculations to them.
      // This is an element of dynamic processor load balancing.
      if (MyTasks.size() >= COUNT_TASKS_TO_MOVE_IN_GLOBAL && Tasks.empty()) {
        std::lock_guard<std::mutex> LockMtx{TasksMtx};

        Tasks.insert(Tasks.end(), std::make_move_iterator(MyTasks.begin()),
                     std::make_move_iterator(MyTasks.end()));
        MyTasks.clear();
        isThereTasks.unlock();
      }
    }
    CurrResult += Result;

    {
      std::lock_guard<std::mutex> LockMtx{TasksMtx};
      --ThreadsExists;

      if (ThreadsExists == 0 && Tasks.empty()) {
        // This tasks finalize calculating.
        Tasks.insert(Tasks.end(), CountThreads, {/* isTask */ false});
        isThereTasks.unlock();
      }
    }
  }
  return CurrResult;
}
