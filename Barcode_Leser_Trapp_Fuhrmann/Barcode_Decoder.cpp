#include "opencv2/opencv.hpp"
#include "stdafx.h"
#include <hash_map>
#include <sstream>
#include <string>
#include <algorithm>
#include "Barcode_Decoder.h"
using namespace cv;
using namespace stdext;
using namespace std;

#define SPACE 255
#define BAR 0
#define BARCODESIZE 12

Barcode_Decoder::Barcode_Decoder(void){};

void Barcode_Decoder::initMap(){
	//Bar = positive digits
	//Space = negative digits
	//example of first entry: "-32-11" = -3,2,-1,1 = 0001101 = 0
	
	digitsMap["-32-11"] = 0;
	digitsMap["-22-21"] = 1;
	digitsMap["-21-22"] = 2;
	digitsMap["-14-11"] = 3;
	digitsMap["-11-32"] = 4;
	digitsMap["-12-31"] = 5;
	digitsMap["-11-14"] = 6;
	digitsMap["-13-12"] = 7;
	digitsMap["-12-13"] = 8;
	digitsMap["-31-12"] = 9;	
}


//helper for rounding numbers => simulates Math.round()
float Barcode_Decoder::round(float num)
{
	return num < 0.0 ? ceil(num - 0.5) : floor(num + 0.5);
}

// reads a single digit starting from the current position of pos Point
// returns an int digit on succes
// returns -1 when pos.x is out of bounds
int Barcode_Decoder::read_digit(const Mat& img, cv::Point& pos, int direction, int unit_width)
{
  int pattern[4] = {0, 0, 0, 0};
  stringstream result;

   for (int i = 0; i < 4; i++)
   {
		int startPos = img.at<uchar>(pos);
		int add = 0;
	
		if(startPos == BAR) // When reading BAR the result should be incremented
		{ 
			add = 1;
		}
		else                // When reading SPACE the result should be decremented
		{
			add = -1;
		}

		while(img.at<uchar>(pos) == startPos){
			pattern[i] += add;
			pos.x++;
			if(pos.x >= img.cols -1) // out of bounds
			{
				return -1;
			}
		}
  
		int number = round((float)pattern[i] / unit_width); // image data / barcode is often distorted, so a rounding for approximation is needed  
		result << number * direction;	// flip digit accoring to reading direction => left block = number * 1; right block = number * -1;
  }

  string str = result.str(); 
  return digitsMap[str];
}


// will skip SPACE pixels and find the first BAR pixel.
// returns true when at least 10 SPACE pixels where skipped before hitting a BAR pixel.
// returns false when less then 10 SPACE pixels where skipped before hitting a BAR pixel or pos.x is out of bounds.
bool Barcode_Decoder::traversSpace(const Mat& img, cv::Point& pos)
{
  int spaceCounter = 0;
  while (img.at<uchar>(pos.y,pos.x) == SPACE){
	  if(pos.x >= img.cols -1) // out of bounds
	  {
		return false;
	  }
	  pos.x++;	 
	  spaceCounter++;
  }
  
  if(spaceCounter > 9) // Space must be at least 10 pixels wide 
  { 
	 return true;
  }
  else
  {
	 return false;
  }
}


// unit width is needed as a refernce for reading digits of the two data blocks
// returns the mean value of a single bar when starting pattern was found
// returns -1 when no pattern was found or pos.x is out of bounds
int Barcode_Decoder::getUnitWidth(const Mat& img, cv::Point& pos) 
{
  int pattern[3] = { BAR, SPACE, BAR };
  float widths[3] = { 0, 0, 0 };
  for (int i = 0; i < 3; i++)
  {
    while (img.at<uchar>(pos) == pattern[i])
    {
      if(pos.x >= img.cols - 1) // out of bounds
	  { 
		return -1;
	  }

	  pos.x++;
      widths[i]++;
    }
  }
  
  float max = std::max(widths[0],std::max(widths[1],widths[2]));  // determin maxmium value
  float min = std::min(widths[0],std::min(widths[1],widths[2]));  // determin minimum value

  if(min/max >= 0.75)   // when the relation between min and max is beyond a certain threshold (perfect = 1, adequate >= 0.75)
  {
	 return round(float(widths[0] + widths[1] + widths[2]) / 3);	// return mean value
  }
  return -1;
}



// traverses the pattern which appears after the first data block 
bool Barcode_Decoder::traversMidCode(const Mat& img, cv::Point& pos) 
{
  int pattern[5] = { SPACE, BAR, SPACE, BAR, SPACE };
  for (int i = 0; i < 5; i++){
	  while (img.at<uchar>(pos) == pattern[i]){
		  pos.x++;
		  if(pos.x >= img.cols - 1) // out of bounds
		  { 
			return false;
		  }
	  }
   }
   return true;
}

// reads a block -> 6 digits from the barcode
// receives a vector<int> from getBarcode()
// This Method is called two times when attempting to read a barcode ->
  // -> First time the digits vector is empty, direction is indicating to read the left block (direction = 1)
  // -> Second time the vector is already filled with 6 barcode digits, this time the direction indicates to read the right block (direction = -1)
// returns a vector<int> when all digits could be read without error
// returns an empty vector<int> when an error occurs
vector<int> Barcode_Decoder::readBlock(const Mat& img, cv::Point& pos, int direction, int unit_width, vector<int>& digits)  
{ 
	for (int i = 0; i < 6; i++)
	{
		int d = read_digit(img, pos, direction, unit_width); 
							
		if(d < 0) // error 
		{ 
			vector<int> null;
			return null;     // return null when digit is broken
		}
		digits.push_back(d);	// add digit to array
	}

	return digits;
}

// Calculates check digit
// https://en.wikipedia.org/wiki/Check_digit#UPC
// returns true when barcode is valid
// returns false when barcode is invalid
bool Barcode_Decoder::isValidBarcode(vector<int>& block)
{
	int sum = 0;

	if(block.size() == BARCODESIZE) 
	{
		for(int i = 0; i < block.size(); i += 2) // add every odd index of the barcode array
		{
			sum += block[i];		
		}
		if(sum == 0)
		{
			return false;
		}

		sum *= 3; // multiply the result by three
		int firstBlockSum = sum;

		for(int i = 1; i < block.size() -1; i += 2) // add every even index.
		{
			sum += block[i];		
		}

		if(sum != firstBlockSum && 10 - (sum % 10) == block[block.size() - 1]) // comparing last array index with check digit 
		{
			return true; 
		}
	}
	
	return false;
}


// traversing every pixel of the image and calling necessary functions to read a barcode.
// will return the barcode as a vector<int> of digits.
// will return and empty vector<int> when no barcode was found.
vector<int> Barcode_Decoder::getBarcode(const Mat& img, cv::Point& startDraw, cv::Point& endDraw)
{
	cv::Point pos(0,0);
	for(pos.y = 0; pos.y < img.rows; pos.y++)
	{		
		for(pos.x = 0; pos.x < img.cols; pos.x++)
		{	
			if(traversSpace(img, pos)) // skip white pixels (function will return 0 when less then 10 pixels are skipped)
			{  
				int unitWidth = getUnitWidth(img, pos);
				if(unitWidth > 0)									// when appropriate start of barcode was found
				{
					startDraw = pos;
					vector<int> digits;
					readBlock(img, pos, 1, unitWidth, digits);      // read left Block
					if(traversMidCode(img, pos))								//travers non data block
					{	
						if(isValidBarcode(readBlock(img, pos, -1, unitWidth, digits))) // read right Block
						{ 	
							endDraw = pos;
							return digits;						// send digits back to calling function 
						}
					}
				}
			}
		}
	}
	vector<int> null;		
	return null;			// return null when no barcode was found
}

CString Barcode_Decoder::getBarcodeString(CString filePath)
{
  Mat img = cv::imread((LPCTSTR) filePath, CV_LOAD_IMAGE_COLOR);

  Mat rgbCpy;
  img.copyTo(rgbCpy);
  cvtColor(img, img, CV_BGR2GRAY);
  cv::threshold(img, img, 128, 255, cv::THRESH_BINARY); // 128 - 255

  cv::Point startDraw(0,0);
  cv::Point endDraw(0,0);
  vector<int> barcode = getBarcode(img, startDraw, endDraw);	// get barcode
  CString result, result1;
  
  if(barcode.size() > 0) // Display data when barcode found
  {
	for (int i = 0; i < barcode.size(); i++)
	{
		CString a;
		a.Format("%d", barcode[i]);
		result = result + a;
	}
	return result;
  }
  return "No Barcode detected";
}