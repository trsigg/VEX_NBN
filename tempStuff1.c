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

#define firingErrorMargin 0.05
#define sampleTime 25 //number of milliseconds between sampling the flywheel velocity and control adjustments in flywheel task
#define notFiringCutoff 20 //maximum error value considere not firing

#define fireBtn Btn5U
#define seymoreOutBtn Btn5D
#define feedInBtn Btn6U
#define feedOutBtn Btn6D
#define liftBtn Btn5U
#define deployBtn Btn5D
#define liftSwitcherBtn Btn8L
#define emergencyStopBtn Btn8R

#define driveTimer T1
#define firingTimer T2
int errortest = 0;
//waitUntilNotFiring

int initialWait, timeWithoutFiring;
bool firing = false;
//driveStraight
bool driveStraightRunning = false;
int clicks, direction, drivePower, delayAtEnd, timeout;
//turn
float degreesToTurn;
int maxTurnSpeed, waitAtEnd;
//flywheel variables
int flywheelVelocity=0, targetVelocity=0, flywheelPower=0, defaultPower=0;
float Integral=0, Error=0, Ki=0, Kp=0, Kd=0;

float P, I, D; //debugging

//begin helper functions region
int limit(int input, int min, int max) {
	if (input <= max && input >= min) {
		return input;
	}
	else {
		return (input > max ? max : min);
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

void setLauncherPower(int power, int minVal=0, int maxVal=127) {
	flywheelPower = limit(power, minVal, maxVal);
	motor[ce] = flywheelPower;
	motor[rb] = flywheelPower;
	motor[er] = flywheelPower;
	motor[us] = flywheelPower;
}

void setFlywheelRange(int range) {
	int velocities[4] = {0, 170, 185, 234};
	int defaultPowers[4] = {0, 45, 54, 82};
	float Kps[4] = {0, 2.6, 2.4, 56};
	float Kis[4] = {0, 0.001, 0.001, 0.01};
	float Kds[4] = {0, 1.5, 2.5, 80};

	Integral = 0;
	int limitedRange = limit(range, 0, 4);
	Kp = Kps[limitedRange];
	Ki = Kis[limitedRange];
	Kd = Kds[limitedRange];
	targetVelocity = velocities[limitedRange];
	defaultPower = defaultPowers[limitedRange];
}
//end set functions region

//autonomous region
task turnTask() {
	SensorValue[gyro] = 0; //clear the gyro
	//turn
	setDrivePower(-sgn(degreesToTurn) * maxTurnSpeed, sgn(degreesToTurn) * maxTurnSpeed);
	while (abs(SensorValue[gyro]) < abs(degreesToTurn * 10)) {}
	//brake
	setDrivePower(sgn(degreesToTurn) * 10, -sgn(degreesToTurn) * 10);
	int brakeDelay = limit(250, 0, waitAtEnd);
	wait1Msec(brakeDelay);
	setDrivePower(0, 0);

	if (waitAtEnd > 250) wait1Msec(waitAtEnd - 250); //wait at end
}

void turn(float _degreesToTurn_, int _maxTurnSpeed_=55, bool runAsTask=false, int _waitAtEnd_=250) {
	degreesToTurn = _degreesToTurn_;
	maxTurnSpeed = _maxTurnSpeed_;
	waitAtEnd = _waitAtEnd_;

	if (runAsTask) {
		startTask(turnTask);
	}
	else {
		SensorValue[gyro] = 0; //clear the gyro
		//turn
		setDrivePower(sgn(degreesToTurn) * maxTurnSpeed, -sgn(degreesToTurn) * maxTurnSpeed);
		while (abs(SensorValue[gyro]) < abs(degreesToTurn * 10)) {}
		//brake
		setDrivePower(-sgn(degreesToTurn) * 10, sgn(degreesToTurn) * 10);
		int brakeDelay = limit(250, 0, waitAtEnd);
		wait1Msec(brakeDelay);
		setDrivePower(0, 0);

		if (waitAtEnd > 250) wait1Msec(waitAtEnd - 250); //wait at end
	}
}

task driveStraightTask()
{
	driveStraightRunning = true;

	int coeff = 15;
	int totalClicks = 0;
	int slavePower = drivePower - 2;
	int error = 0;

	SensorValue[leftEncoder] = 0;
	SensorValue[rightEncoder] = 0;
	SensorValue[gyro] = 0;
	clearTimer(driveTimer);

	while (abs(totalClicks) < clicks  && time1(driveTimer) < timeout)
	{
		setDrivePower(drivePower * direction, slavePower * direction);

		error = SensorValue[gyro];

		slavePower += error / coeff;

		totalClicks += (abs(SensorValue[leftEncoder]) + abs(SensorValue[rightEncoder])) / 2;
		SensorValue[leftEncoder] = 0;
		SensorValue[rightEncoder] = 0;

		wait1Msec(100);
	}
	setDrivePower(0, 0);
	wait1Msec(delayAtEnd);
	driveStraightRunning = false;
}

void driveStraight(int _clicks_, int _delayAtEnd_=250, int _drivePower_=60, bool startAsTask=false, int _timeout_=15000) {
	clicks = abs(_clicks_);
	direction = sgn(_clicks_);
	drivePower = _drivePower_;
	delayAtEnd = _delayAtEnd_;
	timeout = _timeout_;
	if (startAsTask) {
		startTask(driveStraightTask);
	}
	else { //runs as function
		int coeff = 15;
		int totalClicks = 0;
		int slavePower = drivePower - 2;
		int error = 0;

		SensorValue[leftEncoder] = 0;
		SensorValue[rightEncoder] = 0;
		SensorValue[gyro] = 0;
		clearTimer(driveTimer);

		while (abs(totalClicks) < clicks  && time1(driveTimer) < timeout)
		{
			setDrivePower(drivePower * direction, slavePower * direction);

			error = SensorValue[gyro];

			slavePower += error / coeff;

			totalClicks += abs(SensorValue[leftEncoder]);
			SensorValue[leftEncoder] = 0;
			SensorValue[rightEncoder] = 0;

			wait1Msec(100);
		}
		setDrivePower(0, 0);
		wait1Msec(delayAtEnd);
	}
}

task waitUntilNotFiringTask() {
	firing = true;
	//if (initialWait == 0) {
	//	while (SensorValue[flywheelSwitch] == 1) { EndTimeSlice(); }
	//}
	//else {
		wait1Msec(initialWait);
	//}
	clearTimer(T2/*firingTimer*/);

	while (time1[T2/*firingTimer*/] < timeWithoutFiring) {
		if (SensorValue[flywheelSwitch] == 0) clearTimer(firingTimer);
		EndTimeSlice();
	}
	firing = false;
}

void waitUntilNotFiring(int _initialWait_=0, int _timeWithoutFiring_=2500) {
	initialWait = _initialWait_;
	timeWithoutFiring = _timeWithoutFiring_;
	startTask(waitUntilNotFiringTask);
}

int velRan;
bool inVelRan = false;
bool unpressed;
bool shouldFire = false;

task fire() {
	while (true) {
		velRan = targetVelocity * firingErrorMargin;
		inVelRan = Error < velRan;
		unpressed = SensorValue[flywheelSwitch] == 1;
		shouldFire = inVelRan || unpressed;
		if (shouldFire) {
			motor[seymore] = 127;
		}
		else {
			motor[seymore] = 0;
		}
		EndTimeSlice();
	}
}

task feedToTop() {
	motor[seymore] = 127;
	while (SensorValue[flywheelSwitch] == 1) { EndTimeSlice(); }
	motor[seymore] = 0;
}
//end autonomous region

//begin user input region
task lift() {
	while (vexRT[deployBtn] == 0) { EndTimeSlice(); }
	setLauncherPower(-40, -127, 0);
	wait1Msec(75);
	setLauncherPower(0);
	wait1Msec(750);

	while (true) {
		setLauncherPower(-127*vexRT[liftBtn] - 40*vexRT[deployBtn], -127, 0);
		EndTimeSlice();
	}
}

task seymoreControl() {
	while (true) {
		motor[seymore] = /*(Error < firingErrorMargin * targetVelocity) || SensorValue[flywheelSwitch] == 1 ?*/ 127*vexRT[fireBtn] - 127*vexRT[seymoreOutBtn] /*: 0*/;
		EndTimeSlice();
	}
}

task flywheel() {
	TVexJoysticks buttons[4] = { Btn8D, Btn7U, Btn7R, Btn7D }; //creating a pseudo-hash associating buttons with velocities and default motor powers

	while (true)
	{
		for (int i = 0; i < 4; i++)
		{
			if (vexRT[buttons[i]] == 1)
			{
				setFlywheelRange(i);
			}
			EndTimeSlice();
		}
	}
}

task flywheelStabilization() { //modulates motor powers to maintain constant flywheel velocity
	float PrevError = 0;
	float DeltaE = 0;

	while(true)	{
		SensorValue[flywheelEncoder] = 0;
		PrevError = Error;
		wait1Msec(sampleTime);

		flywheelVelocity = abs(SensorValue[flywheelEncoder]);
		Error = targetVelocity - flywheelVelocity;
		errortest = Error;
		DeltaE = Error - PrevError;
		Integral += (Error + PrevError)/2;
		setLauncherPower(defaultPower + Kp*Error + Ki*Integral + Kd*DeltaE);

		//debug
		P = Kp*Error;
		I = Ki*Integral;
		D = Kd*DeltaE;
	}
}
//end user input region

//begin task control region
task liftSwitcher() {
	while (true) {
		//switch to lift
		while (vexRT[liftSwitcherBtn] == 0) { EndTimeSlice(); }
		stopTask(flywheel);
		stopTask(flywheelStabilization);
		stopTask(seymoreControl);
		setLauncherPower(0);
		startTask(lift);
		while (vexRT[liftSwitcherBtn] == 1) { EndTimeSlice();	}
		//switch to flywheel
		while (vexRT[liftSwitcherBtn] == 0) { EndTimeSlice(); }
		startTask(flywheel);
		startTask(flywheelStabilization);
		startTask(seymoreControl);
		setLauncherPower(0);
		stopTask(lift);
		while (vexRT[liftSwitcherBtn] == 1) { EndTimeSlice();	}
	}
}

void initializeTasks() {
	startTask(flywheel);
	startTask(flywheelStabilization);
	startTask(seymoreControl);
	startTask(liftSwitcher);
}

void emergencyStop() {
	stopTask(flywheel);
	stopTask(flywheelStabilization);
	stopTask(seymoreControl);
	stopTask(liftSwitcher);

	initializeTasks();
}
//end task control region

int feedMePower = 0;

task usercontrol() {
	initializeTasks();
	setFlywheelRange(0);

	while (true)
	{
		while (vexRT[emergencyStopBtn] == 0)
		{
			setDrivePower(sgn(vexRT[Ch2]) * vexRT[Ch2] * vexRT[Ch2] / 127, sgn(vexRT[Ch3]) * vexRT[Ch3] * vexRT[Ch3] / 127); //drive
			motor[feedMe] = 127*vexRT[feedInBtn] - 127*vexRT[feedOutBtn];
			feedMePower = motor[feedMe];
			EndTimeSlice();
		}

		emergencyStop(); //reassign emstop button
	}
}

//autonomii
void pre_auton() { bStopTasksBetweenModes = true; }

task skillPointAuto() { //fire into opposing net
	setFlywheelRange(2);
	wait1Msec(2000);
	motor[feedMe] = 127;
	startTask(fire);
	wait1Msec(7000);
	stopTask(fire);
}

task stationaryAuto() { //fire into our net
	setFlywheelRange(3);
	wait1Msec(2000);
	motor[feedMe] = 127;
	startTask(fire);
	wait1Msec(7000);
	stopTask(fire);
}

task hoardingAuto() { //push balls into our corner
	driveStraight(2000); //drive forward
	turn(-15); //turn
	driveStraight(-1000, 80); //back up to push first stack into start zone
	turn(18); //turn toward second stack
	setFlywheelRange(1);
	motor[feedMe] = 127; //start feed
	driveStraight(2300, 750); //pick up second stack
	//fire second stack
	startTask(fire);
	waitUntilNotFiring();
	while (firing) { EndTimeSlice(); }
	stopTask(fire);
	startTask(feedToTop);

	turn(-65); //turn toward third stack
	//pick up third stack
	driveStraight(1100);
}

task classicAuto() {
	setFlywheelRange(3);
	motor[feedMe] = 127;
	//fire four initial preloads
	startTask(fire);
	waitUntilNotFiring();
	while (firing) { EndTimeSlice(); }
	stopTask(fire);
	startTask(feedToTop);

	setFlywheelRange(2);
	turn(21); //turn toward first stack
	//pick up first stack
	startTask(feedToTop);
	driveStraight(900);

	turn(-18); //turn toward net
	driveStraight(1150); //drive toward net
	stopTask(feedToTop);
	startTask(fire);
	waitUntilNotFiring(3000);
	while (firing) { EndTimeSlice(); }
	stopTask(fire);

	//pick up second stack
	startTask(feedToTop);
	driveStraight(950); //drive into net for realignment
	driveStraight(-750); //move back
	//fire second stack
	stopTask(feedToTop);
	startTask(fire);
	waitUntilNotFiring();
	while (firing) { EndTimeSlice(); }
	stopTask(fire);

	turn(-65); //turn toward third stack
	//pick up third stack
	driveStraight(1100);
}

task skillz() {
	//start flywheel
	setFlywheelRange(2);

	wait1Msec(1000);
	startTask(fire);
	//wait until first set of preloads are fired
	waitUntilNotFiring(12000);
	while (firing) { EndTimeSlice(); }
	stopTask(fire);
	startTask(feedToTop);

	turn(108); //turn toward middle stack
	motor[feedMe] = 127; //startTask(feedToTop); //start feeding
	driveStraight(2300); //drive across field
	turn(-15); // turn toward starting tiles
	driveStraight(1200); //drive across field
	turn(-60); //turn toward net

	//fire remaining balls
	stopTask(feedToTop);
	startTask(fire);
	while (true) { EndTimeSlice(); }
}

task autonomous() {
	initializeTasks();
	stopTask(seymoreControl);

	//startTask(skillPointAuto);
	//startTask(stationaryAuto);
	//startTask(hoardingAuto);
	//startTask(classicAuto);
	//startTask(skillz);

	if (SensorValue[ternaryPot] < 1187) {
		if (SensorValue[binaryPot] < 1217) {
			startTask(stationaryAuto);
		}
		else {
			startTask(skillPointAuto);
		}
	}
	else if (SensorValue[ternaryPot] < 2577) {
		if (SensorValue[binaryPot] < 1217) {
			startTask(classicAuto);
		}
		else {
			startTask(hoardingAuto);
		}
	}
	else {
		startTask(skillz);
	}

	while(true) { EndTimeSlice(); }
}
