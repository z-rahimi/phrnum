
#pragma once

#include <vector>
#include <string>

#include "FFState.h"
#include "Hypothesis.h"
//#include "LexicalReordering.h"
#include "WordsRange.h"
#include "WordsBitmap.h"
#include "ReorderingStack.h"
#include "TranslationOption.h"


namespace Moses
{
class LexicalReorderingState;

//! Factory class for lexical reordering states
class LexicalReorderingConfiguration {
  public:
    enum ModelType {Monotonic, MSD, MSLR, LeftRight, None};
    enum Direction {Forward, Backward, Bidirectional};
    enum Condition {F, E, FE};
    
    LexicalReorderingConfiguration(ScoreProducer *scoreProducer, const std::string &modelType);
   
    LexicalReorderingState *CreateLexicalReorderingState(const InputType &input) const;
    
    size_t GetNumScoreComponents() const;
    size_t GetNumberOfTypes() const;
    
    ScoreProducer *GetScoreProducer() const {
      return m_scoreProducer;
    }
    
    ModelType GetModelType() const {
      return m_modelType;
    }
    
    Direction GetDirection() const {
      return m_direction;
    }
    //my code: phrNum
    void setPhrNum(std::map<int, std::map<int, std::vector<float> > > nonParaPhrNum, 
		   std::map<int, std::vector<float> > average,
		   std::map<int, std::vector<float> > variance) {
      m_average = average;
      m_variance = variance;
      m_nonParaPhrNum = nonParaPhrNum;
    }
    //end
    
    Condition GetCondition() const {
      return m_condition;
    }
    
    bool IsPhraseBased() const {
      return m_phraseBased;
    }
    
    bool CollapseScores() const {
      return m_collapseScores;
    }
    
  private:
    ScoreProducer *m_scoreProducer;
    ModelType m_modelType;
    bool m_phraseBased;
    bool m_collapseScores;
    Direction m_direction;
    Condition m_condition;
    
public:
  std::map<int, std::map<int, std::vector<float> > > m_nonParaPhrNum;
    std::map<int, std::vector<float> > m_average, m_variance;
};

  //! Abstract class for lexical reordering model states
class LexicalReorderingState : public FFState {
  public:

    virtual int Compare(const FFState& o) const = 0;
    virtual LexicalReorderingState* Expand(const TranslationOption& hypo, Scores& scores) const = 0;
    //my code: Coil
    virtual LexicalReorderingState* CoilExpand(const TranslationOption& topt, Scores& scores, int segNum) const {};
    //virtual LexicalReorderingState* BWCExpand(const TranslationOption& topt, Scores& scores) const {};
    //

    static LexicalReorderingState* CreateLexicalReorderingState(const std::vector<std::string>& config,
								LexicalReorderingConfiguration::Direction dir, const InputType &input);

  protected:
    typedef int ReorderingType;
    
    const LexicalReorderingConfiguration &m_configuration;
    // The following is the true direction of the object, which can be Backward or Forward even if the Configuration has Bidirectional.
    LexicalReorderingConfiguration::Direction m_direction;
    size_t m_offset;
    const Scores *m_prevScore;
    
    inline LexicalReorderingState(const LexicalReorderingState *prev, const TranslationOption &topt) :
        m_configuration(prev->m_configuration), m_direction(prev->m_direction), m_offset(prev->m_offset),
        m_prevScore(topt.GetCachedScores(m_configuration.GetScoreProducer())) {}

    inline LexicalReorderingState(const LexicalReorderingConfiguration &config, LexicalReorderingConfiguration::Direction dir, size_t offset) 
      : m_configuration(config), m_direction(dir), m_offset(offset), m_prevScore(NULL) {}

    // copy the right scores in the right places, taking into account forward/backward, offset, collapse
    void CopyScores(Scores& scores, const TranslationOption& topt, ReorderingType reoType) const;
    //my code:phrNum
    float poDist(float mean, int step) const;
    int fact(int num) const;
    //end
    void ClearScores(Scores& scores) const;
    int ComparePrevScores(const Scores *other) const;
    
    //constants for the different type of reorderings (corresponding to indexes in the table file)
    static const ReorderingType M = 0;  // monotonic
    static const ReorderingType NM = 1; // non-monotonic
    static const ReorderingType S = 1;  // swap
    static const ReorderingType D = 2;  // discontinuous
    static const ReorderingType DL = 2; // discontinuous, left
    static const ReorderingType DR = 3; // discontinuous, right
    static const ReorderingType R = 0;  // right
    static const ReorderingType L = 1;  // left
};

class BidirectionalReorderingState : public LexicalReorderingState {
  private:
    const LexicalReorderingState *m_backward;
    const LexicalReorderingState *m_forward;
  public:
    BidirectionalReorderingState(const LexicalReorderingConfiguration &config, const LexicalReorderingState *bw, const LexicalReorderingState *fw, size_t offset) :
      LexicalReorderingState(config, LexicalReorderingConfiguration::Bidirectional, offset), m_backward(bw), m_forward(fw) {}
      
    ~BidirectionalReorderingState() {
      delete m_backward;
      delete m_forward;
    }
    
    virtual int Compare(const FFState& o) const;
    virtual LexicalReorderingState* Expand(const TranslationOption& topt, Scores& scores) const;
    
     //my code: Coil
//    virtual LexicalReorderingState* CoilExpand(const TranslationOption& topt, Scores& scores, int segNum) const;
    //   
};

//! State for the standard Moses implementation of lexical reordering models
//! (see Koehn et al, Edinburgh System Description for the 2005 NIST MT Evaluation)
class PhraseBasedReorderingState : public LexicalReorderingState {
  private:
    WordsRange m_prevRange;
    std::string m_prevClassStr;
    bool m_first;
  public:
    PhraseBasedReorderingState(const LexicalReorderingConfiguration &config, LexicalReorderingConfiguration::Direction dir, size_t offset);
    PhraseBasedReorderingState(const PhraseBasedReorderingState *prev, const TranslationOption &topt);
    
    virtual int Compare(const FFState& o) const;
    virtual LexicalReorderingState* Expand(const TranslationOption& topt, Scores& scores) const;
    
    //my code: phrNum
    virtual LexicalReorderingState* CoilExpand(const TranslationOption& topt, Scores& scores, int segNum) const;
    void CopyCoilScores(Moses::Scores& scores, const Moses::TranslationOption& topt, Moses::LexicalReorderingState::ReorderingType reoType, int segNum, int delta) const;
    virtual int calStep(WordsRange currRange) const;
   // virtual LexicalReorderingState* BWCExpand(const TranslationOption& topt, Scores& scores) const;
    //void CopyBWCScores(Scores& scores, const TranslationOption& topt, ReorderingType reoType) const;
    //end

    ReorderingType GetOrientationTypeMSD(WordsRange currRange) const;
    ReorderingType GetOrientationTypeMSLR(WordsRange currRange) const;
    ReorderingType GetOrientationTypeMonotonic(WordsRange currRange) const;
    ReorderingType GetOrientationTypeLeftRight(WordsRange currRange) const;
};

  //! State for a hierarchical reordering model 
  //! (see Galley and Manning, A Simple and Effective Hierarchical Phrase Reordering Model, EMNLP 2008)
  //!backward state (conditioned on the previous phrase)
class HierarchicalReorderingBackwardState : public LexicalReorderingState {
  private:
    ReorderingStack m_reoStack;
  public:
    HierarchicalReorderingBackwardState(const LexicalReorderingConfiguration &config, size_t offset);
    HierarchicalReorderingBackwardState(const HierarchicalReorderingBackwardState *prev,
        const TranslationOption &topt, ReorderingStack reoStack);
    
    virtual int Compare(const FFState& o) const;
    virtual LexicalReorderingState* Expand(const TranslationOption& hypo, Scores& scores) const;

  private:
    ReorderingType GetOrientationTypeMSD(int reoDistance) const;
    ReorderingType GetOrientationTypeMSLR(int reoDistance) const;
    ReorderingType GetOrientationTypeMonotonic(int reoDistance) const;
    ReorderingType GetOrientationTypeLeftRight(int reoDistance) const;
};


  //!forward state (conditioned on the next phrase)
class HierarchicalReorderingForwardState : public LexicalReorderingState {
  private:
    bool m_first;
    WordsRange m_prevRange;
    WordsBitmap m_coverage;
    
  public:
    HierarchicalReorderingForwardState(const LexicalReorderingConfiguration &config, size_t sentenceLength, size_t offset);
    HierarchicalReorderingForwardState(const HierarchicalReorderingForwardState *prev, const TranslationOption &topt);
    
    virtual int Compare(const FFState& o) const;
    virtual LexicalReorderingState* Expand(const TranslationOption& hypo, Scores& scores) const;

  private:
    ReorderingType GetOrientationTypeMSD(WordsRange currRange, WordsBitmap coverage) const;
    ReorderingType GetOrientationTypeMSLR(WordsRange currRange, WordsBitmap coverage) const;
    ReorderingType GetOrientationTypeMonotonic(WordsRange currRange, WordsBitmap coverage) const;
    ReorderingType GetOrientationTypeLeftRight(WordsRange currRange, WordsBitmap coverage) const;
};

}
