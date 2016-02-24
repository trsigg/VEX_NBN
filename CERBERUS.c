#pragma config(Sensor, dgtl1,  flywheelEncoder, sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  leftEncoder,    sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  rightEncoder,   sensorQuadEncoder)
#pragma config(Sensor, dgtl7,  flywheelSwitch, sensorDigitalIn)
#pragma config(Sensor, dgtl8,  solenoidOne,    sensorDigitalOut)
#pragma config(Sensor, dgtl9,  solenoidTwo,    sensorDigitalOut)
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
#define kp 20. //TO TUNE
#define ki 0.5 //TO TUNE
#define kd 6. //TO TUNE
#define firingErrorMargin .05 //TO TUNE //percent error allowable in flywheel velocity for firing
#define bangBangErrorMargin .07 //TO TUNE
#define integralMargin .02 //TO TUNE

#define fireBtn Btn5U
#define seymoreOutBtn Btn5D
#define seymoreManualOverrideBtn Btn8U
#define punchBtn Btn5U
#define feedInBtn Btn6U
#define feedOutBtn Btn6D
#define switchLauncherModesBtn Btn8L
#define liftBtn Btn8R

bool flywheelRunning = true;
bool velocityUpdated = false;
float flywheelVelocity = 0;
float targetVelocity = 0;
int flywheelPower = 0;
int defaultPower = 0;
int puncherPower = 80;
bool driveStraightRunning = false;
int clicks, rightDirection, leftDirection, drivePower, delayAtEnd, timeout;

float debug;
float errorDebug;
int bangBangCount = 0;
int bbpercentup;
float bangBangPerSec = 0;

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
	flywheelPower = limit(power, 0, 127) * (flywheelRunning ? 1 : -1);
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

task fire() {
	//int shotsFired = 0;
	motor[feedMe] = 127;
	motor[seymore] = SensorValue[flywheelSwitch] == 1 || abs(targetVelocity - flywheelVelocity) > firingErrorMargin * targetVelocity ? 127 : 0;
	while (/*shotsFired < shotsToFire*/true) {
		while (SensorValue[flywheelSwitch] == 1 || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity) { EndTimeSlice(); }
		motor[seymore] = 0;
		while(!(SensorValue[flywheelSwitch] == 1 || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity)) { EndTimeSlice(); }
		motor[seymore] = 127;
	}
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
	bool automaticStop = false;

	while (true) {
		while (vexRT[fireBtn] == 0 && vexRT[seymoreOutBtn] == 0 && vexRT[seymoreManualOverrideBtn] == 0) { EndTimeSlice(); }
		if (vexRT[fireBtn] == 1) {
			motor[seymore] = (SensorValue[flywheelSwitch] == 1 || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity || !automaticStop ? 127 : 0);
			while (vexRT[fireBtn] == 1 && (SensorValue[flywheelSwitch] == 1  || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity || !automaticStop)) { EndTimeSlice(); }
		}
		else if (vexRT[seymoreOutBtn] == 1) {
			motor[seymore] = -127;
			while (vexRT[seymoreOutBtn] == 1) { EndTimeSlice(); }
		}
		else {
			automaticStop = !automaticStop;
			while (vexRT[seymoreManualOverrideBtn] == 1) { EndTimeSlice(); }
		}
		motor[seymore] = 0;
	}
}

task puncher() {
	while (true)
	{
		while (vexRT[punchBtn] == 0) { EndTimeSlice(); }
		setLauncherPower(puncherPower);
		while (vexRT[punchBtn] == 1) { EndTimeSlice(); }
		setLauncherPower(0);
	}
}

task puncherSpeeds() {
	TVexJoysticks buttons[4] = {Btn7U, Btn7R, Btn7D, Btn7L}; //creating a pseudo-hash associating buttons with motor powers
	int powers[4] = {80, 85, 90, 110};

	while (true)
	{
		for (int i = 0; i < 4; i++)
		{
			if (vexRT[buttons[i]] == 1)
			{
				puncherPower = powers[i];
			}
			EndTimeSlice();
		}
	}
}

task flywheel() {
	TVexJoysticks buttons[5] = {Btn8D, Btn7U, Btn7R, Btn7D, Btn7L}; //creating a pseudo-hash associating buttons with velocities and default motor powers
	float velocities[5] = {0.0, 6.21, 7.64, 1.01, 1.01};
	int defaultPowers[5] = {0, 49, 87, 82, 127};

	while (true)
	{
		for (int i = 0; i < 5; i++)
		{
			if (vexRT[buttons[i]] == 1)
			{
				targetVelocity = velocities[i];
				defaultPower = defaultPowers[i];
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

	while (true)
	{
		prevError = targetVelocity - flywheelVelocity;
		integral = 0;

		while (abs(targetVelocity - flywheelVelocity) < bangBangErrorMargin * flywheelVelocity && targetVelocity > 0) //PID control
		{
			wait1Msec(sampleTime);
			while (!velocityUpdated) { EndTimeSlice(); }
			error = (targetVelocity - flywheelVelocity);
			errorDebug = error;
			velocityUpdated = false;

			if (abs(error) < integralMargin * flywheelVelocity)
			{
				integral += (prevError + error) * sampleTime / 2;
			}

			setLauncherPower(defaultPower + kp * error + ki * integral + kd * (error - prevError) / sampleTime);
			prevError = error;
			debug = flywheelPower - defaultPower;
		}

		//bang bang control
		bangBangCount += 1;
		numbbup += (targetVelocity > flywheelVelocity ? 1 : 0);
		bbpercentup = 100 * numbbup / bangBangCount;
		bangBangPerSec = (float)((float)bangBangCount * 1000) / (float)(time1(T1) + .1);
		while (abs(targetVelocity - flywheelVelocity) > bangBangErrorMargin * flywheelVelocity && targetVelocity > 0) {
			setLauncherPower((targetVelocity > flywheelVelocity) ? (100) : ( 0));
			EndTimeSlice();
		}

		setLauncherPower(defaultPower);
		while (targetVelocity == 0) { EndTimeSlice(); } //pauses while
	}
}

task lift() {
	SensorValue[solenoidOne] = 0;
	SensorValue[solenoidTwo] = 0;

	while (true) {
		while (vexRT[liftBtn] == 0) { EndTimeSlice(); }
		SensorValue[solenoidOne] = 1 - SensorValue[solenoidOne];
		while (vexRT[liftBtn] == 1) { EndTimeSlice(); }
		while (vexRT[liftBtn] == 0) { EndTimeSlice(); }
		SensorValue[solenoidTwo] = 1 - SensorValue[solenoidTwo];
		while (vexRT[liftBtn] == 1) { EndTimeSlice(); }
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

task launcherMode() {
	while (true) {
		while (vexRT[switchLauncherModesBtn] == 0) { EndTimeSlice(); }
		flywheelRunning = !flywheelRunning;
		if (flywheelRunning) {
			resetFlywheelVars();
			stopTask(puncher);
			startTask(flywheel);
			startTask(flywheelStabilization);
			startTask(seymoreControl);
			startTask(calcVelocity);
		}
		else {
			setLauncherPower(0);
			stopTask(flywheel);
			stopTask(flywheelStabilization);
			stopTask(seymoreControl);
			stopTask(calcVelocity);
			startTask(puncher);
			startTask(puncherSpeeds);
		}
		while (vexRT[switchLauncherModesBtn] == 1) { EndTimeSlice(); }
	}
}

void initializeTasks() {
	if (flywheelRunning) {
		resetFlywheelVars();
		startTask(flywheel);
		startTask(flywheelStabilization);
		startTask(seymoreControl);
		startTask(calcVelocity);
	}
	else {
		startTask(puncher);
		startTask(puncherSpeeds);
	}
	startTask(launcherMode);
	startTask(feedMeControl);
	startTask(lift);
}

void emergencyStop() {
	stopTask(flywheel);
	stopTask(flywheelStabilization);
	stopTask(puncher);
	stopTask(feedMeControl);
	stopTask(seymoreControl);
	stopTask(calcVelocity);
	stopTask(launcherMode);

	initializeTasks();
}
//end task control region

void pre_auton() { bStopTasksBetweenModes = true; }

task autonomous() {
	//fire puncher preload
	flywheelRunning = false;
	setLauncherPower(127);
	wait1Msec(1000);
	setLauncherPower(0);
	wait1Msec(2000);
	//start flywheel
	flywheelRunning = true;
	initializeTasks();
	targetVelocity = 3.61;
	defaultPower = 45;

	driveStraight(20, 1, -1, 50, 125); //turn to face first stack
	motor[feedMe] = 127;
	wait1Msec(125); //prevent breaker overload
	driveStraight(800, 1, 1, 100, 250); //pick up first stack
	driveStraight(8, -1, 1, 50, 250); //turn toward net
	//drive toward net
	driveStraight(900, 1, 1, 100, 500, true);
	while (driveStraightRunning) { EndTimeSlice(); }
	//fire first stack
	startTask(fire);
	//motor[seymore] = 127;
	wait1Msec(4000);
	stopTask(fire);
	motor[seymore] = 0;
	//change to first range
	//targetVelocity = 4.11;
	//defaultPower = 50;
	//drive over second stack to bar
	driveStraight(1100, 1, 1, 100, 500);
	//aim for second stack
	driveStraight(400, -1, -1, 100, 0, true);
	while (driveStraightRunning) { EndTimeSlice(); }
	//fire second stack
	//motor[seymore] = 127;
	startTask(fire);

	while (true){ EndTimeSlice(); }
}

task usercontrol() {
	stopTask(fire);
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
