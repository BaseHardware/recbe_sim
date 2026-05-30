#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "TClonesArray.h"
#include "TFile.h"
#include "TTree.h"

#ifndef __CLING__
#include "simobj/Primary.h"
#include "simobj/Step.h"
#include "simobj/Track.h"
#endif

using namespace std;

namespace {

    constexpr bool kShowCopyNumber = true;
    constexpr bool kShowNDaughters = true;

    // Geant4 internal unit과 비슷하게,
    // length는 mm, energy는 MeV로 저장되어 있다고 가정.
    string format_value(double v, int precision = 4) {
        ostringstream os;
        os << setprecision(precision) << defaultfloat << v;
        return os.str();
    }

    string best_length(double mm) {
        const double a = std::abs(mm);

        double scale = 1.0;
        string unit  = "mm";

        if (a >= 1000.0) {
            scale = 1000.0;
            unit  = "m";
        } else if (a >= 10.0) {
            scale = 10.0;
            unit  = "cm";
        } else if (a > 0.0 && a < 1.0e-6) {
            scale = 1.0e-6;
            unit  = "nm";
        } else if (a > 0.0 && a < 1.0e-3) {
            scale = 1.0e-3;
            unit  = "um";
        }

        return format_value(mm / scale) + " " + unit;
    }

    string best_energy(double mev) {
        const double a = std::abs(mev);

        double scale = 1.0;
        string unit  = "MeV";

        if (a >= 1000.0) {
            scale = 1000.0;
            unit  = "GeV";
        } else if (a > 0.0 && a < 1.0e-6) {
            scale = 1.0e-6;
            unit  = "eV";
        } else if (a > 0.0 && a < 1.0e-3) {
            scale = 1.0e-3;
            unit  = "keV";
        }

        return format_value(mev / scale) + " " + unit;
    }

    string fit_string(const string &s, size_t width) {
        if (s.size() <= width) return s;
        if (width <= 3) return s.substr(0, width);
        return "..." + s.substr(s.size() - width + 3);
    }

    string volume_name(const simobj::Step *s) {
        string v = s->GetVolumeName().Data();
        if (v.empty()) return "OutOfWorld";
        return v;
    }

    string process_name(const simobj::Step *s) {
        string p = s->GetProcessName().Data();
        if (p.empty()) return " ";
        return p;
    }

    double step_distance(const simobj::Step *a, const simobj::Step *b) {
        const double dx = b->GetX() - a->GetX();
        const double dy = b->GetY() - a->GetY();
        const double dz = b->GetZ() - a->GetZ();
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    void print_track_header(const simobj::Track *trk) {
        cout << "\n";
        cout << "**********************************************************************************"
                "***********************\n";
        cout << "* G4Track Information: "
             << "Particle = " << trk->GetName() << ",   Track ID = " << trk->GetTrackID()
             << ",   Parent ID = " << trk->GetParentID() << "\n";
        cout << "**********************************************************************************"
                "***********************\n";
    }

    void print_column_header() {
        cout << setw(6) << "Step#" << setw(13) << "X" << setw(13) << "Y" << setw(13) << "Z"
             << setw(13) << "KineE" << setw(13) << "dEStep" << setw(13) << "NonIonEdep" << setw(13)
             << "StepLeng" << setw(13) << "TrakLeng" << setw(24) << "Volume" << setw(18)
             << "Process";

        if (kShowCopyNumber) {
            cout << setw(9) << "CopyNo" << setw(11) << "EnvCopyNo";
        }

        if (kShowNDaughters) {
            cout << setw(8) << "#daug";
        }

        cout << "\n";
    }

    void print_step_row(const string &step_label, const simobj::Step *s, double step_length,
                        double track_length, const string &process_override = "") {
        string proc = process_override.empty() ? process_name(s) : process_override;

        cout << setw(6) << step_label << setw(13) << best_length(s->GetX()) << setw(13)
             << best_length(s->GetY()) << setw(13) << best_length(s->GetZ()) << setw(13)
             << best_energy(s->GetKineticEnergy()) << setw(13)
             << best_energy(s->GetDepositedEnergy()) << setw(13)
             << best_energy(s->GetNonIonDepositedEnergy()) << setw(13) << best_length(step_length)
             << setw(13) << best_length(track_length) << setw(24) << fit_string(volume_name(s), 24)
             << setw(18) << fit_string(proc, 18);

        if (kShowCopyNumber) {
            cout << setw(9) << s->GetCopyNumber() << setw(11) << s->GetEnvelopeCopyNumber();
        }

        if (kShowNDaughters) {
            cout << setw(8) << s->GetNDaughters();
        }

        cout << "\n";
    }

    string ask_filename() {
        string filename;

        while (true) {
            cout << "Input ROOT file path";
            cout << " [default: ./simout.root] : ";

            getline(cin, filename);

            if (filename.empty()) {
                filename = "./simout.root";
            }

            unique_ptr<TFile> test_file(TFile::Open(filename.c_str(), "READ"));

            if (test_file && !test_file->IsZombie()) {
                return filename;
            }

            cerr << "Error: cannot open file: " << filename << endl;
        }
    }

    Long64_t ask_entry_number(Long64_t n_entries) {
        if (n_entries <= 0) {
            cerr << "Error: this tree has no entries." << endl;
            return -1;
        }

        if (n_entries == 1) {
            cout << "This file contains only one event. Use event 0." << endl;
            return 0;
        }

        while (true) {
            cout << "This file contains " << n_entries << " events." << endl;
            cout << "Input event number [0 - " << n_entries - 1 << "] : ";

            string line;
            getline(cin, line);

            try {
                size_t pos     = 0;
                Long64_t entry = stoll(line, &pos);

                if (pos != line.size()) {
                    throw invalid_argument("trailing characters");
                }

                if (entry >= 0 && entry < n_entries) {
                    return entry;
                }
            } catch (...) {
                // fall through
            }

            cerr << "Invalid event number. Please input an integer between 0 and " << n_entries - 1
                 << "." << endl;
        }
    }

} // namespace

void dump_tracks(const char *filename = nullptr, Long64_t entry = -1) {
    string filename_str;

    if (filename == nullptr || string(filename).empty()) {
        filename_str = ask_filename();
    } else {
        filename_str = filename;
    }

    unique_ptr<TFile> input(TFile::Open(filename_str.c_str(), "READ"));

    if (!input || input->IsZombie()) {
        cerr << "Error: cannot open file: " << filename_str << endl;
        return;
    }

    TTree *tree = static_cast<TTree *>(input->Get("tree"));

    if (!tree) {
        cerr << "Error: cannot find TTree named 'tree'." << endl;
        return;
    }

    const Long64_t n_entries = tree->GetEntries();

    if (entry < 0) {
        entry = ask_entry_number(n_entries);
        if (entry < 0) return;
    }

    if (entry >= n_entries) {
        cerr << "Error: entry " << entry << " is out of range. "
             << "Nentries = " << n_entries << endl;
        return;
    }

    TClonesArray *tcaTrack   = nullptr;
    TClonesArray *tcaStep    = nullptr;
    simobj::Primary *primary = nullptr;

    tree->SetBranchAddress("Steps", &tcaStep);
    tree->SetBranchAddress("Tracks", &tcaTrack);
    tree->SetBranchAddress("Primary", &primary);

    tree->GetEntry(entry);

    if (!tcaTrack || !tcaStep) {
        cerr << "Error: failed to load Tracks or Steps branch." << endl;
        return;
    }

    const int nTrack = tcaTrack->GetEntries();

    cout << "\n";
    cout << "================================================================================\n";
    cout << " File  : " << filename_str << "\n";
    cout << " Event : " << entry << " / " << n_entries - 1 << "\n";
    cout << " Ntrk  : " << nTrack << "\n";
    cout << "================================================================================\n";

    for (int idx_track = 0; idx_track < nTrack; ++idx_track) {
        auto *trk = static_cast<simobj::Track *>(tcaTrack->At(idx_track));

        if (!trk) continue;

        print_track_header(trk);
        print_column_header();

        const int nStep = trk->GetNStep();

        const simobj::Step *prev = &trk->GetFirstStep();

        double track_length = 0.0;

        print_step_row("0", prev, 0.0, 0.0);

        for (int idx_step = 0; idx_step < nStep; ++idx_step) {
            const int step_index = trk->GetStepIndex(idx_step);

            auto *nowstep = static_cast<simobj::Step *>(tcaStep->At(step_index));

            if (!nowstep) {
                cout << setw(6) << idx_step + 1 << "  <invalid step index: " << step_index << ">\n";
                continue;
            }

            const double step_length = step_distance(prev, nowstep);
            track_length += step_length;

            print_step_row(std::to_string(idx_step + 1), nowstep, step_length, track_length);

            prev = nowstep;
        }

        const simobj::Step *final_step = &trk->GetFinalStep();

        const double step_length = step_distance(prev, final_step);
        track_length += step_length;

        print_step_row("END", final_step, step_length, track_length);
    }
}
