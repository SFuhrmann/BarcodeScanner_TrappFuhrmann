#include "opencv2/opencv.hpp"
#include <hash_map>
#include <sstream>
#include <string>
#include <algorithm>
using namespace cv;
using namespace stdext;
using namespace std;


#define SPACE 255
#define BAR 0

Mat imgCpy;
hash_map<string, int> digitsMap;

void initMap(){
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
float round(float num)
{
	return num < 0.0 ? ceil(num - 0.5) : floor(num + 0.5);
}

// reads a single digit starting from the current position of pos Point
// returns an int digit on succes
// returns -1 when pos.x is out of bounds
int read_digit(const Mat& img, cv::Point& pos, int direction, int unit_width)
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
bool traversSpace(const Mat& img, cv::Point& pos)
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
int getUnitWidth(const Mat& img, cv::Point& pos) 
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
bool traversMidCode(const Mat& img, cv::Point& pos) 
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
vector<int> readBlock(const Mat& img, cv::Point& pos, int direction, int unit_width, vector<int>& digits)  
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

// checks if block is valid
// receives a vector<int> from readBlock()
// returns true when block is valid
// returns false when block is invalid
// An invalid block could be an empty vector or a vector consisting of to many zeroes.
bool isValidBlock(vector<int>& block) 
{
	int zeroCounter = 0;
	int blockSize = block.size();


	if(blockSize < 6) 
	{
		return false;
	}

	for(int i = 0; i < blockSize; i++){
		if(block[i] == 0)
		{
			zeroCounter++;
		}
	}

	if(zeroCounter >= 5){
		return false;
	} 

	return true;
}


// traversing every pixel of the image and calling necessary functions to read a barcode.
// will return the barcode as a vector<int> of digits.
// will return and empty vector<int> when no barcode was found.
vector<int> getBarcode(const Mat& img, cv::Point& startDraw, cv::Point& endDraw)
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
					if(isValidBlock(readBlock(img, pos, 1, unitWidth, digits))) // read left Block
					{ 
						if(traversMidCode(img, pos))								//travers non data block
						{	
							if(isValidBlock(readBlock(img, pos, -1, unitWidth, digits))) // read right Block
							{ 	
								endDraw = pos;
								return digits;						// send digits back to calling function 
							}
						}
					}
				}
			}
		}
	}
	vector<int> null;		
	return null;			// return null when no barcode was found
}

// Draws a line where the barcode was found
void drawLine(const Mat& img, cv::Point& startPoint, cv::Point& endPoint)
{
	Mat tmp;
	img.copyTo(tmp);
	
	line(tmp,startPoint,endPoint,Scalar(0,255,0),2,8); // 2, 8
	imshow("image", tmp );
}

int main()
{
  namedWindow("image", CV_WINDOW_AUTOSIZE);
  Mat img = cv::imread("C:/Users/Public/Pictures/Sample Pictures/KoalaBarcode_noise1.jpg", CV_LOAD_IMAGE_COLOR);
  //Mat img = cv::imread("C:/Users/Public/Pictures/Sample Pictures/KoalaBarcode2.jpg", CV_LOAD_IMAGE_COLOR);
  //Mat img = cv::imread("C:/Users/Public/Pictures/Sample Pictures/KoalaBarcode3.jpg", CV_LOAD_IMAGE_COLOR);
  //Mat img = cv::imread("C:/Users/Public/Pictures/Sample Pictures/Testpattern1.png", CV_LOAD_IMAGE_COLOR);
  //Mat img = cv::imread("C:/Users/Public/Pictures/Sample Pictures/BarCodeUpscaled.png", CV_LOAD_IMAGE_COLOR);
  //Mat img = cv::imread("C:/Users/Public/Pictures/Sample Pictures/Barcode_UPC_test1.png", CV_LOAD_IMAGE_COLOR);
  //Mat img = cv::imread("C:/Users/Public/Pictures/Sample Pictures/BarcodeUPC2.png", CV_LOAD_IMAGE_COLOR);
  //Mat img = cv::imread("C:/Users/Public/Pictures/Sample Pictures/Barcode_UPC_rota1.png", CV_LOAD_IMAGE_COLOR);
  //Mat img = cv::imread("C:/Users/Public/Pictures/Sample Pictures/Barcode_UPC_rota2.png", CV_LOAD_IMAGE_COLOR);
  //Mat img = cv::imread("C:/Users/Public/Pictures/Sample Pictures/KoalaBarcode_rota1.jpg", CV_LOAD_IMAGE_COLOR);
  //Mat img = cv::imread("C:/Users/Public/Pictures/Sample Pictures/Barcode_UPC_rota180.png", CV_LOAD_IMAGE_COLOR);

  
  initMap();
  Mat rgbCpy;
  img.copyTo(rgbCpy);
  cvtColor(img, img, CV_BGR2GRAY);
  cv::threshold(img, img, 128, 255, cv::THRESH_BINARY); // 128 - 255

  cv::Point startDraw(0,0);
  cv::Point endDraw(0,0);
  vector<int> barcode = getBarcode(img, startDraw, endDraw);	// get barcode
  
  if(barcode.size() > 0) // Display data when barcode found
  {
	drawLine(rgbCpy, startDraw, endDraw); // draw Line and display image
	printf("Barcode gefunden! : \n");
	//printf("x : \n");
	for (int i = 0; i < 12; i++){
		std::cout << barcode[i];			// print barcode digits
	}
	std::cout << std::endl;
  }
  else  // print error message when no barcode was found
  {
	printf("Kein Barcode gefunden");
	imshow("image", rgbCpy );
  }

  waitKey(0);
}

