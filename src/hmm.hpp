#ifndef __HIDDENMARKOVMODEL_HPP
#define __HIDDENMARKOVMODEL_HPP

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <exception>
#include <stdexcept>
#include <vector>
#include <string>
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

std::string print_transitions(const Matrix<double>& matrix, const std::map<std::string, std::size_t>& indices, bool log_prob = false){
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

std::string print_distributions(const std::vector<Distribution*>& dists, const std::vector<std::string>& names, bool log_prob=false){
	std::ostringstream oss;
	for(std::size_t state_id = 0; state_id < dists.size(); ++state_id){
		Distribution* dist = dists[state_id];
		if(dist == nullptr) oss << "Silent";
		else{
			oss << names[state_id] << " : ";
			bool used_log_prob = dist->uses_log_probabilities();
			dist->log_probabilities(log_prob);
			oss << *dist;
			dist->log_probabilities(used_log_prob);
		}
		
		oss << std::endl;
	}
	return oss.str();
}

std::string print_names(const std::vector<std::size_t>& ids, const std::vector<std::string>& names){
	std::ostringstream oss;
	for(std::size_t id : ids){
		if(id < names.size()){
			oss << names[id] << " ";
		}
	}
	oss << std::endl;
	return oss.str();
}

std::string print_pi_begin(const std::vector<double>& pi, const std::vector<std::string>& names, bool log_prob=false){
	std::ostringstream oss;
	oss << "Pi begin : ";
	for(std::size_t state_id = 0; state_id < pi.size(); ++state_id){
		oss << names[state_id] << "(";
		if(log_prob) oss << pi[state_id];
		else oss << exp(pi[state_id]);
		oss << ") ";
	}
	oss << std::endl;
	return oss.str();
}

std::string print_pi_end(const std::vector<double>& pi, const std::vector<std::string>& names, bool log_prob=false){
	std::ostringstream oss;
	oss << "Pi end : ";
	for(std::size_t state_id = 0; state_id < pi.size(); ++state_id){
		oss << names[state_id] << "(";
		if(log_prob) oss << pi[state_id];
		else oss << exp(pi[state_id]);
		oss << ") ";
	}
	oss << std::endl;
	return oss.str();
}

std::string print_prob(const std::vector<double>& probs, bool log_prob=false){
	std::ostringstream oss;
	for(double d : probs){
		if(log_prob){oss << d << " ";}
		else oss << exp(d) << " ";
	}
	return oss.str();
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

std::ostream& operator<<(std::ostream& out, const std::vector<State>& vec){
	out << "States :" << std::endl;
	for(const State& s : vec){
		out << s << std::endl;
	}
	out << "----------------------------------------------" << std::endl;
	return out;
}

std::ostream& operator<<(std::ostream& out, const std::vector<State*>& vec){
	out << "States :" << std::endl;
	for(const State* s : vec){
		out << *s << std::endl;
	}
	out << "----------------------------------------------" << std::endl;
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
	std::vector<std::string> _alphabet;
	std::vector<std::size_t> _free_pi_begin;
	std::vector<std::size_t> _free_pi_end;
	std::vector<std::pair<std::size_t, std::size_t>> _free_transitions;
	/* Only discrete ! */
	std::vector<std::pair<std::size_t, std::string>> _free_emissions; //TODO : For now, free/fixed parameters PER state, do it for every parameter. 
	
	void _clear_raw_data() {
		for(Distribution* dist : _B){
			if(dist != nullptr) delete dist;
		}
		_states_indices.clear();
		_states_names.clear();
		_A.clear();
		_B.clear();
		_pi_begin.clear();
		_pi_end.clear();
		_is_finite = false;
		_silent_states_index = std::size_t();
		_M = std::size_t(); 
		_N = std::size_t();
		_alphabet.clear();
		_free_pi_begin.clear();
		_free_pi_end.clear();
		_free_transitions.clear();
		_free_emissions.clear();
		
	}

public:
	/* Default constructor. Inits an empty hmm with default values. */
	HiddenMarkovModel() : 
		HiddenMarkovModel(std::to_string((std::ptrdiff_t)this)) {}

	HiddenMarkovModel(const std::string& name) : 
		HiddenMarkovModel(name, State(hmm_config::kDefaultStartStateLabel), 
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

	HiddenMarkovModel(const HiddenMarkovModel& other) :
		_name(other._name), _begin(), _end(), _graph(other._graph), 
		_states_indices(other._states_indices), _states_names(other._states_names),
		_A(other._A), _B(other._B.size()), _pi_begin(other._pi_begin), _pi_end(other._pi_end),
		_is_finite(other._is_finite), _silent_states_index(other._silent_states_index),
		_M(other._M), _N(other._N), _alphabet(other._alphabet), _free_pi_begin(other._free_pi_begin), 
		_free_pi_end(other._free_pi_end), _free_transitions(other._free_transitions),
		_free_emissions(other._free_emissions) {
			_begin = _graph.get_vertex(*other._begin);
			_end = _graph.get_vertex(*other._end);
			for(std::size_t i = 0; i < other._B.size(); ++i){
				_B[i] = (other._B[i] == nullptr) ? nullptr : other._B[i]->clone();
			}
		}

	HiddenMarkovModel(HiddenMarkovModel&& other) : 
		_name(std::move(other._name)), _begin(std::move(other._begin)), _end(std::move(other._end)), 
		_graph(std::move(other._graph)), _states_indices(std::move(other._states_indices)), 
		_states_names(std::move(other._states_names)), _A(std::move(other._A)), _B(std::move(other._B)), 
		_pi_begin(std::move(other._pi_begin)), _pi_end(std::move(other._pi_end)), _is_finite(other._is_finite), 
		_silent_states_index(std::move(other._silent_states_index)), _M(other._M), _N(other._N), 
		_alphabet(std::move(other._alphabet)), _free_pi_begin(std::move(other._free_pi_begin)), 
		_free_pi_end(std::move(other._free_pi_end)), _free_transitions(std::move(other._free_transitions)),
		_free_emissions(std::move(other._free_emissions)) {}

	HiddenMarkovModel& operator=(const HiddenMarkovModel& other){
		if(this != &other){
			_clear_raw_data();
			_name = other._name;
			_graph = other._graph;
			_begin = _graph.get_vertex(*other._begin);
			_end = _graph.get_vertex(*other._end);
			_states_indices = other._states_indices;
			_states_names = other._states_names;
			_A = other._A;
			for(std::size_t i = 0; i < other._B.size(); ++i){
				_B[i] = (other._B[i] == nullptr) ? nullptr : other._B[i]->clone();
			}
			_pi_begin = other._pi_begin;
			_pi_end = other._pi_end;
			_is_finite = other._is_finite;
			_silent_states_index = other._silent_states_index;
			_M = other._M;
			_N = other._N;
			_alphabet = other._alphabet;
			_free_pi_begin = other._free_pi_begin;
			_free_pi_end = other._free_pi_end;
			_free_transitions = other._free_transitions;
			_free_emissions = other._free_emissions;
		}
		return *this;
	}

	HiddenMarkovModel& operator=(HiddenMarkovModel&& other){
		if(this != &other){
			_clear_raw_data();
			_name = std::move(other._name);
			_begin = std::move(other._begin);
			_end = std::move(other._end);
			_graph = std::move(other._graph);
			_states_indices = std::move(other._states_indices);
			_states_names = std::move(other._states_names);
			_A = std::move(other._A);
			_B = std::move(other._B);
			_pi_begin = std::move(other._pi_begin);
			_pi_end = std::move(other._pi_end);
			_is_finite = other._is_finite;
			_silent_states_index = other._silent_states_index;
			_M = other._M;
			_N = other._N;
			_alphabet = std::move(other._alphabet);
			_free_pi_begin = std::move(other._free_pi_begin);
			_free_pi_end = std::move(other._free_pi_end);
			_free_transitions = std::move(other._free_transitions);
			_free_emissions = std::move(other._free_emissions);
		}
		return *this;
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

	State& get_state(const State& state) {
		try{
			return *_graph.get_vertex(state);
		}
		catch(const VertexNotFoundException<State>& e){
			throw StateNotFoundException(e.trigger(), error_message::kHMMGetStateNotFound);
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
	void brew(bool normalize = true) {
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
		auto fill_normalize = [this, &pi_end, &states_indices] (const std::vector<Edge<State>*>& edges, std::vector<double>& prob_vec_to_fill, bool normalize) {
			std::vector<double> vec_to_normalize;
			vec_to_normalize.reserve(edges.size());
			double prob_sum = 0;
			double prob;
			for(Edge<State>* edge : edges){ 
				prob = (edge->weight() == nullptr) ? 0 : *(edge->weight());
				prob_sum += prob;
				vec_to_normalize.push_back(log(prob));
			}
			if(prob_sum != 1.0 && normalize){
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
		double prob_sum = fill_normalize(out_edges, pi_begin, normalize);
		if(prob_sum == 0.0) { throw std::logic_error("hmm has no begin transition"); }
		
		/* Check if end state has out edges. */
		State& end_state = end();
		if(_graph.get_out_edges(end_state).size() > 0) { throw std::logic_error("end state cannot have successors"); }

		/* Iterate through all the other states and add them to the matrix. */
		for(State* p_state : states){
			/* Fill normal transitions aka matrix A. */
			std::vector<Edge<State>*> out_edges = _graph.get_out_edges(*p_state);
			double prob_sum = fill_normalize(out_edges, A[states_indices[p_state->name()]], normalize);
			if(prob_sum == 0.0) { throw std::logic_error("hmm has no transition from " + p_state->to_string()); }
		}

		/* Determine whether the hmm is finite by summing the end state in transitions probabilities. */
		double prob_sum_to_end = 0;
		for(std::size_t i = 0; i < A.size(); ++i) { prob_sum_to_end += exp(pi_end[i]); }
		if(prob_sum_to_end > 0.0) { finite = true; }

		/* Fill emission matrix with the states PDFs. */
		std::vector<Distribution*> B(num_states);
		for(State* p_state : states){
			Distribution* distribution = nullptr;
			if(! p_state->is_silent()) {
				distribution = p_state->distribution().clone();
				distribution->log_probabilities(true);
				if(normalize){
					distribution->log_normalize();	
				}
			}
			B[states_indices[p_state->name()]] = distribution;
		}

		/* Get alphabet. Only discrete ! */
		std::vector<std::string> alphabet;
		for(const State* p_state : states){
			if(! p_state->is_silent()){
				std::vector<std::string> dist_symbols = static_cast<DiscreteDistribution*>(&p_state->distribution())->symbols();
				for(const std::string& symbol : dist_symbols){
					if(std::find(alphabet.begin(), alphabet.end(), symbol) == alphabet.end()){
						alphabet.push_back(symbol);
					}
				}
			}
		}

		/* Set free emissions. Only discrete ! */
		std::vector<std::pair<std::size_t, std::string>> free_emissions;
		for(const State* p_state : states){
			if((! p_state->is_silent()) && p_state->has_free_emission()){
				std::size_t state_id = states_indices[p_state->name()];
				for(const std::string& symbol : alphabet){
					free_emissions.push_back(std::make_pair(state_id, symbol));	
				}
			}
		}
		
		/* Set free transitions. */
		std::vector<std::pair<std::size_t, std::size_t>> free_transitions;
		std::vector<std::size_t> free_pi_begin;
		std::vector<std::size_t> free_pi_end;
		for(const State* p_state : states){
			if(p_state->has_free_transition()){
				std::size_t state_id = states_indices[p_state->name()];
				auto out_edges = _graph.get_out_edges(*p_state);
				for(auto& edge: out_edges){
					if(*(edge->to()) == end()){ free_pi_end.push_back(state_id); }
					else { free_transitions.push_back(std::make_pair(state_id, states_indices[edge->to()->name()])); }
				}
			}
		}
		if(begin().has_free_transition()){
			auto begin_out_edges = _graph.get_out_edges(begin());
			for(auto edge: begin_out_edges){
				free_pi_begin.push_back(states_indices[edge->to()->name()]);
			}	
		}
		
		/* Set fields for the hmm raw values. */
		_A = std::move(A);
		_B = std::move(B);
		_pi_begin = std::move(pi_begin);
		_pi_end = std::move(pi_end);
		_states_indices = std::move(states_indices);
		_states_names = std::move(states_names);
		_is_finite = finite;
		_silent_states_index = normal_states_index;
		_alphabet = std::move(alphabet); // DISCRETE ONLY !!
		_free_emissions = std::move(free_emissions);
		_free_transitions = std::move(free_transitions);
		_free_pi_begin = std::move(free_pi_begin);
		_free_pi_end = std::move(free_pi_end);
	}

	Matrix<double>& raw_transitions() { return _A; }
	std::vector<double>& raw_pi_begin() { return _pi_begin; }
	std::vector<double>& raw_pi_end() { return _pi_end; }
	std::vector<Distribution*>& raw_pdfs() { return _B; }
	std::map<std::string, std::size_t>& states_indices() { return _states_indices; }

	std::vector<double> forward(const std::vector<std::string>& sequence, std::size_t t_max = 0) {
		if(t_max == 0) t_max = sequence.size();
		auto forward_init = [&sequence, this]() {
			std::vector<double> alpha_0(_A.size(), utils::kNegInf);
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
			std::vector<double> alpha_1(_A.size(), utils::kNegInf);
			/* First iterate over non-silent states. */
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				alpha_1[i] = alpha_0[i] + (*_B[i])[sequence[0]];
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
		auto forward_step = [&sequence, this](const std::vector<double>& alpha_prev_t, std::size_t t) {
			std::vector<double> alpha_t(_A.size(), utils::kNegInf);
			/* Normal states. */
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				alpha_t[i] = utils::kNegInf;
				for(std::size_t j = 0; j < _A.size(); ++j){
					alpha_t[i] = utils::sum_log_prob(alpha_t[i], alpha_prev_t[j] + _A[j][i]);
				}
				alpha_t[i] = alpha_t[i] + (*_B[i])[sequence[t]];
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
		if(sequence.size() == 0) throw std::logic_error("forward on empty sequence");
		else{
			std::vector<double> alpha = forward_init();
			for(std::size_t t = 1; t < std::min(sequence.size(), t_max); ++t) {
				alpha = forward_step(alpha, t);
			}
 			return alpha;
		}
	}

	std::pair<std::vector<double>, double> forward_terminate(const std::vector<double>& alpha_T){
		double log_prob = utils::kNegInf;
		std::vector<double> alpha_end(_A.size(), utils::kNegInf);
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

	std::vector<double> backward(const std::vector<std::string>& sequence, std::size_t t_min = 0) {
		auto backward_init = [this]() {
			std::vector<double> beta_T(_A.size());
			if(_is_finite){
				for(std::size_t i = _A.size() - 1; i >= _silent_states_index; --i){
					beta_T[i] = _pi_end[i];
					for(std::size_t j = _A.size() - 1; j > i; --j){
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
		auto backward_step = [&sequence, this](const std::vector<double>& beta_next_t, std::size_t t) -> std::vector<double> {
			std::vector<double> beta_t(_A.size());
			/* First pass over silent states, considering next step non-silent states. */
			for(std::size_t i = _A.size() - 1; i >= _silent_states_index; --i){
				beta_t[i] = utils::kNegInf;
				for(std::size_t j = 0; j < _silent_states_index; j++){
					beta_t[i] = utils::sum_log_prob(beta_t[i], beta_next_t[j] + _A[i][j] + (*_B[j])[sequence[t + 1]]);
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
					beta_t[i] = utils::sum_log_prob(beta_t[i], beta_next_t[j] + _A[i][j] + (*_B[j])[sequence[t + 1]]);
				}
				for(std::size_t j = _silent_states_index; j < _A.size(); ++j){
					beta_t[i] = utils::sum_log_prob(beta_t[i], beta_t[j] + _A[i][j]);
				}
			}
			return beta_t;
		};
		if(t_min > 0) --t_min;
		if(sequence.size() == 0) throw std::runtime_error("backward on empty sequence");
		else{
			std::vector<double> beta = backward_init();
			for(std::size_t t = sequence.size() - 2; t >= t_min && t < sequence.size(); --t){
				beta = backward_step(beta, t);
			}
			return beta;
		}
	}

	std::pair<std::vector<double>, double> backward_terminate(const std::vector<double>& beta_0, const std::vector<std::string>& sequence){
		std::vector<double> beta_end(_A.size());
			/* First pass over silent states, considering next step non-silent states. */
			for(std::size_t i = _A.size() - 1; i >= _silent_states_index; --i){
				beta_end[i] = utils::kNegInf;
				for(std::size_t j = 0; j < _silent_states_index; j++){
					beta_end[i] = utils::sum_log_prob(beta_end[i], beta_0[j] + _A[i][j] + (*_B[j])[sequence[0]]);
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
				beta_end[i] = _pi_begin[i] + (*_B[i])[sequence[0]] + beta_0[i];
				log_prob = utils::sum_log_prob(log_prob, beta_end[i]);
			}
			for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
				beta_end[i] = _pi_begin[i] +  beta_end[i];
				log_prob = utils::sum_log_prob(log_prob, beta_end[i]);
			}
			return std::make_pair(beta_end, log_prob);
	}

	double log_likelihood(const std::vector<std::string>& sequence, bool do_fwd = true){
		if(do_fwd){
			return forward_terminate(forward(sequence)).second;
		}
		else{
			return backward_terminate(backward(sequence), sequence).second;
		}
	}

	double log_likelihood(const std::vector<std::vector<std::string>>& sequences, bool do_fwd = true){
		double likelihood = 0;
		for(const std::vector<std::string>& sequence : sequences){
			likelihood += log_likelihood(sequence, do_fwd);	
		}
		return likelihood;
	}

	double likelihood(const std::vector<std::string>& sequence, bool do_fwd = true){
		return exp(log_likelihood(sequence, do_fwd));
	}

	double likelihood(const std::vector<std::vector<std::string>>& sequences, bool do_fwd = true){
		return exp(log_likelihood(sequences, do_fwd));
	}

	

	void sample() {

	}

	class Traceback {
		struct Node; // forward declaration.
		typedef std::shared_ptr<Node> NodePtr;
		struct Node{
			NodePtr previous;
			std::size_t value;
			Node(std::size_t v) : previous(), value(v) {}
			Node(std::size_t v, NodePtr p) : previous(p), value(v) {}
			void set_previous(const NodePtr& p){ previous = p; }
		};

		std::size_t _nodes;
		std::vector<NodePtr> _previous_nodes;
		std::vector<NodePtr> _current_nodes;

		void _init_previous() {
			for(std::size_t i = 0; i < _nodes; ++i){
				_previous_nodes[i] = NodePtr(new Node(i));
			}
		}
		void _init_current() {
			for(std::size_t i = 0; i < _nodes; ++i){
					_current_nodes[i] = NodePtr(new Node(i));
			}
		}

	public:
		Traceback(std::size_t num_nodes) : 
			_nodes(num_nodes), _previous_nodes(_nodes), _current_nodes(_nodes) {
				_init_previous();
				_init_current();
			}

		void add_link(std::size_t previous, std::size_t current, bool link_to_current = false) {
			_current_nodes[current]->set_previous((link_to_current) ? _current_nodes[previous] : _previous_nodes[previous]);
		}

		void next_column() {
			_previous_nodes = _current_nodes;
			_init_current();
		}

		void reset() {
			_init_previous();
			_init_current();
		}

		std::vector<std::size_t> from(std::size_t k){
			std::vector<std::size_t> traceback;
			NodePtr node_ptr = _previous_nodes[k];
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
			for(NodePtr p_node : _current_nodes){
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

	std::vector<double> viterbi_init(Traceback& psi, const std::vector<std::string>& sequence) {
			std::vector<double> phi_0(_A.size(), utils::kNegInf);
			/* First iterate over the silent states to compute the max probability of
			passing through silent states before emitting the first symbol. */
			double max_phi;
			double current_phi;
			std::size_t max_psi;
			for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
				max_phi = _pi_begin[i];
				max_psi = _A.size();
				for(std::size_t j = _silent_states_index; j < i; ++j){
					current_phi = _A[j][i] + phi_0[j];
					if(current_phi > max_phi){
						max_phi = current_phi;
						max_psi = j;
					}
				}
				if(max_phi != utils::kNegInf){
					phi_0[i] = max_phi;
				}
				if(max_psi < _A.size()){
					psi.add_link(max_psi, i, true);	
				}
			}
			psi.next_column();
			std::vector<double> phi_1(_A.size(), utils::kNegInf);
			/* Fill phi_1 for non-silent states. */
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				max_phi = _pi_begin[i];
				max_psi = _A.size();
				for(std::size_t j = _silent_states_index; j < _A.size(); ++j){
					current_phi = _A[j][i] + phi_0[j];
					if(current_phi > max_phi){
						max_phi = current_phi;
						max_psi = j;
					}
				}
				if(max_phi != utils::kNegInf){
					phi_1[i] = max_phi + (*_B[i])[sequence[0]];
				}
				if(max_psi < _A.size()){
					psi.add_link(max_psi, i);
				}
			}
			/* Then silent states, in toporder. */
			for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
				max_phi = utils::kNegInf;
				max_psi = _A.size();
				for(std::size_t j = 0; j < i; ++j){
					current_phi = _A[j][i] + phi_1[j];
					if(current_phi > max_phi){
						max_phi = current_phi;
						max_psi = j;
					}
				}
				if(max_phi != utils::kNegInf && max_psi < _A.size()){
					phi_1[i] = max_phi;
					psi.add_link(max_psi, i, true);
				}
			}
			psi.next_column();
			return phi_1;
		};

	std::vector<double> viterbi_step(const std::vector<double>& phi_prev_t, Traceback& psi, std::size_t t, const std::vector<std::string>& sequence) {
		std::vector<double> phi_t(_A.size(), utils::kNegInf);
			double max_phi;
			double current_phi;
			std::size_t max_psi;
			/* Normal states. */
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				max_phi = utils::kNegInf;
				max_psi = _A.size();
				for(std::size_t j = 0; j < _A.size(); ++j){
					current_phi = phi_prev_t[j] + _A[j][i];
					if(current_phi > max_phi){
						max_phi = current_phi;
						max_psi = j;
					}
				}
				if(max_phi != utils::kNegInf && max_psi != _A.size()){
					phi_t[i] = max_phi + (*_B[i])[sequence[t]];
					psi.add_link(max_psi, i);
				}
			}
			/* Silent states. */
			for(std::size_t i = _silent_states_index; i < _A.size(); ++i){
				max_phi = utils::kNegInf;
				max_psi = _A.size();
				for(std::size_t j = 0; j < i; ++j){
					current_phi = phi_t[j] + _A[j][i];
					if(current_phi > max_phi){
						max_phi = current_phi;
						max_psi = j;
					}
				}
				if(max_phi != utils::kNegInf && max_psi != _A.size()){
					phi_t[i] = max_phi;
					psi.add_link(max_psi, i, true);
				}
			}
			psi.next_column();
			return phi_t;
	}

	std::size_t viterbi_terminate(std::vector<double>& phi_T){
		double max_phi_T = utils::kNegInf;
		std::size_t max_state_index = _A.size();
		if(_is_finite){
			/* Add end transitions.*/
			for(std::size_t i = 0; i < _A.size(); ++i){
				phi_T[i] = phi_T[i] + _pi_end[i];
				if(phi_T[i] > max_phi_T){
					max_phi_T = phi_T[i];
					max_state_index = i;
				}
			}
		}
		else{
			/* Only consider normal states. */
			for(std::size_t i = 0; i < _silent_states_index; ++i){
				if(phi_T[i] > max_phi_T){
					max_phi_T = phi_T[i];
					max_state_index = i;
				}
			}
		}
		return max_state_index;
	}

	std::pair<std::vector<std::string>, double> viterbi_decode(const std::vector<std::string>& sequence, std::size_t t_max = 0) {
		if(t_max == 0) t_max = sequence.size();
		if(sequence.size() == 0) throw std::logic_error("viterbi on empty sequence");
		else{
			Traceback psi(_A.size());
			std::vector<double> phi = viterbi_init(psi, sequence);
			for(std::size_t t = 1; t < std::min(sequence.size(), t_max); ++t) {
				phi = viterbi_step(phi, psi, t, sequence);
			}
			std::size_t max_state_index = viterbi_terminate(phi);
			double max_phi_T = phi[max_state_index];
			if(max_phi_T != utils::kNegInf && max_state_index < _A.size()){
				std::vector<std::size_t> path_indices = psi.from(max_state_index);
				std::vector<std::string> path;
				path.reserve(path_indices.size());
				for(std::size_t path_index : path_indices){
					path.push_back(_states_names[path_index]);
				}
				return std::make_pair(path, max_phi_T);
			}
			else{
				/* Sequence is impossible. */
				return std::make_pair(std::vector<std::string>(), utils::kNegInf);
			}
		}
	}

	std::pair<std::vector<std::string>, double> decode(const std::vector<std::string>& sequence) {
		return viterbi_decode(sequence);
	}

	static unsigned int delta(std::size_t i, std::size_t j){
		return (unsigned int)(i == j);
	}

	static unsigned int delta(std::string i, std::string j){
		return (unsigned int)(i == j);
	}

	class TransitionCount{
		std::vector<std::vector<unsigned int>> _transitions_counts;
		std::vector<std::vector<unsigned int>> _pi_begin_counts;
		std::vector<std::vector<unsigned int>> _pi_end_counts;
		const std::vector<std::pair<std::size_t, std::size_t>>* _free_transitions;
		const std::vector<std::size_t>* _free_pi_begin;
		const std::vector<std::size_t>* _free_pi_end;
	public:
		TransitionCount(const std::vector<std::pair<std::size_t, std::size_t>>& free_transitions,
			const std::vector<std::size_t>& free_pi_begin,
			const std::vector<std::size_t>& free_pi_end,
			std::size_t num_states) :
				_transitions_counts(num_states, std::vector<unsigned int>(free_transitions.size(), 0)),
				_pi_begin_counts(num_states, std::vector<unsigned int>(free_pi_begin.size(), 0)), 
				_pi_end_counts(num_states, std::vector<unsigned int>(free_pi_end.size(), 0)),
				_free_transitions(&free_transitions),
				_free_pi_begin(&free_pi_begin),
				_free_pi_end(&free_pi_end) {}

		/* /!\ Both transition counts need to represent the same hmm in a given training !! */
		TransitionCount& operator=(const TransitionCount& other) {
			if(this != &other){
				for(std::size_t m = 0; m < _transitions_counts.size(); ++m){
					for(std::size_t id = 0; id < _transitions_counts[m].size(); ++id){
						_transitions_counts[m][id] = other._transitions_counts[m][id];
					}
					for(std::size_t id = 0; id < _pi_begin_counts[m].size(); ++id){
						_pi_begin_counts[m][id] = other._pi_begin_counts[m][id];
					}
					for(std::size_t id = 0; id < _pi_end_counts[m].size(); ++id){
						_pi_end_counts[m][id] = other._pi_end_counts[m][id];
					}
				}	
			}
			return *this;
		}

		void add(const TransitionCount& other, std::size_t m, std::size_t l){
			for(std::size_t id = 0; id < _transitions_counts[m].size(); ++id){
				_transitions_counts[m][id] += other._transitions_counts[l][id];
			}
			for(std::size_t id = 0; id < _pi_begin_counts[m].size(); ++id){
				_pi_begin_counts[m][id] += other._pi_begin_counts[l][id];
			}
			for(std::size_t id = 0; id < _pi_end_counts[m].size(); ++id){
				_pi_end_counts[m][id] += other._pi_end_counts[l][id];
			}
		}

		/* Returns the transitions count of given transition for a path finishing at state m. */
		unsigned int count(std::size_t m, std::size_t free_transition_id) const {
			return _transitions_counts[m][free_transition_id];
		}

		unsigned int count_begin(std::size_t m, std::size_t free_transition_id) const {
			return _pi_begin_counts[m][free_transition_id];
		}

		unsigned int count_end(std::size_t m, std::size_t begin_transition_id) const {
			return _pi_end_counts[m][begin_transition_id];
		}

		/* Test wether a transition from i to j occurs in the given traceback.
		This method should only return 0 or 1. */
		unsigned int any_of_transitions(const std::vector<std::size_t>& traceback, std::size_t i, std::size_t j){
			unsigned int delta_sum = 0;
			for(std::size_t l = 0; l < traceback.size() - 1; ++l){
				delta_sum += delta(traceback[l], i) * delta(traceback[l+1], j);
			}
			return delta_sum;
		}

		/* Updates all the Tij counts for paths finishing at m by using the previous Tij 
		counts for paths finishing at l and increments it if i == l and j == m. */
		void update(const TransitionCount& previous_counts, const std::vector<std::size_t>& traceback){
			if(traceback.size() >= 2) {
				std::size_t l = traceback[0]; std::size_t m = traceback[traceback.size() - 1];
				std::size_t i,j;
				copy_begin(previous_counts, l, m);
				for(std::size_t free_transition_id = 0; free_transition_id < _transitions_counts[m].size(); ++free_transition_id){
					i = (*_free_transitions)[free_transition_id].first;
					j = (*_free_transitions)[free_transition_id].second;
					_transitions_counts[m][free_transition_id] = previous_counts.count(l, free_transition_id) + any_of_transitions(traceback, i, j);
				}	
			}
		}

		void copy_begin(const TransitionCount& previous_counts, std::size_t l, std::size_t m){
			for(std::size_t begin_transition_id = 0; begin_transition_id < _pi_begin_counts[m].size(); ++begin_transition_id){
				_pi_begin_counts[m][begin_transition_id] = previous_counts.count_begin(l, begin_transition_id); 
			}
		}

		void update_begin(const std::vector<std::size_t>& traceback){
			if(!traceback.empty()){
				std::size_t l = traceback[0]; std::size_t m = traceback[traceback.size() - 1];
				std::size_t i,j;
				for(std::size_t begin_transition_id = 0; begin_transition_id < _pi_begin_counts[m].size(); ++begin_transition_id){
					j = (*_free_pi_begin)[begin_transition_id];
					_pi_begin_counts[m][begin_transition_id] = delta(l, j); 
				}
				if(traceback.size() >= 2){
					for(std::size_t free_transition_id = 0; free_transition_id < _transitions_counts[m].size(); ++free_transition_id){
						i = (*_free_transitions)[free_transition_id].first;
						j = (*_free_transitions)[free_transition_id].second;
						_transitions_counts[m][free_transition_id] = any_of_transitions(traceback, i, j);
					}
				}
			}
		}

		/* Adds 1 to the end transition count of state_id for path arriving at state_id. */
		void update_end(std::size_t state_id){
			for(std::size_t end_transition_id = 0; end_transition_id < _pi_end_counts[state_id].size(); ++end_transition_id){
				if((*_free_pi_end)[end_transition_id] == state_id){
					_pi_end_counts[state_id][end_transition_id] += 1;
				}
			}
		}

		void reset(){
			for(std::size_t m = 0; m < _transitions_counts.size(); ++m){
				for(std::size_t id = 0; id < _transitions_counts[m].size(); ++id){
					_transitions_counts[m][id] = 0;
				}
				for(std::size_t id = 0; id < _pi_begin_counts[m].size(); ++id){
					_pi_begin_counts[m][id] = 0;
				}
				for(std::size_t id = 0; id < _pi_end_counts[m].size(); ++id){
					_pi_end_counts[m][id] = 0;
				}
			}
		}

		std::string to_string(std::size_t m, const std::vector<std::string>& names, const std::string& from = "") const {
			std::ostringstream oss;
			std::string name = from.empty() ? names[m] : from;
			oss << "From state " << name << std::endl;
			oss << "Begin counts : " << std::endl;
			for(std::size_t begin_transition_id = 0; begin_transition_id < _pi_begin_counts[m].size(); ++begin_transition_id){
				oss << "(" << names[(*_free_pi_begin)[begin_transition_id]] << " = " << _pi_begin_counts[m][begin_transition_id] << ") "; 
			}
			oss << std::endl << "Mid counts : " << std::endl;
			for(std::size_t transition_id = 0; transition_id < _transitions_counts[m].size(); ++transition_id){
				oss << "(" << names[(*_free_transitions)[transition_id].first] << "->" << 
				names[(*_free_transitions)[transition_id].second] << " = " << _transitions_counts[m][transition_id] << ") "; 
			}
			oss << std::endl << "End counts : " << std::endl;
			for(std::size_t end_transition_id = 0; end_transition_id < _pi_end_counts[m].size(); ++end_transition_id){
				oss << "(" << names[(*_free_pi_end)[end_transition_id]] << " = " << _pi_end_counts[m][end_transition_id] << ") "; 
			}
			oss << std::endl;
			return oss.str();
		}
	};

	class EmissionCount{
		std::vector<std::vector<unsigned int>> _emissions_counts;
		const std::vector<std::pair<std::size_t, std::string>>* _free_emissions;
		std::size_t _silent_states_index;
	public:
		EmissionCount(const std::vector<std::pair<std::size_t, std::string>>& free_emissions, std::size_t num_states, std::size_t silent_states_index) :
			_emissions_counts(num_states, std::vector<unsigned int>(free_emissions.size(), 0)),
			_free_emissions(&free_emissions), _silent_states_index(silent_states_index) {}

		EmissionCount& operator=(const EmissionCount& other) {
			if(this != &other){
				for(std::size_t m = 0; m < _emissions_counts.size(); ++m){
					for(std::size_t id = 0; id < _emissions_counts[m].size(); ++id){
						_emissions_counts[m][id] = other._emissions_counts[m][id];
					}
				}	
			}
			return *this;
		}

		unsigned int count(std::size_t m, std::size_t free_emission_id) const {
			return _emissions_counts[m][free_emission_id];
		}

		/* Adds the counts for arriving at state m of other EmissionCount to the counts of arriving 
		at state 0 of this EmissionCount. Both counts should have the same sizes. */
		void add(const EmissionCount& other, std::size_t m, std::size_t l){
			for(std::size_t id = 0; id < _emissions_counts[m].size(); ++id){
				_emissions_counts[m][id] += other._emissions_counts[l][id];
			}	
		}

		std::size_t last_non_silent_state(const std::vector<std::size_t>& traceback){
			for(std::size_t i = traceback.size(); i-- > 0;){
				if(traceback[i] < _silent_states_index){ 
					return traceback[i];
				}
			}
			return _emissions_counts.size(); //Not found sentinel value. Should never happen though.
		}

		void update(const EmissionCount& previous_counts, const std::vector<std::size_t>& traceback, std::string symbol){
			if(!traceback.empty()){
				std::size_t l = traceback[0]; std::size_t m = traceback[traceback.size() - 1];
				std::size_t transmitter = last_non_silent_state(traceback);
				if(transmitter == _emissions_counts.size()) { return; } // Again, this should not happen. 
				std::size_t i; std::string gamma;
				for(std::size_t free_emission_id = 0; free_emission_id < _emissions_counts[m].size(); ++free_emission_id){
					i = (*_free_emissions)[free_emission_id].first;
					gamma = (*_free_emissions)[free_emission_id].second;
					_emissions_counts[m][free_emission_id] = previous_counts.count(l, free_emission_id) + delta(transmitter, i) * delta(gamma, symbol);
				}	
			}
		}

		void reset(){
			for(std::size_t m = 0; m < _emissions_counts.size(); ++m){
				for(std::size_t id = 0; id < _emissions_counts[m].size(); ++id){
					_emissions_counts[m][id] = 0;
				}
			}
		}

		std::string to_string(std::size_t m, const std::vector<std::string>& names, const std::string& from = "") const {
			std::ostringstream oss;
			std::string name = from.empty() ? names[m] : from;
			oss << "From state " << name << std::endl;
			oss << "Emissions counts : " << std::endl;
			for(std::size_t emission_id = 0; emission_id < _emissions_counts[m].size(); ++emission_id){
				oss << "(" << names[(*_free_emissions)[emission_id].first] << "->" << (*_free_emissions)[emission_id].second << " = " << _emissions_counts[m][emission_id] << ") ";
			}
			oss << std::endl;
			return oss.str();
		}
	};

	double train_viterbi(const std::vector<std::vector<std::string>>& sequences, 
		double transition_pseudocount = hmm_config::kDefaultTransitionPseudocount,
		double convergence_threshold = hmm_config::kDefaultConvergenceThreshold,
		unsigned int min_iterations = hmm_config::kDefaultMinIterationsViterbi, 
		unsigned int max_iterations = hmm_config::kDefaultMaxIterationsViterbi){

		/* This holds all the counts for the batch of sequences. */
		TransitionCount total_transition_count(_free_transitions, _free_pi_begin, _free_pi_end, 1);
		EmissionCount total_emission_count(_free_emissions, 1, 0);
		/* This hold the counts for each sequence. */
		TransitionCount previous_transition_count(_free_transitions, _free_pi_begin, _free_pi_end, _A.size());
		TransitionCount next_transition_count(_free_transitions, _free_pi_begin, _free_pi_end, _A.size());
		EmissionCount previous_emission_count(_free_emissions, _A.size(), _silent_states_index);
		EmissionCount next_emission_count(_free_emissions, _A.size(), _silent_states_index);
		unsigned int iteration = 0;
		/* Use likelihood to determine convergence. */
		double delta = utils::kInf;
		double initial_likelihood = log_likelihood(sequences);
		double previous_likelihood = initial_likelihood;
		double current_likelihood;
		while((iteration <= min_iterations || delta > convergence_threshold) 
			&& iteration <= max_iterations) {
			/* Iterate over each sequence and compute the counts. */
			for(const std::vector<std::string>& sequence : sequences){
				/* If sequence is empty, go to next sequence. */
				if(sequence.size() == 0) { continue; }
				Traceback psi(_A.size());
				/* The initial step is a special case, since we use initial transition probabilities which
				are not stored in the raw A matrix. */
				std::vector<double> phi = viterbi_init(psi, sequence);
				/* First iterate only on normal states since emission count are only needed for such states. */
				for(std::size_t m = 0; m < _A.size(); ++m){
					std::vector<std::size_t> traceback_m = psi.from(m);
					next_transition_count.update_begin(traceback_m);
					next_emission_count.update(previous_emission_count, traceback_m, sequence[0]);
				}
				previous_transition_count = next_transition_count;
				previous_emission_count = next_emission_count;
				/* Resetting the traceback since we only need the traceback of current viterbi step. */
				psi.reset();
				/* Main loop for current sequence. */
				for(std::size_t k = 1; k < sequence.size(); ++k){
					phi = viterbi_step(phi, psi, k, sequence);
					for(std::size_t m = 0; m < _A.size(); ++m){
						std::vector<std::size_t> traceback_m = psi.from(m);
						next_transition_count.update(previous_transition_count, traceback_m);
						next_emission_count.update(previous_emission_count, traceback_m, sequence[k]);
					}
					psi.reset();
					previous_transition_count = next_transition_count;
					previous_emission_count = next_emission_count;
				}
				std::size_t max_state_index = viterbi_terminate(phi);
				/* Test wether the sequence is possible. */
				if(max_state_index < _A.size()){
					/* Add 1 to the end transition count of the max state index if model has end state. */
					if(_is_finite){
						next_transition_count.update_end(max_state_index);
					}
					/* Update the total counts. */
					total_transition_count.add(next_transition_count, 0, max_state_index);
					total_emission_count.add(next_emission_count, 0, max_state_index);
				}
				/* Reset counts. */
				next_transition_count.reset();
				previous_transition_count.reset();
				next_emission_count.reset();
				previous_emission_count.reset();
			}
			update_model_from_counts(total_transition_count, total_emission_count, transition_pseudocount);
			total_transition_count.reset();
			total_emission_count.reset();
			current_likelihood = log_likelihood(sequences);
			delta = current_likelihood - previous_likelihood;
			previous_likelihood = current_likelihood;
			++iteration;
		}
		/* Training is done. Update the real HMM from the raw trained values. */
		update_from_raw();
		/* Return total improvement. */
		return current_likelihood - initial_likelihood;
	}

	void update_model_from_counts(const TransitionCount& transitions_counts, 
		const EmissionCount& emissions_counts, double transition_pseudocount){
		update_model_transitions_from_counts(transitions_counts, transition_pseudocount);
		update_model_emissions_from_counts(emissions_counts);
	}

	void update_model_transitions_from_counts(const TransitionCount& transitions_counts, double transition_pseudocount){
		/* Update begin transitions. */
		/* First, sum all the begin transitions counts. */
		unsigned int begin_transitions_count = 0;
		for(std::size_t begin_transition_id = 0; begin_transition_id < _free_pi_begin.size(); ++begin_transition_id){
			begin_transitions_count += transitions_counts.count_begin(0, begin_transition_id) + transition_pseudocount;
		}
		/* Then, normalize the count of each begin transition by using the total count. */
		std::size_t state_id;
		for(std::size_t begin_transition_id = 0; begin_transition_id < _free_pi_begin.size(); ++begin_transition_id){
			state_id = _free_pi_begin[begin_transition_id];
			if(begin_transitions_count > 0){
				_pi_begin[state_id] = log(((double)transitions_counts.count_begin(0, begin_transition_id) + transition_pseudocount) / begin_transitions_count);
			}
		}

		/* Update other transitions (don't forget to take end transitions into account). */
		/* Similarly to begin transitions, sum, for each state, all its out transitions counts. */
		std::unordered_map<std::size_t, unsigned int> out_transitions_counts;
		for(std::size_t transition_id = 0; transition_id < _free_transitions.size(); ++transition_id){
			state_id = _free_transitions[transition_id].first;
			/* If state i not yet in map, init count for state i at 0. */
			if(out_transitions_counts.find(state_id) == out_transitions_counts.end()){
				out_transitions_counts[state_id] = 0;
			}
			out_transitions_counts[state_id] += transitions_counts.count(0, transition_id) + transition_pseudocount;
		}
		/* Also add end transitions counts to the the sum. */
		for(std::size_t end_transition_id = 0; end_transition_id < _free_pi_end.size(); ++end_transition_id){
			state_id = _free_pi_end[end_transition_id];
			if(out_transitions_counts.find(state_id) == out_transitions_counts.end()){
				out_transitions_counts[state_id] = 0;
			}
			out_transitions_counts[state_id] += transitions_counts.count_end(0, end_transition_id) + transition_pseudocount;
		}
		/* Normalize each transition by using the sum. */
		std::size_t from_state, to_state;
		for(std::size_t transition_id = 0; transition_id < _free_transitions.size(); ++transition_id){
			from_state = _free_transitions[transition_id].first; to_state = _free_transitions[transition_id].second;
			if(out_transitions_counts[from_state] > 0){
				_A[from_state][to_state] =  log(((double)transitions_counts.count(0, transition_id) + transition_pseudocount) / out_transitions_counts[from_state]);
			}
		}
		/* Don't forget to update the end transitions ! */
		for(std::size_t end_transition_id = 0; end_transition_id < _free_pi_end.size(); ++end_transition_id){
			state_id = _free_pi_end[end_transition_id];
			if(out_transitions_counts[state_id] > 0){
				_pi_end[state_id] = log(((double)transitions_counts.count_end(0, end_transition_id) + transition_pseudocount) / out_transitions_counts[state_id]);
			}
		}
	}

	void update_model_emissions_from_counts(const EmissionCount& emissions_counts){
		std::unordered_map<std::size_t, unsigned int> all_emissions_counts;
		std::size_t state_id;
		for(std::size_t emission_id = 0; emission_id < _free_emissions.size(); ++emission_id){
			state_id = _free_emissions[emission_id].first;
			if(all_emissions_counts.find(state_id) == all_emissions_counts.end()){
				all_emissions_counts[state_id] = 0;
			}
			all_emissions_counts[state_id] += emissions_counts.count(0, emission_id);
		}
		std::string symbol;
		for(std::size_t emission_id = 0; emission_id < _free_emissions.size(); ++emission_id){
			state_id = _free_emissions[emission_id].first;
			symbol = _free_emissions[emission_id].second;
			if(all_emissions_counts[state_id] > 0) {
				(*(_B[state_id]))[symbol] = log((double)emissions_counts.count(0, emission_id) / all_emissions_counts[state_id]);
			}
		}
	}

	void update_from_raw(){
		/* Update transitions. Since we use log probabilities in the raw data, don't forget to exp() the log prob. */
		std::string from_state_name, to_state_name;
		std::size_t from_state_id, to_state_id;
		double log_probability;
		/* Update begin transitions. */
		for(std::size_t begin_transition_id = 0; begin_transition_id < _free_pi_begin.size(); ++begin_transition_id){
			to_state_id = _free_pi_begin[begin_transition_id];
			log_probability = _pi_begin[to_state_id];
			to_state_name = _states_names[to_state_id];
			_graph.set_weight(begin(), State(to_state_name), exp(log_probability));
		}
		/* Update mid transitions. */
		for(std::size_t transition_id = 0; transition_id < _free_transitions.size(); ++transition_id){
			from_state_id = _free_transitions[transition_id].first;
			to_state_id = _free_transitions[transition_id].second;
			log_probability = _A[from_state_id][to_state_id];
			from_state_name = _states_names[from_state_id];
			to_state_name = _states_names[to_state_id];
			_graph.set_weight(State(from_state_name), State(to_state_name), exp(log_probability));
		}
		/* Update end transitions. */
		for(std::size_t end_transition_id = 0; end_transition_id < _free_pi_end.size(); ++end_transition_id){
			from_state_id = _free_pi_end[end_transition_id];
			log_probability = _pi_end[from_state_id];
			from_state_name = _states_names[from_state_id];
			_graph.set_weight(State(from_state_name), end(), exp(log_probability));
		}

		/* Update emissions. */
		//DISCRETE ONLY !!
		std::string symbol;
		std::size_t state_id;
		std::string state_name;
		for(std::size_t emission_id = 0; emission_id < _free_emissions.size(); ++emission_id){
			state_id = _free_emissions[emission_id].first;
			state_name = _states_names[state_id];
			symbol = _free_emissions[emission_id].second;
			_graph.get_vertex(State(state_name))->distribution()[symbol] = exp((*_B[state_id])[symbol]);
		}
	}



	void train_baum_welch(const std::vector<std::vector<std::string>>& sequences) {
		

	}

	void train_stochastic_em() {

	}

	void save(){

	}

	void load(){

	}

	virtual ~HiddenMarkovModel(){
		for(Distribution* dist : _B){
			if(dist != nullptr) { delete dist; }
		}
	}
};


#endif