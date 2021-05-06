#include "invertedList.hpp"

int main(int argc, char** argv) {
	argc = argc; argv[0] = argv[0];

	setlocale(LC_ALL, "Portuguese");
	std::string prefix = "inverted_lists/inverted_list";

	if(argc == 1){
		// load mode: loads the index and list from memory.
		std::cout<<"Loading index."<<std::endl;
		try{
			InvertedList::loadTermsDict("inverted_lists/index");
			std::cout<<"Index loaded."<<std::endl<<std::endl;
			std::cout<<"Loading URLs."<<std::endl;
			InvertedList::loadUrls("inverted_lists/urls");
			std::cout<<"URLs loaded."<<std::endl<<std::endl;
		} catch(std::runtime_error e){
			std::cout<<e.what()<<std::endl;
			return -1;
		}

		// Examples
		std::string teste;
		while(true){
			std::cout<<"Search for a term: ";
			std::cin>>teste;
			try{
				InvertedList::getDocuments(prefix, teste);
			} catch (std::runtime_error e){
				std::cout<<e.what()<<std::endl;
				return -1;
			}
			std::cout<<"\n---------------------------------------"<<"\n\n\n";
		}
	} else if(std::string(argv[1]) == "-p" && argc == 4){
		// parsing mode: generates the unordered list, index, dict from the collection
		int n_files = pow(2, std::atoi(argv[3]));
		InvertedList::generateParsedSortedLists("collection.jl", prefix, n_files, std::atoi(argv[2]));
		InvertedList::externMemorySort(n_files, prefix);
		InvertedList::saveTermsDict(prefix, "inverted_lists/index");
	} else if(std::string(argv[1]) == "-u"){
		// url parsing mode: generates the file associating url to id of document. This can later be used to fit a hash.
		InvertedList::parseUrls("collection.jl", "inverted_lists/urls");
	} else{
		std::cout<<"No such option."<<std::endl;
	}
}