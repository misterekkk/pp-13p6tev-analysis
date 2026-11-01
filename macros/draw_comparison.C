#include "TBox.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1D.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLine.h"
#include "TPad.h"
#include "TStyle.h"
#include "TSystem.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

    const double kAliceDNdEta = 7.10;
    const double kAliceDNdEtaErr = 0.18;
    const double kAliceDNdEta13 = 6.46;
    const double kAliceDNdEta13Err = 0.19;

    struct Table {
        std::vector<std::vector<double>> rows;
    };

    Table readCsv(const std::string& path) {
        Table t;
        std::ifstream in(path);
        if (!in) {
            ::Error("draw_comparison", "Cannot open %s", path.c_str());
            return t;
        }
        std::string line;
        bool header = true;
        while (std::getline(in, line)) {
            if (line.empty() || line[0] == '#') continue;
            if (header) {
                header = false;
                continue;
            }
            std::vector<double> row;
            std::stringstream ss(line);
            std::string cell;
            while (std::getline(ss, cell, ',')) row.push_back(std::stod(cell));
            t.rows.push_back(row);
        }
        return t;
    }

    struct Binned {
        std::vector<double> edges, value, stat, sys;
    };

    Binned binned(const Table& t) {
        Binned b;
        for (const auto& r : t.rows) {
            b.edges.push_back(r[1]);
            b.value.push_back(r[3]);
            b.stat.push_back(r[4]);
            b.sys.push_back(r[6]);
        }
        if (!t.rows.empty()) b.edges.push_back(t.rows.back()[2]);
        return b;
    }

    std::unique_ptr<TH1D> rebinned(TH1D& fine, const std::vector<double>& edges, const std::string& name) {
        for (double e : edges) {
            const int bin = fine.GetXaxis()->FindBin(e + 1e-9);
            if (std::abs(fine.GetXaxis()->GetBinLowEdge(bin) - e) > 1e-6) {
                ::Error("draw_comparison", "Edge %g not aligned with fine binning of %s", e, fine.GetName());
            }
        }
        std::unique_ptr<TH1D> h(static_cast<TH1D*>(
            fine.Rebin(static_cast<int>(edges.size()) - 1, name.c_str(), edges.data())));
        h->SetDirectory(nullptr);
        return h;
    }

    double nEvents(TFile& f, int bin) {
        auto* h = dynamic_cast<TH1D*>(f.Get("hNevents"));
        return h ? h->GetBinContent(bin) : 0.0;
    }

    void save(TCanvas& c, const std::string& outDir, const std::string& name) {
        c.SaveAs((outDir + "/" + name + ".png").c_str());
        c.SaveAs((outDir + "/" + name + ".pdf").c_str());
    }

    void drawDataVsMc(const Binned& data, TH1D& mc, const std::string& yTitle, const std::string& title,
                      bool logY, double ratioMin, double ratioMax, const std::string& outDir,
                      const std::string& name) {
        const int n = static_cast<int>(data.value.size());
        TGraphErrors gStat(n), gSys(n), gBand(n), gRatio;
        for (int i = 0; i < n; ++i) {
            const double x = 0.5 * (data.edges[i] + data.edges[i + 1]);
            const double hw = 0.5 * (data.edges[i + 1] - data.edges[i]);
            const double v = data.value[i];
            gStat.SetPoint(i, x, v);
            gStat.SetPointError(i, 0.0, data.stat[i]);
            gSys.SetPoint(i, x, v);
            gSys.SetPointError(i, 0.8 * hw, data.sys[i]);
            gBand.SetPoint(i, x, 1.0);
            gBand.SetPointError(i, hw, std::hypot(data.stat[i], data.sys[i]) / v);
            if (mc.GetBinContent(i + 1) > 0) {
                const int k = gRatio.GetN();
                gRatio.SetPoint(k, x, mc.GetBinContent(i + 1) / v);
                gRatio.SetPointError(k, 0.0, mc.GetBinError(i + 1) / v);
            }
        }

        TCanvas c(("c_" + name).c_str(), name.c_str(), 800, 800);
        TPad top("top", "", 0.0, 0.32, 1.0, 1.0);
        TPad bottom("bottom", "", 0.0, 0.0, 1.0, 0.32);
        top.SetBottomMargin(0.02);
        top.SetLeftMargin(0.14);
        top.SetLogx();
        top.SetLogy(logY);
        bottom.SetTopMargin(0.03);
        bottom.SetBottomMargin(0.32);
        bottom.SetLeftMargin(0.14);
        bottom.SetLogx();
        top.Draw();
        bottom.Draw();

        top.cd();
        const double xMin = data.edges.front(), xMax = data.edges.back();
        TH1D frame("frame", (title + ";;" + yTitle).c_str(), 1, xMin, xMax);
        double yMin = 1e30, yMax = 0.0;
        for (double v : data.value) {
            yMin = std::min(yMin, v);
            yMax = std::max(yMax, v);
        }
        frame.SetMinimum(logY ? yMin * 0.2 : 0.0);
        frame.SetMaximum(logY ? yMax * 10.0 : yMax * 1.6);
        frame.GetXaxis()->SetLabelSize(0);
        frame.GetYaxis()->SetTitleOffset(1.4);
        frame.SetStats(false);
        frame.Draw("AXIS");

        gSys.SetFillColorAlpha(kGray + 1, 0.5);
        gSys.SetLineColor(kGray + 1);
        gSys.Draw("2 SAME");
        gStat.SetMarkerStyle(20);
        gStat.SetMarkerSize(0.8);
        gStat.Draw("P SAME");
        mc.SetLineColor(kRed + 1);
        mc.SetLineWidth(2);
        mc.SetStats(false);
        mc.Draw("HIST SAME");

        TLegend leg(0.45, 0.70, 0.88, 0.88);
        leg.SetBorderSize(0);
        leg.SetFillStyle(0);
        leg.SetTextSize(0.035);
        leg.AddEntry(&gStat, "ALICE pp 13 TeV, INEL (stat.)", "pe");
        leg.AddEntry(&gSys, "ALICE syst.", "f");
        leg.AddEntry(&mc, "PYTHIA 8.318 Monash, 13 TeV", "l");
        leg.Draw();

        bottom.cd();
        TH1D rframe("rframe", ";p_{T} [GeV];MC / Data", 1, xMin, xMax);
        rframe.SetMinimum(ratioMin);
        rframe.SetMaximum(ratioMax);
        rframe.SetStats(false);
        rframe.GetXaxis()->SetTitleSize(0.12);
        rframe.GetXaxis()->SetLabelSize(0.10);
        rframe.GetYaxis()->SetTitleSize(0.10);
        rframe.GetYaxis()->SetTitleOffset(0.6);
        rframe.GetYaxis()->SetLabelSize(0.09);
        rframe.GetYaxis()->SetNdivisions(505);
        rframe.Draw("AXIS");
        gBand.SetFillColorAlpha(kGray + 1, 0.5);
        gBand.Draw("2 SAME");
        TLine one(xMin, 1.0, xMax, 1.0);
        one.SetLineStyle(2);
        one.Draw();
        gRatio.SetMarkerStyle(20);
        gRatio.SetMarkerSize(0.7);
        gRatio.SetMarkerColor(kRed + 1);
        gRatio.SetLineColor(kRed + 1);
        gRatio.Draw("P SAME");

        save(c, outDir, name);
    }

}

void draw_comparison(const char* mc13Path = "output/analysis_13TeV.root",
                     const char* mc136Path = "output/analysis.root",
                     const char* refDir = "reference/ins1797443", const char* outDir = "plots/comparison") {
    gSystem->mkdir(outDir, kTRUE);
    gStyle->SetOptStat(0);

    TFile f13(mc13Path, "READ");
    TFile f136(mc136Path, "READ");
    if (f13.IsZombie() || f136.IsZombie()) {
        ::Error("draw_comparison", "Cannot open MC files");
        return;
    }
    const std::string ref = refDir;
    const double nInel13 = nEvents(f13, 1);

    auto fine = [&](const char* name) { return dynamic_cast<TH1D*>(f13.Get((std::string("comparison/") + name).c_str())); };
    TH1D* finePi = fine("hPtMidY_pi");
    TH1D* fineK = fine("hPtMidY_K");
    TH1D* fineP = fine("hPtMidY_p");
    if (!finePi || !fineK || !fineP || nInel13 <= 0) {
        ::Error("draw_comparison", "Comparison histograms missing in %s", mc13Path);
        return;
    }

    struct Spectrum {
        const char* table;
        TH1D* fine;
        const char* name;
        const char* title;
    };
    const std::vector<Spectrum> spectra = {
        {"Table_1.csv", finePi, "spectrum_pi", "#pi^{+}+#pi^{-}, |y|<0.5"},
        {"Table_2.csv", fineK, "spectrum_K", "K^{+}+K^{-}, |y|<0.5"},
        {"Table_6.csv", fineP, "spectrum_p", "p+#bar{p}, |y|<0.5"},
    };
    for (const auto& s : spectra) {
        const Binned data = binned(readCsv(ref + "/" + s.table));
        auto mc = rebinned(*s.fine, data.edges, std::string(s.name) + "_mc");
        mc->Scale(1.0 / nInel13, "width");
        drawDataVsMc(data, *mc, "1/N_{ev} d^{2}N/(dp_{T}dy) [GeV^{-1}]", s.title, true, 0.0, 2.0, outDir, s.name);
    }

    struct Ratio {
        const char* table;
        TH1D* num;
        const char* name;
        const char* title;
    };
    const std::vector<Ratio> ratios = {
        {"Table_21.csv", fineK, "ratio_K_pi", "(K^{+}+K^{-}) / (#pi^{+}+#pi^{-})"},
        {"Table_42.csv", fineP, "ratio_p_pi", "(p+#bar{p}) / (#pi^{+}+#pi^{-})"},
    };
    for (const auto& r : ratios) {
        const Binned data = binned(readCsv(ref + "/" + r.table));
        auto num = rebinned(*r.num, data.edges, std::string(r.name) + "_num");
        auto den = rebinned(*finePi, data.edges, std::string(r.name) + "_den");
        num->Divide(den.get());
        drawDataVsMc(data, *num, "Ratio", r.title, false, 0.0, 2.0, outDir, r.name);
    }

    {
        const double nGt0 = nEvents(f136, 2);
        auto* src = dynamic_cast<TH1D*>(f136.Get("comparison/hEtaCh_INELgt0"));
        auto* src13 = dynamic_cast<TH1D*>(f13.Get("comparison/hEtaCh_INELgt0"));
        auto* mid = dynamic_cast<TH1D*>(f136.Get("comparison/hNchMidEta_INELgt0"));
        if (src && src13 && mid && nGt0 > 0) {
            std::unique_ptr<TH1D> h(static_cast<TH1D*>(src->Clone("dndeta_136")));
            std::unique_ptr<TH1D> h13(static_cast<TH1D*>(src13->Clone("dndeta_13")));
            h->SetDirectory(nullptr);
            h13->SetDirectory(nullptr);
            h->Scale(1.0 / nGt0, "width");
            h13->Scale(1.0 / nEvents(f13, 2), "width");

            TCanvas c("c_dndeta", "dndeta", 800, 600);
            c.SetLeftMargin(0.13);
            h->SetTitle("Charged particles, INEL>0;#eta;dN_{ch}/d#eta");
            h->GetXaxis()->SetRangeUser(-1.5, 1.5);
            h->SetMinimum(6.0);
            h->SetMaximum(9.0);
            h->SetLineColor(kRed + 1);
            h->SetLineWidth(2);
            h->Draw("HIST");
            h13->SetLineColor(kBlue + 1);
            h13->SetLineStyle(2);
            h13->SetLineWidth(2);
            h13->Draw("HIST SAME");

            TBox box(-0.5, kAliceDNdEta - kAliceDNdEtaErr, 0.5, kAliceDNdEta + kAliceDNdEtaErr);
            box.SetFillColorAlpha(kGray + 1, 0.5);
            box.Draw();
            TLine line(-0.5, kAliceDNdEta, 0.5, kAliceDNdEta);
            line.SetLineWidth(3);
            line.Draw();
            TBox box13(-0.5, kAliceDNdEta13 - kAliceDNdEta13Err, 0.5, kAliceDNdEta13 + kAliceDNdEta13Err);
            box13.SetFillColorAlpha(kAzure - 9, 0.5);
            box13.Draw();
            TLine line13(-0.5, kAliceDNdEta13, 0.5, kAliceDNdEta13);
            line13.SetLineWidth(3);
            line13.SetLineColor(kBlue + 1);
            line13.Draw();
            h->Draw("HIST SAME");
            h13->Draw("HIST SAME");

            TLegend leg(0.14, 0.64, 0.70, 0.88);
            leg.SetBorderSize(0);
            leg.SetFillStyle(0);
            leg.SetTextSize(0.035);
            leg.AddEntry(&box, "ALICE 13.6 TeV: #LTdN_{ch}/d#eta#GT_{|#eta|<0.5} = 7.10 #pm 0.18", "f");
            leg.AddEntry(h.get(), Form("PYTHIA 13.6 TeV: #LTdN_{ch}/d#eta#GT_{|#eta|<0.5} = %.2f #pm %.2f",
                                       mid->GetMean(), mid->GetMeanError()), "l");
            leg.AddEntry(&box13, "ALICE 13 TeV: #LTdN_{ch}/d#eta#GT_{|#eta|<0.5} = 6.46 #pm 0.19", "f");
            auto* mid13 = dynamic_cast<TH1D*>(f13.Get("comparison/hNchMidEta_INELgt0"));
            leg.AddEntry(h13.get(), Form("PYTHIA 13 TeV: #LTdN_{ch}/d#eta#GT_{|#eta|<0.5} = %.2f #pm %.2f",
                                         mid13 ? mid13->GetMean() : 0.0, mid13 ? mid13->GetMeanError() : 0.0), "l");
            leg.Draw();
            save(c, outDir, "dndeta_13p6");
        }
    }

    const Table tPi = readCsv(ref + "/Table_26.csv"), tK = readCsv(ref + "/Table_27.csv"),
                tP = readCsv(ref + "/Table_31.csv");
    const std::vector<std::pair<const char*, std::pair<const Table*, TH1D*>>> meanPt = {
        {"#pi", {&tPi, finePi}}, {"K", {&tK, fineK}}, {"p", {&tP, fineP}}};
    {
        TGraphErrors gData(3), gMc(3);
        for (int i = 0; i < 3; ++i) {
            const auto& row = meanPt[i].second.first->rows.at(0);
            const TH1D* h = meanPt[i].second.second;
            gData.SetPoint(i, i + 1 - 0.08, row[1]);
            gData.SetPointError(i, 0.0, std::hypot(row[2], row[4]));
            gMc.SetPoint(i, i + 1 + 0.08, h->GetMean());
            gMc.SetPointError(i, 0.0, h->GetMeanError());
        }
        TCanvas c("c_meanpt", "meanpt", 800, 600);
        c.SetLeftMargin(0.13);
        TH1D frame("frame_meanpt", "#LTp_{T}#GT, |y|<0.5, 13 TeV;;#LTp_{T}#GT [GeV]", 3, 0.5, 3.5);
        for (int i = 0; i < 3; ++i) frame.GetXaxis()->SetBinLabel(i + 1, meanPt[i].first);
        frame.GetXaxis()->SetLabelSize(0.07);
        frame.SetMinimum(0.3);
        frame.SetMaximum(1.1);
        frame.Draw("AXIS");
        gData.SetMarkerStyle(20);
        gData.Draw("P SAME");
        gMc.SetMarkerStyle(24);
        gMc.SetMarkerColor(kRed + 1);
        gMc.SetLineColor(kRed + 1);
        gMc.Draw("P SAME");
        TLegend leg(0.16, 0.72, 0.60, 0.88);
        leg.SetBorderSize(0);
        leg.SetFillStyle(0);
        leg.AddEntry(&gData, "ALICE (stat. #oplus syst.)", "pe");
        leg.AddEntry(&gMc, "PYTHIA 8.318 Monash", "pe");
        leg.Draw();
        save(c, outDir, "meanpt_13");
    }

    auto* mid = dynamic_cast<TH1D*>(f136.Get("comparison/hNchMidEta_INELgt0"));
    auto* mid13 = dynamic_cast<TH1D*>(f13.Get("comparison/hNchMidEta_INELgt0"));
    if (mid13) {
        printf("\n13 TeV <dNch/deta> |eta|<0.5 INEL>0: MC %.3f +- %.3f, ALICE %.2f +- %.2f, MC/ALICE %.3f\n",
               mid13->GetMean(), mid13->GetMeanError(), kAliceDNdEta13, kAliceDNdEta13Err,
               mid13->GetMean() / kAliceDNdEta13);
    }
    if (mid) {
        const double mc = mid->GetMean(), mcErr = mid->GetMeanError();
        printf("\n13.6 TeV <dNch/deta> |eta|<0.5 INEL>0: MC %.3f +- %.3f, ALICE %.2f +- %.2f, MC/ALICE %.3f, "
               "(MC-ALICE)/sigma %.2f\n",
               mc, mcErr, kAliceDNdEta, kAliceDNdEtaErr, mc / kAliceDNdEta,
               (mc - kAliceDNdEta) / std::hypot(kAliceDNdEtaErr, mcErr));
    }
    printf("13 TeV <pT> |y|<0.5 [GeV]:\n");
    for (const auto& [label, p] : meanPt) {
        const auto& row = p.first->rows.at(0);
        const double err = std::hypot(row[2], row[4]);
        printf("  %-4s MC %.3f +- %.3f, ALICE %.3f +- %.3f, MC/ALICE %.3f, (MC-ALICE)/sigma %.1f\n", label,
               p.second->GetMean(), p.second->GetMeanError(), row[1], err, p.second->GetMean() / row[1],
               (p.second->GetMean() - row[1]) / err);
    }
}
