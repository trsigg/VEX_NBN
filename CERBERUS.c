#pragma config(Sensor, dgtl1,  flywheelEncoder1, sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  leftEncoder,    sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  rightEncoder,   sensorQuadEncoder)
#pragma config(Sensor, dgtl7,  flywheelSwitch, sensorDigitalIn)
#pragma config(Sensor, dgtl8,  flywheelEncoder2, sensorQuadEncoder)
#pragma config(Motor,  port1,           ce,            tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           rb,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           er,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           us,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           rfdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           rbdrive,       tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port7,           lfdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           lbdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           feedMe,        tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          seymore,       tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)
//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)
#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

#define maxAcc 30 //the maximum amount a motor's power value can be safely changed in .1 seconds
#define sampleTime 50. //number of milliseconds between sampling the flywheel velocity and control adjustments in flywheel task
//PID constants
#define kp 13.0 //TO TUNE
#define ki 0.5 //TO TUNE
#define kd 5.0 //TO TUNE
#define firingErrorMargin .04 //TO TUNE //percent error allowable in flywheel velocity for firing
#define bangBangErrorMargin .03 //TO TUNE
#define integralMargin .04 //TO TUNE

#define fireBtn Btn5U
#define seymoreOutBtn Btn5D
#define seymoreManualOverrideBtn Btn8U
#define feedInBtn Btn6U
#define feedOutBtn Btn6D

bool velocityUpdated = false;
float flywheelVelocity = 0;
float targetVelocity = 0;
int flywheelPower = 0;
int defaultPower = 0;
bool driveStraightRunning = false;
int clicks, rightDirection, leftDirection, drivePower, delayAtEnd, timeout; //driveStraight
int ballsToFire, fireTimeout;
bool automaticStop = false;

float debug;
float errorDebug;
int bangBangCount = 0;
bool incorrectPID = false;
int bbpercentup;
float bangBangPerSec = 0;
float avgError = 0;
int seymoreState = 0;

//begin helper functions region
int limit(int input, int min, int max) {
	if (abs(input) <= max && abs(input) >= min) {
		return input;
	}
	else {
		return ((abs(input) > max) ? (max * sgn(input)) : (min * sgn(input)));
	}
}

task calcVelocity() {
	while (true) {
		SensorValue[flywheelEncoder1] = 0;
		SensorValue[flywheelEncoder2] = 0;
		wait1Msec(sampleTime);
		flywheelVelocity = abs((float)(SensorValue[flywheelEncoder1] + SensorValue[flywheelEncoder2])) / (float)(2 * sampleTime);
		velocityUpdated = true;
	}
}
//end helper functions region

//set functions region
void setDrivePower(int right, int left) {
	motor[rfdrive] = right;
	motor[rbdrive] = right;
	motor[lfdrive] = left;
	motor[lbdrive] = left;
}

void setLauncherPower(int power) {
	flywheelPower = limit(power, 0, 127);
	motor[ce] = flywheelPower;
	motor[rb] = flywheelPower;
	motor[er] = flywheelPower;
	motor[us] = flywheelPower;
}
//end set functions region

//autonomous region
task driveStraightTask()
{
	driveStraightRunning = true;

	int coeff = 5;
  int totalClicks = 0;
  int slavePower = drivePower;
  int error = 0;

  SensorValue[leftEncoder] = 0;
  SensorValue[rightEncoder] = 0;
  clearTimer(T1);

  while(abs(totalClicks) < clicks && time1(T1) < timeout)
  {
    setDrivePower(drivePower * leftDirection, slavePower * rightDirection);

    error = -SensorValue[leftEncoder] - SensorValue[rightEncoder];

    slavePower += error / coeff;

    totalClicks += SensorValue[leftEncoder];
    SensorValue[leftEncoder] = 0;
    SensorValue[rightEncoder] = 0;

    wait1Msec(100);
  }
  setDrivePower(0, 0);
  wait1Msec(delayAtEnd);
  driveStraightRunning = false;
}

void driveStraight(int _clicks_, int _leftDirection_, int _rightDirection_, int _drivePower_, int _delayAtEnd_=0, bool startAsTask=false, int _timeout_=2500) {
	clicks = _clicks_;
	rightDirection = _rightDirection_;
	leftDirection = _leftDirection_;
	drivePower = _drivePower_;
	delayAtEnd = _delayAtEnd_;
	timeout = _timeout_;
	if (startAsTask) {
		startTask(driveStraightTask);
	}
	else { //runs as function
		int coeff = 5;
	  int totalClicks = 0;
	  int slavePower = drivePower;
	  int error = 0;

	  SensorValue[leftEncoder] = 0;
	  SensorValue[rightEncoder] = 0;
	  clearTimer(T1);

	  while(abs(totalClicks) < clicks  && time1(T1) < timeout)
	  {
	    setDrivePower(drivePower * leftDirection, slavePower * rightDirection);

	    error = -SensorValue[leftEncoder] - SensorValue[rightEncoder];

	    slavePower += error / coeff;

	    totalClicks += SensorValue[leftEncoder];
	    SensorValue[leftEncoder] = 0;
	    SensorValue[rightEncoder] = 0;

	    wait1Msec(100);
	  }
	  setDrivePower(0, 0);
	  wait1Msec(delayAtEnd);
	}
}

task fireTask() {
	int shotsFired = 0;
	clearTimer(T2);
	motor[feedMe] = 127;
	motor[seymore] = 127;
	while (shotsFired < ballsToFire && time1(T2) < fireTimeout) {
		while ((SensorValue[flywheelSwitch] == 1 || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity) && time1(T2) < fireTimeout) { EndTimeSlice(); }
		motor[seymore] = 0;
		shotsFired++;
		wait1Msec(1000);
		//while(!(SensorValue[flywheelSwitch] == 1 || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity) && time1(T2) < fireTimeout) { EndTimeSlice(); }
		motor[seymore] = 127;
	}
	motor[seymore] = 0;
}

void fire(int _ballsToFire_, int _fireTimeout_=4000) {
		ballsToFire = _ballsToFire_;
		fireTimeout = _fireTimeout_;
		startTask(fireTask);
}
//end autonomous region

//begin user input region
task feedMeControl() {
	while (true) {
		while (vexRT[feedInBtn] == 0 && vexRT[feedOutBtn] == 0) { EndTimeSlice(); }
		if (vexRT[feedInBtn] == 1) {
			motor[feedMe] = 127;
			while (vexRT[feedInBtn] == 1) { EndTimeSlice(); }
		}
		else {
			motor[feedMe] = -127;
			while (vexRT[feedOutBtn] == 1) { EndTimeSlice(); }
		}
		motor[feedMe] = 0;
	}
}

task seymoreControl() {
	//bool automaticStop = true;

	while (true) {
		seymoreState = 0;
		while (vexRT[fireBtn] == 0 && vexRT[seymoreOutBtn] == 0 && vexRT[seymoreManualOverrideBtn] == 0) { EndTimeSlice(); }
		if (vexRT[fireBtn] == 1) {
			seymoreState = 1;
			motor[seymore] = 127/*(SensorValue[flywheelSwitch] == 1 || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity || !automaticStop ? 127 : 0)*/;
			while (vexRT[fireBtn] == 1 && (SensorValue[flywheelSwitch] == 1  || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity || !automaticStop)) { EndTimeSlice(); }
		}
		else if (vexRT[seymoreOutBtn] == 1) {
			seymoreState = 2;
			motor[seymore] = -127;
			while (vexRT[seymoreOutBtn] == 1) { EndTimeSlice(); }
		}
		else {
			seymoreState = 3;
			automaticStop = !automaticStop;
			while (vexRT[seymoreManualOverrideBtn] == 1) { EndTimeSlice(); }
		}
		motor[seymore] = 0;
	}
}

task flywheel() {
	TVexJoysticks buttons[5] = {Btn8D, Btn7U, Btn7R, Btn7D, Btn7L}; //creating a pseudo-hash associating buttons with velocities and default motor powers
	float velocities[5] = {0.0, 7.00, 7.74, 8.79, 9.54};
	int defaultPowers[5] = {0, 40, 50, 67, 80};

	while (true)
	{
		for (int i = 0; i < 5; i++)
		{
			if (vexRT[buttons[i]] == 1)
			{
				targetVelocity = velocities[i];
				defaultPower = defaultPowers[i];
				if (i == 4) {
					automaticStop = true;
				}
				else {
					automaticStop = false;
				}
			}
			EndTimeSlice();
		}
	}
}

task flywheelStabilization() { //modulates motor powers to maintain constant flywheel velocity
	clearTimer(T1);
	float prevError;
	float error;
	float integral;
	int numbbup = 0; //debug
	float totalError = 0;
  int numloops = 0;

	while (true)
	{
		prevError = targetVelocity - flywheelVelocity;
		integral = 0;

		while (abs(targetVelocity - flywheelVelocity) < bangBangErrorMargin * flywheelVelocity && targetVelocity > 0/*true*/) //PID control
		{
			wait1Msec(sampleTime);
			while (!velocityUpdated) { EndTimeSlice(); }
			error = (targetVelocity - flywheelVelocity);
			errorDebug = error;
			totalError += abs(error);
    	numloops += 1;
    	avgError = totalError / numloops;

			velocityUpdated = false;

			if (abs(error) < integralMargin * flywheelVelocity)
			{
				integral += (prevError + error) * sampleTime / 2;
			}

			setLauncherPower(defaultPower + kp * error + ki * integral + kd * (error - prevError) / sampleTime);
			prevError = error;
			debug = flywheelPower - defaultPower;
			incorrectPID = sgn(debug) != sgn(error);
		}

		//bang bang control
		bangBangCount += 1;
		numbbup += (targetVelocity > flywheelVelocity ? 1 : 0);
		bbpercentup = 100 * numbbup / bangBangCount;
		bangBangPerSec = (float)((float)bangBangCount * 1000) / (float)(time1(T1) + .1);
		while (abs(targetVelocity - flywheelVelocity) > bangBangErrorMargin * flywheelVelocity  * 0.75 && targetVelocity > 0) {
			setLauncherPower((targetVelocity > flywheelVelocity) ? (127) : ( 0));
			EndTimeSlice();
		}

		setLauncherPower(defaultPower);
		while (targetVelocity == 0) { EndTimeSlice(); } //pauses while
	}
}
//end user input region

//begin task control region
void resetFlywheelVars() {
	velocityUpdated = false;
	flywheelVelocity = 0;
	targetVelocity = 0;
	flywheelPower = 0;
	defaultPower = 0;
}

void initializeTasks() {
	resetFlywheelVars();
	startTask(flywheel);
	startTask(flywheelStabilization);
	startTask(seymoreControl);
	startTask(calcVelocity);
	startTask(feedMeControl);
}

void emergencyStop() {
	stopTask(flywheel);
	stopTask(flywheelStabilization);
	stopTask(feedMeControl);
	stopTask(seymoreControl);
	stopTask(calcVelocity);

	initializeTasks();
}
//end task control region

void pre_auton() { bStopTasksBetweenModes = true; }

task autonomous() {
	//start flywheel
	initializeTasks();
	stopTask(feedMeControl);
	stopTask(seymoreControl);
	targetVelocity = 9.54;
	defaultPower = 80;

	motor[seymore] = 127;//fire(4, 6000); //fire four initial preloads
	motor[feedMe] = 127;
	wait1Msec(3500);
	motor[seymore] = 0;
	//set to first range
	targetVelocity = 7.00;
	defaultPower = 40;

	driveStraight(15, 1, -1, 50, 125); //turn to face first stack
	wait1Msec(125); //prevent breaker overload
	driveStraight(600, 1, 1, 100, 250); //pick up first stack
	driveStraight(5, -1, 1, 30, 250); //turn toward net
	//drive toward net
	driveStraight(800, 1, 1, 100, 500, true);
	while (driveStraightRunning) { EndTimeSlice(); }
	//fire first stack
	motor[seymore] = 127; //fire(3);
	wait1Msec(4000);
	motor[seymore] = 0;
	//change to first range
	//targetVelocity = 4.11;
	//defaultPower = 50;
	//drive over second stack to bar
	driveStraight(1000, 1, 1, 100, 500);
	//aim for second stack
	driveStraight(400, -1, -1, 100, 0, true);
	while (driveStraightRunning) { EndTimeSlice(); }
	//fire second stack
	//motor[seymore] = 127;
	motor[seymore] = 127;//fire(15, 15000);

	while (true){ EndTimeSlice(); }
}

task usercontrol() {
	stopTask(fireTask);
	initializeTasks();

	while (true)
	{
		while (true)
		{
			setDrivePower(sgn(vexRT[Ch2]) * vexRT[Ch2] * vexRT[Ch2] / 127, sgn(vexRT[Ch3]) * vexRT[Ch3] * vexRT[Ch3] / 127);
			EndTimeSlice();
		}

		emergencyStop(); //reassign emstop button
	}
}
