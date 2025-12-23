#ifndef __simcore_EventTrigger_h__
#define __simcore_EventTrigger_h__

#include "tls.hh"

namespace simcore {
    class EventTrigger {
      public:
        static EventTrigger &GetInstance() { return fgInstance; }

        void Reset() { fTriggered = false; }
        void Trigger() { fTriggered = true; }
        bool IsTriggered() const { return fTriggered; }

      private:
        bool fTriggered;
        static G4ThreadLocal EventTrigger fgInstance;
    };
} // namespace simcore
#endif
