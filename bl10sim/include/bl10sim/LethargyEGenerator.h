#ifndef __bl10sim_LethargyEGenerator_h__
#define __bl10sim_LethargyEGenerator_h__
#include <string>
#include <vector>

namespace bl10sim {
    class LethargyEnergyGenerator {
      public:
        LethargyEnergyGenerator();
        virtual ~LethargyEnergyGenerator();

        void SetInputFilename(const std::string &a) { fFluxFilename = a; }
        std::string GetInputFilename() const { return fFluxFilename; }

        void SetTrimLastones(bool a) { fTrimLastones = a; }
        bool GetTrimLastones() const { return fTrimLastones; }

        double Generate() const;

        bool Initialize();

      private:
        bool fTrimLastones;
        bool fReady;
        std::string fFluxFilename;
        std::vector<double> fCumulative;
        std::vector<double> fBoundaries;
    };
}; // namespace bl10sim
#endif
