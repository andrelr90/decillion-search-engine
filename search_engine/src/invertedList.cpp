#include "invertedList.hpp"

std::list<std::tuple<int, int, int>> InvertedList::invertedList;
std::unordered_map<std::string, std::tuple<int, long, long, float>> InvertedList::termsDict;
std::unordered_map<int, std::string> InvertedList::urls; 
std::unordered_map<int, float> InvertedList::pageranks;

void InvertedList::fitInvertedList(int id, std::string text){
	// Iterate though a document savinf each term at the inverted list. Also, saves it in the dictionary of terms and gets its relative frequency along documents.
	std::string temp;
	std::istringstream ss(text);
	int positionWord = 1;
	int idTerm;
	std::unordered_map<std::string, int> unique_terms;
	while(ss>>temp){
		temp = HtmlParser::toLowerCaseLatin(temp);

		if(termsDict.find(temp) == termsDict.end()){ // Saves the term dicts.
			termsDict.insert({temp, std::make_tuple(termsDict.size()+1, 0, 0, 0)});
		}
		idTerm = std::get<0>(termsDict.at(temp));
		
		if (unique_terms.find(temp) == unique_terms.end()){ // Updates relative term frequency.
			unique_terms.insert({temp, 0});
			std::get<3>(termsDict.at(temp)) += 1;
		}

		invertedList.push_back(std::make_tuple(idTerm, id, positionWord));

		positionWord++;
	}
}

void InvertedList::printInvertedList(){
	// Prints the inverted list currently on memory.
	for (auto i = invertedList.begin(); i != invertedList.end(); ++i){
		std::cout<<std::get<0>(*i)<<" ";
		std::cout<<std::get<1>(*i)<<" ";
		std::cout<<std::get<2>(*i)<<std::endl;
	}
}

bool InvertedList::compare_tuples (const std::tuple<int, int, int>& first, const std::tuple<int, int, int>& second){
	// Compares a tuple considering first the id f term, then, the id of document, and, finally, the position of the term.
	if (std::get<0>(first) < std::get<0>(second))
  		return true;
  	else if (std::get<0>(first) > std::get<0>(second))
  		return false;
  	if (std::get<1>(first) < std::get<1>(second))
  		return true;
  	else if (std::get<1>(first) > std::get<1>(second))
  		return false;
  	if (std::get<2>(first) < std::get<2>(second))
  		return true;
  	else if (std::get<2>(first) > std::get<2>(second))
  		return false;
  	return true;
}

void InvertedList::invertedListToFile(std::string file){
	// Saves the current inverted list on a file on extern memory. The space character is used as separator.
	std::ofstream invertedlist(file.c_str());
	for (auto i = invertedList.begin(); i != invertedList.end(); ++i){
		invertedlist<<std::get<0>(*i)<<" ";
		invertedlist<<std::get<1>(*i)<<" ";
		invertedlist<<std::get<2>(*i)<<std::endl;
	}
}

std::tuple<int, int, int> InvertedList::lineToTuple(std::string str){
	// Converts a line string to a tuple of three integers.
	unsigned int i = 0;
	int int_part1, int_part2, int_part3;

	std::string aux = "";
	while(str[i] != ' '){
		aux += str[i];
		i++;
	}
	i++;
	int_part1 = atoi(aux.c_str());

	aux = "";
	while(str[i] != ' '){
		aux += str[i];
		i++;
	}
	i++;
	int_part2 = atoi(aux.c_str());
	
	aux = "";
	while(i != str.length()){
		aux += str[i];
		i++;
	}
	i++;
	int_part3 = atoi(aux.c_str());

	return std::make_tuple(int_part1, int_part2, int_part3);
}

void InvertedList::externMemorySort(int n_files, std::string prefix){
	// Performs sorting in secondary memory using a merge schema.
	// Expects a power of 2 number of initial inverted lists. 
	int it = 1;
	std::string name_output_file;
	while(n_files!=1){
		for(int i=1; i<=n_files; i+=2){	// Iterates through files merging each two of them.
			name_output_file = prefix + "_it_" + std::to_string(it+1) + "_" + std::to_string((i+1)/2); // New file name merging the two files below.
			std::string name_file1 = prefix + "_it_" + std::to_string(it) + "_" + std::to_string(i);   // Name of the old iteration file 1.
			std::string name_file2 = prefix + "_it_" + std::to_string(it) + "_" + std::to_string(i+1); // Name of the old iteration file 2.

			std::ifstream file1(name_file1.c_str());
			std::ifstream file2(name_file2.c_str());
			std::ofstream output_file(name_output_file.c_str(), std::ios_base::app);
			std::string line_file1, line_file2;
			std::tuple<int, int, int> line_file1_tuple, line_file2_tuple;

			std::getline(file1, line_file1);
			std::getline(file2, line_file2);
			while(true){
				line_file1_tuple = lineToTuple(line_file1);
				line_file2_tuple = lineToTuple(line_file2);
				if (compare_tuples(line_file1_tuple, line_file2_tuple)){ // Compares line tuples in order to determine which line will appear first on the merged file.
					output_file << (line_file1 + "\n");
					if(!std::getline(file1, line_file1)){ // In case one file ends, saves the rest of the other file in the merged file.
						output_file << (line_file2 + "\n");
						while(std::getline(file2, line_file2))
							output_file << (line_file2 + "\n");
						break;
					}
				} else if (compare_tuples(line_file2_tuple, line_file1_tuple)){ // Compares line tuples in order to determine which line will appear first on the merged file.
					output_file << (line_file2 + "\n");
					if(!std::getline(file2, line_file2)){ // In case one file ends, saves the rest of the other file in the merged file.
						output_file << (line_file1 + "\n");
						while(std::getline(file1, line_file1))
							output_file << (line_file1 + "\n");
						break;
					}
				}
			}
			file1.close();
			file2.close();
			output_file.close();

			std::remove(name_file1.c_str()); // Deletes last iteration files
			std::remove(name_file2.c_str()); // Deletes last iteration files
		}
		n_files /= 2;
		it += 1;
	}
	std::rename(name_output_file.c_str(), prefix.c_str()); // Renames the last iteration file to prefix name.
}

void InvertedList::generateParsedSortedLists(std::string collection_file, std::string prefix, int documents_per_file){
	// Generates the sorted inverted lists.
	std::ifstream collection(collection_file.c_str());
	std::string json_line;

	int id_counter = 1;
	while(std::getline(collection, json_line)){ // For each file in the collection:
		rapidjson::Document document;
		document.Parse<0>(json_line.c_str());	// Parses the file using RapidJSON.

		GumboOutput* output = gumbo_parse( document["html_content"].GetString() ); // Get and parse the html content of the document.
		std::string text = HtmlParser::cleantext(output->root);
		gumbo_destroy_output(&kGumboDefaultOptions, output);

		fitInvertedList(id_counter, text);

		if (id_counter%documents_per_file == 0){ // Saves the inverted list on secondary memory when it is full and emptys the main memory one.
			invertedList.sort(compare_tuples);   // Sorts the inverted list on main memory.
			invertedListToFile(prefix + "_it_1_" + std::to_string(id_counter/documents_per_file));
			invertedList.clear();
		}

		if(id_counter%100 == 0){
			std::cout<<id_counter<<std::endl;
		}

		id_counter++;
	}
	invertedList.sort(compare_tuples); // Sorts the last inverted list.
	invertedListToFile(prefix + "_it_1_" + std::to_string(int(id_counter/documents_per_file)+1)); // Saves the last sorted inverted list.

	saveIdTermsDict(prefix+"_id_terms_dict"); // Saves the dictionary of terms found in the collection on secondary memory.
	collection.close();
}

void InvertedList::parseUrls(std::string collection_file, std::string output_file){
	// Gets the URL of each document in the collection and saves it on a separate file for further access.
	std::ifstream collection(collection_file.c_str());
	std::string json_line;

	int id_counter = 1;
	while(std::getline(collection, json_line)){
		rapidjson::Document document;
		document.Parse<0>(json_line.c_str());

		urls.insert({id_counter, document["url"].GetString()}); // Gets the URL.
		if(id_counter % 100 == 0)
			std::cout<<id_counter<<"\n";

		id_counter++;
	}
	collection.close();

	std::ofstream output(output_file.c_str());
	for(auto& it: urls){
		output << it.first << " " << it.second << "\n"; // Saves the URL and the id of the related document.
	}
	output.close();
}

void InvertedList::loadUrls(std::string urls_fileName){
	// Loads the URL file saved on memory.
	std::string id, url;
	std::ifstream urls_file(urls_fileName);
	if(!urls_file)
		throw std::runtime_error(std::string("URLs file not found. Run with -u option.")); // If URL not found, user should run -p mode.
	while(urls_file >> id){
		urls_file >> url;
		urls.insert({atoi(id.c_str()), url});
	}
	urls_file.close();
}

void InvertedList::saveIdTermsDict(std::string file_name){
	// Saves the dictionary of terms to ids in secondary memory.
	std::ofstream termsDict_file(file_name.c_str());
	for (auto& it: termsDict){
		termsDict_file << it.first << " ";
		termsDict_file << std::get<0>(it.second) << "\n";
	}
	termsDict_file.close();
}

void InvertedList::saveTermsDict(std::string invertedList_fileName, std::string output_fileName){
	// Terms dict is the final index file. Iterates through the inverted list indexing its, saving the position where each block of terms starts and ends.
	std::unordered_map<int, std::string> idToTerm;
	for (auto& it: termsDict)
		idToTerm.insert({std::get<0>(it.second), it.first});

	std::ifstream invertedList_file(invertedList_fileName.c_str());
	std::string line;
	std::tuple<int, int, int> tupleLine;
	int currentId, id;
	long starterLine = 0, lastLine = 0;
	std::getline(invertedList_file, line);

	bool endOfFile = false;
	int counter = 0;
	while(!endOfFile){
		tupleLine = lineToTuple(line);
		currentId = std::get<0>(tupleLine);
		id = std::get<0>(tupleLine);

		starterLine = lastLine;
		while(currentId == id){ // Iterates through the block, until the id of term is different and indicates the beginning of a new block.
			lastLine = invertedList_file.tellg();
			if (!std::getline(invertedList_file, line)){
				endOfFile = true;
				break;
			}
			tupleLine = lineToTuple(line);
			id = std::get<0>(tupleLine);
		}
		std::get<1>(termsDict.at(idToTerm.at(currentId))) = starterLine; // Gets first line of the block.
		std::get<2>(termsDict.at(idToTerm.at(currentId))) = lastLine;    // Gets last line of the block, which will also be the first of the next block.
		counter+=1;
	}
	invertedList_file.close();

	std::ofstream termsDict_file(output_fileName.c_str());
	for (auto& it: termsDict){ // Saves the final index on secondary memory.
		termsDict_file << it.first << " ";
		termsDict_file << std::get<0>(it.second) << " ";
		termsDict_file << std::get<1>(it.second) << " ";
		termsDict_file << std::get<2>(it.second) << " ";
		termsDict_file << std::get<3>(it.second) << "\n";
	}
	termsDict_file.close();
}

void InvertedList::loadTermsDict(std::string index_fileName){
	// Loads the index into main memory.
	std::ifstream index_file(index_fileName.c_str());
	if(!index_file)
		throw std::runtime_error(std::string("Index file not found. Run with -p option."));
	std::string term, aux;
	int id;
	long beginPosition, endPosition;
	float fileOccurrences;
	while(index_file >> term){
		index_file >> aux;
		id = atoi(aux.c_str());
		index_file >> aux;
		beginPosition = std::stol(aux);
		index_file >> aux;
		endPosition = std::stol(aux);
		index_file >> aux;
		fileOccurrences = std::stof(aux);

		termsDict.insert({term, std::make_tuple(id, beginPosition, endPosition, fileOccurrences)});
	}
	index_file.close();
}

void InvertedList::loadPageRankDict(std::string pageRank_fileName){
	std::unordered_map<std::string, int> urlToId;
	if (urls.size() != 0){
		for (auto& it: urls)
			urlToId.insert({it.second, it.first});
	} else {
		throw std::runtime_error(std::string("Load the URLS first."));
	}

	std::ifstream pageRankFile(pageRank_fileName.c_str());
	if (!pageRankFile)
		throw std::runtime_error(std::string("PageRank file not found."));
	std::string json_line;

	while(std::getline(pageRankFile, json_line)){
		rapidjson::Document document;
		document.Parse<0>(json_line.c_str());

		pageranks.insert({urlToId.at(document["url"].GetString()), document["pagerank"].GetFloat()}); // Gets the pagerank.
	}
	pageRankFile.close();
}

float InvertedList::getPageRankById(int idDoc){
	return pageranks.at(idDoc);
}

float InvertedList::getTfIdfWeight(std::string term, float n_occurences){
	float tf = 1 + log2(n_occurences);
	float idf =  log2( 1000068 / std::get<3>(termsDict.at(term)) );
	return (tf*idf);
}

std::pair<std::unordered_map<int, float>, std::vector<std::pair<int, std::vector<int>>>> InvertedList::getDocuments(std::string invertedList_fileName, std::string term){
	// Gets the block of a given term in the inverted list
	if(termsDict.find(term) == termsDict.end()){
		// return std::unordered_map<int, float>();	//Word not found
		return {std::unordered_map<int, float>(), std::vector<std::pair<int, std::vector<int>>>()};	//Word not found
	}

	long beginPosition_file = std::get<1>(termsDict.at(term));
	long endPosition_file = std::get<2>(termsDict.at(term));

	std::ifstream invertedList_file(invertedList_fileName.c_str(), std::ifstream::binary);
	if(!invertedList_file) // inverted list doesn't exist
		throw std::runtime_error(std::string("Inverted list file not found. Run with -p option."));
	std::string line;
	std::unordered_map<int, float> termHash;
	std::vector<std::pair<int, std::vector<int>>> documentPositions;

	invertedList_file.seekg(beginPosition_file);
	std::getline(invertedList_file, line);
	termHash.insert({std::get<1>(lineToTuple(line)), 1});
	while(invertedList_file.tellg() < endPosition_file){ // generates a list with the tuples of the block to be returned.
		std::getline(invertedList_file, line);
		std::tuple<int, int, int> lineTuple = lineToTuple(line);
		if ( termHash.find(std::get<1>(lineTuple)) == termHash.end() ){
			termHash.insert({std::get<1>(lineTuple), 1});
		} else{
			termHash.at(std::get<1>(lineTuple)) += 1;
		}

		//Retrieve term Positions
		if(documentPositions.size() == 0 || documentPositions[documentPositions.size()-1].first != std::get<1>(lineTuple)){
			std::vector<int> auxList;
			auxList.push_back(std::get<2>(lineTuple));
			documentPositions.push_back({std::get<1>(lineTuple), auxList});
		} else{
			documentPositions[documentPositions.size()-1].second.push_back(std::get<2>(lineTuple));
		}
	}
	invertedList_file.close();

	for (auto& it: termHash) {
		it.second = InvertedList::getTfIdfWeight(term, it.second);
	}

	return {termHash, documentPositions};
}

std::string InvertedList::getUrlById(int id){
	return urls.at(id);
}