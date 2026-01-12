#ifndef __simcore_SafeTermination_h__
#define __simcore_SafeTermination_h__
#include <csignal>

namespace simcore {
    class SafeTermination {
      public:
        SafeTermination(const SafeTermination &)            = delete;
        SafeTermination &operator=(const SafeTermination &) = delete;
        SafeTermination(SafeTermination &&)                 = delete;
        SafeTermination &operator=(SafeTermination &&)      = delete;

        static void RegisterSignalHandler();
        static void DeregisterSignalHandler();

        static SafeTermination &GetInstance();

        void Terminate() { fgStopFlag = 1; };
        void Reset() { fgStopFlag = 0; };

        bool IsTerminated() const;

      private:
        SafeTermination() : fgStopFlag(0) {};
        volatile sig_atomic_t fgStopFlag;
    };
} // namespace simcore

#endif
