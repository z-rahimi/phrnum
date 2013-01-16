#include "PhrNumLex.h"
#include <sstream>
#include "FFState.h"
#include "LexicalReorderingState.h"
#include "StaticData.h"

namespace Moses
{
  PhrNumLex::PhrNumLex(std::vector<FactorType>& f_factors,
		       std::vector<FactorType>& e_factors, /*const string& lexfilePath1,
		      const string& nonParaPhrNumfilePath, const string& paraPhrNumfilePath,*/const string& filePath, const string& modeltype, const std::vector< float > weights):
    m_configuration(this, modeltype){
    
    VERBOSE(3, "enter to PhrNumLex\n");
    
    switch(m_configuration.GetCondition()){
        case LexicalReorderingConfiguration::FE:
        case LexicalReorderingConfiguration::E:
            m_factorsE = e_factors;
            if(m_factorsE.empty()){
                UserMessage::Add("TL factor mask for lexical reordering is unexpectedly empty");
                exit(1);
            }
            if(m_configuration.GetCondition() == LexicalReorderingConfiguration::E)
                break; // else fall through
        case LexicalReorderingConfiguration::F:
            m_factorsF = f_factors;
            if(m_factorsF.empty()){
                UserMessage::Add("SL factor mask for lexical reordering is unexpectedly empty");
                exit(1);
            }
            break;
        default:
            UserMessage::Add("Unknown conditioning option!");
            exit(1);
    }
    
    const_cast<ScoreIndexManager&>(StaticData::Instance().GetScoreIndexManager()).AddScoreProducer(this);
    const_cast<StaticData&>(StaticData::Instance()).SetWeightsForScoreProducer(this, weights);
    VERBOSE(3, "scoreProducer finish\n");
    
    m_table = LexicalReorderingTable::LoadAvailable(filePath ,/*lexfilePath1, */"", "" , m_factorsF, m_factorsE, std::vector<FactorType>(), "phrnumlex");
    VERBOSE(1, "load data was finished\n");
    m_configuration.setPhrNum(m_table->m_nonParaPhrNum, m_table->m_average, m_table->m_variance);  
  }
  
  PhrNumLex::~PhrNumLex() {}
  

/*Scores PhrNumLex::GetProbLex(const Phrase& f, const Phrase& e) const {
    return m_table->GetScore(f, e, Phrase(Output));
}*/

  
 FFState* PhrNumLex::Evaluate(const Hypothesis& hypo,
                                     const FFState* prev_state,
                                     ScoreComponentCollection* out) const  {	
    VERBOSE(3, "Coil::Evaluate\n");
    const int segmentNum = hypo.GetNumSegment();
    //m_configuration.SetNumSegment(numSegment);
    Scores score(GetNumScoreComponents(), 0);
    const LexicalReorderingState *prev = dynamic_cast<const LexicalReorderingState *>(prev_state);
    LexicalReorderingState *next_state = prev->CoilExpand(hypo.GetTranslationOption(), score, segmentNum);
    for(int i =0 ;i<score.size();i++){
      VERBOSE(3,"score:"<< score.at(i) << " ,");
    }
    VERBOSE(3,""<< "\n");
    
    out->PlusEquals(this, score);
    return next_state;
  }
  
  const FFState* PhrNumLex::EmptyHypothesisState(const InputType &input) const {
    VERBOSE(3, "Coil::EmptyHypothesisState\n");
    return m_configuration.CreateLexicalReorderingState(input);
   }
}