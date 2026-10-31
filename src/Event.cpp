#include "ppanalysis/Event.hpp"

#include "TTree.h"

namespace ppanalysis {

    void EventBranches::attachTo(TTree& tree) {
        tree.Branch(branch::kPdg, &pdg_);
        tree.Branch(branch::kCharge, &charge_);
        tree.Branch(branch::kPt, &pt_);
        tree.Branch(branch::kEta, &eta_);
        tree.Branch(branch::kPhi, &phi_);
        tree.Branch(branch::kY, &y_);
    }

    void EventBranches::clear() {
        pdg_.clear();
        charge_.clear();
        pt_.clear();
        eta_.clear();
        phi_.clear();
        y_.clear();
    }

    void EventBranches::push(const ParticleRecord& particle) {
        pdg_.push_back(particle.pdg);
        charge_.push_back(particle.charge);
        pt_.push_back(static_cast<float>(particle.pt));
        eta_.push_back(static_cast<float>(particle.eta));
        phi_.push_back(static_cast<float>(particle.phi));
        y_.push_back(static_cast<float>(particle.y));
    }

}  // namespace ppanalysis
