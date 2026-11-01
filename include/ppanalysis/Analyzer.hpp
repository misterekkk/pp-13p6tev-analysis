#pragma once

#include "ppanalysis/Categories.hpp"
#include "ppanalysis/Event.hpp"

#include <memory>
#include <string>
#include <vector>

namespace ppanalysis {

    inline constexpr double kInelGt0EtaMax = 1.0;
    bool isInelGt0(const std::vector<ParticleRecord>& particles);

    class ComparisonHistograms {
    public:
        static constexpr double kMidEtaMax = 0.5;
        static constexpr double kMidYMax = 0.5;

        ComparisonHistograms();

        void fillParticle(const ParticleRecord& particle, bool inelGt0);
        void endEvent(bool inelGt0);
        void writeTo(TDirectory& dir) const;

        const TH1D& midEtaMultiplicity() const { return *hNchMidEta_; }
        const TH1D& ptPions() const { return *hPtPi_; }
        const TH1D& ptKaons() const { return *hPtK_; }
        const TH1D& ptProtons() const { return *hPtP_; }

    private:
        int countMidEta_ = 0;

        std::unique_ptr<TH1D> hNchMidEta_;
        std::unique_ptr<TH1D> hEtaCh_;
        std::unique_ptr<TH1D> hPtPi_;
        std::unique_ptr<TH1D> hPtK_;
        std::unique_ptr<TH1D> hPtP_;
    };

    class Analyzer {
    public:
        explicit Analyzer(const std::vector<ParticleCategory>& categories);

        void run(const std::string& inputPath);
        void processEvent(const std::vector<ParticleRecord>& particles);
        void write(const std::string& outputPath) const;

        long nEventsInel() const { return nInel_; }
        long nEventsInelGt0() const { return nInelGt0_; }
        const std::vector<CategoryHistograms>& histograms() const { return histograms_; }
        const ComparisonHistograms& comparison() const { return comparison_; }

    private:
        std::vector<CategoryHistograms> histograms_;
        ComparisonHistograms comparison_;
        long nInel_ = 0;
        long nInelGt0_ = 0;
    };

}  // namespace ppanalysis
