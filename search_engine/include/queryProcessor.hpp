#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <locale.h>
#include <math.h>

#include <unordered_map>
#include <tuple>
#include <list>
#include <chrono>

#include "invertedList.hpp"

class QueryProcessor{
private:
	static float lambda;
	std::string query;
	std::string term;
	std::unordered_map <std::string, float> unique_terms;
	std::vector <float> query_vector;
	std::list<std::tuple<int, float, float, float>> results;
	std::unordered_map <int, std::vector<float>> document_vectors;
	float query_norm;
	std::string prefix;
	std::unordered_map <int, std::vector<std::vector<int>>> document_positions;
	std::unordered_map <std::string, std::vector<int>> termToPositionQuery;
	int sizeQuery;

	static float sigmoidActivation(float x);
	static float calculateWeight(float w1, float w2, float w3);
	static bool compareSim(std::tuple<int, float, float, float> t1, std::tuple<int, float, float, float> t2);
	float calculatePositionSignal(int idDoc);
	void getUniqueTerms();
	void getDocumentsTfIdf();
	void calculateSimilarities();
	void printResults(int limit, double duration);

public:
	QueryProcessor(std::string query, std::string prefix);
};