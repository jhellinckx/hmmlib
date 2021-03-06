#include <cstddef>
#include <cstdlib>
#include <cstdio> // std::remove file
#include <cmath>
#include <limits>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <string>
#include <stdlib.h>
#include <math.h>
#include <utility>
#include <tuple> // std::tie
#include "utils.hpp"
#include "hmm.hpp" // tested hmm library

#define ASSERT(expr) assert(expr, #expr, __FILE__, __LINE__)
#define ASSERT_VERBOSE(expr, msg) assert(expr, #expr, __FILE__, __LINE__, msg)
#define ASSERT_ABORT(expr, msg) assert(expr, #expr, __FILE__, __LINE__, msg, true)
#define ASSERT_EXCEPT(instruction, except_type) assert_except<except_type>([&](){instruction;}, #instruction, #except_type, __FILE__, __LINE__)

#define TEST_UNIT(name, instructions) run_unit_test(name, [&](){instructions});

#define VERBOSE 1
#define BIG_SEPARATOR std::string(60, '=')
#define THIN_SEPARATOR std::string(60, '-')
#define RED "\033[31m"
#define GREEN "\033[32m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define BOLDRED "\033[1;31m"
#define BOLDGREEN "\033[1;32m"
#define BOLDMAGENTA "\033[1;35m"
#define BOLDCYAN "\033[1;36m"
#define RESET "\033[0m"


void print_exception(const std::exception& e){ std::cerr << RED << e.what() << RESET << std::endl; }

unsigned int assertions = 0;
unsigned int units = 0;
unsigned int failed = 0;
unsigned int successful = 0;

template<typename Runnable>
void run_unit_test(const std::string& name, Runnable runnable){
	++units;
	if(VERBOSE) std::cout << THIN_SEPARATOR << std::endl << "Testing " << MAGENTA << name << RESET << "..."  << std::endl;
	runnable();
}

void assert(bool assertion, const char* assertion_c_str, const char* filename, long int line, std::string fail_message = "", bool fail_abort = false){
	++assertions;
	if(VERBOSE) std::cout << assertion_c_str << " ? ";
	if(assertion == true){
		++successful;
		if(VERBOSE) std::cout << BOLDGREEN << "OK" << RESET << std::endl;
	}
	else{
		++failed;
		if(VERBOSE) { 
			fail_message = (fail_message.length() > 0) ? ": " + fail_message : "";
			std::cout << BOLDRED << "FAIL" << RESET
			<< " -> " << filename  << ": " << + "line " << line 
			<< fail_message << std::endl;
		}
		if(fail_abort) abort();
	}
}

template<typename ExceptionType, typename Runnable>
void assert_except(Runnable runnable, const char* instruction_c_str, const char* exception_c_str, const char* filename, long int line){
	try{
		runnable();
		assert(false, instruction_c_str, filename, line, std::string(exception_c_str) + " expected but no exception was thrown");
	}
	catch(const ExceptionType& e){
		assert(true, instruction_c_str, filename, line);
	}
	catch(const std::exception& e){
		assert(false, instruction_c_str, filename, line, std::string(exception_c_str) + " expected but another exception was thrown: " + e.what());
	}
}

void tests_init(){
	if(VERBOSE){
		std::cout << BIG_SEPARATOR << std::endl;
		std::cout << BOLDMAGENTA << "Running " << "tests" << "..." << RESET << std::endl;
	}
}

void tests_results(){
	if(VERBOSE){
		std::cout << BIG_SEPARATOR << std::endl;
		std::cout << BOLDMAGENTA << "Ran " << assertions << " assertion(s) for "<< units << " test(s) : " << RESET;
		std::cout << BOLDGREEN << successful << " succeeded " << RESET;
		std::cout << BOLDRED << failed << " failed." << RESET << std::endl;
	}
	if(failed > 0) throw std::runtime_error("Tests failed.");
}

void round_all(std::vector<double>& vec, int precision){
	for(auto& d : vec){
		d = utils::round_double(d, precision);
	}
}

void round_all(Matrix& matrix, int precision){
	for(auto& row : matrix){
		round_all(row, precision);
	}
}

void round_all(std::vector<DiscreteDistribution>& dists, int precision){
	for(auto& dist : dists){
		dist.round(precision);
	}
}

void exp_all(std::vector<double>& vec){
	for(auto& d : vec){ d = exp(d); }
}

void exp_all(Matrix& matrix){
	for(auto& row : matrix) { exp_all(row); }
}

void exp_all(std::vector<DiscreteDistribution>& dists){
	for(auto& dist : dists){ dist.log_probabilities(false); }
}

HiddenMarkovModel generate_random(std::size_t num_states, std::vector<std::string> alphabet,
	std::size_t n_trans, std::size_t n_emi){
		std::size_t params = 0;
		HiddenMarkovModel generated("generated");
		std::vector<State> states;
		states.reserve(num_states);
		std::vector<std::string> not_emitted;
		for(std::size_t i = 0; i < num_states; ++i){
			not_emitted = alphabet;
			DiscreteDistribution dist;
			std::size_t emi = std::min(alphabet.size(), n_emi);	
			for(std::size_t j = 0; j < emi; ++j){
				std::size_t rand_emi = (std::size_t) rand() % emi;
				++params;
				dist[not_emitted[rand_emi]] = (std::size_t) rand() % 100;
				not_emitted.erase(not_emitted.begin() + std::ptrdiff_t(rand_emi));
				--emi;
			}
			State s("state_"+std::to_string(i), dist);
			//s.fix_transition();
			//s.fix_emission();
			states.push_back(s);
			generated.add_state(s);
		}
		std::vector<State> no_trans;
		for(std::size_t i = 0; i < num_states; ++i){
			no_trans = states;
			std::size_t trans = std::min(num_states, n_trans);
			for(std::size_t j = 0; j < trans; ++j){
				std::size_t rand_trans = (std::size_t) rand() % trans;
				generated.add_transition(states[i], no_trans[rand_trans], (std::size_t) rand() % 100);
				no_trans.erase(no_trans.begin() + std::ptrdiff_t(rand_trans));
				++params;
				--trans;
			}
		}
		no_trans = states;
		std::size_t trans = std::min(num_states, n_trans);
		for(std::size_t j = 0; j < trans; ++j){
			std::size_t rand_trans = (std::size_t) rand() % trans;
			generated.add_transition(generated.begin(), no_trans[rand_trans], (std::size_t) rand() % 100);
			no_trans.erase(no_trans.begin() + std::ptrdiff_t(rand_trans));
			++params;
			--trans;
		}
		std::cout << params << std::endl;
		generated.brew();
		return generated;
}

void mem_bench() {
	std::vector<std::string> alphabet = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"};
	HiddenMarkovModel hmm = generate_random(100, alphabet, 5, 10);
	
	std::vector<std::size_t> lengths = {200};
	std::vector<std::vector<std::string>> sequences;
	std::size_t rand_symbol;
	sequences.reserve(lengths.size());
	for(std::size_t length : lengths){
		std::vector<std::string> sequence;
		sequence.reserve(length);
		for(std::size_t i = 0; i < length; ++i){
			rand_symbol = (std::size_t) rand() % alphabet.size();
			sequence.push_back(alphabet[rand_symbol]);
		}
		sequences.push_back(sequence);
	}

	HiddenMarkovModel train1 = hmm;
	//HiddenMarkovModel train2 = hmm;
	// HiddenMarkovModel fwd = hmm;
	// HiddenMarkovModel bwd = hmm;
	// HiddenMarkovModel viter = hmm;

	train1.set_training(LinearMemoryViterbiTraining(nullptr));
	train1.train(sequences, 0.0, hmm_config::kDefaultConvergenceThreshold, 0, 1);

	
	//train2.set_training(LinearMemoryBaumWelchTraining(nullptr));
	//train2.train(sequences, 0.0, hmm_config::kDefaultConvergenceThreshold, 0, 1);

	
	// std::cout << "Forward" << std::endl;
	// for(std::size_t i = 0; i< sequences.size(); ++i){
	// 	fwd.log_likelihood(sequences[i], true);	
	// }
	// std::cout << std::endl;
	
	
	// std::cout << "Backward" << std::endl;
	// for(std::size_t i = 0; i< sequences.size(); ++i){
	// 	bwd.log_likelihood(sequences[i], false);	
	// }
	// std::cout << std::endl;

	// std::cout << "Viterbi Decode" << std::endl;
	// for(std::size_t i = 0; i< sequences.size(); ++i){
	// 	viter.decode(sequences[i], false);	
	// }
	// std::cout << std::endl;
}



int main(){
	try{
		/* Create hmm examples. */

		/* Simple fair/biased model. */
		HiddenMarkovModel casino_hmm("casino");
		DiscreteDistribution fair_dist({{"H", 0.5}, {"T", 0.5}});
		DiscreteDistribution biased_dist({{"H", 0.75}, {"T", 0.25}});
		State fair = State("fair", fair_dist);
		State biased = State("biased", biased_dist);
		casino_hmm.add_state(fair);
		casino_hmm.add_transition(casino_hmm.begin(), fair, 0.5);
		casino_hmm.add_state(biased);
		casino_hmm.add_transition(casino_hmm.begin(), biased, 0.5);
		casino_hmm.add_transition(fair, fair, 0.9);
		casino_hmm.add_transition(fair, biased, 0.1);
		casino_hmm.add_transition(biased, biased, 0.9);
		casino_hmm.add_transition(biased, fair, 0.1);
		casino_hmm.brew();
		/* Precomputed casino values. */
		std::vector<std::string> casino_symbols({"T","H","H","T","T","T","H","H"});
		double casino_precomputed_init_fwd_fair = 0.25;
		double casino_precomputed_init_fwd_biased = 0.125;
		double casino_precomputed_mid_fwd_fair = 0.0303;
		double casino_precomputed_mid_fwd_biased = 0.0191;
		double casino_precomputed_end_fwd_fair = 0.0015;
		double casino_precomputed_end_fwd_biased = 0.0013;
		double casino_precomputed_init_bwd_fair = 1;
		double casino_precomputed_init_bwd_biased = 1;
		double casino_precomputed_mid_bwd_fair = 0.0679;
		double casino_precomputed_mid_bwd_biased = 0.0366;
		double casino_precomputed_end_bwd_fair = 0.0075;
		double casino_precomputed_end_bwd_biased = 0.0071;
		double casino_precomputed_likelihood = 0.0028;
		std::vector<std::string> casino_precomputed_viterbi_path_2_states({"fair", "fair", "fair", "fair", "fair", "fair", "fair", "fair"});
		/* Training casino. */
		// std::vector<std::vector<std::string>> casino_training_sequences_1 = 
		// {{"H", "T", "H"}, {"H", "H", "H", "H"}, {"T", "H", "T", "H", "T"}, 
		// {"T", "H", "T", "H", "T", "H", "T", "H", "T", "H"}, 
		// {"T", "T", "H", "H", "H", "H", "H", "H", "H", "H", "H"}, 
		// {"T", "H", "T", "H", "T", "H", "T", "H", "T", "H", "T", "H", "T", "H", "T", "H"}, 
		// {"T", "T", "T"}};	

		std::vector<std::vector<std::string>> casino_training_sequences_2 = 
		{	{"T", "H", "H", "T"}, {"T", "H", "H", "T"}, {"T", "H", "H", "T"}, 
			{"T", "H", "T", "H"}, {"T", "T", "T", "T"}, {"T", "T", "T", "T"}, 
			{"T", "H", "T", "H"}, {"H", "T", "H", "H"}, {"H", "T", "H", "H"}};

		std::vector<std::vector<double>> casino_precomputed_viterbi_trained_transitions = {{1, 0}, {0, 1}};
		std::vector<double> casino_precomputed_viterbi_trained_pi_begin = {0.7778, 0.2222};
		std::vector<DiscreteDistribution> casino_precomputed_viterbi_trained_distributions;
		casino_precomputed_viterbi_trained_distributions.push_back(DiscreteDistribution({{"H", 0.3571}, {"T", 0.6429}}));
		casino_precomputed_viterbi_trained_distributions.push_back(DiscreteDistribution({{"H", 0.75}, {"T", 0.25}}));
		double casino_precomputed_viterbi_improvement = utils::round_double(1.7561325574, 4);

		std::vector<std::vector<double>> casino_precomputed_viterbi_trained_transitions_pc = {{0.9565, 0.0435}, {0.125, 0.875}};
		std::vector<double> casino_precomputed_viterbi_trained_pi_begin_pc = {0.7273, 0.2727};
		std::vector<DiscreteDistribution> casino_precomputed_viterbi_trained_distributions_pc;
		casino_precomputed_viterbi_trained_distributions_pc.push_back(DiscreteDistribution({{"H", 0.3571}, {"T", 0.6429}}));
		casino_precomputed_viterbi_trained_distributions_pc.push_back(DiscreteDistribution({{"H", 0.75}, {"T", 0.25}}));
		double casino_precomputed_viterbi_improvement_pc = utils::round_double(1.69606009321, 4);

		std::vector<std::vector<double>> casino_precomputed_bw_trained_transitions = {{0, 1}, {0.5183, 0.4817}};
		std::vector<double> casino_precomputed_bw_trained_pi_begin = {0.7128, 0.2872};
		std::vector<DiscreteDistribution> casino_precomputed_bw_trained_distributions;
		casino_precomputed_bw_trained_distributions.push_back(DiscreteDistribution({{"H", 0}, {"T", 1}}));
		casino_precomputed_bw_trained_distributions.push_back(DiscreteDistribution({{"H", 0.7450}, {"T", 0.2550}}));
		double casino_precomputed_bw_improvement = utils::round_double(5.05069902785, 4);

		/* Simple hmm with 3 states emitting nucleobases. */
		HiddenMarkovModel nucleobase_3_states_hmm("nucleobase 3 states");
		DiscreteDistribution dist1({{"A", 0.35}, {"C", 0.20}, {"G", 0.05}, {"T", 0.40}});
		DiscreteDistribution dist2({{"A", 0.25}, {"C", 0.25}, {"G", 0.25}, {"T", 0.25}});
		DiscreteDistribution dist3({{"A", 0.10}, {"C", 0.40}, {"G", 0.40}, {"T", 0.10}});
		State s1("s1", dist1);
		State s2("s2", dist2);
		State s3("s3", dist3);
		nucleobase_3_states_hmm.add_state(s1);
		nucleobase_3_states_hmm.add_state(s2);
		nucleobase_3_states_hmm.add_state(s3);
		nucleobase_3_states_hmm.begin_transition(s1, 0.90);
		nucleobase_3_states_hmm.begin_transition(s2, 0.10);
		nucleobase_3_states_hmm.add_transition(s1, s1, 0.80);
		nucleobase_3_states_hmm.add_transition(s1, s2, 0.20);
		nucleobase_3_states_hmm.add_transition(s2, s2, 0.30);
		nucleobase_3_states_hmm.add_transition(s2, s3, 0.10);
		nucleobase_3_states_hmm.add_transition(s3, s3, 0.70);
		nucleobase_3_states_hmm.end_transition(s3, 0.30);
		nucleobase_3_states_hmm.end_transition(s2, 0.60);
		nucleobase_3_states_hmm.brew(); 
		/* Precomputed values. */
		std::vector<std::string> nucleobase_symbols({"A", "C", "G", "A", "C", "T", "A", "T", "T", "C", "G", "A", "T"});
		double nucleobase_precomputed_viterbi_log_likelihood = utils::round_double(-23.834436455461574, 4);
		std::vector<std::string> nucleobase_precomputed_viterbi_path_3_states({"s1", "s1", "s1", "s1", "s1", "s1", "s1", "s1", "s1", "s1", "s1", "s1", "s2"});
		
		std::vector<std::vector<std::string>> nucleobase_training_sequences;
		nucleobase_training_sequences.push_back(nucleobase_symbols);

		std::vector<std::vector<double>> nucleobase_precomputed_bw_trained_transitions = 
		{ {0.9167, 0.0833, 0}, {0, 0, 0}, {0, 0, 0.8149} };
		std::vector<DiscreteDistribution> nucleobase_precomputed_bw_trained_distributions;
		nucleobase_precomputed_bw_trained_distributions.push_back(DiscreteDistribution({{"A", 0.3333}, {"C", 0.25}, {"G", 0.1667}, {"T", 0.25}}));
		nucleobase_precomputed_bw_trained_distributions.push_back(DiscreteDistribution({{"A", 0}, {"C", 0}, {"G", 0}, {"T", 1}}));
		nucleobase_precomputed_bw_trained_distributions.push_back(DiscreteDistribution({{"A", 0.2482}, {"C", 0.1851}, {"G", 0.1851}, {"T", 0.3816}}));
		std::vector<double> nucleobase_precomputed_bw_trained_pi_begin = {1, 0, 0};
		std::vector<double> nucleobase_precomputed_bw_trained_pi_end = {0, 1, 0.1851};
		double nucleobase_precomputed_bw_improvement = utils::round_double(3.23843686377, 4);

		/* Profile hmm with 10 states. */
		HiddenMarkovModel profile_10_states_hmm("profile 10 states");
		DiscreteDistribution i_d({{"A", 0.25}, {"C", 0.25}, {"G", 0.25}, {"T", 0.25}});
		/* Create insert states. */
		State i0 = State("I0", i_d);
		State i1 = State("I1", i_d);
		State i2 = State("I2", i_d);
		State i3 = State("I3", i_d);
		/* Create match states. */
		State m1 = State("M1", DiscreteDistribution({{"A", 0.95},  {"C", 0.01}, {"G", 0.01},  {"T", 0.03 }}));
		State m2 = State("M2", DiscreteDistribution({{"A", 0.003}, {"C", 0.99}, {"G", 0.003}, {"T", 0.004}}));
		State m3 = State("M3", DiscreteDistribution({{"A", 0.01},  {"C", 0.01}, {"G", 0.01},  {"T", 0.97 }}));
		/* Create delete states. */
		State d1 = State("D1");
		State d2 = State("D2");
		State d3 = State("D3");
		/* Add all the states. */
		profile_10_states_hmm.add_state(i0);
		profile_10_states_hmm.add_state(i1);
		profile_10_states_hmm.add_state(i2);
		profile_10_states_hmm.add_state(i3);
		profile_10_states_hmm.add_state(m1);
		profile_10_states_hmm.add_state(m2);
		profile_10_states_hmm.add_state(m3);
		profile_10_states_hmm.add_state(d1);
		profile_10_states_hmm.add_state(d2);
		profile_10_states_hmm.add_state(d3);
		/* Transitions from match states. */
		profile_10_states_hmm.add_transition(profile_10_states_hmm.begin(), m1, 0.5);
		profile_10_states_hmm.add_transition(profile_10_states_hmm.begin(), i0, 0.1);
		profile_10_states_hmm.add_transition(profile_10_states_hmm.begin(), d1, 0.4);
		profile_10_states_hmm.add_transition(m1, m2, 0.9);
		profile_10_states_hmm.add_transition(m1, i1, 0.05);
		profile_10_states_hmm.add_transition(m1, d2, 0.05);
		profile_10_states_hmm.add_transition(m2, m3, 0.9);
		profile_10_states_hmm.add_transition(m2, i2, 0.05);
		profile_10_states_hmm.add_transition(m2, d3, 0.05);
		profile_10_states_hmm.add_transition(m3, profile_10_states_hmm.end(), 0.9);
		profile_10_states_hmm.add_transition(m3, i3, 0.1);
		/* Transitions from insert states. */
		profile_10_states_hmm.add_transition(i0, i0, 0.70);
		profile_10_states_hmm.add_transition(i0, d1, 0.15);
		profile_10_states_hmm.add_transition(i0, m1, 0.15);
		profile_10_states_hmm.add_transition(i1, i1, 0.70);
		profile_10_states_hmm.add_transition(i1, d2, 0.15);
		profile_10_states_hmm.add_transition(i1, m2, 0.15);
		profile_10_states_hmm.add_transition(i2, i2, 0.70);
		profile_10_states_hmm.add_transition(i2, d3, 0.15);
		profile_10_states_hmm.add_transition(i2, m3, 0.15);
		profile_10_states_hmm.add_transition(i3, i3, 0.85);
		profile_10_states_hmm.add_transition(i3, profile_10_states_hmm.end(), 0.15);
		/* Transitions from delete states. */
		profile_10_states_hmm.add_transition(d1, d2, 0.15);
		profile_10_states_hmm.add_transition(d1, i1, 0.15);
		profile_10_states_hmm.add_transition(d1, m2, 0.70);
		profile_10_states_hmm.add_transition(d2, d3, 0.15);
		profile_10_states_hmm.add_transition(d2, i2, 0.15);
		profile_10_states_hmm.add_transition(d2, m3, 0.70);
		profile_10_states_hmm.add_transition(d3, i3, 0.30);
		profile_10_states_hmm.add_transition(d3, profile_10_states_hmm.end(), 0.70);
		profile_10_states_hmm.brew();

		/* Precomputed values. */

		/* Profile viterbi decode */
		std::vector<std::vector<std::string>> profile_viterbi_decode_sequences = 
		{{"A"}, {"G", "A"}, {"A", "C"}, {"A", "T"}, {"A", "T", "C", "C"}, 
		{"A", "C", "G", "T", "G"}, {"A", "T", "T", "T"}, {"T", "A", "C", "C", "C", "T", "C"}, 
		{"T", "G", "T", "C", "A", "A", "C", "A", "C", "T"}, {"A", "C", "T"}, {"G", "G", "C"},
		{"G", "A", "T"}, {"A", "C", "C"}};

		std::vector<double> precomputed_profile_viterbi_log_likelihoods = 
		{	-5.99396767733, -10.0935892966, -4.21225854395, -4.23266741558, -11.2621196295,
			-10.9816115001, -9.25905106665, -16.5306107603, -16.4516996541, -1.10103156526,
			 -11.0481012413, -8.33228903491, -5.67574254376};
		for(double& likelihood : precomputed_profile_viterbi_log_likelihoods){ likelihood = utils::round_double(likelihood, 4); }
		
		std::vector<std::vector<std::string>> precomputed_profile_viterbi_paths =
		{ 	{"M1", "D2", "D3"},
			{"D1", "D2", "I2", "I2", "D3"},
			{"M1", "M2", "D3"},
			{"M1", "D2", "M3"},
			{"M1", "D2", "M3", "I3", "I3"},
			{"M1", "M2", "I2", "I2", "I2", "D3"},
			{"M1", "I1", "I1", "D2", "M3"},
			{"D1", "D2", "M3", "I3", "I3", "I3", "I3", "I3", "I3"},
			{"I0", "I0", "I0", "I0", "I0", "I0", "I0", "M1", "M2", "M3"},
			{"M1", "M2", "M3"},
			{"I0", "I0", "D1", "M2", "D3"},
			{"D1", "I1", "I1", "D2", "M3"},
			{"M1", "M2", "M3"}};
			
		/* Profile observation probability */
		std::vector<double> precomputed_profile_observation_likelihoods = 
		{	-5.64533551635, -8.56462831313, -4.16525171411, -4.03902020142, 
			-9.66057354724, -10.0430248815, -8.32206040729, -14.7638542203, 
			-16.0844468561, -1.08398911807, -9.61825642651, -7.50353655895, 
			-5.42134753659};
		for(double& likelihood : precomputed_profile_observation_likelihoods){ likelihood = utils::round_double(likelihood, 4); }
		std::vector<std::vector<std::string>>& profile_observation_likelihood_sequences = profile_viterbi_decode_sequences;

		/* Profile training */
		std::vector<std::vector<std::string>> profile_training_sequences_1 = 
		{{"A", "C", "T"}, {"A", "C", "T"}, {"A", "C", "C"}, {"A", "C", "T", "C"}, 
		{"A", "C", "T"}, {"A", "C", "T"}, {"C", "C", "T"}, {"C", "C", "C"}, 
		{"A", "A", "T"}, {"C", "T"}, {"A", "T"}, {"C", "T"}, {"C", "T"}, {"C", "T"}, 
		{"C", "T"}, {"C", "T"}, {"C", "T"}, {"A", "C", "T"}, {"A", "C", "T"}, 
		{"C", "T"}, {"A", "C", "T"}, {"C", "T"}, {"C", "T"}, {"C", "T"}, {"C", "T"}};
		std::vector<std::vector<std::string>> profile_training_sequences_2 = 
		{ {"A", "C", "T", "A", "T"} };

		/* Viterbi */
		std::vector<std::vector<double>> profile_precomputed_viterbi_trained_transitions = 
		{	{0.7, 0.0, 0.0, 	0.0, 	0.15, 	0.0, 	0.0, 	 0.15, 	0.0, 	0.0}, 
			{0.0, 0.7, 0.0, 	0.0, 	0.0,	0.15, 	0.0, 	 0.0, 	0.15, 	0.0}, 
			{0.0, 0.0, 0.5, 	0.0, 	0.0,	0.0,	0.0, 	 0.0, 	0.0, 	0.5}, 
			{0.0, 0.0, 0.0, 	0.0, 	0.0,	0.0, 	0.0, 	 0.0, 	0.0, 	0.0}, 
			{0.0, 0.0, 0.0, 	0.0, 	0.0,	0.9167, 0.0, 	 0.0, 	0.0833, 0.0}, 
			{0.0, 0.0, 0.0417, 	0.0, 	0.0,	0.0, 	0.9583,	 0.0, 	0.0, 	0.0}, 
			{0.0, 0.0, 0.0, 	0.0417, 0.0,	0.0, 	0.0, 	 0.0, 	0.0, 	0.0}, 
			{0.0, 0.0, 0.0, 	0.0, 	0.0, 	1.0, 	0.0, 	 0.0, 	0.0, 	0.0}, 
			{0.0, 0.0, 0.0, 	0.0, 	0.0, 	0.0, 	1.0, 	 0.0, 	0.0, 	0.0}, 
			{0.0, 0.0, 0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	 0.0, 	0.0, 	0.0}};
		std::vector<DiscreteDistribution> profile_precomputed_viterbi_trained_distributions;
		profile_precomputed_viterbi_trained_distributions.push_back(DiscreteDistribution({{"A", 0.25  }, {"C", 0.25	 }, {"T", 0.25	}, {"G", 0.25}}));
		profile_precomputed_viterbi_trained_distributions.push_back(DiscreteDistribution({{"A", 0.25  }, {"C", 0.25	 }, {"T", 0.25	}, {"G", 0.25}}));
		profile_precomputed_viterbi_trained_distributions.push_back(DiscreteDistribution({{"A", 0.0	  }, {"C", 1.0	 }, {"T", 0.0	}, {"G", 0.0 }}));
		profile_precomputed_viterbi_trained_distributions.push_back(DiscreteDistribution({{"A", 0.0	  }, {"C", 1.0	 }, {"T", 0.0	}, {"G", 0.0 }}));
		profile_precomputed_viterbi_trained_distributions.push_back(DiscreteDistribution({{"A", 0.9167}, {"C", 0.0833}, {"T", 0.0	}, {"G", 0.0 }}));
		profile_precomputed_viterbi_trained_distributions.push_back(DiscreteDistribution({{"A", 0.0417}, {"C", 0.9583}, {"T", 0.0	}, {"G", 0.0 }}));
		profile_precomputed_viterbi_trained_distributions.push_back(DiscreteDistribution({{"A", 0.0	  }, {"C", 0.0417}, {"T", 0.9583}, {"G", 0.0 }}));
		std::vector<double> profile_precomputed_viterbi_trained_pi_begin = {0.0, 0.0, 0.0, 	0.0, 	0.48, 	0.0, 	0.0,  0.52, 	0.0, 	0.0};
		std::vector<double> profile_precomputed_viterbi_trained_pi_end = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.9583, 0.0, 0.0, 1.0};
		double profile_precomputed_viterbi_improvement = utils::round_double(19.9265503604, 4);


		std::vector<std::vector<double>> profile_precomputed_viterbi_trained_transitions_pc = 
		{	{0.3333,0.0, 	0.0, 	0.0, 	0.3333, 0.0, 	0.0, 	 0.3333,0.0, 	0.0}, 
			{0.0, 	0.3333, 0.0, 	0.0, 	0.0, 	0.3333, 0.0, 	 0.0, 	0.3333, 0.0}, 
			{0.0, 	0.0, 	0.4, 	0.0, 	0.0, 	0.0, 	0.2, 	 0.0, 	0.0, 	0.4}, 
			{0.0, 	0.0, 	0.0, 	0.3333, 0.0, 	0.0, 	0.0, 	 0.0, 	0.0, 	0.0}, 
			{0.0, 	0.0667, 0.0, 	0.0, 	0.0, 	0.8, 	0.0, 	 0.0, 	0.1333, 0.0}, 
			{0.0, 	0.0, 	0.0741, 0.0, 	0.0, 	0.0, 	0.8889,	 0.0, 	0.0, 	0.037}, 
			{0.0, 	0.0, 	0.0, 	0.0769, 0.0, 	0.0, 	0.0, 	 0.0, 	0.0, 	0.0},
			{0.0, 	0.0625, 0.0, 	0.0, 	0.0, 	0.875, 	0.0, 	 0.0, 	0.0625, 0.0}, 
			{0.0, 	0.0, 	0.25, 	0.0, 	0.0, 	0.0, 	0.5, 	 0.0, 	0.0, 	0.25}, 
			{0.0, 	0.0, 	0.0, 	0.3333, 0.0, 	0.0, 	0.0, 	 0.0, 	0.0, 	0.0}};
		std::vector<DiscreteDistribution> profile_precomputed_viterbi_trained_distributions_pc;
		profile_precomputed_viterbi_trained_distributions_pc.push_back(DiscreteDistribution({{"A", 0.25},	 {"C", 0.25},	 {"T", 0.25},	 {"G", 0.25}}));
		profile_precomputed_viterbi_trained_distributions_pc.push_back(DiscreteDistribution({{"A", 0.25},	 {"C", 0.25},	 {"T", 0.25},	 {"G", 0.25}}));
		profile_precomputed_viterbi_trained_distributions_pc.push_back(DiscreteDistribution({{"A", 0.0},	 {"C", 1.0},	 {"T", 0.0},	 {"G", 0.0}}));
		profile_precomputed_viterbi_trained_distributions_pc.push_back(DiscreteDistribution({{"A", 0.0},	 {"C", 1.0},	 {"T", 0.0},	 {"G", 0.0}}));
		profile_precomputed_viterbi_trained_distributions_pc.push_back(DiscreteDistribution({{"A", 0.9167},	 {"C", 0.0833},	 {"T", 0.0},	 {"G", 0.0}}));
		profile_precomputed_viterbi_trained_distributions_pc.push_back(DiscreteDistribution({{"A", 0.0417},	 {"C", 0.9583},	 {"T", 0.0},	 {"G", 0.0}}));
		profile_precomputed_viterbi_trained_distributions_pc.push_back(DiscreteDistribution({{"A", 0.0},	 {"C", 0.0417},	 {"T", 0.9583},	 {"G", 0.0}}));
		std::vector<double> profile_precomputed_viterbi_trained_pi_begin_pc = {0.0357,0.0, 	0.0, 	0.0, 	0.4643, 0.0, 	0.0, 	 0.5, 	0.0, 	0.0};
		std::vector<double> profile_precomputed_viterbi_trained_pi_end_pc = {0.0, 0.0, 0.0, 0.6667, 0.0, 0.0, 0.9231, 0.0, 0.0, 0.6667};
		double profile_precomputed_viterbi_improvement_pc = utils::round_double(15.0271603832, 4);

		/* Baum-Welch */
		std::vector<std::vector<double>> profile_precomputed_bw_1_iter_1_seq_trained_transitions = 
		{	{0.5944,0.0, 	0.0,	0.0, 	0.1406, 0.0, 	0.0, 	0.265, 	0.0, 	0.0},
			{0.0, 	0.6543, 0.0,	0.0, 	0.0, 	0.0304, 0.0, 	0.0, 	0.3153, 0.0},
		 	{0.0, 	0.0, 	0.5347, 0.0, 	0.0, 	0.0, 	0.3974,	0.0, 	0.0, 	0.068},
		 	{0.0, 	0.0, 	0.0, 	0.5085, 0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	0.0},
		 	{0.0, 	0.0424, 0.0, 	0.0, 	0.0, 	0.9405, 0.0, 	0.0, 	0.0171, 0.0},
		 	{0.0, 	0.0, 	0.3167, 0.0, 	0.0, 	0.0, 	0.6593,	0.0, 	0.0, 	0.0241},
		 	{0.0, 	0.0, 	0.0, 	0.6486, 0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	0.0},
		 	{0.0, 	0.6356, 0.0, 	0.0, 	0.0, 	0.1462, 0.0, 	0.0, 	0.2182, 0.0},
		 	{0.0, 	0.0, 	0.2298, 0.0, 	0.0, 	0.0, 	0.729, 	0.0, 	0.0, 	0.0413},
		 	{0.0, 	0.0, 	0.0, 	0.4464, 0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	0.0}};
		std::vector<DiscreteDistribution> profile_precomputed_bw_1_iter_1_seq_trained_distributions;
		profile_precomputed_bw_1_iter_1_seq_trained_distributions.push_back(DiscreteDistribution({{"A", 0.4836}, {"C", 0.2707}, {"T", 0.2457}, {"G", 0.0}}));
		profile_precomputed_bw_1_iter_1_seq_trained_distributions.push_back(DiscreteDistribution({{"A", 0.3954}, {"C", 0.3069}, {"T", 0.2977}, {"G", 0.0}}));
		profile_precomputed_bw_1_iter_1_seq_trained_distributions.push_back(DiscreteDistribution({{"A", 0.4635}, {"C", 0.0208}, {"T", 0.5157}, {"G", 0.0}}));
		profile_precomputed_bw_1_iter_1_seq_trained_distributions.push_back(DiscreteDistribution({{"A", 0.489 }, {"C", 0.0011}, {"T", 0.5099}, {"G", 0.0}}));
		profile_precomputed_bw_1_iter_1_seq_trained_distributions.push_back(DiscreteDistribution({{"A", 0.9998}, {"C", 0.0   }, {"T", 0.0002}, {"G", 0.0}}));
		profile_precomputed_bw_1_iter_1_seq_trained_distributions.push_back(DiscreteDistribution({{"A", 0.0018}, {"C", 0.9981}, {"T", 0.0001}, {"G", 0.0}}));
		profile_precomputed_bw_1_iter_1_seq_trained_distributions.push_back(DiscreteDistribution({{"A", 0.0001}, {"C", 0.0001}, {"T", 0.9998}, {"G", 0.0}}));
		std::vector<double> profile_precomputed_bw_1_iter_1_seq_trained_pi_begin = {0.0203,0.0, 	0.0, 	0.0, 	0.9516, 0.0, 	0.0, 	0.0281, 0.0, 	0.0};
		std::vector<double> profile_precomputed_bw_1_iter_1_seq_trained_pi_end = {0.0,0.0,0.0,0.4915,0.0,0.0,0.3514,0.0,0.0,0.5536};

		std::vector<std::vector<double>> profile_precomputed_bw_batch_trained_transitions = 
		{	{0.0,	0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	1.0, 	0.0, 	0.0}, 
			{0.0,	0.0002, 0.0, 	0.0, 	0.0, 	0.0164, 0.0, 	0.0, 	0.9834, 0.0}, 
			{0.0,	0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	0.0009,	0.0, 	0.0, 	0.9991}, 
			{0.0,	0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	0.0}, 
			{0.0,	0.0061, 0.0, 	0.0, 	0.0, 	0.9328, 0.0, 	0.0, 	0.0611, 0.0}, 
			{0.0,	0.0, 	0.0325, 0.0, 	0.0, 	0.0, 	0.9673,	0.0, 	0.0, 	0.0002}, 
			{0.0,	0.0, 	0.0, 	0.0434,	0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	0.0}, 
			{0.0, 	0.0561, 0.0, 	0.0, 	0.0, 	0.9439, 0.0, 	0.0, 	0.0, 	0.0}, 
			{0.0, 	0.0, 	0.6508, 0.0, 	0.0, 	0.0, 	0.3492,	0.0, 	0.0, 	0.0}, 
			{0.0, 	0.0, 	0.0, 	0.9888,	0.0, 	0.0, 	0.0, 	0.0, 	0.0, 	0.0}};
		std::vector<DiscreteDistribution> profile_precomputed_bw_batch_trained_distributions;
		profile_precomputed_bw_batch_trained_distributions.push_back(DiscreteDistribution({{"A", 0.255 }, {"C", 0.745 }, {"T", 0.0   }, {"G", 0.0}}));
		profile_precomputed_bw_batch_trained_distributions.push_back(DiscreteDistribution({{"A", 0.5102}, {"C", 0.4898}, {"T", 0.0   }, {"G", 0.0}}));
		profile_precomputed_bw_batch_trained_distributions.push_back(DiscreteDistribution({{"A", 0.0   }, {"C", 0.9955}, {"T", 0.0045}, {"G", 0.0}}));
		profile_precomputed_bw_batch_trained_distributions.push_back(DiscreteDistribution({{"A", 0.0   }, {"C", 1.0   }, {"T", 0.0   }, {"G", 0.0}}));
		profile_precomputed_bw_batch_trained_distributions.push_back(DiscreteDistribution({{"A", 0.9313}, {"C", 0.0687}, {"T", 0.0   }, {"G", 0.0}}));
		profile_precomputed_bw_batch_trained_distributions.push_back(DiscreteDistribution({{"A", 0.0732}, {"C", 0.9268}, {"T", 0.0   }, {"G", 0.0}}));
		profile_precomputed_bw_batch_trained_distributions.push_back(DiscreteDistribution({{"A", 0.0   }, {"C", 0.0113}, {"T", 0.9887}, {"G", 0.0}}));
		std::vector<double> profile_precomputed_bw_batch_trained_pi_begin = {0.0186,0.0, 	0.0, 	0.0, 	0.4171, 0.0, 	0.0, 	0.5643, 0.0, 	0.0};
		std::vector<double> profile_precomputed_bw_batch_trained_pi_end = {0.0,0.0,0.0,1.0,0.0,0.0,0.9566,0.0,0.0,0.0112};

		tests_init();
		
		/* IEEE 754 floating points are required in order to use std::infinity numeric limit. */
		TEST_UNIT(
			"platform type",
			ASSERT_ABORT(std::numeric_limits<double>::is_iec559, "IEEE 754 required");
		)

		TEST_UNIT(
			"graph",
			/* Topological sort. */
			Graph<std::string> g;
			g.add_vertex("B");
			g.add_vertex("E");
			g.add_vertex("A");
			g.add_vertex("D");
			g.add_vertex("C");
			g.add_edge("A", "B");
			g.add_edge("A", "D");
			g.add_edge("B", "C");
			g.add_edge("C", "D");
			g.add_edge("D", "E");
			g.add_edge("C", "E");
			std::vector<std::string> precomputed_toposort({"A", "B", "C", "D", "E"}); 
			g.topological_sort();
			std::vector<std::string*> vertices = g.get_vertices();
			std::vector<std::string> toposort;
			for(std::string* str_ptr : vertices) {
				toposort.push_back(*str_ptr);
			}
			ASSERT(toposort == precomputed_toposort);
			/* Subgraph. */
			std::vector<std::string> sub_vertices({"C","E"});
			Graph<std::string> subgraph = g.sub_graph(sub_vertices);
			std::vector<std::string> sub_graph_vertices;
			std::vector<std::string*> sub_ptrs = subgraph.get_vertices();
			for(std::string* sub_ptr : sub_ptrs){
				sub_graph_vertices.push_back(*sub_ptr);
			}
			ASSERT(sub_graph_vertices == sub_vertices);
			ASSERT(subgraph.has_edge("C", "E"));
			ASSERT(!subgraph.has_vertex("A"));
			ASSERT(!subgraph.has_vertex("B"));
			ASSERT(!subgraph.has_vertex("D"));
		)


		TEST_UNIT(
			"state creation/distribution",
			/* Construct silent states with same name. */
			State s1("state");
			State s2("state");
			ASSERT(s1 == s2);
			/* If no distribution is given at construction, state is silent. */
			ASSERT(s1.is_silent());
			/* Throw exception if access distribution of silent state. */
			ASSERT_EXCEPT(s1.distribution(), StateDistributionException);
			/* An empty distribution makes the state silent. */
			DiscreteDistribution dist1;
			State s3("state", dist1);
			ASSERT(s3.is_silent());
			/* State has own copy of distribution. */
			ASSERT(s3.distribution() == dist1);
			dist1["A"] = 0;
			ASSERT(s3.distribution() != dist1);
			/* A discrete distribution with probabilites summing to 0 is considered empty. */
			dist1["B"] = 0;
			State s4("state", dist1);
			ASSERT(s4.is_silent());
			/* Probabilites summing to i > 0 makes a state not silent. */
			dist1["C"] = 0.4;
			State s5("state", dist1);
			ASSERT(!s5.is_silent());
			/* Check distribution type. */
			ASSERT(s3.distribution().is_discrete());
			ASSERT(!s3.distribution().is_continuous());
			/* Despite having a distribution, s3 is equal to s1 because they have the same name. 
			States equality is currently equivalent to their name equality. */
			ASSERT(s3 == s1);
			/* State copy constructor copies the distribution. */
			State s6(s3);
			ASSERT(s6.distribution() == s3.distribution());
			/* DiscreteDistribution is constructible from an initializer_list of (std::string, double) pairs. */
			DiscreteDistribution dist2({{"A", 0.2}, {"G", 0.4}, {"C", 0.1}, {"T", 0.3}});
			ASSERT(dist2["A"] == 0.2);
			ASSERT((dist2["A"] = 0.5) == 0.5);
			/* Copy constructor/assignment operator for DiscreteDistribution. */
			DiscreteDistribution dist3 = dist2;
			ASSERT(dist3["A"] == 0.5);
			ASSERT(dist2 == dist3);
			/* Accessing a symbol not contained by the distribution will call the default
			constructor of the symbol and add it to the distribution
			(this has ultimately the same behavior as std::map). */
			double default_value = dist2["NotKey"];
			ASSERT(double() == default_value);
			/* Accessing "NotKey" created an entry in the distribution : (NotKey : 0). */
			ASSERT(dist2["NotKey"] == default_value);
			/* Thus, distributions will now differ. */
			ASSERT((dist2 != dist3));
		)
		
		TEST_UNIT(
			"add/remove state",
			HiddenMarkovModel hmm = HiddenMarkovModel();
			State s("s");
			/* Not yet added */
			ASSERT(!hmm.has_state("s"));
			ASSERT(!hmm.has_state(s));
			hmm.add_state(s);
			/* Check if the state was added to the hmm. */
			ASSERT(hmm.has_state("s"));
			ASSERT(hmm.has_state(s));
			/* Try to add an existing state. */
			ASSERT_EXCEPT(hmm.add_state(s), StateExistsException);
			hmm.remove_state("s");
			/* Remove a state not contained by the hmm. */
			ASSERT_EXCEPT(hmm.remove_state("s"), StateNotFoundException);
			ASSERT(!hmm.has_state("s"));
			ASSERT(!hmm.has_state(s));
		)

		TEST_UNIT(
			"add/remove transition",
			HiddenMarkovModel hmm = HiddenMarkovModel();
			State s1("s1");
			State s2("s2");
			hmm.add_state(s1);
			/* Throw an exception when a transition is added with a state not contained by the hmm. */
			ASSERT_EXCEPT(hmm.add_transition(s1, s2, 0.3), StateNotFoundException);
			hmm.add_state(s2);
			/* State is added, transition OK. */
			hmm.add_transition(s1, s2, 0.3);
			/* Check if transition exists. */
			ASSERT(hmm.has_transition(s1, s2));
			ASSERT(!hmm.has_transition(s2, s1));
			/* Removing it twice throws an exception. */
			hmm.remove_transition(s1, s2);
			ASSERT_EXCEPT(hmm.remove_transition(s1, s2), TransitionNotFoundException);
			/* Check successful removal. */
			ASSERT(!hmm.has_transition(s1, s2));
			/* Re-add it. */
			hmm.add_transition(s1, s2, 0.3);
			ASSERT(hmm.has_transition(s1, s2));
			/* Set it. */
			hmm.set_transition(s1, s2, 0.1);
			ASSERT(hmm.get_transition(s1, s2) == 0.1);
			/* Add transition where from == to. */
			hmm.add_transition(s1, s1, 0.9);
			ASSERT(hmm.has_transition(s1, s1));
			/* Remove it. */
			hmm.remove_transition(s1, s1);
			ASSERT(!hmm.has_transition(s1, s1));
			/* Removing a state removes its transitions from and to other states. */
			hmm.remove_state(s1);
			ASSERT(!hmm.has_transition(s1, s2));
		)

		TEST_UNIT(
			"initial probability aka pi",
			State s1("s1");
			HiddenMarkovModel hmm;
			hmm.add_state(s1);
			/* Set initial probability by adding a transition to the being state of the hmm. */
			hmm.add_transition(hmm.begin(), s1, 0.4);
			ASSERT(hmm.has_state(s1));
			ASSERT(hmm.has_transition(hmm.begin(), s1));
			/* Or by directly calling begin_transition. */
			State s2("s2");
			hmm.add_state(s2);
			hmm.begin_transition(s2, 0.5);
			ASSERT(hmm.has_transition(hmm.begin(), s2));
		)

		TEST_UNIT(
			"save/load hmm",
			std::string tmp_filename = "test_hmm_file_tmp";
			std::string extension = "hmm";
			std::string hmm_name = "save_test";
			DiscreteDistribution save_s1_dist({{"a", 0.8}, {"b", 0.2}});
			DiscreteDistribution save_s2_dist({{"c", 0.2}, {"a", 0.5}});
			HiddenMarkovModel hmm(hmm_name);
			hmm.set_training(LinearMemoryBaumWelchTraining(nullptr));
			State save_s1("save_s1", save_s1_dist);
			State save_s2("save_s2", save_s2_dist);
			State save_s3("save_s3");
			hmm.add_state(save_s1);
			hmm.add_state(save_s2);
			hmm.add_state(save_s3);
			double save_s1_save_s1_trans = 0.1;
			double save_s1_save_s2_trans = 0.2;
			double save_s2_save_s2_trans = 0.3;
			double save_s2_save_s3_trans = 0.9;
			double save_s1_begin = 0.5;
			double save_s2_end = 0.89;
			hmm.add_transition(save_s1, save_s1, save_s1_save_s1_trans);
			hmm.add_transition(save_s1, save_s2, save_s1_save_s2_trans);
			hmm.add_transition(save_s2, save_s2, save_s2_save_s2_trans);
			hmm.add_transition(save_s2, save_s3, save_s2_save_s3_trans);
			hmm.begin_transition(save_s1, save_s1_begin);
			hmm.end_transition(save_s2, save_s2_end);
			hmm.save(tmp_filename, extension);
			
			HiddenMarkovModel loaded_hmm;
			loaded_hmm.load(tmp_filename, extension);
			std::remove(std::string(tmp_filename + "." + extension).c_str());

			ASSERT(hmm.forward_type() == loaded_hmm.forward_type());
			ASSERT(hmm.backward_type() == loaded_hmm.backward_type());
			ASSERT(hmm.decoding_type() == loaded_hmm.decoding_type());
			ASSERT(hmm.training_type() == loaded_hmm.training_type());
			ASSERT(hmm.begin() == loaded_hmm.begin());
			ASSERT(hmm.end() == loaded_hmm.end());
			ASSERT(loaded_hmm.has_state(save_s1));
			ASSERT(loaded_hmm.has_state(save_s2));
			ASSERT(loaded_hmm.get_state(save_s1).distribution() == save_s1_dist);
			ASSERT(loaded_hmm.get_state(save_s2).distribution() == save_s2_dist);
			ASSERT_EXCEPT(loaded_hmm.get_state(save_s3).distribution(), StateDistributionException);
			ASSERT(loaded_hmm.get_transition(save_s1, save_s2) == save_s1_save_s2_trans);
			ASSERT(loaded_hmm.get_transition(save_s1, save_s1) == save_s1_save_s1_trans);
			ASSERT(loaded_hmm.get_transition(save_s2, save_s3) == save_s2_save_s3_trans);
			ASSERT(loaded_hmm.get_transition(loaded_hmm.begin(), save_s1) == save_s1_begin);
			ASSERT(loaded_hmm.get_transition(save_s2, loaded_hmm.end()) == save_s2_end);
			ASSERT_EXCEPT(loaded_hmm.get_transition(save_s2, save_s1), TransitionNotFoundException);
		)

		TEST_UNIT(
			"brew",
			HiddenMarkovModel hmm;
			DiscreteDistribution dist1 = DiscreteDistribution({{"A",0.3}, {"T", 0.2}, {"G", 0.5}});
			hmm.add_state(State("s1", dist1));
			dist1["C"] = 0.2;
			hmm.add_state(State("s2", dist1));
			double s2_t = 0.5;
			hmm.add_transition("s1", "s2", s2_t);
			/* No begin transition throws exception. */
			ASSERT_EXCEPT(hmm.brew(), std::logic_error);
			hmm.add_transition(hmm.begin(), "s1", 1);
			/* Each state needs to have at least one out transition with prob > 0. */
			ASSERT_EXCEPT(hmm.brew(), std::logic_error);
			hmm.add_transition("s2","s1",1);
			/* Now OK ! */
			hmm.brew();
			std::size_t s1_index = hmm.states_indices()["s1"];
			std::size_t s2_index =  hmm.states_indices()["s2"];
			double brewed_transition = hmm.raw_transitions()[s1_index][s2_index];
			ASSERT(brewed_transition == log(1.0));
			hmm.add_state(State("s3", dist1));
			hmm.add_state(State("s4", dist1));
			double s3_t = 0.2;
			double s4_t = 0.3;
			hmm.add_transition("s1","s3", s3_t);
			hmm.add_transition("s1","s4", s4_t);
			/* Set sum of out transitions > 0 for each state. */
			hmm.add_transition("s3","s1", s3_t);
			hmm.add_transition("s4","s1", s4_t);
			hmm.brew();
			ASSERT(hmm.raw_transitions().size() == 4);
			ASSERT(hmm.raw_pdfs().size() == 4);
			std::size_t s1_i = hmm.states_indices()["s1"];
			std::size_t s2_i =  hmm.states_indices()["s2"];
			std::size_t s3_i =  hmm.states_indices()["s3"];
			std::size_t s4_i =  hmm.states_indices()["s4"];
			double t_2 = hmm.raw_transitions()[s1_i][s2_i];
			double t_3 = hmm.raw_transitions()[s1_i][s3_i];
			double t_4 = hmm.raw_transitions()[s1_i][s4_i];
			/* Sum to 1, no normalization was needed, same probabilities. */
			ASSERT(t_2 == log(s2_t));
			ASSERT(t_3 == log(s3_t));
			ASSERT(t_4 == log(s4_t));
			ASSERT(utils::round_double(exp((*(hmm.raw_pdfs()[s2_index]))["A"])) == 0.25);
			hmm.add_state(State("s5", dist1));
			hmm.add_state(State("s6", dist1));
			double s5_t = 0.2; 
			double s6_t = 0.6;
			hmm.remove_transition("s2","s1");
			hmm.add_transition("s2","s5",s5_t);
			hmm.add_transition("s2","s6",s6_t);
			hmm.add_transition("s5","s2",s5_t);
			hmm.add_transition("s6","s2",s6_t);
			hmm.brew();
			std::size_t s2_n = hmm.states_indices()["s2"];
			std::size_t s5_n = hmm.states_indices()["s5"];
			std::size_t s6_n = hmm.states_indices()["s6"];
			double t_5 = hmm.raw_transitions()[s2_n][s5_n];
			double t_6 = hmm.raw_transitions()[s2_n][s6_n];
			/* sum is 0.2 + 0.6 thus with normalization 0.2 becomes 0.25, 0.6 become 0.75. */
			ASSERT(utils::round_double(exp(t_5)) == 0.25);
			ASSERT(utils::round_double(exp(t_6)) == 0.75);
		)

		TEST_UNIT(
			"forward",
			HiddenMarkovModel hmm = casino_hmm;
			std::vector<std::string> symbols = casino_symbols;
			/* Test init forward aka t = 1 aka first forward column. */
			/* Second parameter gives t. */
			std::vector<double> init_fwd = hmm.forward(symbols, 1);
			ASSERT(init_fwd.size() == 2);
			double init_fwd_fair = utils::round_double(exp(init_fwd[hmm.states_indices()["fair"]]), 2);
			double init_fwd_biased = utils::round_double(exp(init_fwd[hmm.states_indices()["biased"]]), 3);
			ASSERT(init_fwd_fair == casino_precomputed_init_fwd_fair);
			ASSERT(init_fwd_biased == casino_precomputed_init_fwd_biased);
			/* Test middle column. */
			std::vector<double> mid_fwd = hmm.forward(symbols, 4);
			double mid_fwd_fair = utils::round_double(exp(mid_fwd[hmm.states_indices()["fair"]]), 4);
			double mid_fwd_biased = utils::round_double(exp(mid_fwd[hmm.states_indices()["biased"]]), 4);
			ASSERT(mid_fwd_fair == casino_precomputed_mid_fwd_fair);
			ASSERT(mid_fwd_biased == casino_precomputed_mid_fwd_biased);
			/* Test last fwd column. */
			/* T not given, iterate on all the symbols. */
			std::vector<double> fwd_end = hmm.forward(symbols);
			double end_fwd_fair = utils::round_double(exp(fwd_end[hmm.states_indices()["fair"]]), 4);
			double end_fwd_biased = utils::round_double(exp(fwd_end[hmm.states_indices()["biased"]]), 4);
			ASSERT(end_fwd_fair == casino_precomputed_end_fwd_fair);
			ASSERT(end_fwd_biased == casino_precomputed_end_fwd_biased);
		)

		TEST_UNIT(
			"backward",
			/* Same model as forward test. */
			HiddenMarkovModel hmm = casino_hmm;
			std::vector<std::string> symbols = casino_symbols;
			/* Test init backward. */
			std::vector<double> init_bwd = hmm.backward(symbols, symbols.size());
			ASSERT(init_bwd.size() == 2);
			double init_bwd_fair = utils::round_double(exp(init_bwd[hmm.states_indices()["fair"]]), 3);
			double init_bwd_biased = utils::round_double(exp(init_bwd[hmm.states_indices()["biased"]]), 3);
			ASSERT(init_bwd_fair == casino_precomputed_init_bwd_fair);
			ASSERT(init_bwd_biased == casino_precomputed_init_bwd_biased);
			/* Test middle column. */
			std::vector<double> mid_bwd = hmm.backward(symbols, 4);
			double mid_bwd_fair = utils::round_double(exp(mid_bwd[hmm.states_indices()["fair"]]), 4);
			double mid_bwd_biased = utils::round_double(exp(mid_bwd[hmm.states_indices()["biased"]]), 4);
			ASSERT(mid_bwd_fair == casino_precomputed_mid_bwd_fair);
			ASSERT(mid_bwd_biased == casino_precomputed_mid_bwd_biased);
			/* Test last backward column. */
			std::vector<double> bwd_end = hmm.backward(symbols);
			double end_bwd_fair = utils::round_double(exp(bwd_end[hmm.states_indices()["fair"]]), 4);
			double end_bwd_biased = utils::round_double(exp(bwd_end[hmm.states_indices()["biased"]]), 4);
			ASSERT(end_bwd_fair == casino_precomputed_end_bwd_fair);
			ASSERT(end_bwd_biased == casino_precomputed_end_bwd_biased);
		)

		TEST_UNIT(
			"observation likelihood (casino)",
			/* Same model as fwd/bwd. */
			HiddenMarkovModel hmm = casino_hmm;
			/* Test likelihood with forward algorithm. */
			double forward_observation_likelihood = utils::round_double(hmm.likelihood(casino_symbols), 4);
			ASSERT(forward_observation_likelihood == casino_precomputed_likelihood);
			/* Test likelihood with backward algorithm. */
			double backward_observation_likelihood = utils::round_double(hmm.likelihood(casino_symbols, false), 4);
			ASSERT(backward_observation_likelihood == casino_precomputed_likelihood);
		)

		TEST_UNIT(
			"observation likelihood (profile)",
				HiddenMarkovModel hmm = profile_10_states_hmm;
				std::size_t n_tests = 3;
				/* Randomly choose n_tests sequences. Useless to test all 25 sequences. */
				for(std::size_t i = 0; i < n_tests; ++i){
					std::size_t random_sequence = (std::size_t)rand() % profile_observation_likelihood_sequences.size();	
					const std::vector<std::string>& seq = profile_observation_likelihood_sequences[random_sequence];
					double forward_observation_likelihood = utils::round_double(hmm.log_likelihood(seq), 4);
					ASSERT(forward_observation_likelihood == precomputed_profile_observation_likelihoods[random_sequence]);
					double backward_observation_likelihood = utils::round_double(hmm.log_likelihood(seq,false), 4);
					ASSERT(backward_observation_likelihood == precomputed_profile_observation_likelihoods[random_sequence]);
				}
		)

		TEST_UNIT(
			"viterbi decode (casino)",
			HiddenMarkovModel hmm = casino_hmm;
			std::vector<std::string> symbols = casino_symbols;
			std::vector<std::string> viterbi_path_2_states = hmm.decode(symbols).first;
			ASSERT(viterbi_path_2_states == casino_precomputed_viterbi_path_2_states);
		)

		TEST_UNIT(
			"viterbi decode/likelihood (nucleobase)",
			HiddenMarkovModel hmm = nucleobase_3_states_hmm;
			auto viterbi_decode = hmm.decode(nucleobase_symbols);
			std::vector<std::string> viterbi_path_3_states = viterbi_decode.first;
			double viterbi_log_likelihood = utils::round_double(viterbi_decode.second, 4);
			ASSERT(viterbi_log_likelihood == nucleobase_precomputed_viterbi_log_likelihood);
			ASSERT(viterbi_path_3_states == nucleobase_precomputed_viterbi_path_3_states);
		)

		TEST_UNIT(
			"viterbi decode (profile)",
			HiddenMarkovModel hmm = profile_10_states_hmm;
			std::size_t n_tests = 3;
			std::vector<std::string> viterbi_path;
			double viterbi_log_likelihood;
			/* Randomly choose n_tests sequences. Useless to test all 25 sequences. */
			for(std::size_t i = 0; i < n_tests; ++i){
				std::size_t random_sequence = (std::size_t)rand() % profile_viterbi_decode_sequences.size();	
				const std::vector<std::string>& seq = profile_viterbi_decode_sequences[random_sequence];
				std::tie(viterbi_path, viterbi_log_likelihood) = hmm.decode(seq);
				viterbi_log_likelihood = utils::round_double(viterbi_log_likelihood, 4);
				ASSERT(viterbi_path == precomputed_profile_viterbi_paths[random_sequence]);
				ASSERT(viterbi_log_likelihood == precomputed_profile_viterbi_log_likelihoods[random_sequence]);
			}
		)

		TEST_UNIT(
			"viterbi training (batch of sequences) basic (casino)",
			HiddenMarkovModel hmm = casino_hmm;
			hmm.set_training(LinearMemoryViterbiTraining(nullptr));
			double viterbi_improvement = utils::round_double(hmm.train(casino_training_sequences_2), 4);
			std::vector<std::vector<double>> viterbi_trained_transitions = hmm.raw_transitions();
			exp_all(viterbi_trained_transitions);
			round_all(viterbi_trained_transitions, 4);
			std::vector<double> viterbi_trained_pi_begin = hmm.raw_pi_begin();
			exp_all(viterbi_trained_pi_begin);
			round_all(viterbi_trained_pi_begin, 4);
			std::vector<DiscreteDistribution> viterbi_trained_distributions;
			for(auto dist_p : hmm.raw_pdfs()){
				if(dist_p != nullptr){
					viterbi_trained_distributions.push_back(*((DiscreteDistribution*)dist_p));
				}
			}
			exp_all(viterbi_trained_distributions);
			round_all(viterbi_trained_distributions, 4);
			ASSERT(viterbi_improvement == casino_precomputed_viterbi_improvement);
			ASSERT(viterbi_trained_transitions == casino_precomputed_viterbi_trained_transitions);
			ASSERT(viterbi_trained_pi_begin == casino_precomputed_viterbi_trained_pi_begin);
			ASSERT(viterbi_trained_distributions == casino_precomputed_viterbi_trained_distributions);
		)

		TEST_UNIT(
			"viterbi training (batch of sequences) with pseudocounts (casino)",
			HiddenMarkovModel hmm = casino_hmm;
			hmm.set_training(LinearMemoryViterbiTraining(nullptr));
			double viterbi_improvement = utils::round_double(hmm.train(casino_training_sequences_2, 1.0), 4);
			std::vector<std::vector<double>> viterbi_trained_transitions = hmm.raw_transitions();
			exp_all(viterbi_trained_transitions);
			round_all(viterbi_trained_transitions, 4);
			std::vector<double> viterbi_trained_pi_begin = hmm.raw_pi_begin();
			exp_all(viterbi_trained_pi_begin);
			round_all(viterbi_trained_pi_begin, 4);
			std::vector<DiscreteDistribution> viterbi_trained_distributions;
			for(auto dist_p : hmm.raw_pdfs()){
				if(dist_p != nullptr){
					viterbi_trained_distributions.push_back(*((DiscreteDistribution*)dist_p));
				}
			}
			exp_all(viterbi_trained_distributions);
			round_all(viterbi_trained_distributions, 4);
			ASSERT(viterbi_improvement == casino_precomputed_viterbi_improvement_pc);
			ASSERT(viterbi_trained_transitions == casino_precomputed_viterbi_trained_transitions_pc);
			ASSERT(viterbi_trained_pi_begin == casino_precomputed_viterbi_trained_pi_begin_pc);
			ASSERT(viterbi_trained_distributions == casino_precomputed_viterbi_trained_distributions_pc);
		)

		TEST_UNIT(
			"viterbi training (batch of sequences) with silent states (profile)",
			HiddenMarkovModel hmm = profile_10_states_hmm;
			hmm.set_training(LinearMemoryViterbiTraining(nullptr));
			double viterbi_improvement = utils::round_double(hmm.train(profile_training_sequences_1), 4);
			std::vector<std::vector<double>> viterbi_trained_transitions = hmm.raw_transitions();
			exp_all(viterbi_trained_transitions);
			round_all(viterbi_trained_transitions, 4);
			std::vector<double> viterbi_trained_pi_begin = hmm.raw_pi_begin();
			exp_all(viterbi_trained_pi_begin);
			round_all(viterbi_trained_pi_begin, 4);
			std::vector<DiscreteDistribution> viterbi_trained_distributions;
			for(auto dist_p : hmm.raw_pdfs()){
				if(dist_p != nullptr){
					viterbi_trained_distributions.push_back(*((DiscreteDistribution*)dist_p));
				}
			}
			exp_all(viterbi_trained_distributions);
			round_all(viterbi_trained_distributions, 4);
			ASSERT(viterbi_improvement == profile_precomputed_viterbi_improvement);
			ASSERT(viterbi_trained_transitions == profile_precomputed_viterbi_trained_transitions);
			ASSERT(viterbi_trained_pi_begin == profile_precomputed_viterbi_trained_pi_begin);
			ASSERT(viterbi_trained_distributions == profile_precomputed_viterbi_trained_distributions);
		)

		TEST_UNIT(
			"viterbi training (batch of sequences) with pseudocounts and with silent states (profile)",
			HiddenMarkovModel hmm = profile_10_states_hmm;
			hmm.set_training(LinearMemoryViterbiTraining(nullptr));
			double viterbi_improvement = utils::round_double(hmm.train(profile_training_sequences_1, 1.0), 4);
			std::vector<std::vector<double>> viterbi_trained_transitions = hmm.raw_transitions();
			exp_all(viterbi_trained_transitions);
			round_all(viterbi_trained_transitions, 4);
			std::vector<double> viterbi_trained_pi_begin = hmm.raw_pi_begin();
			exp_all(viterbi_trained_pi_begin);
			round_all(viterbi_trained_pi_begin, 4);
			std::vector<DiscreteDistribution> viterbi_trained_distributions;
			for(auto dist_p : hmm.raw_pdfs()){
				if(dist_p != nullptr){
					viterbi_trained_distributions.push_back(*((DiscreteDistribution*)dist_p));
				}
			}
			exp_all(viterbi_trained_distributions);
			round_all(viterbi_trained_distributions, 4);
			ASSERT(viterbi_improvement == profile_precomputed_viterbi_improvement_pc);
			ASSERT(viterbi_trained_transitions == profile_precomputed_viterbi_trained_transitions_pc);
			ASSERT(viterbi_trained_pi_begin == profile_precomputed_viterbi_trained_pi_begin_pc);
			ASSERT(viterbi_trained_distributions == profile_precomputed_viterbi_trained_distributions_pc);
		)

		TEST_UNIT(
			"baum-welch training (batch of sequences) basic (casino)",
			HiddenMarkovModel hmm = casino_hmm;
			hmm.set_training(LinearMemoryBaumWelchTraining(nullptr));
			double bw_improvement = utils::round_double(hmm.train(casino_training_sequences_2), 4);
			std::vector<std::vector<double>> bw_trained_transitions = hmm.raw_transitions();
			exp_all(bw_trained_transitions);
			round_all(bw_trained_transitions, 4);
			std::vector<double> bw_trained_pi_begin = hmm.raw_pi_begin();
			exp_all(bw_trained_pi_begin);
			round_all(bw_trained_pi_begin, 4);
			std::vector<DiscreteDistribution> bw_trained_distributions;
			for(auto dist_p : hmm.raw_pdfs()){
				if(dist_p != nullptr){
					bw_trained_distributions.push_back(*((DiscreteDistribution*)dist_p));
				}
			}
			exp_all(bw_trained_distributions);
			round_all(bw_trained_distributions, 4);
			ASSERT(bw_improvement == casino_precomputed_bw_improvement);
			ASSERT(bw_trained_transitions == casino_precomputed_bw_trained_transitions);
			ASSERT(bw_trained_pi_begin == casino_precomputed_bw_trained_pi_begin);
			ASSERT(bw_trained_distributions == casino_precomputed_bw_trained_distributions);
		)

		TEST_UNIT(
			"baum-welch training (1 sequence) with end state (nucleobase)",
			HiddenMarkovModel hmm = nucleobase_3_states_hmm;
			hmm.set_training(LinearMemoryBaumWelchTraining(nullptr));
			double bw_improvement = utils::round_double(hmm.train(nucleobase_training_sequences), 4);
			std::vector<std::vector<double>> bw_trained_transitions = hmm.raw_transitions();
			exp_all(bw_trained_transitions);
			round_all(bw_trained_transitions, 4);
			std::vector<double> bw_trained_pi_begin = hmm.raw_pi_begin();
			exp_all(bw_trained_pi_begin);
			round_all(bw_trained_pi_begin, 4);
			std::vector<double> bw_trained_pi_end = hmm.raw_pi_end();
			exp_all(bw_trained_pi_end);
			round_all(bw_trained_pi_end, 4);
			std::vector<DiscreteDistribution> bw_trained_distributions;
			for(auto dist_p : hmm.raw_pdfs()){
				if(dist_p != nullptr){
					bw_trained_distributions.push_back(*((DiscreteDistribution*)dist_p));
				}
			}
			exp_all(bw_trained_distributions);
			round_all(bw_trained_distributions, 4);
			ASSERT(bw_improvement == nucleobase_precomputed_bw_improvement);
			ASSERT(bw_trained_transitions == nucleobase_precomputed_bw_trained_transitions);
			ASSERT(bw_trained_pi_begin == nucleobase_precomputed_bw_trained_pi_begin);
			ASSERT(bw_trained_pi_end == nucleobase_precomputed_bw_trained_pi_end);
			ASSERT(bw_trained_distributions == nucleobase_precomputed_bw_trained_distributions);
		)

		TEST_UNIT(
			"baum-welch training (1 sequence and 1 iteration) with silent states and silent begin/end paths (profile)",
			HiddenMarkovModel hmm = profile_10_states_hmm;
			hmm.set_training(LinearMemoryBaumWelchTraining(nullptr));
			hmm.train(profile_training_sequences_2, 0.0, hmm_config::kDefaultConvergenceThreshold, hmm_config::kDefaultMinIterations, 1);
			std::vector<std::vector<double>> bw_trained_transitions = hmm.raw_transitions();
			exp_all(bw_trained_transitions);
			round_all(bw_trained_transitions, 4);
			std::vector<double> bw_trained_pi_begin = hmm.raw_pi_begin();
			exp_all(bw_trained_pi_begin);
			round_all(bw_trained_pi_begin, 4);
			std::vector<double> bw_trained_pi_end = hmm.raw_pi_end();
			exp_all(bw_trained_pi_end);
			round_all(bw_trained_pi_end, 4);
			std::vector<DiscreteDistribution> bw_trained_distributions;
			for(auto dist_p : hmm.raw_pdfs()){
				if(dist_p != nullptr){
					bw_trained_distributions.push_back(*((DiscreteDistribution*)dist_p));
				}
			}
			exp_all(bw_trained_distributions);
			round_all(bw_trained_distributions, 4);
			ASSERT(bw_trained_transitions == profile_precomputed_bw_1_iter_1_seq_trained_transitions);
			ASSERT(bw_trained_pi_begin == profile_precomputed_bw_1_iter_1_seq_trained_pi_begin);
			ASSERT(bw_trained_pi_end == profile_precomputed_bw_1_iter_1_seq_trained_pi_end);
			ASSERT(bw_trained_distributions == profile_precomputed_bw_1_iter_1_seq_trained_distributions);
		)

		TEST_UNIT(
			"baum-welch training (batch of sequences and 10 iterations) with silent states and silent begin/end paths (profile)",
			HiddenMarkovModel hmm = profile_10_states_hmm;
			hmm.set_training(LinearMemoryBaumWelchTraining(nullptr));
			hmm.train(profile_training_sequences_1, 0.0, hmm_config::kDefaultConvergenceThreshold, hmm_config::kDefaultMinIterations, 10);
			std::vector<std::vector<double>> bw_trained_transitions = hmm.raw_transitions();
			exp_all(bw_trained_transitions);
			round_all(bw_trained_transitions, 4);
			std::vector<double> bw_trained_pi_begin = hmm.raw_pi_begin();
			exp_all(bw_trained_pi_begin);
			round_all(bw_trained_pi_begin, 4);
			std::vector<double> bw_trained_pi_end = hmm.raw_pi_end();
			exp_all(bw_trained_pi_end);
			round_all(bw_trained_pi_end, 4);
			std::vector<DiscreteDistribution> bw_trained_distributions;
			for(auto dist_p : hmm.raw_pdfs()){
				if(dist_p != nullptr){
					bw_trained_distributions.push_back(*((DiscreteDistribution*)dist_p));
				}
			}
			exp_all(bw_trained_distributions);
			round_all(bw_trained_distributions, 4);
			ASSERT(bw_trained_transitions == profile_precomputed_bw_batch_trained_transitions);
			ASSERT(bw_trained_pi_begin == profile_precomputed_bw_batch_trained_pi_begin);
			ASSERT(bw_trained_pi_end == profile_precomputed_bw_batch_trained_pi_end);
			ASSERT(bw_trained_distributions == profile_precomputed_bw_batch_trained_distributions);
		)

		/* Test factory */

		/* Test fix / free parameters */

		/* Test update from raw model */

		/* Train stochastic EM */

		/* Profile HMM */

		/* MLE */

		/* Randomized params */

		tests_results();

	} catch(const std::exception& e){
		print_exception(e);
		return 1;
	}
	return 0;
}
