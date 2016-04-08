#ifndef __HIDDENMARKOVMODEL_HPP
#define __HIDDENMARKOVMODEL_HPP

#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>	
#include <math.h>	// log, exp
#include <utility>	// std::pair
#include <memory> // std::shared_ptr
#include <iomanip> // std::setprecision
#include "constants.hpp"
#include "state.hpp"
#include "graph.hpp"
#include "utils.hpp"
#define CYAN "\033[36m"
#define RESET "\033[0m"

template<typename Elem>
using Matrix = std::vector<std::vector<Elem>>;

std::string print_matrix(const Matrix<double>& matrix, const std::map<std::string, std::size_t>& indices, bool log_prob = false){
	std::size_t longest_string = 0;
	for(std::size_t i = 0; i < matrix.size(); ++i){
		for(std::size_t j = 0; j < matrix[i].size(); ++j){
			std::string double_string = (log_prob) ? std::to_string(matrix[i][j]) : std::to_string(exp(matrix[i][j]));
			if(double_string.length() > longest_string) longest_string = double_string.length();
		}
	}
	std::vector<std::string> sorted_names(indices.size());
	std::ostringstream out;
	for(auto& pair : indices) {
		sorted_names[pair.second] = pair.first;
	}
	out << std::string(longest_string + 1, ' ');
	for(std::string& name : sorted_names) {
		out << std::string(longest_string - name.length(), ' ');
		out << CYAN << name << RESET;
		out << ' ';
	}
	out << std::endl;
	for(std::size_t i = 0; i < matrix.size(); ++i){
		out << std::string(longest_string - sorted_names[i].length(), ' ');
		out << CYAN << sorted_names[i] << RESET;
		out << ' ';
		for(std::size_t j = 0; j < matrix[i].size(); ++j){
			std::string double_string = (log_prob) ? std::to_string(matrix[i][j]) : std::to_string(exp(matrix[i][j]));
			out << std::string(longest_string - double_string.length(), ' ');
			out << double_string;
			out << ' ';
		}
		out << std::endl;
	}
	return out.str();	
}

std::ostream& operator<<(std::ostream& out, const std::vector<double>& vec){
	for(double d : vec){
		out << exp(d) << " ";
	}
	out << std::endl;
	return out;
}

std::ostream& operator<<(std::ostream& out, const std::vector<std::size_t>& vec){
	for(std::size_t ul : vec){
		out << ul << " ";
	}
	out << std::endl;
	return out;
}

std::ostream& operator<<(std::ostream& out, const std::vector<std::string>& vec){
	for(const std::string& s : vec){
		out << s << " ";
	}
	out << std::endl;
	return out;
}

std::ostream& operator<<(std::ostream& out, const std::vector<Distribution*>& vec){
	for(const Distribution* dist : vec){
		if(dist == nullptr) out << "Silent" << std::endl;
		else out << *dist << std::endl;
	}
	return out;
}

/* <-------- Exceptions --------> */

class HMMException : public std::logic_error {
protected:
	HMMException(const std::string& message) :
		std::logic_error(message) {}
};

class StateNotFoundException : public HMMException {
public:
	template<typename T>
	StateNotFoundException(const T& t, const std::string& msg) : 
		HMMException(error_message::format("StateNotFoundException: " + msg, t)) {}

	StateNotFoundException(const std::string& msg) : 
		HMMException("StateNotFoundException: " + msg) {}
};

class StateExistsException : public HMMException {
public:
	template<typename T>
	StateExistsException(const T& t, const std::string& msg) : 
		HMMException(error_message::format("StateExistsException: " + msg, t)) {}
};

class TransitionNotFoundException : public HMMException {
public:
	template<typename T>
	TransitionNotFoundException(const T& t, const std::string& msg) : 
		HMMException(error_message::format("TransitionNotFoundException: " + msg, t)) {}

	TransitionNotFoundException(const std::string& msg) : 
		HMMException("TransitionNotFoundException: " + msg) {}
};

class TransitionExistsException : public HMMException {
public:
	template<typename T>
	TransitionExistsException(const T& t, const std::string& msg) : 
		HMMException(error_message::format("TransitionExistsException: " + msg, t)) {}
};

class TransitionLogicException : public HMMException {
public:
	template<typename T>
	TransitionLogicException(const T& t, const std::string& msg) : 
		HMMException(error_message::format("TransitionLogicException: " + msg, t)) {}
};

/* <----------------------------> */

class HiddenMarkovModel {
private:
	/* Name of this hmm. */
	std::string _name;

	/* Begin and end states. */
	State* _begin;
	State* _end;

	/* Holds the states and the transitions when building the hmm. */
	Graph<State> _graph;

	/* Generated by brew. */
	std::map<std::string, std::size_t> _states_indices;
	std::vector<std::string> _states_names;
	Matrix<double> _A;
	std::vector<Distribution*> _B;
	std::vector<double> _pi_begin;
	std::vector<double>_pi_end;
	bool _is_finite;
	std::size_t _silent_states_index;
	std::size_t _M; //TODO
	std::size_t _N;

	void _clear_raw_data() {
		for(Distribution* dist : _B){
			if(dist != nullptr) delete dist;
		}
		_states_indices.clear();
		_states_names.clear();
		_A.clear();
		_pi_begin.clear();
		_pi_end.clear();
		_is_finite = false;
		_silent_states_index = std::size_t();
		_M = std::size_t(); 
		_N = std::size_t();
	}

public:
	/* Default constructor. Inits an empty hmm with default values. */
	HiddenMarkovModel() : 
		HiddenMarkovModel(std::to_string((ptrdiff_t)this)) {}

	HiddenMarkovModel(const std::string& name) : 
		HiddenMarkovModel(name, State(hmm_config::kDefaultStartStateLabel + name), 
			State(hmm_config::kDefaultEndStateLabel + name)) {}

	HiddenMarkovModel(const State& begin, const State& end) :
		HiddenMarkovModel(std::to_string((ptrdiff_t)this), begin, end) {}

	/* Complete constructor. */
	HiddenMarkovModel(const std::string& name, const State& begin, const State& end) : 
		_name(name), _begin(nullptr), _end(nullptr), _graph() {
			_graph.add_vertex(begin);
			_begin = _graph.get_vertex(begin);
			_graph.add_vertex(end);
			_end = _graph.get_vertex(end);
	}

	std::string name() const { return _name; }
	void set_name(const std::string& name) { _name = name; } 
	std::size_t num_states() const { return _graph.num_vertices(); }
	std::size_t num_transitions() const { return _graph.num_edges(); }	

	bool has_state(const State& state) const {
		return _graph.has_vertex(state);
	}

	bool has_transition(const State& from_state, const State& to_state) const {
		return _graph.has_edge(from_state, to_state);
	}

	State& begin() { 
		if(_begin != nullptr){
			return *_begin;
		}
		else{
			throw StateNotFoundException(error_message::kHMMHasNoBeginState);
		}
	}

	State& end() {
		if(_end != nullptr){
			return *_end;
		}
		else{
			throw StateNotFoundException(error_message::kHMMHasNoEndState);
		}
	}

	/* See behavior of Graph::add_vertex() */	
	void add_state(const State& state){
		try{
			_graph.add_vertex(state);	
		}
		catch(const VertexExistsException<State>& e){
			throw StateExistsException(e.trigger(), error_message::kHMMAddStateExists);
		}
	}

	/* See behavior of Graph::remove_vertex() */
	void remove_state(const State& state){
		if(state == *_begin) _begin = nullptr;
		else if(state == *_end) _end = nullptr;
		try{
			_graph.remove_vertex(state);	
		}
		catch(const VertexNotFoundException<State>& e){
			throw StateNotFoundException(e.trigger(), error_message::kHMMRemoveStateNotFound);
		}
	}

	std::string transition_string(const State& from, const State& to) const {
		return from.to_string() + " -> " + to.to_string();
	}

	/* See behavior of Graph::add_edge() */
	void add_transition(const State& from, const State& to, double probability){
		if(from == end()) throw TransitionLogicException(transition_string(from, to), error_message::kAddedTransitionFromEndState);
		if(to == begin()) throw TransitionLogicException(transition_string(from, to), error_message::kAddedTransitionToBeginState);
		if(probability < 0) throw TransitionLogicException(transition_string(from, to), error_message::kAddedTransitionNegativeProbability);
		try{
			_graph.add_edge(from, to, probability);	
		} 
		catch(const EdgeExistsException<Edge<State>>& e){
			throw TransitionExistsException(transition_string(*(e.trigger().from()), *(e.trigger().to())), error_message::kHMMAddTransitionExists);
		}
		catch(const IncidentVertexNotFoundException<State>& e){
			throw StateNotFoundException(e.trigger(), error_message::kAddTransitionStateNotFound);
		}
	}

	void begin_transition(const State& state, double probability) {
		add_transition(begin(), state, probability);
	}

	void end_transition(const State& state, double probability) {
		add_transition(state, end(), probability);
	}

	/* See behavior of Graph::remove_edge() */
	void remove_transition(const State& from, const State& to){
		try{
			_graph.remove_edge(from, to);	
		}
		catch(const EdgeNotFoundException<Edge<State>>& e){
			throw TransitionNotFoundException(transition_string(*(e.trigger().from()), *(e.trigger().to())), error_message::kHMMRemoveTransitionNotFound);
		}
		
	}

	/* Prepares the hmm before calling algorithms on it. */
	void brew() {
		/* Get rid of previous data. */
		_clear_raw_data();

		/* Get the states from graph. */
		std::vector<State*> states = _graph.get_vertices();

		/* Remove begin and end states. */
		states.erase(std::remove_if(states.begin(), states.end(), [this](State* p_state){ return (*p_state) == begin() || (*p_state) == end(); }), states.end());
		std::vector<State*> silent_states;
		std::size_t num_states = states.size();

		/* Keep track of the matrix index of each state. */
		std::map<std::string, std::size_t> states_indices;
		std::vector<std::string> states_names(num_states);

		/* Init size of raw transition matrix. */
		Matrix<double> A(num_states);
		std::vector<double> pi_begin(num_states, utils::kNegInf);
		std::vector<double> pi_end(num_states, utils::kNegInf);

		/* Transitions to end state exist and are not empty. */
		bool finite = false;

		/* Check if silent/end states are indeed silent. */
		if(!begin().is_silent()) { throw std::logic_error("begin state has to be silent."); }
		if(!end().is_silent()) { throw std::logic_error("end state has to be silent."); }

		std::size_t normal_states_index = 0;
		for(State* p_state : states){
			if(p_state->is_silent()) {
				silent_states.push_back(p_state);
			}
			else{
				A[normal_states_index] = std::vector<double>(num_states, utils::kNegInf);
				/* Map state name to row index. */
				states_indices[p_state->name()] = normal_states_index;
				states_names[normal_states_index] = p_state->name();
				++normal_states_index;
			}
		}
		std::size_t num_silent_states = silent_states.size();
		/* This points to the beginning of the silent states array in A and B. */
		std::size_t silent_states_index = normal_states_index;
		/* We use a topological sort on the silent states sub graph in order to adapt 
		the HMM to silent states. */
		/* See p. 71 at http://www.upch.edu.pe/facien/fc/dbmbqf/zimic/ubioinfo/bks/Bioinformatics/Biological%20Sequence%20Analysis%20Hmm%20Bioinformatics%20(Durbin).pdf */
		/* Start by dereferencing silent states pointers to pass them to subgraph. */
		std::vector<State> silent_states_values;
		silent_states_values.reserve(num_silent_states);
		for(State* p_state : silent_states) { silent_states_values.push_back(*p_state); }
		Graph<State> subgraph = _graph.sub_graph(silent_states_values);
		subgraph.topological_sort();
		/* Get toposorted silent states. */
		silent_states = subgraph.get_vertices();
		/* Init the toposorted silent states rows in the matrix. */
		for(State* p_silent_state : silent_states){
			A[silent_states_index] = std::vector<double>(num_states, utils::kNegInf);
			states_indices[p_silent_state->name()] = silent_states_index;
			states_names[silent_states_index] = p_silent_state->name();
			++silent_states_index;
		}

		/* Fill transitions with log probabilities and check whether a normalization is needed. */
		auto fill_normalize = [this, &pi_end, &states_indices] (const std::vector<Edge<State>*>& edges, std::vector<double>& prob_vec_to_fill) {
			std::vector<double> vec_to_normalize;
			vec_to_normalize.reserve(edges.size());
			double prob_sum = 0;
			double prob;
			for(Edge<State>* edge : edges){ 
				prob = (edge->weight() == nullptr) ? 0 : *(edge->weight());
				prob_sum += prob;
				vec_to_normalize.push_back(log(prob));
			}
			if(prob_sum != 1.0){
				utils::for_each_log_normalize(vec_to_normalize.begin(), vec_to_normalize.end(), log(prob_sum));
			}
			std::size_t i = 0;
			for(double& log_prob : vec_to_normalize){
				if(*(edges[i]->to()) == end()){
					pi_end[states_indices[edges[i]->from()->name()]] = log_prob;
				}
				else{
					prob_vec_to_fill[states_indices[edges[i]->to()->name()]] = log_prob;
				}
				i++;
			}
			return prob_sum;
		};

		/* Add the begin state transitions. */
		State& begin_state = begin();
		if(_graph.get_in_edges(begin_state).size() > 0) { throw std::logic_error("begin state cannot have predecessors"); }
		std::vector<Edge<State>*> out_edges = _graph.get_out_edges(begin_state);
		/* Begin transitions are added in the first row of A s.t. A[0][i] == pi[i]. */
		double prob_sum = fill_normalize(out_edges, pi_begin);
		if(prob_sum == 0.0) { throw std::logic_error("hmm has no begin transition"); }
		
		/* Check if end state has out edges. */
		State& end_state = end();
		if(_graph.get_out_edges(end_state).size() > 0) { throw std::logic_error("end state cannot have successors"); }

		/* Iterate through all the other states and add them to the matrix. */
		for(State* p_state : states){
			/* Fill normal transitions aka matrix A. */
			std::vector<Edge<State>*> out_edges = _graph.get_out_edges(*p_state);
			double prob_sum = fill_normalize(out_edges, A[states_indices[p_state->name()]]);
			if(prob_sum == 0.0) { throw std::logic_error("hmm has no transition from " + p_state->to_string()); }
		}

		/* Determine whether the hmm is finite by summing the end state in transitions probabilities. */
		double prob_sum_to_end = utils::kNegInf;
		for(std::size_t i = 0; i < A.size(); ++i) { prob_sum_to_end += exp(A[i][0]); }
		if(prob_sum_to_end > 0.0) { finite = true; }

		/* Fill emission matrix with the states PDFs. */
		std::vector<Distribution*> B(num_states);
		for(State* p_state : states){
			Distribution* distribution = nullptr;
			if(! p_state->is_silent()) {
				distribution = p_state->distribution().clone();
				distribution->log_normalize();
			}
			B[states_indices[p_state->name()]] = distribution;
		}
		/* Set fields for the hmm raw values. */
		_A = std::move(A);
		_B = std::move(B);
		_pi_begin = std::move(pi_begin);
		_pi_end = std::move(pi_end);
		_states_indices = std::move(states_indices);
		_states_names = std::move(states_names);
		_is_finite = finite;
		_silent_states_index = silent_states_index;
	}

	Matrix<double>& raw_transitions() { return _A; }
	std::vector<Distribution*>& raw_pdfs() { return _B; }
	std::map<std::string, std::size_t>& states_indices() { return _states_indices; }

	template<typename SymbolContainer>
	std::vector<double> forward(const SymbolContainer& symbols, std::size_t t_max = 0) {
		if(t_max == 0) t_max = symbols.size();
		auto forward_init = [&symbols, this]() {
			std::vector<double> alpha_0(_A.size());
			/* First iterate over the silent states to compute the probability of
			passing through silent states before emitting the first symbol. */
			for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
				alpha_0[i] = _pi_begin[i];
				for(std::size_t j = _silent_states_index; j < i; ++j){
					alpha_0[i] = utils::sum_log_prob(alpha_0[i], _A[j][i] + alpha_0[j]);
				}
			}
			/* Fill alpha_0 for non-silent states. To compute alpha_0, we need to 
			sum the probability to directly begin at non-silent state i (pi)
			with the probabilities to transit from all the silent states which
			have a begin probability > 0 to non-silent state i. */
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				alpha_0[i] = _pi_begin[i];
				for(std::size_t j = _silent_states_index; j < _A.size(); ++j){
					alpha_0[i] = utils::sum_log_prob(alpha_0[i], _A[j][i] + alpha_0[j]);
				}
				
			}
			/* We can now compute alpha_1. */
			std::vector<double> alpha_1(_A.size());
			/* First iterate over non-silent states. */
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				alpha_1[i] = alpha_0[i] + (*_B[i])[symbols[0]];
			}
			/* Then silent states, in toporder. */
			for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
				alpha_1[i] = utils::kNegInf;
				for(std::size_t j = 0; j < i; ++j){
					alpha_1[i] = utils::sum_log_prob(alpha_1[i], _A[j][i] + alpha_1[j]);
				}
			}
			return alpha_1;
		};
		auto forward_step = [&symbols, this](const std::vector<double>& alpha_prev_t, std::size_t t) {
			std::vector<double> alpha_t(_A.size());
			/* Normal states. */
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				alpha_t[i] = utils::kNegInf;
				for(std::size_t j = 0; j < _A.size(); ++j){
					alpha_t[i] = utils::sum_log_prob(alpha_t[i], alpha_prev_t[j] + _A[j][i]);
				}
				alpha_t[i] = alpha_t[i] + (*_B[i])[symbols[t]];
			}
			/* Silent states. */
			for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
				alpha_t[i] = utils::kNegInf;
				for(std::size_t j = 0; j < i; ++j){
					alpha_t[i] = utils::sum_log_prob(alpha_t[i], alpha_t[j] + _A[j][i]);
				}
			}
			return alpha_t;
		};
		if(symbols.size() == 0) throw std::logic_error("forward on empty symbol list");
		else{
			std::vector<double> alpha = forward_init();
			for(std::size_t t = 1; t < std::min(symbols.size(), t_max); ++t) {
				alpha = forward_step(alpha, t);
			}
			return alpha;
		}
	}

	std::pair<std::vector<double>, double> forward_terminate(const std::vector<double>& alpha_T){
		double log_prob = utils::kNegInf;
		std::vector<double> alpha_end(_A.size());
		if(_is_finite){
			/* Sum all and add end transitions. */
			for(std::size_t i = 0; i < alpha_T.size(); ++i){
				alpha_end[i] = alpha_T[i] + _pi_end[i];
				log_prob = utils::sum_log_prob(log_prob, alpha_end[i]);
			}
		}
		else{
			/* Non finite hmm end in non-silent states. */
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				alpha_end[i] = alpha_T[i];
				log_prob = utils::sum_log_prob(log_prob, alpha_end[i]);
			}
			for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
				alpha_end[i] = utils::kNegInf;
			}
		}
		return std::make_pair(alpha_end, log_prob);
	}

	template<typename SymbolContainer>
	std::vector<double> backward(const SymbolContainer& symbols, std::size_t t_min = 0) {
		auto backward_init = [this]() {
			std::vector<double> beta_T(_A.size());
			if(_is_finite){
				for(std::size_t i = _A.size() - 1; i >= _silent_states_index; --i){
					beta_T[i] = _pi_end[i];
					for(std::size_t j = _A.size(); j > i; --j){
						beta_T[i] = utils::sum_log_prob(beta_T[i], _A[i][j] + beta_T[j]);
					}
				}
				for(std::size_t i = 0; i < _silent_states_index; ++i){
					beta_T[i] = _pi_end[i];
					for(std::size_t j = _silent_states_index; j < _A.size(); ++j){
						beta_T[i] = utils::sum_log_prob(beta_T[i], _A[i][j] + beta_T[j]); 
					}
				}
			}
			else{
				for(std::size_t i = 0; i < _silent_states_index; ++i){
					beta_T[i] = 0.0;
				}
				for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
					beta_T[i] = utils::kNegInf;
				}
			}
			return beta_T;
		};
		auto backward_step = [&symbols, this](const std::vector<double>& beta_next_t, std::size_t t) -> std::vector<double> {
			std::vector<double> beta_t(_A.size());
			/* First pass over silent states, considering next step non-silent states. */
			for(std::size_t i = _A.size() - 1; i >= _silent_states_index; --i){
				beta_t[i] = utils::kNegInf;
				for(std::size_t j = 0; j < _silent_states_index; j++){
					beta_t[i] = utils::sum_log_prob(beta_t[i], beta_next_t[j] + _A[i][j] + (*_B[j])[symbols[t + 1]]);
				}
			}
			/* Second pass over silent states, considering current step silent states. */
			for(std::size_t i = _A.size() - 1; i >= _silent_states_index; --i){
				for(std::size_t j = _silent_states_index; j < _A.size(); j++){
					beta_t[i] = utils::sum_log_prob(beta_t[i], beta_t[j] + _A[i][j]);
				}
			}
			/* Finally pass over non-silent states. */
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				beta_t[i] = utils::kNegInf;
				for(std::size_t j = 0; j < _silent_states_index; ++j){
					beta_t[i] = utils::sum_log_prob(beta_t[i], beta_next_t[j] + _A[i][j] + (*_B[j])[symbols[t + 1]]);
				}
				for(std::size_t j = _silent_states_index; j < _A.size(); ++j){
					beta_t[i] = utils::sum_log_prob(beta_t[i], beta_t[j] + _A[i][j]);
				}
			}
			return beta_t;
		};
		if(t_min > 0) --t_min;
		if(symbols.size() == 0) throw std::runtime_error("backward on empty symbol list");
		else{
			std::vector<double> beta = backward_init();
			for(std::size_t t = symbols.size() - 2; t >= t_min && t < symbols.size(); --t){
				beta = backward_step(beta, t);
			}
			return beta;
		}
	}

	template<typename SymbolContainer>
	std::pair<std::vector<double>, double> backward_terminate(const std::vector<double>& beta_0, SymbolContainer& symbols){
		std::vector<double> beta_end(_A.size());
			/* First pass over silent states, considering next step non-silent states. */
			for(std::size_t i = _A.size() - 1; i >= _silent_states_index; --i){
				beta_end[i] = utils::kNegInf;
				for(std::size_t j = 0; j < _silent_states_index; j++){
					beta_end[i] = utils::sum_log_prob(beta_end[i], beta_0[j] + _A[i][j] + (*_B[j])[symbols[0]]);
				}
			}
			/* Second pass over silent states, considering current step silent states. */
			for(std::size_t i = _A.size() - 1; i >= _silent_states_index; --i){
				for(std::size_t j = _silent_states_index; j < _A.size(); j++){
					beta_end[i] = utils::sum_log_prob(beta_end[i], beta_end[j] + _A[i][j]);
				}
			}
			double log_prob = utils::kNegInf;
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				beta_end[i] = _pi_begin[i] + (*_B[i])[symbols[0]] + beta_0[i];
				log_prob = utils::sum_log_prob(log_prob, beta_end[i]);
			}
			for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
				beta_end[i] = _pi_begin[i] +  beta_end[i];
				log_prob = utils::sum_log_prob(log_prob, beta_end[i]);
			}
			return std::make_pair(beta_end, log_prob);
	}

	template<typename SymbolContainer>
	double log_likelihood(const SymbolContainer& symbols, bool do_fwd = true){
		if(do_fwd){
			return forward_terminate(forward(symbols)).second;
		}
		else{
			return backward_terminate(backward(symbols), symbols).second;
		}
			// std::vector<double> bwd = backward(symbols);
			// /* Normal states first. */
			// for(std::size_t i = 0; i < _silent_states_index; ++i){
			// 	bwd[i] = _A[0][i] + (*_B[i])[symbols[0]] + bwd[i];	
			// }
			// /* Do first iteration over silent states j with i == normal state. */
			// for(std::size_t j = _silent_states_index; j < _A.size(); ++j){
			// 	double prob_sum = utils::kNegInf;
			// 	for(std::size_t i = 0; i < _silent_states_index; ++i){
			// 		prob_sum = utils::sum_log_prob(prob_sum, current_bwd[i] + _A[i][j]);
			// 	}
			// 	current_bwd[j] = prob_sum;
			// }
			// /* Do second iteration over silent states j with i == silent state < j. Reverse topological order. */
			// for(std::size_t j = _A.size() - 1; j >= _silent_states_index; --j){
			// 	double prob_sum = current_bwd[j];
			// 	for(std::size_t i = _A.size() - 1; i > j; --i){
			// 		prob_sum = utils::sum_log_prob(prob_sum, current_bwd[i] + _A[j][i]);
			// 	}
			// 	current_bwd[j] = prob_sum;
			// }
			// return current_bwd;

	}

	template<typename SymbolContainer>
	double likelihood(const SymbolContainer& symbols, bool do_fwd = true){
		return exp(log_likelihood(symbols, do_fwd));
	}

	void sample() {

	}

	struct Traceback {
		struct Node; // forward declaration.
		typedef std::shared_ptr<Node> NodePtr;
		struct Node{
			NodePtr previous;
			std::size_t value;
			Node(std::size_t v) : previous(), value(v) {}
			Node(std::size_t v, NodePtr p) : previous(p), value(v) {}
		};
		std::size_t nodes;
		std::vector<NodePtr> previous_nodes;
		std::vector<NodePtr> current_nodes;
		Traceback(std::size_t num_nodes) : 
			nodes(num_nodes), previous_nodes(nodes), current_nodes(nodes) {
				for(std::size_t i = 0; i < num_nodes; ++i){
					previous_nodes[i] = NodePtr(new Node(i));
				}
			}

		void add(std::size_t current){
			current_nodes[current] = NodePtr(new Node(current));
		}

		void add_link(std::size_t previous, std::size_t current, bool link_to_current = false) {
			current_nodes[current] = (link_to_current) ?
										NodePtr(new Node(current, current_nodes[previous])) : 
										NodePtr(new Node(current, previous_nodes[previous]));
		}

		void next_column() {
			previous_nodes = current_nodes;
		}

		std::vector<std::size_t> from(std::size_t k){
			std::vector<std::size_t> traceback;
			NodePtr node_ptr = current_nodes[k];
			traceback.push_back(node_ptr->value);
			while(node_ptr->previous){
				node_ptr = node_ptr->previous; 
				traceback.push_back(node_ptr->value);
			}
			std::reverse(traceback.begin(), traceback.end());
			return traceback;
		}

		std::string to_string() const {
			std::ostringstream oss;
			for(NodePtr p_node : current_nodes){
				oss << p_node->value << " -> ";
				if(p_node->previous){
					oss << p_node->previous->value;
				}
				else{
					oss << "END";
				}
				oss << " / ";
			}
			return oss.str();
		}

	};

	template<typename SymbolContainer>
	std::pair<std::vector<std::string>, double> viterbi(const SymbolContainer& symbols, std::size_t t_max = 0) {
		if(t_max == 0) t_max = symbols.size();
		auto viterbi_init = [&symbols, this]() {
			std::vector<double> delta_0(_A.size(), utils::kNegInf);
			Traceback psi(_A.size());
			/* First iterate over the silent states to compute the max probability of
			passing through silent states before emitting the first symbol. */
			double max_delta;
			double current_delta;
			std::size_t max_psi;
			for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
				max_delta = _pi_begin[i];
				max_psi = _A.size();
				for(std::size_t j = _silent_states_index; j < i; ++j){
					current_delta = _A[j][i] + delta_0[j];
					if(current_delta > max_delta){
						max_delta = current_delta;
						max_psi = j;
					}
				}
				if(max_delta != utils::kNegInf){
					delta_0[i] = max_delta;
				}
				if(max_psi < _A.size()){
					psi.add_link(max_psi, i, true);	
				}
				else{
					psi.add(i);
				}
			}
			psi.next_column();
			std::vector<double> delta_1(_A.size(), utils::kNegInf);
			/* Fill delta_1 for non-silent states. */
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				max_delta = _pi_begin[i];
				max_psi = _A.size();
				for(std::size_t j = _silent_states_index; j < _A.size(); ++j){
					current_delta = _A[j][i] + delta_0[j];
					if(current_delta > max_delta){
						max_delta = current_delta;
						max_psi = j;
					}
				}
				if(max_delta != utils::kNegInf){
					delta_1[i] = max_delta + (*_B[i])[symbols[0]];
				}
				if(max_psi < _A.size()){
					psi.add_link(max_psi, i);
				}
				else{
					psi.add(i);
				}
			}
			/* Then silent states, in toporder. */
			for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
				max_delta = utils::kNegInf;
				max_psi = _A.size();
				for(std::size_t j = 0; j < i; ++j){
					current_delta = _A[j][i] + delta_1[j];
					if(current_delta > max_delta){
						max_delta = current_delta;
						max_psi = j;
					}
				}
				if(max_delta != utils::kNegInf && max_psi < _A.size()){
					delta_1[i] = max_delta;
					psi.add_link(max_psi, i, true);
				}
			}
			psi.next_column();
			return std::make_pair(delta_1, psi);
		};
		auto viterbi_step = [&symbols, this](const std::vector<double>& delta_prev_t, Traceback& psi, std::size_t t) {
			std::vector<double> delta_t(_A.size(), utils::kNegInf);
			/* Normal states. */
			double max_delta;
			double current_delta;
			std::size_t max_psi;
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				max_delta = utils::kNegInf;
				max_psi = _A.size();
				for(std::size_t j = 0; j < _A.size(); ++j){
					current_delta = delta_prev_t[j] + _A[j][i];
					if(current_delta > max_delta){
						max_delta = current_delta;
						max_psi = j;
					}
				}
				if(max_delta != utils::kNegInf && max_psi != _A.size()){
					delta_t[i] = max_delta + (*_B[i])[symbols[t]];
					psi.add_link(max_psi, i);
				}
			}
			/* Silent states. */
			for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
				max_delta = utils::kNegInf;
				max_psi = _A.size();
				for(std::size_t j = 0; j < i; ++j){
					current_delta = delta_t[j] + _A[j][i];
					if(current_delta > max_delta){
						max_delta = current_delta;
						max_psi = j;
					}
				}
				if(max_delta != utils::kNegInf && max_psi != _A.size()){
					delta_t[i] = max_delta;
					psi.add_link(max_psi, i, true);
				}
			}
			psi.next_column();
			return delta_t;
		};
		if(symbols.size() == 0) throw std::logic_error("viterbi on empty symbol list");
		else{
			std::pair<std::vector<double>, Traceback> delta_tb = viterbi_init();
			std::vector<double>& delta = delta_tb.first;
			Traceback& traceback = delta_tb.second;
			for(std::size_t t = 1; t < std::min(symbols.size(), t_max); ++t) {
				delta = viterbi_step(delta, traceback, t);
			}
			return terminate_viterbi(delta, traceback);
		}
	}

	std::pair<std::vector<std::string>, double> terminate_viterbi(std::vector<double>& delta_T, Traceback& traceback){
		double max_delta_T = utils::kNegInf;
		std::size_t max_state_index = _A.size();
		if(_is_finite){
			/* Add end transitions.*/
			for(std::size_t i = 0; i < _A.size(); ++i){
				delta_T[i] = delta_T[i] + _pi_end[i];
				if(delta_T[i] > max_delta_T){
					max_delta_T = delta_T[i];
					max_state_index = i;
				}
			}
		}
		else{
			/* Only consider normal states. */
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				if(delta_T[i] > max_delta_T){
					max_delta_T = delta_T[i];
					max_state_index = i;
				}
			}
		}
		if(max_delta_T != utils::kNegInf && max_state_index < _A.size()){
			std::vector<std::size_t> path_indices = traceback.from(max_state_index);
			std::vector<std::string> path;
			path.reserve(path_indices.size());
			for(std::size_t path_index : path_indices){
				path.push_back(_states_names[path_index]);
			}
			return std::make_pair(path, max_delta_T);
		}
		else{
			/* Symbols sequence is impossible. */
			return std::make_pair(std::vector<std::string>(), utils::kNegInf);
		}
	}

	template<typename Symbol>
	std::pair<std::vector<std::string>, double> decode(std::vector<Symbol>& symbols) {
		return viterbi(symbols);
	}

	void train() {

	}

	void save(){

	}

	void load(){

	}

	virtual ~HiddenMarkovModel(){
		_clear_raw_data();
	}
};


#endif