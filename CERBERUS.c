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
#define fireErrorMargin .05 //percent error allowable in flywheel velocity for firing
#define sampleTime 500 //number of milliseconds between sampling the flywheel velocity and control adjustments in flywheel task
//PID constants
#define kp 1 //TO TUNE
#define ki 1 //TO TUNE
#define kd 1 //TO TUNE
#define firingErrorMargin .05 //TO TUNE
#define bangBangErrorMargin .1 //TO TUNE
#define integralMargin .075 //TO TUNE

#define seymoreInBtn Btn5U
#define seymoreOutBtn Btn5D
#define seymoreManualOverrideBtn Btn8U
#define punchBtn Btn5U
#define feedInBtn Btn6U
#define feedOutBtn Btn6D
#define switchLauncherModesBtn Btn8L
#define emergencyStopBtn Btn8R

bool flywheelRunning = false;
bool velocityUpdated = false;
bool continuousFire = false;
float flywheelVelocity = 0;
float targetVelocity = 0;
int flywheelPower = 0;
int targetPower = 0;
int defaultPower = 0;

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
	bool automaticStop = true;

	while (true) {
		while (vexRT[seymoreInBtn] == 0 && vexRT[seymoreOutBtn] == 0 && vexRT[seymoreManualOverrideBtn] == 0) { EndTimeSlice(); }
		if (vexRT[seymoreInBtn] == 1) {
			motor[seymore] = (SensorValue[feedSwitch] == 1 || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity || !automaticStop ? 127 : 0);
			while (vexRT[seymoreInBtn] == 1 && (SensorValue[feedSwitch] == 1  || abs(targetVelocity - flywheelVelocity) < firingErrorMargin * targetVelocity || !automaticStop)) { EndTimeSlice(); }
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
		setLauncherPower(127);
		while (vexRT[punchBtn] == 1) { EndTimeSlice(); }
		setLauncherPower(0);
	}
}

task flywheel() {
	TVexJoysticks buttons[4] = {Btn7D, Btn7L, Btn7R, Btn7U}; //creating a pseudo-hash associating buttons with velocities and default motor powers
	float velocities[4] = {3.91, 4.30, 5.03, 5.60};
	int defaultPowers[4] = {40, 46, 63, 82};

	while (true)
	{
		for (int i = 0; i < 4; i++)
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

		while (abs(flywheelVelocity - getFlywheelVelocity()) < bangBangErrorMargin * flywheelVelocity) //PID control
		{
			wait1Msec(sampleTime);
			while (!velocityUpdated) { EndTimeSlice(); }
			error = (flywheelVelocity - getFlywheelVelocity());

			if (abs(error) < integralMargin * flywheelVelocity)
			{
				integral += error;
			}

			targetPower = defaultPower + kp * error + ki * integral + kd * (error - prevError) / sampleTime;
			prevError = error;
		}

		//bang bang control
		setLauncherPower((flywheelVelocity < flywheelVelocity) ? (127) : (0));

		while (abs(flywheelVelocity - flywheelVelocity) > bangBangErrorMargin * flywheelVelocity) { EndTimeSlice(); }

		setLauncherPower(defaultPower);
	}
}
//end user input region

//begin task control region
void resetFlywheelVars() {
	flywheelRunning = false;
	velocityUpdated = false;
	continuousFire = false;
	flywheelVelocity = 0;
	targetVelocity = 0;
	flywheelPower = 0;
	targetPower = 0;
	defaultPower = 0;
}

task launcherMode() {
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
		stopTask(flywheel);
		stopTask(flywheelStabilization);
		stopTask(spinUpControl);
		stopTask(seymoreControl);
		stopTask(calcVelocity);
		startTask(puncher);
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
	AutonomousCodePlaceholderForTesting();
}

task usercontrol() {
	initializeTasks();

	while (true)
	{
		while (vexRT[emergencyStopBtn] == 0)
		{
			setDrivePower(vexRT[Ch2], vexRT[Ch3]);
			EndTimeSlice();
		}

		emergencyStop();
	}
}
