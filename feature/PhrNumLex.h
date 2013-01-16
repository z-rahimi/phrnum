#ifndef moses_PhrNumLex_h
#define moses_PhrNumLex_h

#include <string>
#include <vector>
#include "Factor.h"
#include "Phrase.h"
#include "TypeDef.h"
#include "Util.h"
#include "WordsRange.h"
#include "ScoreProducer.h"
#include "FeatureFunction.h"

#include "LexicalReorderingState.h"
#include "LexicalReorderingTable.h"

using namespace std;
namespace Moses
{

class PhrNumLex : public StatefulFeatureFunction {
public:   
    PhrNumLex(std::vector<FactorType>& f_factors, std::vector<FactorType>& e_factors,/*const string& lexfilePath1,
		      const string& nonParaPhrNumfilePath, const string& paraPhrNumfilePath,*/ const string& lexfilePath, const string& modeltype, const std::vector< float > weights);
    virtual ~PhrNumLex();
    
    virtual size_t GetNumScoreComponents() const {
      return/* m_configuration.GetNumScoreComponents() +*/ 2;
    }
    
    virtual std::string GetScoreProducerDescription() const {
        return "PhrNumLex";
    }
    
    std::string GetScoreProducerWeightShortName() const {
        return "pl";
    }
    
//     Scores GetProbLex(const Phrase& f, const Phrase& e) const;
    
    virtual FFState* Evaluate(const Hypothesis& cur_hypo,
                              const FFState* prev_state,
                              ScoreComponentCollection* accumulator) const;
			  
     const FFState* EmptyHypothesisState(const InputType &input) const;
//void EmptyHypothesisState(const InputType &input);			      
    
   
    
    LexicalReorderingConfiguration m_configuration;
    std::string m_modelTypeString;
    std::vector<std::string> m_modelType;
    LexicalReorderingTable* m_table;
    size_t m_numScoreComponents;
    std::vector<LexicalReorderingConfiguration::Condition> m_condition;
    std::vector<FactorType> m_factorsE, m_factorsF;
    int number;
    
};

}

#endif
