#include "queryProcessor.hpp"
#include "fcgio.h"

using namespace std;

// Maximum bytes for query
const unsigned long STDIN_MAX = 256;

string get_request_content(const FCGX_Request & request) {
    char * content_length_str = FCGX_GetParam("CONTENT_LENGTH", request.envp);
    unsigned long content_length = STDIN_MAX;

    if (content_length_str) {
        content_length = strtol(content_length_str, &content_length_str, 10);

        if (*content_length_str) {
            cerr << "Can't Parse 'CONTENT_LENGTH='" << FCGX_GetParam("CONTENT_LENGTH", request.envp) << "'. Consuming stdin up to " << STDIN_MAX << endl;
        }

        if (content_length > STDIN_MAX) {
            content_length = STDIN_MAX;
        }
    } else {
        // Do not read from stdin if CONTENT_LENGTH is missing
        content_length = 0;
    }
    
    char * content_buffer = new char[content_length];
    cin.read(content_buffer, content_length);
    content_length = cin.gcount();

    // ignore() doesn't set the eof bit in some versions of glibc++
    // so use gcount() instead of eof()...
    do cin.ignore(1024); while (cin.gcount() == 1024);

    string content(content_buffer, content_length);
    delete [] content_buffer;
    return content;
}

int main(int argc, char** argv) {
	argc = argc; argv[0] = argv[0];

	setlocale(LC_ALL, "Portuguese");
	string prefix = "inverted_lists/inverted_list";


	cout<<"Carregando índice."<<endl;
	try{
		InvertedList::loadTermsDict("inverted_lists/index");
		cout<<"Índice carregado."<<endl;
		cout<<"Carregando URLs."<<endl;
		InvertedList::loadUrls("inverted_lists/urls");
		cout<<"URLs carregadas."<<endl;
		cout<<"Carregando pageRanks."<<endl;
		InvertedList::loadPageRankDict("inverted_lists/pagerank.jsonl");
		cout<<"pageRanks carregados."<<endl<<endl;
		cout<<"--------------------------------------"<<endl<<endl;
	} catch(runtime_error e){
		cout<<e.what()<<endl;
		return -1;
	}

	// FastCGI
	streambuf * cin_streambuf  = cin.rdbuf();
    streambuf * cout_streambuf = cout.rdbuf();
    streambuf * cerr_streambuf = cerr.rdbuf();

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    string query = "";
	while (FCGX_Accept_r(&request) == 0) {
		fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        query = get_request_content(request);

        cin.rdbuf(cin_streambuf);
    	cout.rdbuf(cout_streambuf);
    	cerr.rdbuf(cerr_streambuf);
        cout<<"Processing "<<query<<endl;

        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);

        cout << "Content-type: application/json\r\n\r\n";
        if (query.length() == 0) {
            cout << "{\"error\": true}";
		} else{
			cout << "{";
			try{
				// Creates the and processes the query.
				QueryProcessor qp(query, prefix);
				cout<<", \"error\": false";
			} catch (runtime_error e){
				cout<<", \"error\": true";
			}
			cout << "}";
		}
	}

	cin.rdbuf(cin_streambuf);
    cout.rdbuf(cout_streambuf);
    cerr.rdbuf(cerr_streambuf);
}


/*
	For FastCGI info, refer to:
	- http://chriswu.me/blog/writing-hello-world-in-fcgi-with-c-plus-plus/
	- http://chriswu.me/blog/getting-request-uri-and-content-in-c-plus-plus-fcgi/
	For multi-thread future implementation:
	- https://gist.github.com/dermesser/e2f9b66457ae19ebd116
*/