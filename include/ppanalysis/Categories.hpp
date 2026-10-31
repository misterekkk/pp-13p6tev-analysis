#pragma once

#include "ppanalysis/Event.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "TH1D.h"

class TDirectory;

namespace ppanalysis {

    class ParticleCategory {
    public:
        using Predicate = std::function<bool(const ParticleRecord&)>;

        ParticleCategory(std::string name, std::string label, Predicate accepts);

        const std::string& name() const { return name_; }
        const std::string& label() const { return label_; }
        bool accepts(const ParticleRecord& particle) const { return accepts_(particle); }

    private:
        std::string name_;
        std::string label_;
        Predicate accepts_;
    };

    std::vector<ParticleCategory> makeStandardCategories();

    namespace binning {

        inline constexpr int kNBins = 400;   inline constexpr double kNMin = 0.0;    inline constexpr double kNMax = 400.0;
        inline constexpr int kPtBins = 100;  inline constexpr double kPtMin = 0.0;   inline constexpr double kPtMax = 10.0;
        inline constexpr int kEtaBins = 100; inline constexpr double kEtaMin = -5.0; inline constexpr double kEtaMax = 5.0;
        inline constexpr int kPhiBins = 64;

    }  // namespace binning

    class CategoryHistograms {
    public:
        explicit CategoryHistograms(const ParticleCategory& category);

        void fillParticle(const ParticleRecord& particle);
        void endEvent();
        void writeTo(TDirectory& dir) const;

        const ParticleCategory& category() const { return category_; }
        const TH1D& multiplicity() const { return *hN_; }
        const TH1D& pt() const { return *hPt_; }
        const TH1D& eta() const { return *hEta_; }
        const TH1D& phi() const { return *hPhi_; }

    private:
        ParticleCategory category_;
        int countInEvent_ = 0;

        std::unique_ptr<TH1D> hN_;
        std::unique_ptr<TH1D> hPt_;
        std::unique_ptr<TH1D> hEta_;
        std::unique_ptr<TH1D> hPhi_;      // |phi| in [0, pi]
        std::unique_ptr<TH1D> hPhiFull_;  // phi in (-pi, pi]
    };

}  // namespace ppanalysis
