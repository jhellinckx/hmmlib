#ifndef __HMMFACTORY_HPP
#define __HMMFACTORY_HPP

namespace TrainingAlgorithm{
public:
	static const int LINEAR_VITERBI = 1;
	static const int LINEAR_BAUM_WELCH = 2;
	static const int LINEAR_EM = 3;
};

namespace DecodingAlgorithm{
public:
	static const int LINEAR_VITERBI = 1;
}

using namespace TrainingAlgorithm;

class HMMFactory{
private:
	HMMFactory(){}
public:
	template<typename S, typename O>
	static std::unique_ptr make(	const std::vector<S>& states,
									const std::vector<O>& observations,
									const std::vector<double>& transP,
									const std::vector<double>& emiP,
									const std::vector<double>& initP,
									int trainingAlgorithm){

		
	}
};

#endif