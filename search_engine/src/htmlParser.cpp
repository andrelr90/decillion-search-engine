#include "htmlParser.hpp"

bool HtmlParser::isLatinAlphaOrDigit(int value){
	// Expects a wchar_t number representing an unicode encoded char
	// Allow numbers, unicode lowercaseLetters, upperCaseLetters, suplementary latin letters: á, ç, etc except math symbols
	// See for suplementary latin: https://unicode-table.com/pt/blocks/latin-extended-a/
	if(!isdigit(value) && (value < 65 || value > 90) && (value < 97 || value > 122) && ((value < 192 || value > 252) || 
	    (value == 215 || value == 216 || value == 247 || value == 248))){
		return true;
	}
	return false;
}

std::string HtmlParser::removePunctuation(std::string contents){
	// Replaces non latin characters with space character.
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wsContent = converter.from_bytes(contents);
	std::replace_if (wsContent.begin (), wsContent.end (), isLatinAlphaOrDigit, ' ');
	contents = converter.to_bytes(wsContent);
	return contents;
}

 std::string HtmlParser::toLowerCaseLatin(std::string text){
 	// Lowercases each term, considering the unicode table for latin chars
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wsContent = converter.from_bytes(text);
	for(std::wstring::iterator it=wsContent.begin(); it!=wsContent.end(); it++){
		*it = tolower(*it);
		if(int(*it) >= 192 && int(*it) < 192+32){	//See unicode table for reference
			*it = *it+32;
		}
	}
	text = converter.to_bytes(wsContent);
	return text;
}

std::string HtmlParser::cleantext(GumboNode* node) {
	// Uses GUmbo parser built in functions to extract the text of a HTML document
	if (node->type == GUMBO_NODE_TEXT) {
		return std::string(node->v.text.text);
	} else if (node->type == GUMBO_NODE_ELEMENT && node->v.element.tag != GUMBO_TAG_SCRIPT && node->v.element.tag != GUMBO_TAG_STYLE) {
		std::string contents = "";
		GumboVector* children = &node->v.element.children;
		for (unsigned int i = 0; i < children->length; ++i) {
			const std::string text = cleantext((GumboNode*) children->data[i]);
			if (i != 0 && !text.empty()) {
				contents.append(" ");
			}
			contents.append(text);
		}
		contents = removePunctuation(contents);
		return contents;
	} else {
		return "";
	}
}

std::string HtmlParser::cleanTextFile(std::ifstream &in){
	// Cleans a specific file document.
	std::string contents;
	in.seekg(0, std::ios::end);
	contents.resize(in.tellg());
	in.seekg(0, std::ios::beg);
	in.read(&contents[0], contents.size());
	in.close();

	GumboOutput* output = gumbo_parse(contents.c_str());
	std::string text = cleantext(output->root);
	gumbo_destroy_output(&kGumboDefaultOptions, output);

	return text;
}
