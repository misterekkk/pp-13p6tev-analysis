#include "ppanalysis/Analyzer.hpp"

#include <cmath>
#include <memory>
#include <stdexcept>

#include "TDirectory.h"
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

    namespace {

        std::unique_ptr<TH1D> book(const char* name, const char* title, int bins, double min, double max) {
            auto h = std::make_unique<TH1D>(name, title, bins, min, max);
            h->Sumw2();
            return h;
        }

    }  // namespace

    ComparisonHistograms::ComparisonHistograms() {
        hNchMidEta_ = book("hNchMidEta_INELgt0", "Charged, |#eta|<0.5, INEL>0;N_{ch};Events", 200, 0.0, 200.0);
        hEtaCh_ = book("hEtaCh_INELgt0", "Charged, INEL>0;#eta;Particles", 40, -2.0, 2.0);
        hPtPi_ = book("hPtMidY_pi", "#pi^{+}+#pi^{-}, |y|<0.5;p_{T} [GeV];Particles", 2000, 0.0, 20.0);
        hPtK_ = book("hPtMidY_K", "K^{+}+K^{-}, |y|<0.5;p_{T} [GeV];Particles", 2000, 0.0, 20.0);
        hPtP_ = book("hPtMidY_p", "p+#bar{p}, |y|<0.5;p_{T} [GeV];Particles", 2000, 0.0, 20.0);
    }

    void ComparisonHistograms::fillParticle(const ParticleRecord& particle, bool inelGt0) {
        if (std::abs(particle.y) < kMidYMax) {
            switch (std::abs(particle.pdg)) {
                case 211: hPtPi_->Fill(particle.pt); break;
                case 321: hPtK_->Fill(particle.pt); break;
                case 2212: hPtP_->Fill(particle.pt); break;
                default: break;
            }
        }
        if (inelGt0 && particle.charge != 0) {
            hEtaCh_->Fill(particle.eta);
            if (std::abs(particle.eta) < kMidEtaMax) ++countMidEta_;
        }
    }

    void ComparisonHistograms::endEvent(bool inelGt0) {
        if (inelGt0) hNchMidEta_->Fill(countMidEta_);
        countMidEta_ = 0;
    }

    void ComparisonHistograms::writeTo(TDirectory& dir) const {
        TDirectory* sub = dir.mkdir("comparison");
        sub->cd();
        for (const auto* h : {hNchMidEta_.get(), hEtaCh_.get(), hPtPi_.get(), hPtK_.get(), hPtP_.get()}) h->Write();
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
        const bool inelGt0 = isInelGt0(particles);
        for (const auto& p : particles) {
            for (auto& h : histograms_) h.fillParticle(p);
            comparison_.fillParticle(p, inelGt0);
        }
        for (auto& h : histograms_) h.endEvent();
        comparison_.endEvent(inelGt0);

        ++nInel_;
        if (inelGt0) ++nInelGt0_;
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
        comparison_.writeTo(file);

        file.Close();
    }

}  // namespace ppanalysis
