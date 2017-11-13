# -*- coding: utf-8 -*-
import logging
import Artus.Utility.logger as logger
import Artus.Utility.tools as tools
log = logging.getLogger(__name__)

import HiggsAnalysis.KITHiggsToTauTau.datacards.datacards as datacards
import CombineHarvester.CombineTools.ch as ch



class LFVDatacards(datacards.Datacards):
	def __init__(self, channel_list, signal_list, category_list, cb=None, lnN_syst_enable = False, shape_syst_enable = False):
		super(LFVDatacards, self).__init__(cb)
		
		if cb is None:
			
			###Define all background processes for each channel

			backgrounds = {
					"em": ["ZTT", "ZLL", "EWKZ", "TT", "VV", "hww_gg125", "hww_qq125", "W", "QCD"],
					"et": ["ZTT", "ZL", "ZJ", "EWKZ", "TTT", "TTJJ", "VV", "W", "QCD"],
			}
			
			all_mc_bkgs = ["ZTT", "ZL", "ZJ", "ZLL", "EWKZ", "TT", "TTT", "TTJJ", "VV", "VVT", "VVJ", "W", "hww_gg125", "hww_qq125"]
			all_mc_bkgs_no_W = ["ZTT", "ZL", "ZJ", "ZLL", "EWKZ", "TT", "TTT", "TTJJ", "VV", "VVT", "VVJ", "hww_gg125", "hww_qq125"]
			all_mc_bkgs_no_TTJ = ["ZTT", "ZL", "ZJ", "ZLL", "EWKZ", "TT", "TTT", "VV", "VVT", "VVJ", "W", "hww_gg125", "hww_qq125"]


			###Define directory for lnN and shape uncertanty, in which each channel has saved the systematic + process + category. If systematic is not specific for one category, empty string is given. 
			##The values of each systematic are saved in datacard.py
	
			lnN_syst = {
					"em": [
						[self.trigger_efficiency2016_em_syst_args, 	signal_list+all_mc_bkgs,		""],
						[self.electron_efficiency2016_syst_args, 	signal_list+all_mc_bkgs,		""],
						[self.muon_efficiency2016_syst_args,		signal_list+all_mc_bkgs, 		""],
						[self.btag_efficiency2016_syst_args,		["TT", "VV"], 				["LFVZeroJet", "LFVJet"]],
						[self.lumi2016_syst_args, 			["ZL", "ZJ", "ZLL"], 			""],
					],
	
					"et": [
						[self.trigger_efficiency2016_em_syst_args, 	signal_list+all_mc_bkgs_no_W,		""],
						[self.electron_efficiency2016_syst_args, 	signal_list+all_mc_bkgs_no_W,		""],
						[self.tau_efficiency2016_syst_args,		signal_list+all_mc_bkgs, 		""],
						[self.btag_efficiency2016_syst_args,		["TT", "VV"], 				["LFVZeroJet", "LFVJet"]],
						[self.lumi2016_syst_args, 			["ZL", "ZJ", "ZLL", "ZTT"], 		""],
						[self.tau_efficiency2016_corr_syst_args, 	signal_list+all_mc_bkgs, 		""]
					]
			}	
						
						 
			shape_syst = {}			
			

			###Fill Combine Harvester object with relevant information
			
			for channel in channel_list:
			
				###Add channels as process in Combine Harvester			
	
				self.add_processes(
					channel=channel,
					categories=category_list[0],
					bkg_processes=backgrounds[channel],  
					sig_processes=signal_list,
					analysis=["LFV"],
					era=["13TeV"],
					mass=["125"]
					)					

				###Add lnN/shape uncertanty for each channel, process and category

				if lnN_syst_enable:
					for (systematic, process, category) in lnN_syst[channel]:
						if category == "":
							self.cb.cp().channel([channel]).process(process).AddSyst(self.cb, *systematic)
						else:
							self.cb.cp().channel([channel]).process(process).bin(category).AddSyst(self.cb, *systematic)
			
				if shape_syst_enable:
					for (systematic, process) in shape_syst[channel]:
						if category == "":
							self.cb.cp().channel([channel]).process(process).AddSyst(self.cb, *systematic)
						else:
							self.cb.cp().channel([channel]).process(process).bin(category).AddSyst(self.cb, *systematic)
			

	