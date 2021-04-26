#include "queryProcessor.hpp"

float QueryProcessor::lambda;

float QueryProcessor::sigmoidActivation(float x){
	return tanh((lambda*x)/2);
}

float QueryProcessor::calculateWeight(float w1, float w2, float w3){
	// Calculates the final weight of the document.
	float p1 = 5, p2=3, p3=4;
	return p1*w1 + p2*sigmoidActivation(w2) + p3*w3;
}

bool QueryProcessor::compareSim(std::tuple<int, float, float, float> t1, std::tuple<int, float, float, float> t2) { 
    float w1 = calculateWeight(std::get<1>(t1), std::get<2>(t1), std::get<3>(t1));
    float w2 = calculateWeight(std::get<1>(t2), std::get<2>(t2), std::get<3>(t2));
    if(w1 == w2)
    	return std::get<0>(t1) < std::get<0>(t2);
    return (w1 > w2);
}


void QueryProcessor::getUniqueTerms(){
	// We will only load each of the term blocks once.
	std::istringstream queryTerms(query);
	int counter = 0;
	while(queryTerms >> term){
		if(unique_terms.find(term) == unique_terms.end()){
			unique_terms.insert({term, 1});
		} else{
			unique_terms.at(term) += 1;
		}

		// Creates a hash mapping terms to positions in the original query. Used to the positional signal.
		if(termToPositionQuery.find(term) == termToPositionQuery.end()){
			std::vector<int> aux;
			aux.push_back(counter);
			termToPositionQuery.insert({term, aux});
		} else{
			termToPositionQuery.at(term).push_back(counter);
		}
		counter+=1;
	}
	sizeQuery = counter;
}

float QueryProcessor::calculatePositionSignal(int idDoc){
	if(document_positions.find(idDoc) == document_positions.end()){
		// Something went wrong: Term not found in the document.
		return 1;
	}
	std::vector<std::vector<int>> docPositions = document_positions.at(idDoc);
	if(docPositions.size() == 1)	// if there is only one word, the position signal is maximum: 1.
		return 1;
	else if (docPositions.size() == 0)	// if there are no words, it is zero.
		return 0;
	
	std::list<std::pair<int, int>> orderedList;
	float positionalSignal = 0;

	// creates a list ordering two to two terms according to their position
	for (unsigned int i=0; i<docPositions.size()-1; i++){
		unsigned int it1 = 0, it2 = 0;
		orderedList.clear();
		while(true){
			if(it1 >= docPositions[i].size()){
				if(it2 < docPositions[i+1].size())
					orderedList.push_back({2, docPositions[i+1][it2]});
				break;
			} else if(it2 >= docPositions[i+1].size()){
				if(it1 < docPositions[i].size())
					orderedList.push_back({1, docPositions[i][it1]});
				break;
			}
			if(docPositions[i][it1] < docPositions[i+1][it2]){
				orderedList.push_back({1, docPositions[i][it1]});
				it1++;
			} else {
				orderedList.push_back({2, docPositions[i+1][it2]});
				it2++;
			}
		}

		int leastDif = 2147483646; //maximum int - 1
		// Gets the miniumum difference between positions of the two terms.
		for(std::list<std::pair<int, int>>::iterator it = orderedList.begin(); std::next(it, 1) != orderedList.end(); it++){
			if(it->first != std::next(it, 1)->first && it->second != std::next(it, 1)->second){
				int dif = abs(it->second - std::next(it, 1)->second);
				if(dif < leastDif)
					leastDif = dif;
			}
		}
		// returns the signal.
		positionalSignal += 1.0 / (1+log2(leastDif));
	}
	positionalSignal /= (docPositions.size()-1);
	return positionalSignal;
}

void QueryProcessor::getDocumentsTfIdf(){
	int term_counter = 0;
	for(auto& it: unique_terms){
		// Gets the documents with terms that match the query.
		std::pair<std::unordered_map<int, float>, std::vector<std::pair<int, std::vector<int>>>> invertedListBlock =  InvertedList::getDocuments(prefix, it.first);
		std::unordered_map<int, float> docs = invertedListBlock.first;
		std::vector<std::pair<int, std::vector<int>>> positionBlocks = invertedListBlock.second;
		if(docs.empty())
			return;
		// Counts the number of occurences of the term in each document.
		for(auto& query_result: docs){
			if(document_vectors.find(query_result.first) == document_vectors.end()){
				std::vector<float> document_v(unique_terms.size(), 0);
				document_v[term_counter] = query_result.second;
				document_vectors.insert({ query_result.first, document_v });
			} else{
				document_vectors.at(query_result.first)[term_counter] = query_result.second;
			}
		}
		// Gets the tf-idf
		float tfidf_q = InvertedList::getTfIdfWeight(it.first, it.second);
		query_vector.push_back(tfidf_q);
		query_norm += pow(tfidf_q, 2);
		term_counter+=1;

		// Positional: fills the hash mapping terms to their positions in each document.
		for(unsigned int j=0; j<positionBlocks.size(); j++){
			if(document_positions.find(positionBlocks[j].first) == document_positions.end()){
				std::vector<std::vector<int>> auxVect(sizeQuery);
				for(int k: termToPositionQuery.at(it.first)){
					auxVect[k] = (positionBlocks[j].second);
				}
				document_positions.insert({positionBlocks[j].first, auxVect});
			}
			else{
				for(int k: termToPositionQuery.at(it.first)){
					document_positions.at(positionBlocks[j].first)[k] = positionBlocks[j].second;
				}
			}
		}
	}
}


void QueryProcessor::calculateSimilarities(){
	// Calculates the similarities for documents with all the terms. 
	for(auto& it: document_vectors){
		bool full_match = true;
		float djq = 0;
		float document_norm = 0;
		for(unsigned int i=0; i<it.second.size(); i++){
			if(it.second[i] == 0){
				full_match = false;
				break;
			}
			document_norm += pow(it.second[i], 2);
			djq+=(it.second[i]  * query_vector[i]);
		}
		if(full_match){
			// calculates the similarity
			float sim = djq / (sqrt(document_norm) * sqrt(query_norm));
			// calculates the final weight
			results.push_back(std::make_tuple( it.first, sim, InvertedList::getPageRankById(it.first), calculatePositionSignal(it.first) ));
		}
	}
	results.sort(compareSim);
}


void QueryProcessor::printResults(int limit, double duration){
	// prints result in json format.
	int counter_results = 0;
	std::cout<<"\"n_results\":"<<results.size()<<", \"time\":"<<duration<<", ";

	if(results.empty()){
		std::cout << "\"results\": [] ";
		return;
	}

	std::cout << "\"results\": [ ";
	for(auto& it: results){
		int id = std::get<0>(it);
		float sim = std::get<1>(it);
		float pagerank = sigmoidActivation(std::get<2>(it));
		float positional = std::get<3>(it);
		float weight = calculateWeight(std::get<1>(it), std::get<2>(it), std::get<3>(it));
		std::string url = InvertedList::getUrlById(std::get<0>(it));

		std::cout<<"{\"id_doc\": " << id << ","
				<<"\"sim\": " << sim << ","
				<<"\"pagerank\": " << pagerank << ","
				<<"\"positional\": " << positional << ","
				<<"\"weight\": " << weight << ","
				<<"\"url\": \"" << url << "\"}";
		counter_results++;
		if(counter_results == limit || counter_results == results.size())
			break;
		else
			std::cout<<",";
	}
	std::cout<<"]";
}


QueryProcessor::QueryProcessor(std::string query, std::string prefix){
	lambda = pow(10, 3);

	this->prefix = prefix;
	auto start = std::chrono::system_clock::now();

	query = HtmlParser::removePunctuation(query);
	query = HtmlParser::toLowerCaseLatin(query);
	this->query = query;

	this->query_norm = 0;

	this->getUniqueTerms();
	this->getDocumentsTfIdf();
	this->calculateSimilarities();

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> duration = end-start;
	this->printResults(5, duration.count());
}