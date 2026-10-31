#include "ppanalysis/Categories.hpp"

#include <cmath>
#include <utility>

#include "TDirectory.h"
#include "TMath.h"

namespace ppanalysis {

    ParticleCategory::ParticleCategory(std::string name, std::string label, Predicate accepts)
        : name_(std::move(name)), label_(std::move(label)), accepts_(std::move(accepts)) {}

    std::vector<ParticleCategory> makeStandardCategories() {
        auto pdgIs = [](int code) {
            return [code](const ParticleRecord& p) { return p.pdg == code; };
        };
        return {
            ParticleCategory("all", "all final-state", [](const ParticleRecord&) { return true; }),
            ParticleCategory("ch", "charged", [](const ParticleRecord& p) { return p.charge != 0; }),
            ParticleCategory("pip", "#pi^{+}", pdgIs(211)),
            ParticleCategory("pim", "#pi^{-}", pdgIs(-211)),
            ParticleCategory("Kp", "K^{+}", pdgIs(321)),
            ParticleCategory("Km", "K^{-}", pdgIs(-321)),
            ParticleCategory("p", "p", pdgIs(2212)),
            ParticleCategory("pbar", "#bar{p}", pdgIs(-2212)),
        };
    }

    namespace {

        std::unique_ptr<TH1D> book(const std::string& name, const std::string& title,
                                   int bins, double min, double max) {
            auto h = std::make_unique<TH1D>(name.c_str(), title.c_str(), bins, min, max);
            h->Sumw2();
            return h;
        }

    }  // namespace

    CategoryHistograms::CategoryHistograms(const ParticleCategory& category)
        : category_(category) {
        const std::string& n = category.name();
        const std::string& l = category.label();
        using namespace binning;
        hN_ = book("hN_" + n, l + ";Multiplicity N;Events", kNBins, kNMin, kNMax);
        hPt_ = book("hPt_" + n, l + ";p_{T} [GeV];Particles", kPtBins, kPtMin, kPtMax);
        hEta_ = book("hEta_" + n, l + ";#eta;Particles", kEtaBins, kEtaMin, kEtaMax);
        hPhi_ = book("hPhi_" + n, l + ";|#phi| [rad];Particles", kPhiBins, 0.0, TMath::Pi());
        hPhiFull_ = book("hPhiFull_" + n, l + ";#phi [rad];Particles", kPhiBins, -TMath::Pi(), TMath::Pi());
    }

    void CategoryHistograms::fillParticle(const ParticleRecord& particle) {
        if (!category_.accepts(particle)) return;
        hPt_->Fill(particle.pt);
        hEta_->Fill(particle.eta);
        hPhi_->Fill(std::abs(particle.phi));
        hPhiFull_->Fill(particle.phi);
        ++countInEvent_;
    }

    void CategoryHistograms::endEvent() {
        hN_->Fill(countInEvent_);
        countInEvent_ = 0;
    }

    void CategoryHistograms::writeTo(TDirectory& dir) const {
        dir.cd();
        hN_->Write();
        hPt_->Write();
        hEta_->Write();
        hPhi_->Write();
        hPhiFull_->Write();
    }

}  // namespace ppanalysis
