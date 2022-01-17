/*
 *  CepGen: a central exclusive processes event generator
 *  Copyright (C) 2013-2022  Laurent Forthomme
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <TH1.h>

#include "CepGen/Cards/Handler.h"
#include "CepGen/Core/Exception.h"
#include "CepGen/Event/Event.h"
#include "CepGen/Generator.h"
#include "CepGen/Parameters.h"
#include "CepGen/Utils/ArgumentsParser.h"
#include "CepGenAddOns/ROOTWrapper/ROOTCanvas.h"

using namespace std;

unique_ptr<TH1D> h_mass, h_ptpair, h_ptsingle, h_etasingle;
void process_event(const cepgen::Event& ev, unsigned long event_id) {
  CG_LOG << event_id;
  const auto& central_system = ev[cepgen::Particle::CentralSystem];
  const auto &pl1 = central_system[0].momentum(), pl2 = central_system[1].momentum();
  h_mass->Fill((pl1 + pl2).mass());
  h_ptpair->Fill((pl1 + pl2).pt());
  h_ptsingle->Fill(pl1.pt());
  h_etasingle->Fill(pl1.eta());
}

int main(int argc, char* argv[]) {
  cepgen::Generator mg;

  string input_card;
  int num_events;
  bool draw_grid;

  cepgen::ArgumentsParser(argc, argv)
      .addArgument("input,i", "input card", &input_card)
      .addOptionalArgument("num-events,n", "number of events to generate", &num_events, 100)
      .addOptionalArgument("draw-grid,g", "draw the x/y grid", &draw_grid, false)
      .parse();

  mg.setParameters(cepgen::card::Handler::parse(input_card));

  h_mass.reset(new TH1D("invm", ";Dilepton invariant mass;d#sigma/dM (pb/GeV)", 500, 0., 500.));
  h_ptpair.reset(new TH1D("ptpair", ";Dilepton p_{T};d#sigma/dp_{T} (pb/GeV)", 500, 0., 50.));
  h_ptsingle.reset(new TH1D("pt_single", ";Single lepton p_{T};d#sigma/dp_{T} (pb/GeV)", 100, 0., 100.));
  h_etasingle.reset(new TH1D("eta_single", ";Single lepton #eta;d#sigma/d#eta (pb)\\?.2f", 60, -3., 3.));

  CG_LOG << "Process name: " << mg.parameters()->processName() << ".";

  mg.generate(num_events, process_event);

  const double weight = mg.crossSection() / num_events;
  h_mass->Scale(weight, "width");
  h_ptpair->Scale(weight, "width");
  h_ptsingle->Scale(weight, "width");
  h_etasingle->Scale(weight, "width");

  const unordered_map<const char*, TH1D&> plots = {
      {"dilepton_invm", *h_mass},
      {"dilepton_ptpair", *h_ptpair},
      {"singlelepton_pt", *h_ptsingle},
      {"singlelepton_eta", *h_etasingle},
  };
  for (auto& plt : plots) {
    cepgen::ROOTCanvas c(plt.first, "CepGen Simulation");
    if (draw_grid)
      c.SetGrid(true, true);
    plt.second.Draw("hist");
    c.Prettify(&plt.second);
    c.SetLogy();
    c.Save("pdf");
  }

  return 0;
}
