#pragma config(Sensor, dgtl1,  Flywheel,       sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  LEFT,           sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  RIGHT,          sensorQuadEncoder)
#pragma config(Sensor, dgtl7,  IntakeFeed,     sensorTouch)
#pragma config(Sensor, dgtl8,  FeedLaunch,     sensorTouch)
#pragma config(Sensor, dgtl9,  flywheelSwitch, sensorDigitalIn)
#pragma config(Motor,  port1,           Fly1,          tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           Fly2,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           Fly3,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           Fly4,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           FRight,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           BRight,        tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port7,           FLeft,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           BLeft,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           intkae,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          feed,          tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"	//Main competition background code...do not modify!

float Kp = 3;	//TO TUNE
int Error = 0;
float Ki  = 0.001;	//TO TUNE
float Integral = 0;
float Kd = 1.7;	//TO TUNE
int DeltaE = 0;
int power = 0;
float Flyspeed = 0;
int TargetSpeed = 0;
int setpower = 0;
int TargetSpeeds[5] = {0, 215, 300, 183, 191};	//ORIGINAL: {0, 205, 300, 183, 191}
//  TargetSpeeds[5] = {Off, Skillz, Long, 1st, 2nd}
int setpowers[5] = {0, 60, 96, 65, 68};
TVexJoysticks buttons[5] = {Btn8D, Btn7U, Btn7R, Btn7D, Btn7L};
int PrevError;
int Flypower = 0;
int ToggleFeed = 1;
int ToggleIntkae = 1;
int n;

/////////////////////////////////////////////////////////////////////////////////////////
//
//                          Pre-Autonomous Functions
//
// You may want to perform some actions before the competition starts. Do them in the
// following function.
//
/////////////////////////////////////////////////////////////////////////////////////////

void pre_auton()
{
  // Set bStopTasksBetweenModes to false if you want to keep user created tasks running between
  // Autonomous and Tele-Op modes. You will need to manage all user created tasks if set to false.
  bStopTasksBetweenModes = true;

	// All activities that occur before the competition starts
	// Example: clearing encoders, setting servo positions, ...
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//                                 Autonomous Task
//
// This task is used to control your robot during the autonomous phase of a VEX Competition.
// You must modify the code to add your own robot specific commands here.
//
/////////////////////////////////////////////////////////////////////////////////////////

float P, I, D;

task motorcontrol()
{
	SensorValue[Flywheel] = 0;
	while(1)
	{
		Flyspeed = abs(SensorValue[Flywheel]);
		Error = TargetSpeed - Flyspeed;
		DeltaE = Error - PrevError;
		Integral += (Error + PrevError)/2;
		power = setpower + Kp*Error + Ki*Integral + Kd*DeltaE;

		P = Kp*Error;
		I = Ki*Integral;
		D = Kd*DeltaE;

		SensorValue[Flywheel] = 0;
		PrevError = Error;
		wait1Msec(25);
	}
}

task autonomous()
{
	TargetSpeed = 950;
	setpower = 104;
	startTask(motorcontrol);
	motor[Fly1] = power;
	motor[Fly2] = power;
	motor[Fly3] = power;
	motor[Fly4] = power;
	wait1Msec(1000);
	clearTimer(T1);
	while(true)
	{
		if(abs(Error) < 60)
		{
			motor[intkae] = 127;
			motor[feed] = 127;
		}
		else
		{
			motor[intkae] = 0;
			motor[feed] = 0;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//                                 User Control Task
//
// This task is used to control your robot during the user control phase of a VEX Competition.
// You must modify the code to add your own robot specific commands here.
//
/////////////////////////////////////////////////////////////////////////////////////////

//task FeedIntake()
//{
//	while(1)
//	{
//		if(SensorValue[FeedLaunch] == 1)
//		{
//			ToggleIntkae = 1;
//			ToggleFeed = 1;
//		}
//		else
//		{
//			if(SensorValue[IntakeFeed] == 1)
//			{
//				ToggleFeed = 0;
//				ToggleIntkae = 1;
//			}
//			if(SensorValue[Feed2] == 1)
//			{
//				ToggleFeed = 1;
//				ToggleIntkae = 0;
//			}
//		}
//	}
//}

task usercontrol()
{
	SensorValue[LEFT] = 0;
	SensorValue[RIGHT] = 0;
	startTask(motorcontrol);
	//startTask(FeedIntake);
	while(1)
	{
		Flypower = (power > 0 ? power : 0 );
		motor[intkae] = (ToggleIntkae == 1 ? (vexRT[Btn6U]*127 + vexRT[Btn6D]*-127) : 127);
		motor[feed] = (ToggleFeed == 1 ? (vexRT[Btn5U]*127 + vexRT[Btn5D]*-127) : 127);
		motor[Fly1] = Flypower;
		motor[Fly2] = Flypower;
		motor[Fly3] = Flypower;
		motor[Fly4] = Flypower;
		motor[BLeft] = vexRT[Ch3];
		motor[FLeft] = vexRT[Ch3];
		motor[BRight] = vexRT[Ch2];
		motor[FRight] = vexRT[Ch2];
		for (int i = 0; i < 5; i++)
		{
			n = (vexRT[buttons[i]] == 1 ? i : n);
		}
		if(n == 0)
		{
			Integral = 0;
		}
		TargetSpeed = TargetSpeeds[n];
		setpower = setpowers[n];
		Integral = vexRT[Btn8D] == 1 ? 0 : Integral;
	}
}
