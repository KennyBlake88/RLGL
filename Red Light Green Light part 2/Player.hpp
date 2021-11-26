#ifndef Player
#define Player
#include <opencv2/imgproc.hpp>
#include <iostream>
using namespace cv;
using namespace std;


extern Mat symbol;							//The player symbol -- the one in my computer not the live one.
//face declaration							//The player's face -- obviously the live one.
extern bool stillIn;						//Whether or not the player is still in. 
extern int* endOfRoundGracePeriodLocation;	//Their location after the grace period ends; it's an int* of size 2 which contains their angle. 
extern int* currentLocation;				//Their current location; it's an int* of size 2 which contains their angle. 

/*
* void fire()
* -------------
* Sends the arduino the data of what angle to set the servos at, and fires the nerf gun at the specified angles.
*
* int* angle:  The angle we want to move the servos to.
*/
void fire(int*);

/*
* int* calculateAngleNerfGunDifference()
* -------------
* Changes the angle value of where a player is from being based on the camera as a mid point, to be based on the nerf gun as a midpoint.
* 
* int* angle: The angle in the camera.
* 
* returns:
* an int* of 2 values, the x and the y, of where we want to move the camera to. 
*/
int* calculateAngleNerfGunDifference(int*);

bool watchForMovement();
void setFace();
void setSymbol(string symbolPath);
int* getLocation();
bool leftplayingArea();
void removePlayer();



#endif