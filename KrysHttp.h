#include "Krysock.h"
#include <string>
#include <map>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

#define MAX_CHUNK_LEN (1024*1024)
#define MAX_TOTAL_LEN (1024*1024*20)

string http_response (int server_socket);
string read_chunked (int server_socket); 
ssize_t read_http_header (int socket_, void* buf, int max_len);
int read_until (int socket_, void* buf, int max_len, const char* until);
string http_get (string str_host, string str_path, const map<string, string>& params = {}, unsigned short port = 80);
string http_post(string host, string path, string text, const map<string, string>& extra_info = {}, unsigned short port = 80);
