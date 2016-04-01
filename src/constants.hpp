#ifndef __CONSTANTS_HPP
#define __CONSTANTS_HPP

#include <string>
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <typeinfo>
#include <type_traits>

namespace utils {
	extern const double kInf =  std::numeric_limits<double>::infinity();
	extern const double kNegInf = -std::numeric_limits<double>::infinity();

	template <typename T>
	class func_to_string {
		template <typename C> static char test(char[sizeof(&C::to_string)]);
		template <typename C> static long test(...);    
	public:
		static bool exists() {
			return sizeof(test<T>(0)) == sizeof(char);	
		}
	};
}

namespace error_message {
	/* Graph */
	extern const std::string kGetVertexNotFound = "tried to get a vertex but it was not found in the graph";
	extern const std::string kRemoveVertexNotFound = "tried to remove a vertex but it was not found in the graph";
	extern const std::string kVertexNotFound = "vertex was not found in graph";
	extern const std::string kRemoveEdgeNotFound = "tried to remove an edge but it was not found in the graph";
	extern const std::string kEdgeNotFound = "edge was not found in graph";
	extern const std::string kIncidentVertexNotFound = "tried to add an edge but one of its incident vertex was not found in the graph";
	extern const std::string kAddedVertexExists = "tried to add a vertex but an equal vertex was found in the graph";
	extern const std::string kAddedEdgeExists = "tried to add an edge but an equal edge was found in the graph";

	/* State */
	extern const std::string kSilentStateHasNoDistribution = "tried to get the emission probability of a silent state; but a silent state has no distribution";

	/* Distribution */
	extern const std::string kDistributionSymbolNotFound = "symbol not found in distribution";

	/* HMM */
	extern const std::string kHMMHasNoBeginState = "no begin state was found, maybe it has been removed ?";
	extern const std::string kHMMHasNoEndState = "no end state was found, maybe it has been removed ?";
	extern const std::string kHMMRemoveStateNotFound = "tried to remove a state not contained by the hmm";
	extern const std::string kHMMRemoveTransitionNotFound = "tried to remove a transition not contained by the hmm";
	extern const std::string kHMMAddStateExists = "tried to add a state already contained by the hmm";
	extern const std::string kHMMAddTransitionExists = "tried to add a transition already contained by the hmm";
	extern const std::string kAddTransitionStateNotFound = "tried to add a transition with a state not contained by the hmm";

	template<typename T>
	extern std::string format(const std::string& error, const T& t) {
		std::ostringstream oss;
		oss << t << " : " << error << std::endl;
		return oss.str();
	}
}

namespace hmm_config {
	extern const bool kDefaultFreeEmission = true;
	extern const bool kDefaultFreeTransition = true;
	extern const int kAutoStateLabelCountStart = 1;
	extern const std::string kAutoStateLabelString = "state_";
	extern const int kDefaultPi = 0;
	extern const int kDefaultTransitionProbability = 0;
	extern const int kDefaultEmissionProbability = 0;
	extern const std::string kDefaultStartStateLabel = "begin_";
	extern const std::string kDefaultEndStateLabel = "end_";
}

namespace distribution_config {
	extern const std::string kDistributionName = "Distribution";
	extern const std::string kDiscreteDistributionName = "Discrete distribution";
	extern const std::string kContinuousDistributionName = "Continuous distribution";
	extern const std::string kNormalDistributionName = "Normal distribution";
	extern const std::string kUniformDistributionName = "Uniform distribution";
}

#endif