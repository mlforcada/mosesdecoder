// $Id$

/***********************************************************************
Moses - factored phrase-based language decoder
Copyright (C) 2006 University of Edinburgh

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************/

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "Phrase.h"
#include "Hypothesis.h"
#include "TypeDef.h" //FactorArray
#include "InputType.h"
#include "Util.h" //Join()

namespace Moses
{

struct RecombinationInfo
{
	RecombinationInfo() {} //for std::vector
	RecombinationInfo(size_t srcWords, float gProb, float bProb) 
		: numSourceWords(srcWords), betterProb(gProb), worseProb(bProb) {}
	
	size_t numSourceWords;
	float betterProb, worseProb;
};

/***
 * stats relating to decoder operation on a given sentence
 */
class SentenceStats
{
	public:
	
		/***
		 * to be called before decoding a sentence
		 */
		SentenceStats(const InputType& source) {Initialize(source);}
		void Initialize(const InputType& source)
		{
			m_numHyposPruned = 0;
			m_numHyposDiscarded = 0;
			m_numHyposEarlyDiscarded = 0;
			m_timeCollectOpts = 0;
			m_timeBuildHyp = 0;
			m_timeEstimateScore = 0;
			m_timeCalcLM = 0;
			m_timeOtherScore = 0;
			m_timeStack = 0;
			m_totalSourceWords = source.GetSize();
			m_recombinationInfos.clear();
			m_deletedWords.clear();
			m_insertedWords.clear();
		}
		
		/***
		 * to be called after decoding a sentence
		 */
		void CalcFinalStats(const Hypothesis& bestHypo);
		
		unsigned int GetTotalHypos() const {return Hypothesis::GetHypothesesCreated();}
		size_t GetNumHyposRecombined() const {return m_recombinationInfos.size();}
		unsigned int GetNumHyposPruned() const {return m_numHyposPruned;}
		unsigned int GetNumHyposDiscarded() const {return m_numHyposDiscarded;}
		unsigned int GetNumHyposEarlyDiscarded() const {return m_numHyposEarlyDiscarded;}
		float GetTimeCollectOpts() const { return m_timeCollectOpts/(float)CLOCKS_PER_SEC; }
		float GetTimeBuildHyp() const { return m_timeBuildHyp/(float)CLOCKS_PER_SEC; }
		float GetTimeCalcLM() const { return m_timeCalcLM/(float)CLOCKS_PER_SEC; }
		float GetTimeEstimateScore() const { return m_timeEstimateScore/(float)CLOCKS_PER_SEC; }
		float GetTimeOtherScore() const { return m_timeOtherScore/(float)CLOCKS_PER_SEC; }
		float GetTimeStack() const { return m_timeStack/(float)CLOCKS_PER_SEC; }
		size_t GetTotalSourceWords() const {return m_totalSourceWords;}
		size_t GetNumWordsDeleted() const {return m_deletedWords.size();}
		size_t GetNumWordsInserted() const {return m_insertedWords.size();}
		const std::vector<const Phrase*>& GetDeletedWords() const {return m_deletedWords;}
		const std::vector<std::string>& GetInsertedWords() const {return m_insertedWords;}
		
		void AddRecombination(const Hypothesis& worseHypo, const Hypothesis& betterHypo)
		{
			m_recombinationInfos.push_back(RecombinationInfo(worseHypo.GetWordsBitmap().GetNumWordsCovered(), 
													betterHypo.GetTotalScore(), worseHypo.GetTotalScore()));
		}
		void AddPruning() {m_numHyposPruned++;}
		void AddEarlyDiscarded() {m_numHyposEarlyDiscarded++;}
		void AddDiscarded() {m_numHyposDiscarded++;}

		void AddTimeCollectOpts( clock_t t ) { m_timeCollectOpts += t; }
		void AddTimeBuildHyp( clock_t t ) { m_timeBuildHyp += t; }
		void AddTimeCalcLM( clock_t t ) { m_timeCalcLM += t; }
		void AddTimeEstimateScore( clock_t t ) { m_timeEstimateScore += t; }
		void AddTimeOtherScore( clock_t t ) { m_timeOtherScore += t; }
		void AddTimeStack( clock_t t ) { m_timeOtherScore += t; }
		
	protected:
	
		/***
		 * auxiliary to CalcFinalStats()
		 */
		void AddDeletedWords(const Hypothesis& hypo);
	
		//hypotheses
		std::vector<RecombinationInfo> m_recombinationInfos;
		unsigned int m_numHyposPruned;
		unsigned int m_numHyposDiscarded;
		unsigned int m_numHyposEarlyDiscarded;
		clock_t m_timeCollectOpts;
		clock_t m_timeBuildHyp;
		clock_t m_timeEstimateScore;
		clock_t m_timeCalcLM;
		clock_t m_timeOtherScore;
		clock_t m_timeStack;
	
		//words
		size_t m_totalSourceWords;
		std::vector<const Phrase*> m_deletedWords; //count deleted words/phrases in the final hypothesis
		std::vector<std::string> m_insertedWords; //count inserted words in the final hypothesis
};

inline std::ostream& operator<<(std::ostream& os, const SentenceStats& ss)
{
  float totalTime = ss.GetTimeCollectOpts() + ss.GetTimeBuildHyp() + ss.GetTimeEstimateScore() + ss.GetTimeCalcLM() + ss.GetTimeOtherScore() + ss.GetTimeStack();

  return os << "total hypotheses considered = " << ss.GetTotalHypos() << std::endl
            << "     number not fully built = " << ss.GetNumHyposEarlyDiscarded() << std::endl
            << "           number discarded = " << ss.GetNumHyposDiscarded() << std::endl
            << "          number recombined = " << ss.GetNumHyposRecombined() << std::endl
            << "              number pruned = " << ss.GetNumHyposPruned() << std::endl

            << "time to collect opts    " << ss.GetTimeCollectOpts()
<< " (" << (100 * ss.GetTimeCollectOpts()/totalTime) << "%)" << std::endl
	    << "        create hyps     " << ss.GetTimeBuildHyp()
<< " (" << (100 * ss.GetTimeBuildHyp()/totalTime) << "%)" << std::endl
            << "        estimate score  " << ss.GetTimeEstimateScore()
<< " (" << (100 * ss.GetTimeEstimateScore()/totalTime) << "%)" << std::endl
            << "        calc lm         " << ss.GetTimeCalcLM()
<< " (" << (100 * ss.GetTimeCalcLM()/totalTime) << "%)" << std::endl
            << "        other hyp score " << ss.GetTimeOtherScore()
<< " (" << (100 * ss.GetTimeOtherScore()/totalTime) << "%)" << std::endl
            << "        manage stacks   " << ss.GetTimeStack()
<< " (" << (100 * ss.GetTimeStack()/totalTime) << "%)" << std::endl

            << "total source words = " << ss.GetTotalSourceWords() << std::endl
            << "     words deleted = " << ss.GetNumWordsDeleted() << " (" << Join(" ", ss.GetDeletedWords()) << ")" << std::endl
            << "    words inserted = " << ss.GetNumWordsInserted() << " (" << Join(" ", ss.GetInsertedWords()) << ")" << std::endl;
}

}
