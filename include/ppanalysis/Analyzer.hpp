#pragma once

#include "ppanalysis/Categories.hpp"
#include "ppanalysis/Event.hpp"

#include <string>
#include <vector>

namespace ppanalysis {

    inline constexpr double kInelGt0EtaMax = 1.0;
    bool isInelGt0(const std::vector<ParticleRecord>& particles);

    class Analyzer {
    public:
        explicit Analyzer(const std::vector<ParticleCategory>& categories);

        void run(const std::string& inputPath);
        void processEvent(const std::vector<ParticleRecord>& particles);
        void write(const std::string& outputPath) const;

        long nEventsInel() const { return nInel_; }
        long nEventsInelGt0() const { return nInelGt0_; }
        const std::vector<CategoryHistograms>& histograms() const { return histograms_; }

    private:
        std::vector<CategoryHistograms> histograms_;
        long nInel_ = 0;
        long nInelGt0_ = 0;
    };

}  // namespace ppanalysis
