#pragma config(Sensor, dgtl1,  flywheelEncoder, sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  feedSwitch,     sensorDigitalIn)
#pragma config(Motor,  port1,           ce,            tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           rb,            tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port3,           er,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           us,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           rfdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           rbdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           lfdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           lbdrive,       tmotorVex393_MC29, openLoop, reversed)
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
#define kp 6. //TO TUNE
#define ki 2. //TO TUNE
#define kd 2. //TO TUNE
#define firingErrorMargin .02 //TO TUNE //percent error allowable in flywheel velocity for firing
#define bangBangErrorMargin .05 //TO TUNE
#define integralMargin .03 //TO TUNE

#define seymoreInBtn Btn5U
#define seymoreOutBtn Btn5D
#define seymoreManualOverrideBtn Btn8U
#define punchBtn Btn5U
#define feedInBtn Btn6U
#define feedOutBtn Btn6D
#define switchLauncherModesBtn Btn8L
#define emergencyStopBtn Btn8R

bool flywheelRunning = true;
bool velocityUpdated = false;
float flywheelVelocity = 0;
float targetVelocity = 0;
int flywheelPower = 0;
int targetPower = 0;
int defaultPower = 0;
int puncherPower = 80;

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
		flywheelVelocity = (float)(SensorValue[flywheelEncoder]) / (float)(sampleTime);
		velocityUpdated = true;
	}
}

float getFlywheelVelocity() {
	velocityUpdated = false;
	return flywheelVelocity;
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

task spinUpControl() {
	while (true) {
		while (targetPower == flywheelPower) { EndTimeSlice(); }
		while (targetPower - flywheelPower > maxAcc) {
			setLauncherPower(flywheelPower + maxAcc);
			wait1Msec(100);
		}
		setLauncherPower(targetPower);
	}
}
//end set functions region

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
		while (vexRT[seymoreInBtn] == 0 && vexRT[seymoreOutBtn] == 0 && vexRT[seymoreManualOverrideBtn] == 0) { EndTimeSlice(); }
		if (vexRT[seymoreInBtn] == 1) {
			motor[seymore] = (/*SensorValue[feedSwitch] == 1 ||*/ abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity || !automaticStop ? 127 : 0);
			while (vexRT[seymoreInBtn] == 1 && (/*SensorValue[feedSwitch] == 1  ||*/ abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity || !automaticStop)) { EndTimeSlice(); }
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
	int powers[4] = {60, 80, 100, 127};

	while (true)
	{
		for (int i = 0; i < 5; i++)
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
	float velocities[5] = {0.0, 4.00, 4.31, 5.20, 5.65};
	int defaultPowers[5] = {0, 39, 44, 65, 81};

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
	float prevError;
	float error;
	float integral;

	while (true)
	{
		prevError = targetVelocity - getFlywheelVelocity();
		integral = 0;

		while (abs(targetVelocity - getFlywheelVelocity()) < bangBangErrorMargin * targetVelocity) //PID control
		{
			wait1Msec(sampleTime);
			while (!velocityUpdated) { EndTimeSlice(); }
			error = (targetVelocity - getFlywheelVelocity());

			if (abs(error) < integralMargin * targetVelocity)
			{
				integral += error;
			}

			targetPower = defaultPower + kp * error + ki * integral + kd * (error - prevError) / sampleTime;
			prevError = error;
		}

		//bang bang control
		while (abs(targetVelocity - flywheelVelocity) > bangBangErrorMargin * targetVelocity) {
			targetPower = ((targetVelocity > flywheelVelocity) ? (127) : ( 0));
			EndTimeSlice();
		}

		setLauncherPower(defaultPower);
	}
}
//end user input region

//begin task control region
void resetFlywheelVars() {
	velocityUpdated = false;
	flywheelVelocity = 0;
	targetVelocity = 0;
	flywheelPower = 0;
	targetPower = 0;
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
			startTask(spinUpControl);
			startTask(seymoreControl);
			startTask(calcVelocity);
		}
		else {
			setLauncherPower(0);
			stopTask(flywheel);
			stopTask(flywheelStabilization);
			stopTask(spinUpControl);
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
		startTask(spinUpControl);
		startTask(seymoreControl);
		startTask(calcVelocity);
	}
	else {
		startTask(puncher);
		startTask(puncherSpeeds);
	}
	startTask(launcherMode);
	startTask(feedMeControl);
}

void emergencyStop() {
	stopTask(flywheel);
	stopTask(flywheelStabilization);
	stopTask(puncher);
	stopTask(feedMeControl);
	stopTask(spinUpControl);
	stopTask(seymoreControl);
	stopTask(calcVelocity);
	stopTask(launcherMode);

	initializeTasks();
}
//end task control region

void pre_auton() { bStopTasksBetweenModes = true; }

task autonomous() {
	flywheelRunning = false;
	setLauncherPower(127);
	wait1Msec(1000);
	setLauncherPower(0);
	flywheelRunning = true;
	initializeTasks();
	targetVelocity = 4.00;
	defaultPower = 39;

	motor[feedMe] = 127;
	setDrivePower(127, 85);
	wait1Msec(3000);
	setDrivePower(0, 0);
	motor[seymore] = 127;
	while (true) { EndTimeSlice(); }
}

task usercontrol() {
	initializeTasks();

	while (true)
	{
		while (vexRT[emergencyStopBtn] == 0)
		{
			setDrivePower(sgn(vexRT[Ch2]) * vexRT[Ch2] * vexRT[Ch2] / 127, sgn(vexRT[Ch3]) * vexRT[Ch3] * vexRT[Ch3] / 127);
			EndTimeSlice();
		}

		emergencyStop();
	}
}
