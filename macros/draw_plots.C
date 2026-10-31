#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TSystem.h"

#include <memory>
#include <string>
#include <vector>

namespace {

    struct Variable {
        std::string prefix;
        std::string xTitle;
        std::string yTitle;
        bool logY;
    };

    struct Overlay {
        std::string name;
        std::string title;
        std::vector<std::string> categories;
        bool shapeOnly;
    };

    const std::vector<Variable> kVariables = {
        {"hN", "Multiplicity N", "Events", true},
        {"hPt", "p_{T} [GeV]", "Particles", true},
        {"hEta", "#eta", "Particles", false},
        {"hPhi", "#phi [rad]", "Particles", false},
    };

    const std::vector<std::string> kCategories = {"all", "ch", "pip", "pim", "Kp", "Km", "p", "pbar"};

    const std::vector<Overlay> kOverlays = {
        {"pions", "#pi^{+} and #pi^{-}", {"pip", "pim"}, false},
        {"kaons", "K^{+} and K^{-}", {"Kp", "Km"}, false},
        {"protons", "p and #bar{p}", {"p", "pbar"}, false},
        {"species", "Species comparison (unit area)", {"ch", "pip", "Kp", "p"}, true},
    };

    const std::vector<int> kColors = {kBlue + 1, kRed + 1, kGreen + 2, kMagenta + 1};

    bool needsRebin(const Variable& v, const std::string& cat, bool shapeOnly) {
        return v.prefix == "hN" && (cat == "ch" || shapeOnly);
    }

    std::unique_ptr<TH1D> prepare(TFile& file, const Variable& v, const std::string& cat,
                                  const std::string& cloneName, int color, bool shapeOnly) {
        const std::string name = v.prefix + "_" + cat;
        auto* src = dynamic_cast<TH1D*>(file.Get(name.c_str()));
        if (!src) {
            ::Error("draw_plots", "Histogram %s not found", name.c_str());
            return nullptr;
        }
        std::unique_ptr<TH1D> h(static_cast<TH1D*>(src->Clone(cloneName.c_str())));
        h->SetDirectory(nullptr);

        std::string yTitle = v.yTitle;
        if (needsRebin(v, cat, shapeOnly)) {
            h->Rebin(2);
            yTitle += " / 2";
        }
        if (shapeOnly && h->Integral() > 0) {
            h->Scale(1.0 / h->Integral());
            yTitle = "Normalized to unit area";
        }

        h->SetLineColor(color);
        h->SetMarkerColor(color);
        h->SetLineWidth(2);
        h->GetXaxis()->SetTitle(v.xTitle.c_str());
        h->GetYaxis()->SetTitle(yTitle.c_str());
        if (!v.logY) h->SetMinimum(0.0);
        else h->SetMinimum(shapeOnly ? 1e-7 : 0.5);
        return h;
    }

    void save(TCanvas& c, const std::string& outDir, const std::string& name) {
        c.SaveAs((outDir + "/" + name + ".png").c_str());
        c.SaveAs((outDir + "/" + name + ".pdf").c_str());
    }

}

void draw_plots(const char* inputPath = "output/analysis.root", const char* outDir = "plots") {
    gSystem->mkdir(outDir, kTRUE);
    gStyle->SetPadLeftMargin(0.13);
    gStyle->SetTitleOffset(1.3, "Y");

    TFile file(inputPath, "READ");
    if (file.IsZombie()) {
        ::Error("draw_plots", "Cannot open %s", inputPath);
        return;
    }

    gStyle->SetOptStat(1110);
    for (const auto& v : kVariables) {
        for (const auto& cat : kCategories) {
            const std::string name = v.prefix + "_" + cat;
            auto h = prepare(file, v, cat, name + "_draw", kBlue + 1, false);
            if (!h) continue;
            TCanvas c(("c_" + name).c_str(), name.c_str(), 800, 600);
            c.SetLogy(v.logY);
            h->Draw("HIST");
            save(c, outDir, name);
        }
    }

    gStyle->SetOptStat(0);
    for (const auto& v : kVariables) {
        for (const auto& overlay : kOverlays) {
            const std::string name = v.prefix + "_" + overlay.name;
            TCanvas c(("c_" + name).c_str(), name.c_str(), 800, 600);
            c.SetLogy(v.logY);
            TLegend leg(0.72, 0.72, 0.88, 0.88);
            leg.SetBorderSize(0);
            leg.SetFillStyle(0);
            leg.SetTextSize(0.04);

            std::vector<std::unique_ptr<TH1D>> hists;
            double maxY = 0.0;
            for (std::size_t i = 0; i < overlay.categories.size(); ++i) {
                const std::string& cat = overlay.categories[i];
                auto h = prepare(file, v, cat, name + "_" + cat, kColors[i % kColors.size()],
                                 overlay.shapeOnly);
                if (!h) continue;
                maxY = std::max(maxY, h->GetMaximum());
                leg.AddEntry(h.get(), h->GetTitle(), "l");
                hists.push_back(std::move(h));
            }
            if (hists.empty()) continue;

            hists[0]->SetTitle(overlay.title.c_str());
            hists[0]->SetMaximum(v.logY ? maxY * 5.0 : maxY * 1.25);
            for (std::size_t i = 0; i < hists.size(); ++i) hists[i]->Draw(i == 0 ? "HIST" : "HIST SAME");
            leg.Draw();
            save(c, outDir, name);
        }
    }
}
