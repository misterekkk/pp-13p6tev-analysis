#pragma once

#include <vector>

class TTree;

namespace ppanalysis {

    struct ParticleRecord {
        int pdg = 0;
        int charge = 0;
        double pt = 0.0;
        double eta = 0.0;
        double phi = 0.0;
        double y = 0.0;
    };

    namespace branch {

        inline constexpr const char* kTreeName = "events";
        inline constexpr const char* kPdg = "pdg";
        inline constexpr const char* kCharge = "charge";
        inline constexpr const char* kPt = "pt";
        inline constexpr const char* kEta = "eta";
        inline constexpr const char* kPhi = "phi";
        inline constexpr const char* kY = "y";

    }  // namespace branch

    class EventBranches {
    public:
        void attachTo(TTree& tree);

        void clear();
        void push(const ParticleRecord& particle);

        EventBranches() = default;
        EventBranches(const EventBranches&) = delete;
        EventBranches& operator=(const EventBranches&) = delete;

    private:
        std::vector<int> pdg_;
        std::vector<int> charge_;
        std::vector<float> pt_;
        std::vector<float> eta_;
        std::vector<float> phi_;
        std::vector<float> y_;
    };

}  // namespace ppanalysis
