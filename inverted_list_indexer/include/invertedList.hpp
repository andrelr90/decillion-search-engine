#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <locale.h>

#include <unordered_map>
#include <tuple>
#include <list>
#include <chrono>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "htmlParser.hpp"

class InvertedList{
private:
	static std::list<std::tuple<int, int, int>> invertedList;
	static std::unordered_map<std::string, std::tuple<int, long, long, float>> termsDict;
	static std::unordered_map<int, std::string> urls; 
public:
	static void fitInvertedList(int id, std::string text);
	static void printInvertedList();
	static bool compare_tuples (const std::tuple<int, int, int>& first, const std::tuple<int, int, int>& second);
	static void invertedListToFile(std::string file);
	static std::tuple<int, int, int> lineToTuple(std::string str);
	static void externMemorySort(int n_files, std::string prefix);
	static void generateParsedSortedLists(std::string collection_file, std::string prefix, int n_files, int total_docs);

	static void parseUrls(std::string collection_file, std::string output_file);
	static void loadUrls(std::string urls_fileName);
	
	static void saveIdTermsDict(std::string file_name);
	static void saveTermsDict(std::string invertedList_fileName, std::string output_fileName);
	static void loadTermsDict(std::string index_fileName);
	static std::list<std::tuple<int, int, int>> getDocuments(std::string invertedList_fileName, std::string term);
};