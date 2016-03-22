#pragma config(Sensor, in1,    gyro,           sensorGyro)
#pragma config(Sensor, in2,    ternaryPot,     sensorPotentiometer)
#pragma config(Sensor, in3,    binaryPot,      sensorPotentiometer)
#pragma config(Sensor, dgtl1,  flywheelEncoder, sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  leftEncoder,    sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  rightEncoder,   sensorQuadEncoder)
#pragma config(Sensor, dgtl7,  flywheelSwitch, sensorDigitalIn)
#pragma config(Sensor, dgtl8,  LED,            sensorLEDtoVCC)
#pragma config(Motor,  port1,           seymore,       tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           lfdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           lbdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           ce,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           rb,            tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           er,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           us,            tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           rfdrive,       tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           rbdrive,       tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          feedMe,        tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)
//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)
#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

#define sampleTime 50. //number of milliseconds between sampling the flywheel velocity and control adjustments in flywheel task
//PID constants
#define kp 40.0
#define ki 0.2
#define kd 30.
//error ranges
#define firingErrorMargin 0.01
#define bangBangErrorMargin 0.03
#define integralMargin 1.

#define fireBtn Btn5U
#define seymoreOutBtn Btn5D
#define toggleAutoStopBtn Btn8U
#define feedInBtn Btn6U
#define feedOutBtn Btn6D

#define flywheelTimer T1
#define firingTimer T2
#define driveTimer T3

bool automaticStop = true; //seymoreControl
//driveStraight
bool driveStraightRunning = false;
int clicks, rightDirection, leftDirection, drivePower, delayAtEnd, timeout;
//fire
int ballsToFire, shotsFired, fireTimeout;
bool fireRunning;
//turn
float degreesToTurn;
int direction, maxTurnSpeed, waitAtEnd;
//flywheel variables
bool velocityUpdated = false;
bool adjustmentPeriod = false;
float flywheelVelocity = 0;
float targetVelocity = 0;
int flywheelPower = 0;
float integral = 0;

//debugging
float errorDebug;
int bangBangCount = 0;
int bbpercentup;
float bangBangPerSec = 0;
float avgError = 0;
int seymoreState = 0;
float P, I, D;

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
		SensorValue[flywheelEncoder] = 0;
		wait1Msec(sampleTime);
		flywheelVelocity = abs((float)(SensorValue[flywheelEncoder])) / (float)(sampleTime);
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

void setFlywheelRange(int range) {
	float velocities[5] = { 0.0, 4.29, 7.74, 8.79, 9.54 };

	integral = 0;
	targetVelocity = velocities[limit(range, 0, 4)];
	adjustmentPeriod = range != 0;
}
//end set functions region

//autonomous region
task turnTask() {
	//clear the gyro
	SensorType[gyro] = sensorNone;
	wait1Msec(500);
	SensorType[gyro] = sensorGyro;
	wait1Msec(1000);

	//turn
	setDrivePower(direction * maxTurnSpeed, -direction * maxTurnSpeed);
	while (abs(SensorValue[gyro]) < degreesToTurn * 10) {}

	//brake
	setDrivePower(-direction * 10, direction * 10);
	int brakeDelay = limit(250, 0, waitAtEnd);
	wait1Msec(brakeDelay);
	setDrivePower(0, 0);

	if (waitAtEnd > 250) wait1Msec(waitAtEnd - 250); //wait at end
}

void turn(float _degreesToTurn_, int _direction_, int _maxTurnSpeed_ = 60, int _waitAtEnd_ = 250, bool runAsTask = false) { //direction: 1 for right turn, -1 for left turn
	degreesToTurn = _degreesToTurn_;
	direction = _direction_;
	maxTurnSpeed = _maxTurnSpeed_;
	waitAtEnd = _waitAtEnd_;

	if (runAsTask) {
		startTask(turnTask);
	}
	else {
		//clear the gyro
		SensorType[gyro] = sensorNone;
		wait1Msec(500);
		SensorType[gyro] = sensorGyro;
		wait1Msec(1000);

		//turn
		setDrivePower(direction * maxTurnSpeed, -direction * maxTurnSpeed);
		while (abs(SensorValue[gyro]) < degreesToTurn * 10) {}

		//brake
		setDrivePower(-direction * 10, direction * 10);
		int brakeDelay = limit(250, 0, waitAtEnd);
		wait1Msec(brakeDelay);
		setDrivePower(0, 0);

		if (waitAtEnd > 250) wait1Msec(waitAtEnd - 250); //wait at end
	}
}

task driveStraightTask()
{
	driveStraightRunning = true;

	int coeff = 5;
	int totalClicks = 0;
	int slavePower = drivePower;
	int error = 0;

	SensorValue[leftEncoder] = 0;
	SensorValue[rightEncoder] = 0;
	clearTimer(driveTimer);

	while (abs(totalClicks) < clicks && time1(driveTimer) < timeout)
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

void driveStraight(int _clicks_, int _leftDirection_, int _rightDirection_, int _drivePower_, int _delayAtEnd_ = 0, bool startAsTask = false, int _timeout_ = 2500) {
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
		clearTimer(driveTimer);

		while (abs(totalClicks) < clicks  && time1(driveTimer) < timeout)
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
	clearTimer(firingTimer);
	motor[feedMe] = 127;
	motor[seymore] = 127;
	while (shotsFired < ballsToFire && time1(firingTimer) < fireTimeout) {
		while ((SensorValue[flywheelSwitch] == 1 || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity) && time1(firingTimer) < fireTimeout) { EndTimeSlice(); }
		motor[seymore] = 0;
		shotsFired++;
		wait1Msec(1000);
		//while(!(SensorValue[flywheelSwitch] == 1 || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity) && time1(firingTimer) < fireTimeout) { EndTimeSlice(); }
		motor[seymore] = 127;
	}
	motor[seymore] = 0;
}

void fire(int _ballsToFire_, int _fireTimeout_ = 4000) {
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
	while (true) {
		seymoreState = 0;
		while (vexRT[fireBtn] == 0 && vexRT[seymoreOutBtn] == 0 && vexRT[toggleAutoStopBtn] == 0) { EndTimeSlice(); }
		if (true) {
			seymoreState = 1;
			while (!(SensorValue[flywheelSwitch] == 1 || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity || !automaticStop) && vexRT[fireBtn] == 1) { EndTimeSlice(); }
			motor[seymore] = 127;
			while ((SensorValue[flywheelSwitch] == 1 || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity || !automaticStop) && vexRT[fireBtn] == 1) {
				if (SensorValue[flywheelSwitch] == 0) adjustmentPeriod = false;
				EndTimeSlice();
			}
		}
		else if (vexRT[seymoreOutBtn] == 1) {
			seymoreState = 2;
			motor[seymore] = -127;
			while (vexRT[seymoreOutBtn] == 1) { EndTimeSlice(); }
		}
		else {
			seymoreState = 3;
			automaticStop = !automaticStop;
			while (vexRT[toggleAutoStopBtn] == 1) { EndTimeSlice(); }
		}
		motor[seymore] = 0;
	}
}

task flywheel() {
	TVexJoysticks buttons[5] = { Btn8D, Btn7U, Btn7R, Btn7D, Btn7L }; //creating a pseudo-hash associating buttons with velocities and default motor powers

	while (true)
	{
		for (int i = 0; i < 5; i++)
		{
			if (vexRT[buttons[i]] == 1)
			{
				setFlywheelRange(i);

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
	clearTimer(flywheelTimer);
	float prevError;
	float error;
	//debug
	int numbbup = 0;
	float totalError = 0;
	int numloops = 0;

	while (true)
	{
		while (abs(targetVelocity - flywheelVelocity) < bangBangErrorMargin * flywheelVelocity && targetVelocity > 0 || adjustmentPeriod) //PID control
		{
			while (!velocityUpdated) { EndTimeSlice(); }
			error = (targetVelocity - flywheelVelocity);
			velocityUpdated = false;

			if (abs(error) < integralMargin * flywheelVelocity)
			{
				integral += (prevError + error) * sampleTime / 2;
			}

			//debug
			errorDebug = error;
			totalError += abs(error);
			numloops += 1;
			avgError = totalError / numloops;
			P = kp * error;
			I = ki * integral;
			D = kd * (error - prevError) / sampleTime;

			setLauncherPower(kp * error + ki * integral + kd * (error - prevError) / sampleTime);
			prevError = error;
		}

		//bang bang control
		bangBangCount += 1;
		numbbup += (targetVelocity > flywheelVelocity ? 1 : 0);
		bbpercentup = 100 * numbbup / bangBangCount;
		bangBangPerSec = (float)((float)bangBangCount * 1000) / (float)(time1(flywheelTimer) + .1);
		while (abs(targetVelocity - flywheelVelocity) > bangBangErrorMargin * flywheelVelocity  * 0.5 && targetVelocity > 0) {
			setLauncherPower((targetVelocity > flywheelVelocity) ? (127) : (0));
			EndTimeSlice();
		}

		if (targetVelocity == 0) {
			setLauncherPower(0);
			while (targetVelocity == 0) { EndTimeSlice(); }
		}
	}
}
//end user input region

//begin task control region
void resetFlywheelVars() {
	velocityUpdated = false;
	flywheelVelocity = 0;
	targetVelocity = 0;
	flywheelPower = 0;
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

	motor[seymore] = 127;//fire(4, 6000); //fire four initial preloads
	motor[feedMe] = 127;
	wait1Msec(3500);
	motor[seymore] = 0;
	//set to first range
	targetVelocity = 7.00;

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

	while (true) { EndTimeSlice(); }
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
