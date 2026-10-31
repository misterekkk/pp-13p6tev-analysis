#include "ppanalysis/Analyzer.hpp"

#include <cmath>
#include <memory>
#include <stdexcept>

#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

namespace ppanalysis {

    bool isInelGt0(const std::vector<ParticleRecord>& particles) {
        for (const auto& p : particles) {
            if (p.charge != 0 && std::abs(p.eta) < kInelGt0EtaMax) return true;
        }
        return false;
    }

    Analyzer::Analyzer(const std::vector<ParticleCategory>& categories) {
        histograms_.reserve(categories.size());
        for (const auto& category : categories) histograms_.emplace_back(category);
    }

    void Analyzer::run(const std::string& inputPath) {
        std::unique_ptr<TFile> file(TFile::Open(inputPath.c_str(), "READ"));
        if (!file || file->IsZombie()) throw std::runtime_error("Cannot open " + inputPath);

        TTreeReader reader(branch::kTreeName, file.get());
        TTreeReaderArray<int> pdg(reader, branch::kPdg);
        TTreeReaderArray<int> charge(reader, branch::kCharge);
        TTreeReaderArray<float> pt(reader, branch::kPt);
        TTreeReaderArray<float> eta(reader, branch::kEta);
        TTreeReaderArray<float> phi(reader, branch::kPhi);
        TTreeReaderArray<float> y(reader, branch::kY);

        std::vector<ParticleRecord> particles;
        while (reader.Next()) {
            particles.clear();
            for (std::size_t i = 0; i < pdg.GetSize(); ++i) {
                particles.push_back({pdg[i], charge[i], pt[i], eta[i], phi[i], y[i]});
            }
            processEvent(particles);
        }
        if (reader.GetEntryStatus() != TTreeReader::kEntryBeyondEnd) {
            throw std::runtime_error("Error while reading " + inputPath);
        }
    }

    void Analyzer::processEvent(const std::vector<ParticleRecord>& particles) {
        for (const auto& p : particles) {
            for (auto& h : histograms_) h.fillParticle(p);
        }
        for (auto& h : histograms_) h.endEvent();

        ++nInel_;
        if (isInelGt0(particles)) ++nInelGt0_;
    }

    void Analyzer::write(const std::string& outputPath) const {
        TFile file(outputPath.c_str(), "RECREATE");
        if (file.IsZombie()) throw std::runtime_error("Cannot create " + outputPath);

        for (const auto& h : histograms_) h.writeTo(file);

        TH1D hNevents("hNevents", "Event counters;Event class;Events", 2, 0.0, 2.0);
        hNevents.GetXaxis()->SetBinLabel(1, "INEL");
        hNevents.GetXaxis()->SetBinLabel(2, "INEL>0");
        hNevents.SetBinContent(1, static_cast<double>(nInel_));
        hNevents.SetBinContent(2, static_cast<double>(nInelGt0_));
        file.cd();
        hNevents.Write();

        file.Close();
    }

}  // namespace ppanalysis
