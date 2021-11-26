// Red Light Green Light part 2.cpp : This file contains the 'main' function. Program execution begins and ends there.

//Includes:
#include <iostream>							//This is how you get inputs/outputs in C++. (iostream: input/Output stream)
#include <opencv2/highgui/highgui_c.h>		//Needed for CV_WINDOW_AUTOSIZE.
#include <opencv2/highgui.hpp>				//Needed for displaying windows/all GUI functionality.
#include <opencv2/cudaimgproc.hpp>			//Needed for CUDA image processing.
#include <opencv2/cudabgsegm.hpp>			//Needed for CUDA MOG.
#include <opencv2/core/cuda.hpp>			//Needed for basic CUDA functionality
#include <opencv2/cudaobjdetect.hpp>		//Needed for ACCESS_MASK
#include <thread>							//Needed for multithreading
#include <future>							//Needed to get the result from a thread		
#include <stdlib.h>							//Needed for random number generation
#include <map>								//Needed to store sound names/lengths
#include <time.h>							//Needed for true random.

//Both needed for playing sounds.
#include <windows.h>
#include <mmsystem.h>

//The reason why we don't use the namespace cv is because it interferes with the PlaySound() library that's built into windows,
//because of this, any CV item we have to declare with "cv::" beforehand. 
using namespace std;		//standard namespace
using namespace cv::cuda;	//openCV cuda namesapce.



//Main Variables:
int cameraNumber = 0;											//The number of the camera that the user wants to use.
cv::RNG rng;													//Random Number Generator (OpenCV).
GpuMat gpuFrame, gpuMask;										//The frame from the camera, when uploaded to the GPU & The Mask for the Mog Detection.
cv::VideoCapture camera = cv::VideoCapture(cameraNumber);		//The camera, in our case, the camera at index 0. This can be changed to whatever # camera you want.
cv::Ptr<cv::BackgroundSubtractor> background_subtractor_gpu;	//Pointer to a Background Subtractor.
vector<cv::Rect> detectedMovementAreas;							//The movement detected areas, before the scans for symbols.
vector<cv::Rect> symbolDetectedAreas;							//The movement detected areas with symbols in them (players).
cv::Mat frame;													//The base frame from the camera.	
cv::Mat maskFrame;												//The frame where the MOG detection is written to.
int numberOfDetections;											//The amount of players we see.
int playerCount = 0;											//The amount of players currently playing.
const int maxPlayers = 4;										//How many players are allowed to play.
const map<int, int> soundMap = {								//The green light sounds/times associated with those sounds.             
	{1, 4663},		//Sound 1: The normal one, lasts 4.663s.
	{2, 4000},		//Sound 2: Normal but faster: Lasts 4.000s.
	{3, 3430},		//Sound 3: Lasts 3.430s.
	{4, 4267},		//Sound 4: Lasts 4.267s.
	{5, 1715},		//Sound 5: Lasts 1.715s
	{6, 2786},		//Sound 6: Lasts 2.786s.
	{7, 1165}		//Sound 7: Lasts 1.165s.
};
bool scanning = false;											//This is true for the duration of the endRound sound. 
thread threads[];
		
//Window names:
string mainWindow = "Robo Robots Red Light Green Light Robot";	//The main window that the players can see.
string cameraDisplay = "displayWindow";							//The Camera Window. Only the "game master" should be able to see this


//Assets:
cv::Mat greenLight = cv::imread("C:\\Users\\kenny\\source\\repos\\Red Light Green Light part 2\\Red Light Green Light part 2\\assets\\images and videos\\greenlight.png");	//Just a completely green picture, the visual start cue for the players.
cv::Mat redLight = cv::imread("C:\\Users\\kenny\\source\\repos\\Red Light Green Light part 2\\Red Light Green Light part 2\\assets\\images and videos\\redlight.png");		//Just a completely red 1920x1080 picture, the visual stop cue for the players.

//Functions (Documentation above function definition):
int startRound(int);					
int endRound(int);
int playRedLightSound();
int playGreenLightSound(int, int);
int watchForMovement();
void nullCamera();

/*
* Starts a new round, which entails:
*	1. Deciding how long the round will be. 
*		This is done by randomly choosing a sound length, and which has a time in milis associated with it.
*		This is then passed to the playGreenLightSound() function. 
*	2. Playing the sound on a thread for the amount of time decided.
*		Using the PlaySound API, we sleep for the amount of time associated with the name of the sound.
*	3. Showing the greenLight image.
*		This is done using OpenCV's imshow() function in a namedwindow(). 
*/
int startRound(int roundNumber)
{
	int randomNumber =  rand() % 7 + 1;
	cout << randomNumber << endl;
	int milis = soundMap.at(randomNumber);
	thread sound = thread(playGreenLightSound, randomNumber, milis);
	thread nc = thread(nullCamera);
	imshow(mainWindow, greenLight);
	if (cv::waitKey(1) == 'q')
	{
		sound.detach();
		exit(1);
		cv::destroyWindow(mainWindow);
		return 1;
	}
	nc.detach();
	sound.join();
	return 0;
}


int endRound(int roundNumber)
{
	thread sound = thread(playRedLightSound);
	thread watch = thread(watchForMovement);
	scanning = true;
	imshow(mainWindow, redLight);
	if (cv::waitKey(1) == 'q')
	{
		sound.detach();
		exit(1);
		cv::destroyWindow(mainWindow);
		return 0;
	}
	sound.join();
	watch.join();
	return 0;
}

int watchForMovement()
{
	cv::namedWindow("BASE");
	cv::namedWindow("MOG");
	cv::namedWindow("CROPPED");
	Sleep(300);

	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;

	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(100, 100), cv::Point(3, 3));
	cv::Rect mr;


	GpuMat grayScale;
	Sleep(300);
	while (scanning)
	{
		camera >> frame;
		gpuFrame.upload(frame);
		background_subtractor_gpu->apply(gpuFrame, gpuMask);
		gpuMask.download(maskFrame);

		morphologyEx(maskFrame, maskFrame, CV_MOP_CLOSE, element);

		try 
		{
			cv::findContours(maskFrame, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		}
		catch (exception& e)
		{
			cout << e.what() << endl;
		}

		vector< vector< cv::Point> >::iterator itc = contours.begin();
		while (itc != contours.end())
		{
			mr = cv::boundingRect(cv::Mat(*itc));
			rectangle(frame, mr, CV_RGB(255, 0, 0));
			++itc;
		}



		try
		{
			cv::imshow("BASE", frame);
			cv::imshow("MOG", maskFrame);
			cv::imshow("CROPPED", frame(mr));
		}
		catch (exception& e)
		{
			cout << e.what() << endl;
		}

		if (cv::waitKey(1) == 'q')
		{
			return 0;
		}
	}
	cv::destroyWindow("BASE");
	cv::destroyWindow("MOG");
	cv::destroyWindow("CROPPED");

}


void fireOnMovingPlayer()
{
	/*
	* THIS IS WHERE THE ARDUINO CODE WILL GO.
	*/
	return;
}

void searchForSymbolInTargetedArea()
{
	return;
}


int playGreenLightSound(int num, int milis)
{
	string filePath = "C:\\Users\\kenny\\source\\repos\\Red Light Green Light part 2\\Red Light Green Light part 2\\assets\\audios\\roundActive\\" + to_string(num) + ".wav";
	std::wstring stemp = std::wstring(filePath.begin(), filePath.end());
	LPCWSTR sw = stemp.c_str();
	PlaySound(sw, NULL, SND_ASYNC | SND_FILENAME);
	Sleep(milis + 300);
	return 0;
}

int playRedLightSound()
{
	scanning = true;
	PlaySound(L"C:\\Users\\kenny\\source\\repos\\Red Light Green Light part 2\\Red Light Green Light part 2\\assets\\audios\\scanPlayers.wav", NULL, SND_ASYNC | SND_FILENAME);
	Sleep(3625);
	scanning = false;
	return 0;

}



/*
* The function displays the squidgame logo, along with the number of players, until a player wants to play. (Spacebar) increases it.
*/
int displayPlayerCount()
{
	cv::namedWindow(mainWindow);
	cv::VideoCapture currentGif;
	cv::Mat frame;
	string currentGifPath;

	for (int i = 0; i <= maxPlayers; i++)
	{
		currentGifPath = "C:\\Users\\kenny\\OneDrive\\Desktop\\" + to_string(i) + "-players.mp4";
		currentGif = cv::VideoCapture(currentGifPath);

		if (currentGif.isOpened() == false)
		{
			return 1;
		}
		while (1)
		{
			if (!currentGif.read(frame))
			{
				currentGif.release();
  				currentGif = cv::VideoCapture(currentGifPath);
				currentGif.read(frame);
			}

			imshow(mainWindow, frame);
			char key = cv::waitKey(40);
			if(key == ' ')
			{
				playerCount++;
				break;
			}
			else if (key == 'q')
			{
				cv::destroyWindow(mainWindow);
				return 0;
			}

		}
	}
	return 0;
}

void setCameraProperties()
{
	camera.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
	camera.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
}

void nullCamera()
{
	while (!scanning)
	{
		camera >> frame;
	}
	return;
}

void correctCamera()
{
	cv::namedWindow("correctCamera");
	char exit = 'j';
	cv::Rect rect = cv::Rect(cv::Point(0, 675), cv::Point(1280, 720));

	while (exit != ' ')
	{
		camera >> frame;
		cv::rectangle(frame, rect, cv::Scalar(255, 255, 255), cv::FILLED);
		cv::putText(frame, "Press Space if this is the correct camera, 'n' if otherwise", cv::Point(160, 710), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0));

		try 
		{
			cv::imshow("correctCamera", frame);
		}
		catch (exception& e)
		{
			cameraNumber = 0;
			camera = cv::VideoCapture(cameraNumber);
			setCameraProperties();
		}
		exit = cv::waitKey(1);
		if (exit == 'n')
		{
			cameraNumber++;
			camera = cv::VideoCapture(cameraNumber);
			cv::destroyWindow("correctCamera");
			setCameraProperties();
		}
	}

	cv::destroyWindow("correctCamera");
	return;

}

int main()
{
	setCameraProperties();
	srand(time(0) * 30);
	background_subtractor_gpu = cv::cuda::createBackgroundSubtractorMOG2(500, 500., false);

	int roundNumber = 1;
	correctCamera();
	displayPlayerCount();
	cv::namedWindow(mainWindow);

	while (playerCount > 1)
	{
		thread nullThread = thread(nullCamera);	//We want to keep the camera on while the rounds havent started, so we don't have to wait for it to turn on when we motion track.
		startRound(roundNumber);
		playerCount--;
		endRound(roundNumber);
		nullThread.join();

	}
	return 0;
}

/**
* Have questions about my code? (Or just want to make me feel bad about how "good" it is?) Feel free to reach out!
* 
* Email:		brutemold31@Gmail.com
* Instagram:	@kennydblake
* Tiktok:		@kennydblake
* 
*/