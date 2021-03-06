# -*- coding: utf-8 -*-

import logging
import Artus.Utility.logger as logger
log = logging.getLogger(__name__)

import CombineHarvester.CombineTools.ch as ch

import HiggsAnalysis.KITHiggsToTauTau.datacards.datacards as datacards
import HiggsAnalysis.KITHiggsToTauTau.plotting.configs.systematics_libary as SystLib


class QcdFactorsDatacards(datacards.Datacards):
	def __init__(self, quantity="m_vis", year="", mapping_category2binid=None, cb=None):
		super(QcdFactorsDatacards, self).__init__(cb)
		
		if mapping_category2binid is not None:
			self.configs._mapping_category2binid.update(mapping_category2binid)

	 	##Generate instance of systematic libary, in which the relevant information about the systematics are safed

		systematics_list = SystLib.SystematicLibary()
					
		all_mc_bkgs = ["ZTT", "ZL", "ZJ", "TTT", "TTJJ", "VVT", "VVJ", "W"]
		all_mc_bkgs_no_W = ["ZTT", "ZL", "ZJ", "TTT", "TTJJ", "VVT", "VVJ"] #don't no whether this is still needed here...
		signal_processes = ["QCD"]
		categories_for_SSOS_factor_estimation = [channel+"_"+bin for channel in ["et", "mt"] for bin in ["ZeroJet2D", "Boosted2D", "dijet2D_lowboost", "dijet2D_boosted"]]
		
		if cb is None:
			# ======================================================================
			# MT channel
			self.add_processes(
					channel="mt",
					categories=categories_for_SSOS_factor_estimation,
					bkg_processes=all_mc_bkgs,
					sig_processes=signal_processes,
					analysis=["htt"],
					era=["13TeV"],
					mass="125"
			)
		
			# efficiencies
			if year == "2016":
				self.cb.cp().channel(["mt"]).process(all_mc_bkgs_no_W).AddSyst(self.cb, *systematics_list.trigger_efficiency2016_syst_args)
				self.cb.cp().channel(["mt"]).process(all_mc_bkgs).AddSyst(self.cb, *systematics_list.muon_efficiency2016_syst_args)
				self.cb.cp().channel(["mt"]).process(["ZTT", "TTT", "VVT"]).AddSyst(self.cb, *systematics_list.tau_efficiency2016_syst_args)
				self.cb.cp().channel(["mt"]).process(["ZTT", "TTT", "VVT"]).AddSyst(self.cb, *systematics_list.tau_es_syst_args)
				self.cb.cp().channel(["mt"]).process(["ZL"]).AddSyst(self.cb, *systematics_list.muFakeTau_tight_syst_args)
			

			# mu->tau fake ES (only for 1-prongs and 1-prong+Pi0s)
			# self.cb.cp().channel(["mt"]).process(["ZL"]).bin(categoriesForMuFakeTauES).AddSyst(self.cb, *systematics_list.muFakeTau_es_syst_args)

			# fake-rate
			# if year == "2016":
			# 	self.cb.cp().channel(["mt"]).process(["ZL"]).bin(categoriesForMuFakeTauES).AddSyst(self.cb, *systematics_list.muFakeTau2016_syst_args)
	
			# ======================================================================
			# ET channel
			self.add_processes(
					channel="et",
					categories=categories_for_SSOS_factor_estimation,
					bkg_processes=all_mc_bkgs,
					sig_processes=signal_processes,
					analysis=["htt"],
					era=["13TeV"],
					mass="125"
			)

			# efficiencies
			if year == "2016":
				self.cb.cp().channel(["et"]).process(all_mc_bkgs_no_W).AddSyst(self.cb, *systematics_list.trigger_efficiency2016_syst_args)
				self.cb.cp().channel(["et"]).process(all_mc_bkgs_no_W).AddSyst(self.cb, *systematics_list.electron_efficiency2016_syst_args)
				self.cb.cp().channel(["et"]).process(["ZTT", "TTT", "VVT"]).AddSyst(self.cb, *systematics_list.tau_efficiency2016_syst_args)

			
			# e->tau fake
			self.cb.cp().channel(["et"]).AddSyst(self.cb, *systematics_list.eleFakeTau_es_syst_args)
			self.cb.cp().channel(["et"]).AddSyst(self.cb, *systematics_list.eFakeTau_1prong_syst_args)
			self.cb.cp().channel(["et"]).AddSyst(self.cb, *systematics_list.eFakeTau_1prong1pizero_syst_args )
			self.cb.cp().channel(["et"]).AddSyst(self.cb, *systematics_list.eleFakeTau_es_syst_args)
			

			# fake-rate
			if year == "2016":
				self.cb.cp().channel(["et"]).process(["ZL"]).AddSyst(self.cb, *systematics_list.eFakeTau2016_syst_args)
				self.cb.cp().channel(["et"]).process(["ZL"]).AddSyst(self.cb, *systematics_list.eFakeTau_tight_syst_args)

			# ======================================================================
			# All channels
		
			# lumi
			# (hopefully) temporary fix
			if year == "2016":
				self.cb.cp().process(all_mc_bkgs_no_W).AddSyst(self.cb, *systematics_list.lumi2016_syst_args)
			
			# cross section
			self.cb.cp().process(["ZTT", "ZLL", "ZL", "ZJ"]).AddSyst(self.cb, *systematics_list.ztt_cross_section_syst_args)
			if year == "2016":
				self.cb.cp().process(["VVT", "VVJ"]).AddSyst(self.cb, *systematics_list.vv_cross_section2016_syst_args)			
			self.cb.cp().process(["TTT", "TTJJ"]).AddSyst(self.cb, *systematics_list.ttj_cross_section_syst_args)
			self.cb.cp().process(["W"]).AddSyst(self.cb, *systematics_list.wj_cross_section_syst_args)

			# Normalizations
			self.cb.cp().process(["W"]).AddSyst(self.cb, *systematics_list.htt_wnorm_syst_args)
			self.cb.cp().process(["TTT", "TTJ"]).AddSyst(self.cb, *systematics_list.htt_ttnorm_syst_args)
			self.cb.cp().process(["VVJ", "VVT"]).AddSyst(self.cb, *systematics_list.htt_vvnorm_syst_args)
			
			# signal acceptance/efficiency
			self.cb.cp().process(["ZTT"]).AddSyst(self.cb, *systematics_list.ztt_pdf_scale_syst_args)
			self.cb.cp().process(["ZTT"]).AddSyst(self.cb, *systematics_list.ztt_qcd_scale_syst_args)

			# W+jets high->low mt extrapolation uncertainty
			self.cb.cp().channel(["mt", "et"]).process(["W"]).AddSyst(self.cb, "WHighMTtoLowMT_13TeV", "lnN", ch.SystMap()(1.10))

			if year == "2016":
				self.cb.cp().channel(["mt", "et"]).process(["ZTT", "TTT", "VVT"]).AddSyst(self.cb, *systematics_list.tau_efficiency2016_corr_syst_args)

			# jet->tau fakes
			if year == "2016":
				self.cb.cp().channel(["mt", "et"]).process(["ZJ", "TTJJ", "VVJ"]).AddSyst(self.cb, "CMS_htt_jetToTauFake_13TeV", "shape", ch.SystMap()(1.0))
				
			if log.isEnabledFor(logging.DEBUG):
				self.cb.PrintAll()
