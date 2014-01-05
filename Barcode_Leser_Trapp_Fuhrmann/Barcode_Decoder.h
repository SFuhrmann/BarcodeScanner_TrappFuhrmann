#pragma once
#include <hash_map>
#include "opencv2/opencv.hpp"

// Barcode_Decoder
class Barcode_Decoder
{
// Konstruktion
public:
	Barcode_Decoder(void);	

// Implementierung
protected:
	cv::Mat imgCpy;
	std::hash_map<std::string, int> digitsMap;
	float round(float num);
	int read_digit(const cv::Mat& img, cv::Point& pos, int direction, int unit_width);
	bool traversSpace(const cv::Mat& img, cv::Point& pos);
	int getUnitWidth(const cv::Mat& img, cv::Point& pos);
	bool traversMidCode(const cv::Mat& img, cv::Point& pos);
	std::vector<int> readBlock(const cv::Mat& img, cv::Point& pos, int direction, int unit_width, std::vector<int>& digits);
	bool isValidBarcode(std::vector<int>& block);
	std::vector<int> getBarcode(const cv::Mat& img, cv::Point& startDraw, cv::Point& endDraw);

	
public:
	void initMap();
	CString getBarcodeString(CString filePath);
};