#ifndef HTTPDOWNLOADER_H
#define HTTPDOWNLOADER_H

#include <curl\curl.h>
#include <curl\easy.h>
#include <sstream>
#include <iostream>
#include <string>

class HttpDownloader {
public:   
	std::string download(const std::string& url);
private:
};
#endif