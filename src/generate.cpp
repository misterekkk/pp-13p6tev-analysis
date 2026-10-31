#include "ppanalysis/Event.hpp"

#include "Pythia8/Pythia.h"
#include "TFile.h"
#include "TTree.h"

#include <cmath>
#include <iostream>
#include <string>
#include <cstdlib>

namespace {

    struct Options {
        std::string config = "config/pythia_13p6TeV.cmnd";
        std::string output = "output/events.root";
        int nEvents = 1000;
        int seed = 12345;
    };

    Options parseArgs(int argc, char** argv) {
        Options opt;
        for (int i = 1; i < argc; i++) {
            const std::string key = argv[i];
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << key << "\n";
                std::exit(1);
            }
            const std::string value = argv[++i];
            
            if (key == "--config") opt.config = value;
            else if (key == "--output") opt.output = value;
            else if (key == "--nevents") opt.nEvents = std::stoi(value);
            else if (key == "--seed") opt.seed = std::stoi(value);
            else std::cerr << "Unknown option " << key << " (ignored)\n";
        }
        return opt;
    }

    ppanalysis::ParticleRecord toRecord(const Pythia8::Particle& p) {
        ppanalysis::ParticleRecord r;
        r.pdg = p.id();
        r.charge = static_cast<int>(std::lround(p.charge()));
        r.pt = p.pT();
        r.eta = p.eta();
        r.phi = p.phi();
        r.y = p.y();
        return r;
    }

}  // namespace

int main(int argc, char** argv) {
    const Options opt = parseArgs(argc, argv);

    Pythia8::Pythia pythia;
    if (!pythia.readFile(opt.config)) {
        std::cerr << "Cannot read " << opt.config << "\n";
        return 1;
    }
    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = " + std::to_string(opt.seed));
    if (!pythia.init()) return 1;
    pythia.settings.listChanged();

    TFile file(opt.output.c_str(), "RECREATE");
    TTree tree(ppanalysis::branch::kTreeName, "pp");
    ppanalysis::EventBranches branches;
    branches.attachTo(tree);

    for (int iEvent = 0; iEvent < opt.nEvents; ++iEvent) {
        if (!pythia.next()) continue;
        if (iEvent == 0) pythia.event.list();

        branches.clear();
        for (int i = 0; i < pythia.event.size(); ++i) {
            const Pythia8::Particle& p = pythia.event[i];
            if (!p.isFinal()) continue;
            branches.push(toRecord(p));
        }
        tree.Fill();
    }

    tree.Write();
    file.Close();
    pythia.stat();
    return 0;
}
