#ifndef __DISTRIBUTION_HPP
#define __DISTRIBUTION_HPP

#include <string>
#include <algorithm>
#include <map>
#include <numeric>
#include <functional>
#include <fstream>
#include "constants.hpp"
#include "utils.hpp"


/* <-------- Exceptions --------> */

class DistributionException : public std::logic_error {
protected:
	DistributionException(const std::string& message) :
		std::logic_error(message) {}
};

class DistributionSymbolNotFoundException : public DistributionException {
public:
	template<typename T>
	DistributionSymbolNotFoundException(const T& t) : 
		DistributionException(error_message::format("DistributionSymbolNotFoundException: " + error_message::kDistributionSymbolNotFound, t)) {}
};

/* <----------------------------> */

class Distribution {
private:
	std::string _name;
	bool _log;

protected:
	Distribution() : 
		Distribution(distribution_config::kDistributionName) {}
	Distribution(const std::string& name) : _name(name), _log(distribution_config::kDefaultLogUse) {}
	Distribution(const Distribution& other) : _name(other._name), _log(other._log) {}

public:
	virtual std::string name() const { return _name; }
	virtual bool is_discrete() const { return false; }
	virtual bool is_continuous() const { return false; }
	virtual bool uses_log_probabilities() const { return _log; }
	virtual void log_probabilities(bool use_log) { _log = use_log; }

	//virtual void round(int precision = global_config::kDoublePrecision);
	/* Pure virtual methods */
	/* Get probabilities with operator[] */
	virtual bool empty() const = 0;
	virtual std::string to_string() const {
		return _name;
	}
	virtual void log_normalize() = 0;
	virtual double& operator[] (const std::string&) = 0;
	virtual double& operator[] (double) = 0;
	virtual bool operator==(const Distribution& other) const = 0;
	virtual bool operator!=(const Distribution& other) const = 0;
	virtual void save(std::ofstream&) = 0;
	virtual void load(std::ifstream&) = 0;
	/* For polymorphic use */
	virtual Distribution* clone() const = 0;
	virtual ~Distribution() {}
};

std::ostream& operator<<(std::ostream& out, const Distribution& dist){
	out << dist.to_string();
	return out;
}


/* This class is basically a wrapper around std::map... */
class DiscreteDistribution : public Distribution{
private:
	std::map<std::string, double> _distribution;
protected:
	DiscreteDistribution(const std::string& name) :
		Distribution(name), _distribution() {}
public:
	DiscreteDistribution() : 
		DiscreteDistribution(distribution_config::kDiscreteDistributionName) {}

	DiscreteDistribution(std::initializer_list<std::pair<const std::string, double>> distribution) : 
		Distribution(distribution_config::kDiscreteDistributionName), _distribution(distribution) {}

	DiscreteDistribution(const DiscreteDistribution& other) :
		Distribution(other), _distribution(other._distribution) {}

	/* Covariant return type */
	virtual DiscreteDistribution* clone() const {
		return new DiscreteDistribution(*this);
	}

	void round(int precision = global_config::kDoublePrecision){
		for(auto& entry : _distribution){
			entry.second = utils::round_double(entry.second, precision);
		}
	}

	double prob_sum() const {
		double init_sum;
		std::function<double(double, const std::pair<std::string, double>&)> prob_adder;
		if(this->uses_log_probabilities()){
			init_sum = utils::kNegInf;
			prob_adder = [](const double previous, const std::pair<const std::string, double>& entry) { 
							return utils::sum_log_prob(previous, entry.second);
						};
		}
		else{
			init_sum = double(0);
			prob_adder = [](const double previous, const std::pair<const std::string, double>& entry) { 
							return previous + entry.second;
						};
		}
		return std::accumulate(_distribution.begin(), _distribution.end(), init_sum, prob_adder);
	}

	std::string to_string() const {
		std::string str = Distribution::to_string() + ": ";
		std::for_each(_distribution.begin(), _distribution.end(), 
		[&str](const std::pair<const std::string,double>& entry){
			str += entry.first + "(" + std::to_string(entry.second) + ") ";
		});
		str += "-> sum " + std::to_string(prob_sum());
		return str;
	}

	void save(std::ofstream& out) {
		bool used_log = uses_log_probabilities();
		log_probabilities(false);
		out << _distribution.size() << std::endl;
		for(auto& entry : _distribution){
			out << entry.first << '>' << entry.second << std::endl;
		}

		log_probabilities(used_log);
	}

	void load(std::ifstream& in) {
		log_probabilities(false);
		std::string line;
		std::string symbol;
		std::string prob_str;
		std::size_t num_symbols;
		std::getline(in, line);
		num_symbols = (std::size_t) stoi(line);
		for(std::size_t i = 0; i < num_symbols; ++i){
			std::getline(in, line);
			std::tie(symbol, prob_str) = utils::split_first(line, global_config::kProbabilitySeparator);
			_distribution[symbol] = std::stod(prob_str);
		}
	}

	virtual void log_probabilities(bool use_log) {
		/* Only make changes if distriubtion not yet using what is asked. */
		if(use_log != this->uses_log_probabilities()){
			Distribution::log_probabilities(use_log);
			if(use_log){
				std::for_each(_distribution.begin(), _distribution.end(), 
					[](std::pair<const std::string, double>& entry) {
						entry.second = log(entry.second);
					});
			}
			else{
				std::for_each(_distribution.begin(), _distribution.end(), 
					[](std::pair<const std::string, double>& entry) {
						entry.second = exp(entry.second);
					});
			}
		}
	}

	virtual void log_normalize() {
		if(! this->uses_log_probabilities()) {
			log_probabilities(true);
		}
		double probabilities_sum = prob_sum();
		if(exp(probabilities_sum) != 1.0){
			std::for_each(_distribution.begin(), _distribution.end(),
				[&probabilities_sum](std::pair<const std::string, double>& entry) {
					entry.second = utils::log_normalize(entry.second, probabilities_sum);
				});
		}
	}

	bool empty() const {
		return _distribution.size() == 0 || prob_sum() == 0;
	}

	bool is_discrete() const { return true; }

	bool contains(const std::string& symbol) const {
		return _distribution.find(symbol) != _distribution.end();
	}
	bool contains(const std::string& symbol) {
		return _distribution.find(symbol) != _distribution.end();
	}

	std::vector<std::string> symbols() const {
		std::vector<std::string> keys;
		keys.reserve(_distribution.size());
		for(auto entry : _distribution){
			keys.push_back(entry.first);
		}
		return keys;
	}

	virtual double& operator[] (const std::string& symbol) {
		if(!contains(symbol)){
			_distribution[symbol] = (this->uses_log_probabilities()) ? utils::kNegInf : 0.0;
		}
		return _distribution[symbol];
	}

	virtual double& operator[] (double symbol) {
		return operator[](std::to_string(symbol));
	}

	virtual bool operator==(const DiscreteDistribution& other) const {
		return (uses_log_probabilities() == other.uses_log_probabilities()) && (other._distribution == _distribution);
	}

	virtual bool operator==(const Distribution& other) const {
		if(other.is_discrete()){
			return operator==((DiscreteDistribution&) other);
		}
		return false;
	}

	virtual bool operator!=(const Distribution& other) const{
		return ! operator==(other);
	}

	virtual ~DiscreteDistribution() {}
};


class ContinuousDistribution : public Distribution {
protected:
	ContinuousDistribution(const std::string& name) : 
		Distribution(name) {}

public:
	ContinuousDistribution() : 
		ContinuousDistribution(distribution_config::kContinuousDistributionName) {}

	bool is_continuous() const { return true; }
	virtual bool is_normal() const { return false; }
	virtual bool is_uniform() const { return false; }
	virtual ~ContinuousDistribution() {}
};

class NormalDistribution : public ContinuousDistribution {
public:
	NormalDistribution() : ContinuousDistribution(distribution_config::kNormalDistributionName) {}
	bool is_normal() const { return true; }
	virtual ~NormalDistribution() {}
};

class UniformDistribution : public ContinuousDistribution {
public:
	UniformDistribution() : ContinuousDistribution(distribution_config::kUniformDistributionName) {}
	bool is_uniform() const { return true; }
	virtual ~UniformDistribution() {}
};

#endif