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
	unsigned long body_len;                   //http消息体长度
	string str_http_body;           //http消息体
	auto up_rcv_header = make_unique<char[]> (HTTP_HEADER_LIMIT + 1); //http消息头部缓冲区


	//接收http首部消息
	ssize_t http_header_len = read_http_header (server_socket, up_rcv_header.get(), HTTP_HEADER_LIMIT);

	if (http_header_len == -1)
	{
		return "";
	}
	up_rcv_header[http_header_len] = '\0'; //在尾部补上结束符

	
	//尝试解析http首部的Content-Length字段获取消息体长度
	do
	{
		boost::regex expression("\r\nContent-Length:[[:space:]]+([[:digit:]]+)"); //使用正则匹配
		boost::smatch hit;
		if (boost::regex_search (string(up_rcv_header.get()), hit, expression))
		{
			try
			{
				body_len = boost::lexical_cast<unsigned long> (hit[1].str()); //将匹配到的数据转换成整型
			}
			catch (std::bad_cast err) //转换失败则报错返回
			{
				return "";
			}
		}
		else
		{
			break; //无法读取Content-Length 尝试读取chunk
		}
		//--------------------------

		//Content-Length解析成功, 开始读取http报文体

		str_http_body.resize (body_len); //分配缓冲区内存

		unsigned long body_rcv = tcp_read_timeout(server_socket, const_cast<char*>(str_http_body.data()), body_len, 15); //接受数据到缓冲区

		if (body_rcv != body_len) //长度校验
		{
			return string ("");
		}
		//----------------

		return str_http_body; //返回http报文体
	}
	while (0);

	//检查是否有chunked提示
	boost::regex expression("\r\nTransfer-Encoding:[[:space:]]+chunked");
	boost::smatch hit;
	if (!boost::regex_search (string(up_rcv_header.get()), hit, expression)) //chunked格式解析失败则报错
	{
		return string ("");
	}

	str_http_body = read_chunked (server_socket); //以chunked方式读取http回复报文

	return str_http_body; //返回http报文体
}


string read_chunked (int server_socket) //该函数在服务器返回chunked字段时使用, 用于读取http报文体
{
	int read_len; //用于存储tcp读取函数的返回值

	unique_ptr<char[]> up_chunk_len = make_unique<char[]> (128 + 1); //chunked首部缓冲区
	int n_chunk_len; //chunked首部数字解析后所标识的长度

	string str_chunk_content; //数据块中的内容消息体
	string str_output; //最终整合的返回数据

	list<string> list_chunk; //消息体链表
	int n_total_len = 0; //http消息体总字节数

	while (1)
	{
		//读取chunk首部标识长度的信息
		read_len = read_until(server_socket, up_chunk_len.get(), 128, "\r\n");

		if (read_len == -1) //读取失败则记录日志并且返回
		{
			return "";
		}

		up_chunk_len[read_len - 2] = '\0'; //chunk字符串尾部加0

		sscanf(up_chunk_len.get(), "%x", &n_chunk_len); //解析获�chunk长度值

		if (n_chunk_len > MAX_CHUNK_LEN)
		{
			return "";
		}
		else if (n_chunk_len == 0) //如果chunk长度为0则表示已经达到报文结尾, 此时退��循环组装数据
		{
			break;
		}

		str_chunk_content.resize (n_chunk_len + 2); //分配空间准备接收chunk包体信息

		read_len = tcp_read_timeout(server_socket, const_cast<char*>(str_chunk_content.data()), n_chunk_len + 2, 15); //读取chunk包体以及尾部的 "\r\n"

		if (read_len != n_chunk_len + 2) //如果读取异常则退出
		{
			return "";
		}

		str_chunk_content.resize (n_chunk_len); //重置消息长度, 截掉尾部元数据

		list_chunk.emplace_back (move (str_chunk_content)); //在链表中加入本次读取的数据

		if ((n_total_len += n_chunk_len) >  MAX_TOTAL_LEN) //判断是否超过�������大总长度, 如果超过则退出读取
		{
			return "";
		}
	}

	str_output.resize (n_total_len); //预先分配内存准备存总数据
	str_output = "";

	for (auto it : list_chunk) //把每次读取到的缓冲区拼接起来
	{
		str_output += it;
	}

	return str_output; //返回拼接结果
}

ssize_t read_http_header (int socket_, void* buf, int max_len)  //读取http首部
{
	// readval --> 单次读取所获得的长度, offset-->偏移量, tmp_ptr--> 带有偏移的临时缓冲区
	int32_t readval = 0, offset = 0; char* tmp_ptr = (char*)buf;
	const char* header_bondary = "\r\n\r\n"; //边界设定

	while (offset < max_len)    //循环会在超过最大长度时退出
	{

		/*----------------------------------------peek buffer----------------------------------------------*/
		readval = recv_peek (socket_, tmp_ptr, max_len - offset); //窥探缓冲区中数据

		if (readval <= 0)
		{
			return readval;
		}
		/*----------------------------------------------END------------------------------------------------*/

		/*------------------------------------------examine buffer-----------------------------------------*/
		for (int32_t i=0; i<= readval - 4; i++)
		{
			if (memcmp (tmp_ptr + i, header_bondary, 4) == 0) //一旦读取到边界符号则从缓冲区中取出数据并且返回
			{
				readval = tcp_read_timeout (socket_, tmp_ptr, i + 4, 15);

				return readval == i + 4 ? readval + offset : -1;
			}
		}
		/*----------------------------------------------END------------------------------------------------*/

		/*--------------------------------end flag not found, updating offset------------------------------*/
		if (readval + offset >= max_len) //如果已经到达最大缓冲区则退出循环
		{
			errno = EIO;
			break;
		}
		offset += readval;      //如果未找到匹配, �从缓冲区中取走数据并更新偏移值和指针

		if ((readval = tcp_read_timeout (socket_, tmp_ptr, offset, 15)) != offset)
		{
			return -1;
		}

		tmp_ptr += offset;
		/*----------------------------------------------END------------------------------------------------*/
	}

	return -1; //超过缓冲区长度则返回错误
}

int read_until (int socket_, void* buf, int max_len, const char* until)  //读取tcp字节流直到遇到指定内容
{
	// readval --> 单次读取所获得的长度, offset-->偏移量, tmp_ptr--> 带有偏移的临时缓冲区

	int32_t readval = 0, offset = 0; char* tmp_ptr = (char*)buf;

	int until_len = strlen (until);


	while (offset < max_len)    //循环会在超过最大长度时退出
	{

		/*----------------------------------------peek buffer----------------------------------------------*/
		readval = recv_peek (socket_, tmp_ptr, max_len - offset); //窥探缓冲区中数据

		if (readval <= 0)
		{
			return readval;
		}
		/*----------------------------------------------END------------------------------------------------*/


		/*------------------------------------------examine buffer-----------------------------------------*/
		for (int32_t i=0; i<= readval - until_len; i++)
		{
			if (memcmp (tmp_ptr + i, until, until_len) == 0) //一旦读取到边界符号则从缓冲区中取出数据并且返回
			{
				readval = tcp_read_timeout (socket_, tmp_ptr, i + until_len, 15);

				return readval == i + until_len ? readval + offset : -1;
			}
		}
		/*----------------------------------------------END------------------------------------------------*/


		/*--------------------------------end flag not found, updating offset------------------------------*/
		if (readval + offset >= max_len) //如果已经到达最大缓冲区则退出循环
		{
			errno = EIO;
			break;
		}

		offset += readval;      //如果未找到匹配, �从缓冲区中取走数据并更新偏移值和指针

		if ((readval = tcp_read_timeout (socket_, tmp_ptr, offset, 15)) != offset)
		{
			return -1;
		}

		tmp_ptr += offset;
		/*----------------------------------------------END------------------------------------------------*/

	}

	return -1; //超过缓冲区长度则返回错误
}



string http_get (string str_host, string str_path, const map<string, string>& params, unsigned short port)
{
	int server_socket;              //服务器套接字

	//参数的格式化------------------
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
	//-------------------------

	//发送报文的格式化-------------------
	boost::format send_header_format (
			"%1% HTTP/1.1\r\n"
			"Host: %2%\r\n"
			"\r\n");
	send_header_format % str_get % str_host;
	//------------------------------------

	//TCP连接的三次握手-----------------------
	if ((server_socket = tcp_open (str_host.c_str(), port)) == -1)
	{
		return string("");
	}
	//-----------------------------------------

	//发送报文---------------------------------
	if (tcp_write(server_socket, send_header_format.str().c_str(), send_header_format.str().length()) != (int)send_header_format.str().length())
	{
		return string ("");
	}
	//-----------------------------------------

	auto str_response = http_response(server_socket); //取得服务器回复信息并返回消息体

	close (server_socket);

	return str_response;
}

