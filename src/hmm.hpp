#ifndef __HIDDENMARKOVMODEL_HPP
#define __HIDDENMARKOVMODEL_HPP

#include <iostream>
#include <sstream>
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

template<typename Elem>
using Matrix = std::vector<std::vector<Elem>>;

std::ostream& operator<<(std::ostream& out, const Matrix<double>& matrix){
	std::size_t longest_string = 0;
	for(std::size_t i = 0; i < matrix.size(); ++i){
		for(std::size_t j = 0; j < matrix[i].size(); ++j){
			std::string double_string = std::to_string(matrix[i][j]);
			if(double_string.length() > longest_string) longest_string = double_string.length();
		}
	}
	for(std::size_t i = 0; i < matrix.size(); ++i){
		for(std::size_t j = 0; j < matrix[i].size(); ++j){
			std::string double_string = std::to_string(matrix[i][j]);
			out << std::string(longest_string - double_string.length(), ' ');
			out << double_string;
			out << ' ';
		}
		out << std::endl;
	}
	return out;
}

std::ostream& operator<<(std::ostream& out, const std::vector<double>& vec){
	for(double d : vec){
		out << exp(d) << " ";
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
	std::size_t _begin_state_index;
	std::size_t _end_state_index;
	Matrix<double> _A;
	std::vector<Distribution*> _B;
	std::vector<double> _pi_begin;
	std::vector<double> _pi_end;
	bool _is_finite;

	void _clear_raw_data() {
		for(Distribution* dist : _B){
			if(dist != nullptr) delete dist;
		}
		_states_indices.clear();
		_states_names.clear();
		_pi_begin.clear();
		_pi_end.clear();
		_A.clear();
		_is_finite = false;
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
		/* Keep track of the matrix index of each state. */
		std::map<std::string, std::size_t> states_indices;
		std::vector<std::string> states_names(states.size() - 2);
		/* Raw transition matrix. Init its size. Decrement by 2 since
		begin and end states transitions are stored in separated arrays. */
		Matrix<double> A(states.size() - 2);
		/* Begin/end states transitions. */
		std::vector<double> pi_begin(states.size() - 2);
		std::vector<double> pi_end(states.size() - 2);
		/* Transitions to end state exist and > 0 */
		bool finite = false;
		/* Init one row per state and map state to matrix index. 
		Default values set to negative infinity since we use log probabilites. */
		std::size_t i = 0;
		for(State* p_state : states){
			State& state = *p_state;
			if(state != begin() && state != end()){
				A[i] = std::vector<double>(states.size() - 2, utils::kNegInf);
				states_indices[state.name()] = i;
				states_names[i] = state.name();
				++i;
			}
		}
		/* Fill transitions with log probabilities and check whether a normalization is needed. */
		auto fill_normalize = [&states_indices] (const std::vector<Edge<State>*>& edges, std::vector<double>& prob_vec_to_fill, bool update_from, bool normalize = true) {
			std::function<std::size_t(const Edge<State>*)> state_index;
			if(update_from) state_index = [&states_indices](const Edge<State>* edge){ return states_indices[edge->from()->name()]; };
			else state_index = [&states_indices](const Edge<State>* edge){ return states_indices[edge->to()->name()]; };
			double prob_sum = 0;
			double prob;
			for(Edge<State>* edge : edges){
				prob = (edge->weight() == nullptr) ? 0 : *(edge->weight());
				prob_sum += prob;
				prob_vec_to_fill[state_index(edge)] = log(prob);
			}
			if(prob_sum != 1.0 && normalize){
				utils::for_each_log_normalize(prob_vec_to_fill.begin(), prob_vec_to_fill.end(), log(prob_sum));
			}
			return prob_sum;
		};
		for(std::size_t i = 0; i < states.size(); ++i){
			State& state = *states[i];
			/* Fill begin transitions. Throw exception if begin state has predecessors. */
			if(state == begin()){
				if(_graph.get_in_edges(state).size() > 0) throw std::logic_error("begin state cannot have predecessors");
				std::vector<Edge<State>*> out_edges = _graph.get_out_edges(state);
				double prob_sum = fill_normalize(out_edges, pi_begin, false);
				if(prob_sum == 0.0) throw std::logic_error("hmm has no begin transition");
			}
			
			/* Fill end transitions. Throw exception if end state has successors. */
			else if(state == end()){
				if(_graph.get_out_edges(state).size() > 0) throw std::logic_error("end state cannot have successors");
				std::vector<Edge<State>*> in_edges = _graph.get_in_edges(state);
				/* Do not normalize. */
				double prob_sum = fill_normalize(in_edges, pi_end, true, false);
				/* Determine whether the hmm is finite by summing the end state in transitions probabilities. */
				if(prob_sum > 0.0) finite = true;
			}
			/* Fill normal transitions aka matrix A. */
			else{
				std::vector<Edge<State>*> out_edges = _graph.get_out_edges(state);
				double prob_sum = fill_normalize(out_edges, A[states_indices[state.name()]], false);
				if(prob_sum == 0.0) throw std::logic_error("hmm has no transition from " + state.to_string());;
			}
		}
		/* Fill emission matrix with the states PDFs. */
		std::vector<Distribution*> B(states.size() - 2);
		for(const State* p_state : states){
			const State& state = *p_state;
			if(state != begin() && state != end()){
				Distribution* distribution = nullptr;
				if(! state.is_silent()) {
					distribution = state.distribution().clone();
					distribution->log_normalize();
				}
				B[states_indices[state.name()]] = distribution;
			}
		}

		/* Set fields for the hmm raw values. */
		_A = std::move(A);
		_B = std::move(B);
		_states_indices = std::move(states_indices);
		_states_names = std::move(states_names);
		_pi_begin = std::move(pi_begin);
		_pi_end = std::move(pi_end);
		_is_finite = finite;
	}

	Matrix<double>& raw_transitions() { return _A; }
	std::vector<Distribution*>& raw_pdfs() { return _B; }
	std::vector<double>& raw_pi_begin() { return _pi_begin; }
	std::vector<double>& raw_pi_end() { return _pi_end; }
	std::map<std::string, std::size_t>& states_indices() { return _states_indices; }

	template<typename SymbolContainer>
	std::vector<double> forward(const SymbolContainer& symbols, std::size_t t_max = 0) {
		if(t_max == 0) t_max = symbols.size();
		auto init_forward = [&symbols, this]() -> std::vector<double> {
			std::vector<double> init_fwd(_pi_begin.size());
			for(std::size_t i = 0; i < _pi_begin.size(); ++i){
				// Check if state is silent.
				init_fwd[i] = (_B[i] != nullptr) ? _pi_begin[i] + (*_B[i])[symbols[0]] : _pi_begin[i];
			}
			return init_fwd;
		};
		auto iter_forward = [&symbols, this](const std::vector<double>& previous_fwd, std::size_t t) -> std::vector<double> {
			std::vector<double> current_fwd(_A.size());
			for(std::size_t j = 0; j < _A.size(); ++j){
				double prob_sum = utils::kNegInf;
				for(std::size_t i = 0; i < _A.size(); ++i){
					prob_sum = utils::sum_log_prob(prob_sum, previous_fwd[i] + _A[i][j]);	
				}
				current_fwd[j] = (_B[j] != nullptr) ? prob_sum + (*_B[j])[symbols[t]] : prob_sum;
			}
			return current_fwd;
		};
		
		if(symbols.size() == 0) throw std::runtime_error("forward on empty symbol list");
		else{
			std::vector<double> fwd = init_forward();
			for(std::size_t t = 1; t < std::min(symbols.size(), t_max); ++t){
				fwd = iter_forward(fwd, t);
			}
			return fwd;
		}
	}

	template<typename SymbolContainer>
	std::vector<double> backward(const SymbolContainer& symbols, std::size_t t_min = 0) {
		if(t_min == 0) t_min = 1;
		auto init_backward = [this]() -> std::vector<double> {
			std::vector<double> init_bwd(_pi_end.size());
			if(_is_finite){
				for(std::size_t i = 0; i < _pi_end.size(); ++i){
					init_bwd[i] = _pi_end[i];
				}
			}
			else{
				for(std::size_t i = 0; i < _pi_end.size(); ++i){
					init_bwd[i] = 0.0; // log(1) = 0.0 ! 
				}
			}
			return init_bwd;
		};
		auto iter_backward = [&symbols, this](const std::vector<double>& next_bwd, std::size_t t) -> std::vector<double> {
			std::vector<double> current_bwd(_A.size());
			for(std::size_t i = 0; i < _A.size(); ++i){
				double prob_sum = utils::kNegInf;
				double bwd_mul;
				for(std::size_t j = 0; j < _A.size(); ++j){
					bwd_mul = (_B[j] != nullptr) ? _A[i][j] + (*_B[j])[symbols[t]] + next_bwd[j] : _A[i][j] + next_bwd[j];
					prob_sum = utils::sum_log_prob(prob_sum, bwd_mul);	
				}
				current_bwd[i] = prob_sum;
			}
			return current_bwd;
		};
		if(symbols.size() == 0) throw std::runtime_error("backward on empty symbol list");
		else{
			std::vector<double> bwd = init_backward();
			for(std::size_t t = symbols.size() - 1; t >= t_min && t < symbols.size(); --t){
				bwd = iter_backward(bwd, t);
			}
			return bwd;
		}
	}

	template<typename Symbol>
	double log_likelihood(const std::vector<Symbol>& symbols, bool do_fwd = true){
		if(do_fwd){
			std::vector<double> fwd = forward(symbols);
			if(_is_finite){
				double prob_sum = utils::kNegInf;
				for(std::size_t i = 0; i < fwd.size(); ++i){
					prob_sum = utils::sum_log_prob(prob_sum, fwd[i] + _pi_end[i]);
				}
				return prob_sum;
			}
			else{
				double prob_sum = utils::kNegInf;
				for(std::size_t i = 0; i < fwd.size(); ++i){
					prob_sum = utils::sum_log_prob(prob_sum, fwd[i]);
				}
				return prob_sum;
			}
		}
		else{
			std::vector<double> bwd = backward(symbols);
			double prob_sum = utils::kNegInf;
			double bwd_mul;
			for(std::size_t i = 0; i < bwd.size(); ++i){
				bwd_mul = (_B[i] != nullptr) ? _pi_begin[i] + (*_B[i])[symbols[0]] + bwd[i] : _pi_begin[i] + bwd[i];
				prob_sum = utils::sum_log_prob(prob_sum, bwd_mul);	
			}
			return prob_sum;
		}
	}

	template<typename Symbol>
	double likelihood(const std::vector<Symbol>& symbols, bool do_fwd = true){
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

		void add_link(std::size_t previous, std::size_t current) {
			current_nodes[current] = NodePtr(new Node(current, previous_nodes[previous]));
		}

		void next_column() {
			previous_nodes = current_nodes;
		}

		std::vector<std::size_t> trace_back(std::size_t previous, std::size_t max_k){
			std::size_t k = max_k;
			std::vector<std::size_t> tb(k);
			NodePtr node_ptr = previous_nodes[previous];
			while(k > 0){
				tb[k - 1] = node_ptr->value;
				if(!node_ptr->previous) break;
				node_ptr = node_ptr->previous; 
				--k;
			}
			if(k > 0) throw std::logic_error("Could not trace back to " + std::to_string(max_k) + ". Stopped at " + std::to_string(k));
			if(node_ptr->previous) throw std::logic_error("Traceback not completed. Previous value: " + std::to_string(node_ptr->previous->value));
			return tb;
		}

	};

	template<typename Symbol>
	std::pair<std::vector<std::string>, double> viterbi(const std::vector<Symbol>& symbols) {
		auto init_viterbi = [&symbols, this]() -> std::pair<std::vector<double>, Traceback>{
			std::vector<double> init_delta(_pi_begin.size());
			for(std::size_t i = 0; i < _pi_begin.size(); ++i){
				init_delta[i] = _pi_begin[i] + (*_B[i])[symbols[0]];
			}
			Traceback init_psi(symbols.size());
			return std::make_pair(init_delta, init_psi);
		};
		auto iter_viterbi = [&symbols, this](const std::vector<double>& previous_delta, Traceback& psi, std::size_t t) -> std::vector<double> {
			std::vector<double> current_delta(_A.size());
			for(std::size_t j = 0; j < _A.size(); ++j){
				double max_delta = utils::kNegInf;
				double delta_i;
				std::size_t max_i;
				for(std::size_t i = 0; i < _A.size(); ++i){
					delta_i = previous_delta[i] + _A[i][j];
					if(delta_i > max_delta){
						max_delta = delta_i;
						max_i = i;
					}
				}
				std::cout << j << "->" << max_i << " "; 
				current_delta[j] = max_delta + (*_B[j])[symbols[t]];
				psi.add_link(max_i, j);
			}
			std::cout << std::endl;
			psi.next_column();
			return current_delta;
		};

		auto terminate_viterbi = [&symbols, this](const std::vector<double>& delta, Traceback& psi) -> std::pair<std::vector<std::string>, double> {
			std::size_t max_delta = (std::size_t) (std::max_element(delta.begin(), delta.end()) - delta.begin());
			std::vector<std::size_t> path_indices = psi.trace_back(max_delta, symbols.size());
			std::vector<std::string> path(path_indices.size());
			for(std::size_t i = 0; i < path_indices.size(); ++i) {
				path[i] = _states_names[path_indices[i]];
			}
			return std::make_pair(path, delta[max_delta]);
		};
		std::pair<std::vector<double>, Traceback> init_viter = init_viterbi();
		std::vector<double> delta = init_viter.first;
		Traceback psi = init_viter.second;
		std::cout << delta << std::endl;
		for(std::size_t t = 0; t < symbols.size(); ++t){
			delta = iter_viterbi(delta, psi, t);
			std::cout << delta << std::endl;
		}
		return terminate_viterbi(delta, psi);
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