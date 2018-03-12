#ifndef HTTPDOWNLOADER_H
#define HTTPDOWNLOADER_H

#include <curl\curl.h>
#include <curl\easy.h>
#include <sstream>
#include <iostream>
#include <string>

class HttpDownloader {
public:   
	HttpDownloader();
	~HttpDownloader();
	std::string download(const std::string& url);
private:
	void* curl;
};
#endif