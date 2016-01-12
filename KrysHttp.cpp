#include "KrysHttp.h"
#include <iostream>
using namespace std;

#define HTTP_VERSION "1.1"
#define HTTP_HEADER_LIMIT 8192

string http_post(string host, string path, string text, const map<string, string>& extra_info, unsigned short port)
{
	int server_socket;
	auto up_header_buffer = make_unique<char[]> (4096 + 1);
	string str_extra_params;
	string str_send;

	for (auto & it : extra_info)
	{
		str_extra_params += it.first + ": " + it.second + "\r\n";
	}

	
	
	
	str_send = "POST " + path + " HTTP/" HTTP_VERSION "\r\n"
		"Host: " + host + "\r\n"
		"Content-Length: " + boost::lexical_cast<string> (text.length()) + "\r\n"
		+ str_extra_params + 
		"\r\n" + 
		text;

	cout << str_send << endl;


	if ((server_socket = tcp_open (host.c_str(), port)) == -1)
	{
		return "";
	}

	if (tcp_write (server_socket, str_send.c_str(), str_send.length()) != (int)str_send.length())
	{
		return "";
	}


	auto str_response = http_response(server_socket);

	close (server_socket);

	return str_response;
}

string http_response (int server_socket)
{
	unsigned long body_len;                   
	string str_http_body;           
	auto up_rcv_header = make_unique<char[]> (HTTP_HEADER_LIMIT + 1); 


	
	ssize_t http_header_len = read_http_header (server_socket, up_rcv_header.get(), HTTP_HEADER_LIMIT);

	if (http_header_len == -1)
	{
		return "";
	}
	up_rcv_header[http_header_len] = '\0'; 
	
	do
	{
		boost::regex expression("\r\nContent-Length:[[:space:]]+([[:digit:]]+)"); 
		boost::smatch hit;
		if (boost::regex_search (string(up_rcv_header.get()), hit, expression))
		{
			try
			{
				body_len = boost::lexical_cast<unsigned long> (hit[1].str()); 
			}
			catch (std::bad_cast err) 
			{
				return "";
			}
		}
		else
		{
			break; 
		}
		

		

		str_http_body.resize (body_len); 

		unsigned long body_rcv = tcp_read_timeout(server_socket, const_cast<char*>(str_http_body.data()), body_len, 15); 

		if (body_rcv != body_len) 
		{
			return string ("");
		}
		

		return str_http_body; 
	}
	while (0);

	
	boost::regex expression("\r\nTransfer-Encoding:[[:space:]]+chunked");
	boost::smatch hit;
	if (!boost::regex_search (string(up_rcv_header.get()), hit, expression)) 
	{
		return string ("");
	}

	str_http_body = read_chunked (server_socket); 

	return str_http_body; 
}


string read_chunked (int server_socket) 
{
	int read_len; 

	unique_ptr<char[]> up_chunk_len = make_unique<char[]> (128 + 1); 
	int n_chunk_len; 

	string str_chunk_content; 
	string str_output; 

	list<string> list_chunk; 
	int n_total_len = 0; 

	while (1)
	{
		
		read_len = read_until(server_socket, up_chunk_len.get(), 128, "\r\n");

		if (read_len == -1) 
		{
			return "";
		}

		up_chunk_len[read_len - 2] = '\0'; 

		sscanf(up_chunk_len.get(), "%x", &n_chunk_len); 

		if (n_chunk_len > MAX_CHUNK_LEN)
		{
			return "";
		}
		else if (n_chunk_len == 0) 
		{
			break;
		}

		str_chunk_content.resize (n_chunk_len + 2); 

		read_len = tcp_read_timeout(server_socket, const_cast<char*>(str_chunk_content.data()), n_chunk_len + 2, 15); 

		if (read_len != n_chunk_len + 2) 
		{
			return "";
		}

		str_chunk_content.resize (n_chunk_len); 

		list_chunk.emplace_back (move (str_chunk_content)); 

		if ((n_total_len += n_chunk_len) >  MAX_TOTAL_LEN) 
		{
			return "";
		}
	}

	str_output.resize (n_total_len); 
	str_output = "";

	for (auto it : list_chunk) 
	{
		str_output += it;
	}

	return str_output; 
}

ssize_t read_http_header (int socket_, void* buf, int max_len)  
{
	
	int32_t readval = 0, offset = 0; char* tmp_ptr = (char*)buf;
	const char* header_bondary = "\r\n\r\n"; 

	while (offset < max_len)    
	{

		/*----------------------------------------peek buffer----------------------------------------------*/
		readval = recv_peek (socket_, tmp_ptr, max_len - offset); 

		if (readval <= 0)
		{
			return readval;
		}
		/*----------------------------------------------END------------------------------------------------*/

		/*------------------------------------------examine buffer-----------------------------------------*/
		for (int32_t i=0; i<= readval - 4; i++)
		{
			if (memcmp (tmp_ptr + i, header_bondary, 4) == 0) 
			{
				readval = tcp_read_timeout (socket_, tmp_ptr, i + 4, 15);

				return readval == i + 4 ? readval + offset : -1;
			}
		}
		/*----------------------------------------------END------------------------------------------------*/

		/*--------------------------------end flag not found, updating offset------------------------------*/
		if (readval + offset >= max_len) 
		{
			errno = EIO;
			break;
		}
		offset += readval;      

		if ((readval = tcp_read_timeout (socket_, tmp_ptr, offset, 15)) != offset)
		{
			return -1;
		}

		tmp_ptr += offset;
		/*----------------------------------------------END------------------------------------------------*/
	}

	return -1; 
}

int read_until (int socket_, void* buf, int max_len, const char* until)  
{
	

	int32_t readval = 0, offset = 0; char* tmp_ptr = (char*)buf;

	int until_len = strlen (until);


	while (offset < max_len)    
	{

		/*----------------------------------------peek buffer----------------------------------------------*/
		readval = recv_peek (socket_, tmp_ptr, max_len - offset); 

		if (readval <= 0)
		{
			return readval;
		}
		/*----------------------------------------------END------------------------------------------------*/


		/*------------------------------------------examine buffer-----------------------------------------*/
		for (int32_t i=0; i<= readval - until_len; i++)
		{
			if (memcmp (tmp_ptr + i, until, until_len) == 0) 
			{
				readval = tcp_read_timeout (socket_, tmp_ptr, i + until_len, 15);

				return readval == i + until_len ? readval + offset : -1;
			}
		}
		/*----------------------------------------------END------------------------------------------------*/


		/*--------------------------------end flag not found, updating offset------------------------------*/
		if (readval + offset >= max_len) 
		{
			errno = EIO;
			break;
		}

		offset += readval;      

		if ((readval = tcp_read_timeout (socket_, tmp_ptr, offset, 15)) != offset)
		{
			return -1;
		}

		tmp_ptr += offset;
		/*----------------------------------------------END------------------------------------------------*/

	}

	return -1; 
}



string http_get (string str_host, string str_path, const map<string, string>& params, unsigned short port)
{
	int server_socket;              

	
	string str_get = string ("GET ") + str_path + "?";
	for (auto iter = params.begin(); iter != params.end(); ++iter)
	{
		str_get += (iter->first + "=" + iter->second);

		auto tmp = iter;
		tmp++;
		if (tmp != params.end())
		{
			str_get += "&";
		}
	}
	

	
	boost::format send_header_format (
			"%1% HTTP/1.1\r\n"
			"Host: %2%\r\n"
			"\r\n");
	send_header_format % str_get % str_host;
	

	
	if ((server_socket = tcp_open (str_host.c_str(), port)) == -1)
	{
		return string("");
	}
	

	
	if (tcp_write(server_socket, send_header_format.str().c_str(), send_header_format.str().length()) != (int)send_header_format.str().length())
	{
		return string ("");
	}
	

	auto str_response = http_response(server_socket); 

	close (server_socket);

	return str_response;
}

