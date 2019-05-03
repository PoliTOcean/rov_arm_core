#include <wiringPi.h>
#include <json.hpp>
#include <PolitoceanConstants.h>
#include <chrono>
#include <thread>
#include <Subscriber.h>

#define DIR1 28 // Direction GPIO Pin
#define STEP1 27 // Direction GPIO Pin
#define EN_n1 29 // Enable Pin active low

#define delay 0.001

#define CW 1	  //Clockwise Rotation
#define CCW 0 // Counterclockwise Rotation

using namespace Politocean;
using namespace Politocean::Constants;

int Status3;
int Status4;

void cleanup(){

	pinMode(DIR1, INPUT);
	pinMode(STEP1, INPUT);
	pinMode(EN_n1, INPUT);
	
}

void joystickButtCallback(const std::string& payload)
{

	if(payload == "back_up")
	{
		Status3 = 1;
	}
	else if(payload == "back_down")
	{
		Status4 = 1;
	}
	else 
	{
		Status3 = Status4 = 0;
	}
}

int main (void)
{

	wiringPiSetup() ;

	pinMode(DIR1, OUTPUT);
	pinMode(STEP1, OUTPUT);
	pinMode(EN_n1, OUTPUT);

	digitalWrite(EN_n1, HIGH);
	digitalWrite(DIR1, CCW);
	
	Status3 = 0;
	Status4 = 0;
	
	//Iscrizione al subscriber
	Subscriber Sub("127.0.0.1", Rov::ARM_ID);
	Sub.subscribeTo(Topics::ROV_ARM, &joystickButtCallback);
	Sub.connect();
	
	while (Sub.is_connected())
	{	
		while(Status3 == 1)
		{
		
			digitalWrite(EN_n1, LOW);
			digitalWrite(DIR1, CW);
		
			digitalWrite(STEP1, LOW);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			digitalWrite(STEP1, HIGH);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		
		digitalWrite(EN_n1, HIGH);
		
		while(Status4 == 1)
		{
		
			digitalWrite(EN_n1, LOW);
			digitalWrite(DIR1, CCW);
		
			digitalWrite(STEP1, LOW);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			digitalWrite(STEP1, HIGH);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		
		digitalWrite(EN_n1, HIGH);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	cleanup();
	
	return 0;
}






