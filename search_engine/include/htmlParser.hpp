#include <fstream>
#include <iostream>
#include <locale.h>
#include <chrono>
#include <bits/stdc++.h>

#include "gumbo.h"

class HtmlParser{
public:
	static bool isLatinAlphaOrDigit(int value);
	static std::string removePunctuation(std::string contents);
	static std::string toLowerCaseLatin(std::string text);
	static std::string cleantext(GumboNode* node);
	static std::string cleanTextFile(std::ifstream &in);
};