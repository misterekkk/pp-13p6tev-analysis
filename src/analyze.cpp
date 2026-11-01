#include "ppanalysis/Analyzer.hpp"
#include "ppanalysis/Categories.hpp"

#include "TH1.h"

#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

    struct Options {
        std::string input = "output/events.root";
        std::string output = "output/analysis.root";
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

            if (key == "--input") opt.input = value;
            else if (key == "--output") opt.output = value;
            else std::cerr << "Unknown option " << key << " (ignored)\n";
        }
        return opt;
    }

    double percentOutside(const TH1D& h) {
        const double entries = h.GetEntries();
        if (entries <= 0) return 0.0;
        return 100.0 * (h.GetBinContent(0) + h.GetBinContent(h.GetNbinsX() + 1)) / entries;
    }

    void printSummary(const ppanalysis::Analyzer& analyzer) {
        const long nInel = analyzer.nEventsInel();
        const long nInelGt0 = analyzer.nEventsInelGt0();
        std::cout << "\nEvents: INEL = " << nInel << ", INEL>0 = " << nInelGt0;
        if (nInel > 0) std::cout << " (fraction " << double(nInelGt0) / nInel << ")";
        std::cout << "\n\n"
                  << std::left << std::setw(6) << "cat" << std::right
                  << std::setw(11) << "<N>/event" << std::setw(12) << "<pT> [GeV]"
                  << std::setw(16) << "N>400 [evt %]" << std::setw(16) << "|eta|>5 [%]"
                  << std::setw(16) << "pT>10 [%]" << "\n";
        for (const auto& h : analyzer.histograms()) {
            const TH1D& hN = h.multiplicity();
            const TH1D& hEta = h.eta();
            const TH1D& hPt = h.pt();
            std::cout << std::left << std::setw(6) << h.category().name() << std::right << std::fixed
                      << std::setw(11) << std::setprecision(2) << hN.GetMean()
                      << std::setw(12) << std::setprecision(3) << hPt.GetMean()
                      << std::setw(16) << std::setprecision(3) << percentOutside(hN)
                      << std::setw(16) << percentOutside(hEta)
                      << std::setw(16) << std::setprecision(4) << percentOutside(hPt) << "\n";
        }
        std::cout << "(means include entries outside the histogram ranges; "
                     "% = share of entries in under/overflow bins)\n";

        const auto& cmp = analyzer.comparison();
        const TH1D& hMid = cmp.midEtaMultiplicity();
        std::cout << "\n<dNch/deta> in |eta|<0.5, INEL>0: " << std::setprecision(3) << hMid.GetMean()
                  << " +- " << hMid.GetMeanError() << " (stat)\n"
                  << "<pT> in |y|<0.5 [GeV]: pi " << cmp.ptPions().GetMean()
                  << ", K " << cmp.ptKaons().GetMean() << ", p " << cmp.ptProtons().GetMean() << "\n";
    }

}  // namespace

int main(int argc, char** argv) {
    const Options opt = parseArgs(argc, argv);

    TH1::AddDirectory(false);
    TH1::StatOverflows(true);

    try {
        ppanalysis::Analyzer analyzer(ppanalysis::makeStandardCategories());
        analyzer.run(opt.input);
        analyzer.write(opt.output);
        printSummary(analyzer);
        std::cout << "\nWrote " << opt.output << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
