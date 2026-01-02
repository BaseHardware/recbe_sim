#ifndef __simcore_SafeTermination_h__
#define __simcore_SafeTermination_h__

namespace simcore {
    class SafeTermination {
      public:
        SafeTermination(const SafeTermination &)            = delete;
        SafeTermination &operator=(const SafeTermination &) = delete;
        SafeTermination(SafeTermination &&)                 = delete;
        SafeTermination &operator=(SafeTermination &&)      = delete;

        static void RegisterSignalHandler();
        static void DeregisterSignalHandler();
    };
} // namespace simcore

#endif
