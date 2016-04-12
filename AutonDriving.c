#pragma config(Sensor, in1,    gyro,           sensorGyro)
#pragma config(Sensor, in2,    feedResistor,   sensorLineFollower)
#pragma config(Sensor, dgtl1,  flywheelEncoder, sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  leftEncoder,    sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  rightEncoder,   sensorQuadEncoder)
#pragma config(Sensor, dgtl7,  flywheelSwitch, sensorTouch)
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

#define fireBtn Btn5U
#define seymoreOutBtn Btn5D
#define feedInBtn Btn6U
#define feedOutBtn Btn6D
#define toggleAutoStopBtn Btn8U
#define liftBtn Btn5U
#define deployBtn Btn5D
#define liftSwitcherBtn Btn8L
#define emergencyStopBtn Btn8R

#define flywheelTimer T1
#define driveTimer T2
#define fireTimer T3
#define feedTimer T4

int limit(int input, int min, int max) {
	if (input <= max && input >= min) {
		return input;
	}
	else {
		return (input > max ? max : min);
	}
}

void setDrivePower(int right, int left) {
	motor[rfdrive] = right;
	motor[rbdrive] = right;
	motor[lfdrive] = left;
	motor[lbdrive] = left;
}

void setLauncherPower(int power, int minVal=0, int maxVal=127) {
	int flywheelPower = limit(power, minVal, maxVal);
	motor[ce] = flywheelPower;
	motor[rb] = flywheelPower;
	motor[er] = flywheelPower;
	motor[us] = flywheelPower;
}

//calcVelocity
//also used in flywheelStabilization
float flywheelVelocity;
int sampleTime = 50;
bool velocityUpdated = false;

task calcVelocity() {
	while (true) {
		SensorValue[flywheelEncoder] = 0;
		wait1Msec(sampleTime);
		flywheelVelocity = abs((float)(SensorValue[flywheelEncoder])) / (float)(sampleTime);
		velocityUpdated = true;
	}
}
//calcVelocity

//adjustmentTask
float error; //also used in fire, feedControl, and flywheelStabilization
float firingErrorMargin; //also used in fire, feedControl, and setFlywheelRange
int targetVelocity = 0; //also used in setFlywheelRange, fire, feedControl, and flywheelStabilization
bool adjustmentPeriod = false; //also used in setFlywheelRange and flywheelStabilization

task adjustmentTask() {
	while (motor[seymore] == 0 || abs(error) < targetVelocity * firingErrorMargin || SensorValue[flywheelSwitch] == 0) { EndTimeSlice(); }
	adjustmentPeriod = false;
}
//adjustmentTask

//setFlywheelRange
float integral=0, Ki=0, Kp=0, Kd=0; //also used in flywheelStabilization

int velocities[4] = {0, 3.0, 4.0, 6.2};
float firingErrorMargins[4] = {1.0, 1.0, 1.0, 1.0}/*{0.1, 0.1, 0.1, 0.1}*/;
float Kps[4] = {0, 40.0, 80.0, 80.0};
float Kis[4] = {0, 0.005, 0.005, 0.005};
float Kds[4] = {0, 10.0, 10.0, 10.0};

void setFlywheelRange(int range) {
	integral = 0;
	int limitedRange = limit(range, 0, 4);
	Kp = Kps[limitedRange];
	Ki = Kis[limitedRange];
	Kd = Kds[limitedRange];
	firingErrorMargin = firingErrorMargins[limitedRange];
	targetVelocity = velocities[limitedRange];

	adjustmentPeriod = range !=  0;
	startTask(adjustmentTask);
}
//end setFlywheelRange

//turn
float degreesToTurn;
int maxTurnSpeed, waitAtEnd;

void turnEnd() {
	setDrivePower(-sgn(degreesToTurn) * 10, sgn(degreesToTurn) * 10);
	int brakeDelay = limit(250, 0, waitAtEnd);
	wait1Msec(brakeDelay);
	setDrivePower(0, 0);

	if (waitAtEnd > 250) wait1Msec(waitAtEnd - 250); //wait at end
}

task turnTask() {
	while (abs(SensorValue[gyro]) < abs(degreesToTurn * 10)) { EndTimeSlice(); }
	turnEnd();
}

void turn(float _degreesToTurn_, int _maxTurnSpeed_=55, bool runAsTask=false, int _waitAtEnd_=250) {
	degreesToTurn = _degreesToTurn_;
	maxTurnSpeed = _maxTurnSpeed_;
	waitAtEnd = _waitAtEnd_;

	SensorValue[gyro] = 0; //clear the gyro
	setDrivePower(sgn(degreesToTurn) * maxTurnSpeed, -sgn(degreesToTurn) * maxTurnSpeed); //begin turn

	if (runAsTask) {
		startTask(turnTask);
	}
	else {
		while (abs(SensorValue[gyro]) < abs(degreesToTurn * 10)) { EndTimeSlice(); }
		turnEnd();
	}
}
//end turn

//driveStraight
float coeff = 15;
bool driveStraightRunning = false;
int clicks, direction, drivePower, delayAtEnd, driveTimeout, totalClicks, slavePower, driveStraightError;

void driveStraightRuntime() {
	setDrivePower(drivePower * direction, slavePower * direction);

	driveStraightError = SensorValue[gyro];

	slavePower += driveStraightError / coeff;

	totalClicks += (abs(SensorValue[leftEncoder]) + abs(SensorValue[rightEncoder])) / 2;
	SensorValue[leftEncoder] = 0;
	SensorValue[rightEncoder] = 0;
}

void driveStraightEnd() {
	setDrivePower(0, 0);
	wait1Msec(delayAtEnd);
	driveStraightRunning = false;
}

task driveStraightTask() {
	while (abs(totalClicks) < clicks  && time1(driveTimer) < driveTimeout) {
		driveStraightRuntime();

		wait1Msec(100);
	}
	driveStraightEnd();
}

void driveStraight(int _clicks_, int _delayAtEnd_=250, int _drivePower_=60, bool startAsTask=false, int _timeout_=15000) {
	//initialize global variables
	clicks = abs(_clicks_);
	direction = sgn(_clicks_);
	drivePower = _drivePower_;
	delayAtEnd = _delayAtEnd_;
	driveTimeout = _timeout_;

	totalClicks = 0;
	slavePower = drivePower - 2;
	driveStraightError = 0;

	//initialize sensors
	SensorValue[leftEncoder] = 0;
	SensorValue[rightEncoder] = 0;
	SensorValue[gyro] = 0;
	clearTimer(driveTimer);

	if (startAsTask) {
		startTask(driveStraightTask);
	}
	else { //runs as function
		while (abs(totalClicks) < clicks  && time1(driveTimer) < driveTimeout) {
			driveStraightRuntime();
			wait1Msec(100);
		}
		driveStraightEnd();
	}
}
//end driveStraight

//ball counting
int ballsInFeed; //also used in fire and photoresistor

task fireCounting() {
	while (true) {
		while (SensorValue[flywheelSwitch] == 0) { EndTimeSlice(); }
		while (SensorValue[flywheelSwitch] == 1) { EndTimeSlice(); }
		ballsInFeed = limit(ballsInFeed-1, 0, 4);
		wait1Msec(250);
	}
}
//end ball counting

//photoresistor
int photoFeedPower = 0; //also used in feedControl
int resistorCutoff = 2900;

task autoFeeding() {
	motor[feedMe] = 127;
	while (true) { motor[seymore] = photoFeedPower;	}
}

task photoresistor() {
	startTask(fireCounting);
	ballsInFeed = 0;

	while (true) {
		while (ballsInFeed < 4) {
			while (SensorValue[feedResistor] > resistorCutoff) { EndTimeSlice(); }
			while (SensorValue[feedResistor] < resistorCutoff) {
				photoFeedPower = (SensorValue[flywheelSwitch] == 0) ? 127 : 0;
				EndTimeSlice();
			}

			ballsInFeed = limit(ballsInFeed+1, 0, 4);

			clearTimer(feedTimer);
			wait1Msec(50);
			/*while (time1(feedTimer)<50) {
				photoFeedPower = (SensorValue[flywheelSwitch] == 0) ? 127 : 0;
				EndTimeSlice();
			}*/
			photoFeedPower = 0;
		}

		photoFeedPower = (SensorValue[flywheelSwitch] == 0) ? 127 : 0;
		while (SensorValue[flywheelSwitch] == 0) { EndTimeSlice(); }
		photoFeedPower = 0;
	}
}
//end photoresistor

//fire
bool firing = false; //also used in autofeeding
int fireTimeout, ballsToFire;

task fireTask() {
	firing = true;
	stopTask(autoFeeding);
	clearTimer(fireTimer);
	int targetBalls = ballsInFeed - ballsToFire;

	while (ballsInFeed > targetBalls && time1(fireTimer) < fireTimeout) {
		motor[seymore] = (abs(error) < targetVelocity * firingErrorMargin || SensorValue[flywheelSwitch] == 0) ? 127 : 0;
		motor[feedMe] = motor[seymore];
		EndTimeSlice();
	}

	firing = false;
	ballsInFeed = 0;
	startTask(autoFeeding);
}

void fire(int _ballsToFire_=ballsInFeed, int _timeout_=6000) {
	fireTimeout = _timeout_;
	ballsToFire = _ballsToFire_;

	startTask(fireTask);
}
//end fire

//feedControl
bool autoStop = true;
int feedMePower = 0;

task feedControl() {
	motor[seymore] = 0;
	while (true) {
		if (vexRT[fireBtn] == 1 || vexRT[seymoreOutBtn] == 1 || vexRT[toggleAutoStopBtn] == 1) {
			if (vexRT[toggleAutoStopBtn] == 1) {
				autoStop = !autoStop;
				while (vexRT[toggleAutoStopBtn] == 1) { EndTimeSlice(); }
			}
		  motor[seymore] = /*(abs(error) < firingErrorMargin * targetVelocity) || SensorValue[flywheelSwitch] == 0 || !autoStop ?*/ 127*vexRT[fireBtn] - 127*vexRT[seymoreOutBtn] /*: 0*/;
		}
		else {
			motor[seymore] = photoFeedPower;
		}
		motor[feedMe] = 127*vexRT[feedInBtn] - 127*vexRT[feedOutBtn];
		feedMePower =  motor[feedMe];
		EndTimeSlice();
	}
}
//end feedControl

void feed(bool autonomous) {
	startTask(photoresistor);
	if (autonomous) {
		startTask(autoFeeding);
	}
	else {
		startTask(feedControl);
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

//flywheelStabilization
float bangBangErrorMargin=0.1, integralMargin=1.0;

//debug
int bangBangCount;
float avgError, bbpercentup, bangBangPerSec, P, I, D;

task flywheelStabilization() { //modulates motor powers to maintain constant flywheel velocity
	clearTimer(flywheelTimer);
	float prevError;
	//debug
	int numbbup = 0;
	float totalError = 0;
	int numloops = 0;

	while (true)
	{
		while (abs(error) < bangBangErrorMargin * targetVelocity && targetVelocity > 0 || adjustmentPeriod) //PID control
		{
			while (!velocityUpdated) { EndTimeSlice(); }
			error = (targetVelocity - flywheelVelocity);
			velocityUpdated = false;

			if (abs(error) < integralMargin * flywheelVelocity)
			{
				integral += (prevError + error) * sampleTime / 2;
			}

			//debug
			totalError += abs(error);
			numloops += 1;
			avgError = totalError / numloops;
			P = Kp * error;
			I = Ki  * integral;
			D = Kd * (error - prevError) / sampleTime;

			setLauncherPower(Kp * error + Ki * integral + Kd * (error - prevError) / sampleTime);
			prevError = error;
		}

		//bang bang control
		//debug
		bangBangCount += 1;
		numbbup += (targetVelocity > flywheelVelocity ? 1 : 0);
		bbpercentup = 100 * numbbup / bangBangCount;
		bangBangPerSec = (float)((float)bangBangCount * 1000) / (float)(time1(flywheelTimer) + .1);

		while (abs(targetVelocity - flywheelVelocity) > bangBangErrorMargin * flywheelVelocity && targetVelocity > 0) {
			setLauncherPower((targetVelocity > flywheelVelocity) ? (127) : (0));
			EndTimeSlice();
		}

		if (targetVelocity == 0) {
			setLauncherPower(0);
			while (targetVelocity == 0) { EndTimeSlice(); }
		}
	}
}
//end flywheelStabilization

//lifting
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

task liftSwitcher() {
	while (true) {
		//switch to lift
		while (vexRT[liftSwitcherBtn] == 0) { EndTimeSlice(); }
		stopTask(flywheel);
		stopTask(flywheelStabilization);
		setLauncherPower(0);
		startTask(lift);
		while (vexRT[liftSwitcherBtn] == 1) { EndTimeSlice();	}
		//switch to flywheel
		while (vexRT[liftSwitcherBtn] == 0) { EndTimeSlice(); }
		startTask(flywheel);
		startTask(flywheelStabilization);
		setLauncherPower(0);
		stopTask(lift);
		while (vexRT[liftSwitcherBtn] == 1) { EndTimeSlice();	}
	}
}
//end lifting

void initializeTasks(bool autonomous) {
	startTask(flywheel);
	startTask(flywheelStabilization);
	startTask(calcVelocity);
	feed(autonomous);

	if (!autonomous) startTask(liftSwitcher);
}

void emergencyStop() {
	stopTask(flywheel);
	stopTask(flywheelStabilization);
	stopTask(photoresistor);
	stopTask(feedControl);
	stopTask(fireCounting);
	stopTask(lift);
	stopTask(liftSwitcher);

	initializeTasks(false);
}

task usercontrol() {
	initializeTasks(false);
	setFlywheelRange(0);

	while (true)
	{
		while (vexRT[emergencyStopBtn] == 0)
		{
			setDrivePower(sgn(vexRT[Ch2]) * vexRT[Ch2] * vexRT[Ch2] / 127, sgn(vexRT[Ch3]) * vexRT[Ch3] * vexRT[Ch3] / 127); //drive
			EndTimeSlice();
		}

		emergencyStop(); //reassign emstop button
	}
}

//autonomii
void pre_auton() { bStopTasksBetweenModes = true; }

task autonomous() {
	initializeTasks(true);

	//skillPointAuto
	/*setFlywheelRange(2);
	wait1Msec(2000);
	fire(5);
	while (true) { EndTimeSlice(); }*/

	//stationaryAuto
	/*setFlywheelRange(3);
	wait1Msec(2000);
	fire(5);
	while (true) { EndTimeSlice(); } */

	//hoardingAuto
	/*driveStraight(2000); //drive forward
	turn(-15); //turn
	driveStraight(-1000, 80); //back up to push first stack into start zone
	turn(18); //turn toward second stack
	setFlywheelRange(1);
	driveStraight(2300, 750); //pick up second stack
	//fire second stack
	fire();
	while (firing) { EndTimeSlice(); }

	turn(-65); //turn toward third stack
	//pick up third stack
	driveStraight(1100);*/

	//classicAuto
	setFlywheelRange(3);
	//fire four initial preloads
	fire(4);
	while (firing) { EndTimeSlice(); }

	setFlywheelRange(2);
	turn(21); //turn toward first stack
	//pick up first stack
	driveStraight(900);

	turn(-18); //turn toward net
	driveStraight(1150); //drive toward net
	fire();
	while (firing) { EndTimeSlice(); }

	//pick up second stack
	driveStraight(950); //drive into net for realignment
	driveStraight(-750); //move back
	//fire second stack
	fire();
	while (firing) { EndTimeSlice(); }

	turn(-65); //turn toward third stack
	//pick up third stack
	driveStraight(1100);

	//skillz
	/*//start flywheel
	setFlywheelRange(2);

	wait1Msec(1000);
	fire(32);
	//wait until first set of preloads are fired
	while (firing) { EndTimeSlice(); }

	turn(108); //turn toward middle stack
	driveStraight(2300); //drive across field
	turn(-15); // turn toward starting tiles
	driveStraight(1200); //drive across field
	turn(-60); //turn toward net

	//fire remaining balls
	fire();
	while (true) { EndTimeSlice(); }*/
}
