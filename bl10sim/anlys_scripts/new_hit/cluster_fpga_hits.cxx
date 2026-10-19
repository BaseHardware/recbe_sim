// Agglomerative clustering of FPGA energy-deposition steps.
// ROOT usage:
//   root -l -b -q 'cluster_fpga_hits.cxx("steps.root","hits.root",1.0,0.01,0)'
//   unit for spatial radius: mm
//   unit for time unit: ns
// Parameters use the same time and position units as the input TTree.
// spatial_mode: 0 = complete linkage (default), 1 = Edep-weighted centroid.

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "TFile.h"
#include "TParameter.h"
#include "TTree.h"

namespace fpga_cluster {

    struct GroupKey {
        std::string filename, pvname;
        int evtid = 0, copy_no = 0;
        bool operator<(const GroupKey &b) const {
            return std::tie(filename, evtid, pvname, copy_no) <
                   std::tie(b.filename, b.evtid, b.pvname, b.copy_no);
        }
    };

    struct Step {
        Long64_t entry = -1;
        double x = 0, y = 0, z = 0, t = 0;
        double start_t = 0, end_t = 0;
        double edep = 0, niedep = 0;
        int pdg = 0, id = 0, maker_id = 0, maker_pdg = 0;
        std::string maker_proc;
    };

    struct Cluster {
        std::vector<size_t> members;
        double min_t = 0, max_t = 0; // midpoint times for clustering
        double sum_e = 0, cx = 0, cy = 0, cz = 0;
        bool active = true;
    };

    double dist2(const Step &a, const Step &b) {
        const double dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
        return dx * dx + dy * dy + dz * dz;
    }

    // Every merged cluster must have midpoint-time span <= time_window.
    bool compatible_time(const Cluster &a, const Cluster &b, double window) {
        return std::max(a.max_t, b.max_t) - std::min(a.min_t, b.min_t) <= window;
    }

    double spatial_d2(const Cluster &a, const Cluster &b, const std::vector<Step> &steps,
                      int mode) {
        if (mode == 1) {
            const double dx = a.cx - b.cx, dy = a.cy - b.cy, dz = a.cz - b.cz;
            return dx * dx + dy * dy + dz * dz;
        }
        // Complete linkage: maximum of all cross-cluster midpoint distances.
        double maximum = 0;
        for (size_t i : a.members)
            for (size_t j : b.members)
                maximum = std::max(maximum, dist2(steps[i], steps[j]));
        return maximum;
    }

    struct Candidate {
        double d2;
        size_t a, b;
    };
    struct CandidateGreater {
        bool operator()(const Candidate &x, const Candidate &y) const {
            if (x.d2 != y.d2) return x.d2 > y.d2;
            if (x.a != y.a) return x.a > y.a;
            return x.b > y.b;
        }
    };

    std::vector<Cluster> build_clusters(std::vector<Step> &steps, double time_window, double radius,
                                        int mode) {
        // Explicit tie-breaking ensures reproducibility regardless of ROOT entry order.
        std::stable_sort(steps.begin(), steps.end(), [](const Step &a, const Step &b) {
            return std::tie(a.t, a.entry) < std::tie(b.t, b.entry);
        });
        std::vector<Cluster> clusters;
        clusters.reserve(steps.size() * 2);
        for (size_t i = 0; i < steps.size(); ++i) {
            Cluster c;
            c.members.push_back(i);
            c.min_t = c.max_t = steps[i].t;
            c.sum_e           = steps[i].edep;
            c.cx              = steps[i].x;
            c.cy              = steps[i].y;
            c.cz              = steps[i].z;
            clusters.push_back(std::move(c));
        }
        std::priority_queue<Candidate, std::vector<Candidate>, CandidateGreater> heap;
        const double radius2 = radius * radius;
        auto enqueue         = [&](size_t i, size_t j) {
            if (i == j) return;
            if (i > j) std::swap(i, j);
            const Cluster &a = clusters[i], &b = clusters[j];
            if (!a.active || !b.active || !compatible_time(a, b, time_window)) return;
            const double d2 = spatial_d2(a, b, steps, mode);
            if (d2 <= radius2) heap.push({d2, i, j});
        };
        // Only step pairs whose time separation fits the window need an initial edge.
        for (size_t i = 0; i < steps.size(); ++i)
            for (size_t j = i + 1; j < steps.size() && steps[j].t - steps[i].t <= time_window; ++j)
                enqueue(i, j);

        while (!heap.empty()) {
            const Candidate best = heap.top();
            heap.pop();
            if (!clusters[best.a].active || !clusters[best.b].active) continue;
            // Each merge creates a fresh cluster; older heap candidates become stale.
            const Cluster &a = clusters[best.a], &b = clusters[best.b];
            Cluster merged;
            merged.members = a.members;
            merged.members.insert(merged.members.end(), b.members.begin(), b.members.end());
            merged.min_t            = std::min(a.min_t, b.min_t);
            merged.max_t            = std::max(a.max_t, b.max_t);
            merged.sum_e            = a.sum_e + b.sum_e;
            merged.cx               = (a.cx * a.sum_e + b.cx * b.sum_e) / merged.sum_e;
            merged.cy               = (a.cy * a.sum_e + b.cy * b.sum_e) / merged.sum_e;
            merged.cz               = (a.cz * a.sum_e + b.cz * b.sum_e) / merged.sum_e;
            clusters[best.a].active = false;
            clusters[best.b].active = false;
            const size_t k          = clusters.size();
            clusters.push_back(std::move(merged));
            for (size_t i = 0; i < k; ++i)
                if (clusters[i].active) enqueue(i, k);
        }
        std::vector<Cluster> result;
        for (auto &c : clusters)
            if (c.active) result.push_back(std::move(c));
        std::sort(result.begin(), result.end(), [&](const Cluster &a, const Cluster &b) {
            if (a.min_t != b.min_t) return a.min_t < b.min_t;
            const Long64_t ea = steps[*std::min_element(a.members.begin(), a.members.end(),
                                                        [&](size_t i, size_t j) {
                                                            return steps[i].entry < steps[j].entry;
                                                        })]
                                    .entry;
            const Long64_t eb = steps[*std::min_element(b.members.begin(), b.members.end(),
                                                        [&](size_t i, size_t j) {
                                                            return steps[i].entry < steps[j].entry;
                                                        })]
                                    .entry;
            return ea < eb;
        });
        return result;
    }

} // namespace fpga_cluster

void cluster_fpga_hits(const char *input_file  = "output.root",
                       const char *output_file = "hits.root", double time_window = 10,
                       double spatial_radius = 0.01, int spatial_mode = 0) {
    using namespace fpga_cluster;
    if (!input_file || !output_file || time_window < 0 || spatial_radius < 0 ||
        !std::isfinite(time_window) || !std::isfinite(spatial_radius) ||
        (spatial_mode != 0 && spatial_mode != 1) ||
        std::string(input_file) == std::string(output_file)) {
        std::cerr << "[ERROR] Invalid input/output path or clustering parameters" << std::endl;
        return;
    }
    std::unique_ptr<TFile> in(TFile::Open(input_file, "READ"));
    if (!in || in->IsZombie()) {
        std::cerr << "[ERROR] Cannot open input ROOT file: " << input_file << std::endl;
        return;
    }
    TTree *tree = nullptr;
    in->GetObject("steps", tree);
    if (!tree) {
        std::cerr << "[ERROR] Cannot find input TTree: steps" << std::endl;
        return;
    }

    Int_t evtid = 0, copy_no = 0, pdg = 0, id = 0, maker_id = 0, maker_pdg = 0;
    Double_t sx = 0, sy = 0, sz = 0, st = 0, ex = 0, ey = 0, ez = 0, et = 0, edep = 0, niedep = 0;
    Double_t prim_e = 0, prim_t = 0;
    std::string *filename = nullptr, *pvname = nullptr, *maker_proc = nullptr;
    bool branches_ok = true;
    auto set         = [&](const char *name, void *addr) {
        if (!tree->GetBranch(name) || tree->SetBranchAddress(name, addr) < 0) {
            std::cerr << "[ERROR] Cannot bind input branch: " << name << std::endl;
            branches_ok = false;
        }
    };
    set("evtid", &evtid);
    set("filename", &filename);
    set("envelope_copyno", &copy_no);
    set("pvname", &pvname);
    set("start_x", &sx);
    set("start_y", &sy);
    set("start_z", &sz);
    set("start_t", &st);
    set("end_x", &ex);
    set("end_y", &ey);
    set("end_z", &ez);
    set("end_t", &et);
    set("edep", &edep);
    set("niedep", &niedep);
    set("pdg", &pdg);
    set("id", &id);
    set("maker_id", &maker_id);
    set("maker_pdg", &maker_pdg);
    set("maker_proc", &maker_proc);
    set("prim_e", &prim_e);
    set("prim_t", &prim_t);
    if (!branches_ok) return;

    std::map<GroupKey, std::vector<Step>> groups;
    std::map<GroupKey, std::pair<double, double>> primaries;
    const Long64_t n = tree->GetEntries();
    for (Long64_t entry = 0; entry < n; ++entry) {
        if (tree->GetEntry(entry) <= 0 || !filename || !pvname || !maker_proc) {
            std::cerr << "[ERROR] Cannot read step entry: " << entry << std::endl;
            return;
        }
        GroupKey key{*filename, *pvname, evtid, copy_no};
        Step s;
        s.entry      = entry;
        s.x          = (sx + ex) * 0.5;
        s.y          = (sy + ey) * 0.5;
        s.z          = (sz + ez) * 0.5;
        s.t          = (st + et) * 0.5;
        s.start_t    = st;
        s.end_t      = et;
        s.edep       = edep;
        s.niedep     = niedep;
        s.pdg        = pdg;
        s.id         = id;
        s.maker_id   = maker_id;
        s.maker_pdg  = maker_pdg;
        s.maker_proc = *maker_proc;
        groups[key].push_back(std::move(s));
        primaries[key] = {prim_e, prim_t};
    }

    std::unique_ptr<TFile> out(TFile::Open(output_file, "RECREATE"));
    if (!out || out->IsZombie()) {
        std::cerr << "[ERROR] Cannot create output ROOT file: " << output_file << std::endl;
        return;
    }
    out->cd();
    TTree hits("hits", "Clustered FPGA energy-deposition hits");
    TTree links("hit_steps", "Mapping of clustered hits to source step entries");
    // Stack-owned trees must not be deleted by TFile::Close().
    hits.SetDirectory(nullptr);
    links.SetDirectory(nullptr);
    Int_t o_evtid = 0, o_nhits = 0, o_hitid = 0, o_copy_no = 0, o_nsteps = 0, o_ntracks = 0,
          o_nmakers   = 0;
    Double_t o_prim_e = 0, o_prim_t = 0, o_edep = 0, o_niedep = 0, o_ion_edep = 0;
    Double_t o_start_t = 0, o_end_t = 0, o_mean_t = 0, o_mid_min_t = 0, o_mid_max_t = 0;
    Double_t o_x = 0, o_y = 0, o_z = 0, o_spread_x = 0, o_spread_y = 0, o_spread_z = 0,
             o_spread_r = 0, o_radius = 0;
    Double_t o_min_x = 0, o_max_x = 0, o_min_y = 0, o_max_y = 0, o_min_z = 0, o_max_z = 0;
    std::string o_filename, o_pvname;
    std::vector<int> o_maker_id, o_maker_pdg;
    std::vector<std::string> o_maker_proc;
    std::vector<double> o_maker_edep, o_maker_niedep;
    hits.Branch("evtid", &o_evtid);
    hits.Branch("nhits", &o_nhits);
    hits.Branch("hitid", &o_hitid);
    hits.Branch("filename", &o_filename);
    hits.Branch("pvname", &o_pvname);
    hits.Branch("envelope_copyno", &o_copy_no);
    hits.Branch("prim_e", &o_prim_e);
    hits.Branch("prim_t", &o_prim_t);
    hits.Branch("nsteps", &o_nsteps);
    hits.Branch("ntracks", &o_ntracks);
    hits.Branch("nmakers", &o_nmakers);
    hits.Branch("edep", &o_edep);
    hits.Branch("niedep", &o_niedep);
    hits.Branch("ion_edep", &o_ion_edep);
    hits.Branch("start_t", &o_start_t);
    hits.Branch("end_t", &o_end_t);
    hits.Branch("mean_t", &o_mean_t);
    hits.Branch("mid_min_t", &o_mid_min_t);
    hits.Branch("mid_max_t", &o_mid_max_t);
    hits.Branch("x", &o_x);
    hits.Branch("y", &o_y);
    hits.Branch("z", &o_z);
    hits.Branch("spread_x", &o_spread_x);
    hits.Branch("spread_y", &o_spread_y);
    hits.Branch("spread_z", &o_spread_z);
    hits.Branch("spread_r", &o_spread_r);
    hits.Branch("radius", &o_radius);
    hits.Branch("min_x", &o_min_x);
    hits.Branch("max_x", &o_max_x);
    hits.Branch("min_y", &o_min_y);
    hits.Branch("max_y", &o_max_y);
    hits.Branch("min_z", &o_min_z);
    hits.Branch("max_z", &o_max_z);
    hits.Branch("maker_id", &o_maker_id);
    hits.Branch("maker_pdg", &o_maker_pdg);
    hits.Branch("maker_proc", &o_maker_proc);
    hits.Branch("maker_edep", &o_maker_edep);
    hits.Branch("maker_niedep", &o_maker_niedep);

    Long64_t o_step_entry = -1;
    links.Branch("evtid", &o_evtid);
    links.Branch("nhits", &o_nhits);
    links.Branch("hitid", &o_hitid);
    links.Branch("filename", &o_filename);
    links.Branch("pvname", &o_pvname);
    links.Branch("envelope_copyno", &o_copy_no);
    links.Branch("step_entry", &o_step_entry);

    Long64_t hit_count = 0;
    for (auto &group : groups) {
        const GroupKey &key      = group.first;
        std::vector<Step> &steps = group.second;
        std::vector<Cluster> clusters =
            build_clusters(steps, time_window, spatial_radius, spatial_mode);
        o_evtid         = key.evtid;
        o_copy_no       = key.copy_no;
        o_filename      = key.filename;
        o_pvname        = key.pvname;
        o_prim_e        = primaries.at(key).first;
        o_prim_t        = primaries.at(key).second;
        o_nhits         = clusters.size();
        int local_hitid = 0;
        for (const Cluster &c : clusters) {
            o_hitid     = local_hitid++;
            o_nsteps    = static_cast<Int_t>(c.members.size());
            o_x         = c.cx;
            o_y         = c.cy;
            o_z         = c.cz;
            o_mid_min_t = c.min_t;
            o_mid_max_t = c.max_t;
            o_edep      = 0;
            o_niedep    = 0;
            o_mean_t    = 0;
            o_spread_x = o_spread_y = o_spread_z = o_spread_r = o_radius = 0;
            o_start_t = std::numeric_limits<double>::infinity();
            o_end_t   = -std::numeric_limits<double>::infinity();
            o_min_x = o_min_y = o_min_z = std::numeric_limits<double>::infinity();
            o_max_x = o_max_y = o_max_z = -std::numeric_limits<double>::infinity();
            std::set<int> track_ids;
            using MakerKey = std::tuple<int, int, std::string>;
            std::map<MakerKey, std::pair<double, double>> maker_sums;
            for (size_t i : c.members) {
                const Step &s = steps[i];
                o_edep += s.edep;
                o_niedep += s.niedep;
                o_mean_t += s.edep * s.t;
                o_start_t       = std::min(o_start_t, s.start_t);
                o_end_t         = std::max(o_end_t, s.end_t);
                o_min_x         = std::min(o_min_x, s.x);
                o_max_x         = std::max(o_max_x, s.x);
                o_min_y         = std::min(o_min_y, s.y);
                o_max_y         = std::max(o_max_y, s.y);
                o_min_z         = std::min(o_min_z, s.z);
                o_max_z         = std::max(o_max_z, s.z);
                const double dx = s.x - o_x, dy = s.y - o_y, dz = s.z - o_z;
                o_spread_x += s.edep * dx * dx;
                o_spread_y += s.edep * dy * dy;
                o_spread_z += s.edep * dz * dz;
                o_radius = std::max(o_radius, std::sqrt(dx * dx + dy * dy + dz * dz));
                track_ids.insert(s.id);
                auto &sums = maker_sums[{s.maker_id, s.maker_pdg, s.maker_proc}];
                sums.first += s.edep;
                sums.second += s.niedep;
                o_step_entry = s.entry;
                links.Fill();
            }
            o_mean_t /= o_edep;
            o_spread_x = std::sqrt(o_spread_x / o_edep);
            o_spread_y = std::sqrt(o_spread_y / o_edep);
            o_spread_z = std::sqrt(o_spread_z / o_edep);
            o_spread_r = std::sqrt(o_spread_x * o_spread_x + o_spread_y * o_spread_y +
                                   o_spread_z * o_spread_z);
            o_ion_edep = o_edep - o_niedep;
            o_ntracks  = static_cast<Int_t>(track_ids.size());
            o_nmakers  = static_cast<Int_t>(maker_sums.size());
            o_maker_id.clear();
            o_maker_pdg.clear();
            o_maker_proc.clear();
            o_maker_edep.clear();
            o_maker_niedep.clear();
            std::vector<decltype(maker_sums)::const_iterator> sorted;
            for (auto it = maker_sums.cbegin(); it != maker_sums.cend(); ++it)
                sorted.push_back(it);
            std::sort(sorted.begin(), sorted.end(), [](auto a, auto b) {
                if (a->second.first != b->second.first) return a->second.first > b->second.first;
                return a->first < b->first;
            });
            for (auto it : sorted) {
                o_maker_id.push_back(std::get<0>(it->first));
                o_maker_pdg.push_back(std::get<1>(it->first));
                o_maker_proc.push_back(std::get<2>(it->first));
                o_maker_edep.push_back(it->second.first);
                o_maker_niedep.push_back(it->second.second);
            }
            hits.Fill();
            ++hit_count;
        }
    }
    out->cd();
    hits.Write();
    links.Write();
    TParameter<double>("time_window", time_window).Write();
    TParameter<double>("spatial_radius", spatial_radius).Write();
    TParameter<int>("spatial_mode", spatial_mode).Write();
    out->Close();
    std::cout << "[INFO] Input steps: " << n << ", output hits: " << hit_count
              << ", output file: " << output_file << std::endl;
}
