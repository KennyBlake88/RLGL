#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include "Player.hpp"
using namespace cv;
using namespace std;
 
Mat symbol;

void setSymbol(string symbolPath)
{
	symbol = imread(symbolPath);
}