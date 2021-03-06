
#include "DataFormats/TauReco/interface/PFTau.h"

#include "Artus/Consumer/interface/LambdaNtupleConsumer.h"
#include "Artus/Utility/interface/DefaultValues.h"
#include "Artus/Utility/interface/SafeMap.h"
#include "Artus/Utility/interface/Utility.h"

#include "HiggsAnalysis/KITHiggsToTauTau/interface/Producers/PolarisationQuantitiesProducers.h"

#include "TauPolSoftware/TauDecaysInterface/interface/TauPolInterface.h"

#include <Math/VectorUtil.h>


PolarisationQuantitiesProducerBase::PolarisationQuantitiesProducerBase(
		std::string name,
		std::map<KLepton*, RMFLV> product_type::*fittedTausMember,
		std::map<KLepton*, float> product_type::*polarisationOmegasMember,
		std::map<KLepton*, float> product_type::*polarisationOmegaBarsMember,
		std::map<KLepton*, float> product_type::*polarisationOmegaVisiblesMember,
		float product_type::*polarisationCombinedOmegaMember,
		float product_type::*polarisationCombinedOmegaBarMember,
		float product_type::*polarisationCombinedOmegaVisibleMember,
		bool genMatched
) :
	m_name(name),
	m_fittedTausMember(fittedTausMember),
	m_polarisationOmegasMember(polarisationOmegasMember),
	m_polarisationOmegaBarsMember(polarisationOmegaBarsMember),
	m_polarisationOmegaVisiblesMember(polarisationOmegaVisiblesMember),
	m_polarisationCombinedOmegaMember(polarisationCombinedOmegaMember),
	m_polarisationCombinedOmegaBarMember(polarisationCombinedOmegaBarMember),
	m_polarisationCombinedOmegaVisibleMember(polarisationCombinedOmegaVisibleMember),
	m_genMatched(genMatched)
{
}

void PolarisationQuantitiesProducerBase::Init(setting_type const& settings, metadata_type& metadata)
{
	ProducerBase<HttTypes>::Init(settings, metadata);
	
	// add possible quantities for the lambda ntuples consumers
	for (size_t leptonIndex = 0; leptonIndex < 2; ++leptonIndex)
	{
		std::string namePostfix = m_name+"_" + std::to_string(leptonIndex+1);
		
		LambdaNtupleConsumer<HttTypes>::AddFloatQuantity(metadata, "polarisationOmega"+namePostfix, [leptonIndex, this](event_type const& event, product_type const& product) {
			return SafeMap::GetWithDefault((product.*m_polarisationOmegasMember), product.m_flavourOrderedLeptons.at(leptonIndex), DefaultValues::UndefinedFloat);
		});
		
		LambdaNtupleConsumer<HttTypes>::AddFloatQuantity(metadata, "polarisationOmegaBar"+namePostfix, [leptonIndex, this](event_type const& event, product_type const& product) {
			return SafeMap::GetWithDefault((product.*m_polarisationOmegaBarsMember), product.m_flavourOrderedLeptons.at(leptonIndex), DefaultValues::UndefinedFloat);
		});
		
		LambdaNtupleConsumer<HttTypes>::AddFloatQuantity(metadata, "polarisationOmegaVisible"+namePostfix, [leptonIndex, this](event_type const& event, product_type const& product) {
			return SafeMap::GetWithDefault((product.*m_polarisationOmegaVisiblesMember), product.m_flavourOrderedLeptons.at(leptonIndex), DefaultValues::UndefinedFloat);
		});
	}
	
	LambdaNtupleConsumer<HttTypes>::AddFloatQuantity(metadata, "polarisationCombinedOmega"+m_name, [this](event_type const& event, product_type const& product) {
		return (product.*m_polarisationCombinedOmegaMember);
	});
	
	LambdaNtupleConsumer<HttTypes>::AddFloatQuantity(metadata, "polarisationCombinedOmegaBar"+m_name, [this](event_type const& event, product_type const& product) {
		return (product.*m_polarisationCombinedOmegaBarMember);
	});
	
	LambdaNtupleConsumer<HttTypes>::AddFloatQuantity(metadata, "polarisationCombinedOmegaVisible"+m_name, [this](event_type const& event, product_type const& product) {
		return (product.*m_polarisationCombinedOmegaVisibleMember);
	});
}

void PolarisationQuantitiesProducerBase::Produce(
		event_type const& event,
		product_type& product,
		setting_type const& settings, metadata_type const& metadata
) const
{
	assert(product.m_flavourOrderedLeptons.size() > 1);
	
	std::vector<std::vector<TLorentzVector> > inputs;
	std::vector<std::string> types;
	std::vector<int> charges;
	
	for (std::vector<KLepton*>::iterator lepton = product.m_flavourOrderedLeptons.begin();
		 lepton != product.m_flavourOrderedLeptons.end(); ++lepton)
	{

		if (((*lepton)->flavour() == KLeptonFlavour::ELECTRON) || ((*lepton)->flavour() == KLeptonFlavour::MUON))
		{
			inputs.push_back(GetInputLepton(product, *lepton, m_genMatched));
			types.push_back("lepton");
			charges.push_back((*lepton)->charge());
		}
		else if ((*lepton)->flavour() == KLeptonFlavour::TAU)
		{
			KTau* tau = static_cast<KTau*>(*lepton);
			if ((tau->decayMode == reco::PFTau::hadronicDecayMode::kThreeProng0PiZero) && (tau->chargedHadronCandidates.size() > 2))
			{
				inputs.push_back(GetInputA1(product, *lepton, m_genMatched));
				types.push_back("a1");
				charges.push_back((*lepton)->charge());
			}
			else if ((tau->decayMode == reco::PFTau::hadronicDecayMode::kOneProng1PiZero) &&
			         (tau->chargedHadronCandidates.size() > 0) &&
			         ((tau->piZeroCandidates.size() > 0) || (tau->gammaCandidates.size() > 0)))
			{
				inputs.push_back(GetInputRho(product, *lepton, m_genMatched));
				types.push_back("rho");
				charges.push_back((*lepton)->charge());
			}
			else
			{
				inputs.push_back(GetInputPion(product, *lepton, m_genMatched));
				types.push_back("pion");
				charges.push_back((*lepton)->charge());
			}
		}
		

		if (inputs.back().size() > 0)
		{
			TauPolInterface singleTauPolInterface(inputs.back(), types.back(), charges.back());
			if (singleTauPolInterface.isConfigured())
			{
				(product.*m_polarisationOmegasMember)[*lepton] = singleTauPolInterface.getOmega();
				(product.*m_polarisationOmegaBarsMember)[*lepton] = singleTauPolInterface.getOmegabar();
				(product.*m_polarisationOmegaVisiblesMember)[*lepton] = singleTauPolInterface.getVisibleOmega();
			}
		}
	}
	
	if ((inputs.at(0).size() > 0) && (inputs.at(1).size() > 0))
	{
		TauPolInterface diTauPolInterface(inputs.at(0), types.at(0), inputs.at(1), types.at(1), charges.at(0), charges.at(1));
		if (diTauPolInterface.isPairConfigured())
		{
			(product.*m_polarisationCombinedOmegaMember) = diTauPolInterface.getCombOmega();
			(product.*m_polarisationCombinedOmegaBarMember) = diTauPolInterface.getCombOmegaBar();
			(product.*m_polarisationCombinedOmegaVisibleMember) = diTauPolInterface.getCombVisibleOmega();
		}
	}
}

std::vector<TLorentzVector> PolarisationQuantitiesProducerBase::GetInputLepton(product_type& product, KLepton* lepton, bool genMatched) const
{
	std::vector<TLorentzVector> input;
	
	if (genMatched)
	{
		size_t leptonIndex = Utility::Index(product.m_flavourOrderedLeptons, lepton);
		if (product.m_genLeptonsFromBosonDecay.size() > leptonIndex)
		{
			KGenParticle* genParticle = product.m_genLeptonsFromBosonDecay.at(leptonIndex);
			if (std::abs(genParticle->pdgId) == DefaultValues::pdgIdTau)
			{
				RMFLV* genTauLV = &(genParticle->p4);
			
				KGenTau* genTau = SafeMap::GetWithDefault(product.m_validGenTausMap, genParticle, static_cast<KGenTau*>(nullptr));
				if (genTau)
				{
					RMFLV* genTauVisibleLV = &(genTau->visible.p4);
					
					input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(*genTauLV));
					input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(*genTauVisibleLV));
				}
			}
		}
	}
	else if (Utility::Contains((product.*m_fittedTausMember), lepton))
	{
		input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(SafeMap::Get((product.*m_fittedTausMember), lepton)));
		input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(lepton->p4));
	}
	
	return input;
}

std::vector<TLorentzVector> PolarisationQuantitiesProducerBase::GetInputPion(product_type& product, KLepton* lepton, bool genMatched) const
{
	std::vector<TLorentzVector> input;
	
	if (genMatched)
	{
		size_t leptonIndex = Utility::Index(product.m_flavourOrderedLeptons, lepton);
		if (product.m_genLeptonsFromBosonDecay.size() > leptonIndex)
		{
			KGenParticle* genParticle = product.m_genLeptonsFromBosonDecay.at(leptonIndex);
			if (std::abs(genParticle->pdgId) == DefaultValues::pdgIdTau)
			{
				RMFLV* genTauLV = &(genParticle->p4);
			
				KGenTau* genTau = SafeMap::GetWithDefault(product.m_validGenTausMap, genParticle, static_cast<KGenTau*>(nullptr));
				if (genTau)
				{
					RMFLV* genTauVisibleLV = &(genTau->visible.p4);
					
					input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(*genTauLV));
					input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(*genTauVisibleLV));
				}
			}
		}
	}
	else if (Utility::Contains((product.*m_fittedTausMember), lepton))
	{
		input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(SafeMap::Get((product.*m_fittedTausMember), lepton)));
		input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(lepton->p4));
	}
	
	return input;
}

std::vector<TLorentzVector> PolarisationQuantitiesProducerBase::GetInputRho(product_type& product, KLepton* lepton, bool genMatched) const
{
	std::vector<TLorentzVector> input;
	
	if (genMatched)
	{
		size_t leptonIndex = Utility::Index(product.m_flavourOrderedLeptons, lepton);
		if (product.m_genLeptonsFromBosonDecay.size() > leptonIndex)
		{
			KGenParticle* genParticle = product.m_genLeptonsFromBosonDecay.at(leptonIndex);
			if (std::abs(genParticle->pdgId) == DefaultValues::pdgIdTau)
			{
				RMFLV* genTauLV = &(genParticle->p4);
			
				KGenTau* genTau = SafeMap::GetWithDefault(product.m_validGenTausMap, genParticle, static_cast<KGenTau*>(nullptr));
				std::vector<KGenParticle*> genTauChargedHadrons = SafeMap::GetWithDefault(product.m_validGenTausChargedHadronsMap, genParticle, std::vector<KGenParticle*>());
				std::vector<KGenParticle*> genTauNeutralHadrons = SafeMap::GetWithDefault(product.m_validGenTausNeutralHadronsMap, genParticle, std::vector<KGenParticle*>());
				if (genTau &&
				    (genTau->nProngs == 1) && (genTau->nPi0s == 1) &&
				    (genTauChargedHadrons.size() == 1) && (genTauNeutralHadrons.size() == 1))
				{
					input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(*genTauLV));
					
					input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(genTauChargedHadrons.front()->p4));
					input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(genTauNeutralHadrons.front()->p4));
				}
			}
		}
	}
	else if (Utility::Contains((product.*m_fittedTausMember), lepton))
	{
		input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(SafeMap::Get((product.*m_fittedTausMember), lepton)));
		
		KTau* tau = static_cast<KTau*>(lepton);
		input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(tau->sumChargedHadronCandidates()));
		input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(tau->piZeroMomentum()));
	}
	
	return input;
}

std::vector<TLorentzVector> PolarisationQuantitiesProducerBase::GetInputA1(product_type& product, KLepton* lepton, bool genMatched) const
{
	std::vector<TLorentzVector> input;
	
	if (genMatched)
	{
		size_t leptonIndex = Utility::Index(product.m_flavourOrderedLeptons, lepton);
		if (product.m_genLeptonsFromBosonDecay.size() > leptonIndex)
		{
			KGenParticle* genParticle = product.m_genLeptonsFromBosonDecay.at(leptonIndex);
			if (std::abs(genParticle->pdgId) == DefaultValues::pdgIdTau)
			{
				RMFLV* genTauLV = &(genParticle->p4);
			
				KGenTau* genTau = SafeMap::GetWithDefault(product.m_validGenTausMap, genParticle, static_cast<KGenTau*>(nullptr));
				std::vector<KGenParticle*> genTauChargedHadrons = SafeMap::GetWithDefault(product.m_validGenTausChargedHadronsMap, genParticle, std::vector<KGenParticle*>());
				std::vector<KGenParticle*> genTauNeutralHadrons = SafeMap::GetWithDefault(product.m_validGenTausNeutralHadronsMap, genParticle, std::vector<KGenParticle*>());
				if (genTau &&
				    (genTau->nProngs == 3) && (genTau->nPi0s == 0) &&
				    (genTauChargedHadrons.size() == 3) && (genTauNeutralHadrons.size() == 0))
				{
					input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(*genTauLV));
					
					// sort pions from a1 decay according to their charge
					RMFLV* piSingleChargeSign = nullptr;
					RMFLV* piDoubleChargeSign1 = nullptr;
					RMFLV* piDoubleChargeSign2 = nullptr;
					if ((genTauChargedHadrons.at(0)->charge() * genTauChargedHadrons.at(1)->charge()) > 0.0)
					{
						piSingleChargeSign = &(genTauChargedHadrons.at(2)->p4);
						piDoubleChargeSign1 = &(genTauChargedHadrons.at(0)->p4);
						piDoubleChargeSign2 = &(genTauChargedHadrons.at(1)->p4);
					}
					else if ((genTauChargedHadrons.at(0)->charge() * genTauChargedHadrons.at(2)->charge()) > 0.0)
					{
						piSingleChargeSign = &(genTauChargedHadrons.at(1)->p4);
						piDoubleChargeSign1 = &(genTauChargedHadrons.at(0)->p4);
						piDoubleChargeSign2 = &(genTauChargedHadrons.at(2)->p4);
					}
					else // if ((genTauChargedHadrons.at(1)->charge() * genTauChargedHadrons.at(2)->charge()) > 0.0)
					{
						piSingleChargeSign = &(genTauChargedHadrons.at(0)->p4);
						piDoubleChargeSign1 = &(genTauChargedHadrons.at(1)->p4);
						piDoubleChargeSign2 = &(genTauChargedHadrons.at(2)->p4);
					}
		
					input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(*piSingleChargeSign));
					input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(*piDoubleChargeSign1));
					input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(*piDoubleChargeSign2));
				}
			}
		}
	}
	else if (Utility::Contains((product.*m_fittedTausMember), lepton))
	{
		input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(SafeMap::Get((product.*m_fittedTausMember), lepton)));
		
		KTau* tau = static_cast<KTau*>(lepton);
		assert(tau->chargedHadronCandidates.size() > 2);
		
		// sort pions from a1 decay according to their charge
		RMFLV* piSingleChargeSign = nullptr;
		RMFLV* piDoubleChargeSign1 = nullptr;
		RMFLV* piDoubleChargeSign2 = nullptr;
		if ((tau->chargedHadronCandidates.at(0).charge() * tau->chargedHadronCandidates.at(1).charge()) > 0.0)
		{
			piSingleChargeSign = &(tau->chargedHadronCandidates.at(2).p4);
			piDoubleChargeSign1 = &(tau->chargedHadronCandidates.at(0).p4);
			piDoubleChargeSign2 = &(tau->chargedHadronCandidates.at(1).p4);
		}
		else if ((tau->chargedHadronCandidates.at(0).charge() * tau->chargedHadronCandidates.at(2).charge()) > 0.0)
		{
			piSingleChargeSign = &(tau->chargedHadronCandidates.at(1).p4);
			piDoubleChargeSign1 = &(tau->chargedHadronCandidates.at(0).p4);
			piDoubleChargeSign2 = &(tau->chargedHadronCandidates.at(2).p4);
		}
		else // if ((tau->chargedHadronCandidates.at(1).charge() * tau->chargedHadronCandidates.at(2).charge()) > 0.0)
		{
			piSingleChargeSign = &(tau->chargedHadronCandidates.at(0).p4);
			piDoubleChargeSign1 = &(tau->chargedHadronCandidates.at(1).p4);
			piDoubleChargeSign2 = &(tau->chargedHadronCandidates.at(2).p4);
		}
		
		input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(*piSingleChargeSign));
		input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(*piDoubleChargeSign1));
		input.push_back(Utility::ConvertPtEtaPhiMLorentzVector<RMFLV, TLorentzVector>(*piDoubleChargeSign2));
	}
	
	return input;
}


GenMatchedPolarisationQuantitiesProducer::GenMatchedPolarisationQuantitiesProducer() :
	PolarisationQuantitiesProducerBase(
			"GenMatched",
			&product_type::m_svfitTaus, // this map is not used and can be empty
			&product_type::m_polarisationOmegasGenMatched,
			&product_type::m_polarisationOmegaBarsGenMatched,
			&product_type::m_polarisationOmegaVisiblesGenMatched,
			&product_type::m_polarisationCombinedOmegaGenMatched,
			&product_type::m_polarisationCombinedOmegaBarGenMatched,
			&product_type::m_polarisationCombinedOmegaVisibleGenMatched,
			true
	)
{
}

std::string GenMatchedPolarisationQuantitiesProducer::GetProducerId() const
{
	return "GenMatchedPolarisationQuantitiesProducer";
}


PolarisationQuantitiesSvfitProducer::PolarisationQuantitiesSvfitProducer() :
	PolarisationQuantitiesProducerBase(
			"Svfit",
			&product_type::m_svfitTaus,
			&product_type::m_polarisationOmegasSvfit,
			&product_type::m_polarisationOmegaBarsSvfit,
			&product_type::m_polarisationOmegaVisiblesSvfit,
			&product_type::m_polarisationCombinedOmegaSvfit,
			&product_type::m_polarisationCombinedOmegaBarSvfit,
			&product_type::m_polarisationCombinedOmegaVisibleSvfit,
			false
	)
{
}

std::string PolarisationQuantitiesSvfitM91Producer::GetProducerId() const
{
	return "PolarisationQuantitiesSvfitProducer";
}


PolarisationQuantitiesSvfitM91Producer::PolarisationQuantitiesSvfitM91Producer() :
	PolarisationQuantitiesProducerBase(
			"SvfitM91",
			&product_type::m_svfitM91Taus,
			&product_type::m_polarisationOmegasSvfitM91,
			&product_type::m_polarisationOmegaBarsSvfitM91,
			&product_type::m_polarisationOmegaVisiblesSvfitM91,
			&product_type::m_polarisationCombinedOmegaSvfitM91,
			&product_type::m_polarisationCombinedOmegaBarSvfitM91,
			&product_type::m_polarisationCombinedOmegaVisibleSvfitM91,
			false
	)
{
}

std::string PolarisationQuantitiesSvfitProducer::GetProducerId() const
{
	return "PolarisationQuantitiesSvfitM91Producer";
}


PolarisationQuantitiesSimpleFitProducer::PolarisationQuantitiesSimpleFitProducer() :
	PolarisationQuantitiesProducerBase(
			"SimpleFit",
			&product_type::m_simpleFitTaus,
			&product_type::m_polarisationOmegasSimpleFit,
			&product_type::m_polarisationOmegaBarsSimpleFit,
			&product_type::m_polarisationOmegaVisiblesSimpleFit,
			&product_type::m_polarisationCombinedOmegaSimpleFit,
			&product_type::m_polarisationCombinedOmegaBarSimpleFit,
			&product_type::m_polarisationCombinedOmegaVisibleSimpleFit,
			false
	)
{
}

std::string PolarisationQuantitiesSimpleFitProducer::GetProducerId() const
{
	return "PolarisationQuantitiesSimpleFitProducer";
}


/*
PolarisationQuantitiesHHKinFitProducer::PolarisationQuantitiesHHKinFitProducer() :
	PolarisationQuantitiesProducerBase(
			"HHKinFit",
			&product_type::m_hhKinFitTaus,
			&product_type::m_polarisationOmegasHHKinFit,
			&product_type::m_polarisationOmegaBarsHHKinFit,
			&product_type::m_polarisationOmegaVisiblesHHKinFit,
			&product_type::m_polarisationCombinedOmegaHHKinFit,
			&product_type::m_polarisationCombinedOmegaBarHHKinFit,
			&product_type::m_polarisationCombinedOmegaVisibleHHKinFit,
			false
	)
{
}

std::string PolarisationQuantitiesHHKinFitProducer::GetProducerId() const
{
	return "PolarisationQuantitiesHHKinFitProducer";
}
*/
