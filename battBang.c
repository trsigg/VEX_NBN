#pragma config(Sensor, in1,    Yaw,            sensorGyro)
#pragma config(Sensor, in2,    BallFeed,       sensorLineFollower)
#pragma config(Sensor, in3,    BallLaunch,     sensorLineFollower)
#pragma config(Sensor, in4,    modePot,        sensorPotentiometer)
#pragma config(Sensor, in5,    sidePot,        sensorPotentiometer)
#pragma config(Sensor, dgtl1,  FlyWheel,       sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  leftE,          sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  rightE,         sensorQuadEncoder)
#pragma config(Motor,  port1,           Seymore,       tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           LeftDrive1,    tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           LeftDrive2,    tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           Fly1,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           Fly2,          tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           Fly3,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           Fly4,          tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           RightDrive2,   tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           RightDrive1,   tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          FeedMe,        tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma competitionControl(Competition)
#include "Vex_Competition_Includes.c"

//Variables
	//Gyro SetUp
long cumBias = 0;
	//MotorSpeeds
int LeftSpeed = 0;
int RightSpeed = 0;
int Flyspeed = 0;
int SeymoreSpeed = 0;
	//Selection Variables
int n = 0;
TVexJoysticks buttons[4] = {Btn8D, Btn7U, Btn7R, Btn7D};
	//PID Control
int TargetSpeeds[4] = {0, 326, 340, 433};
int TargetSpeed;
int Error = 0;//Error stuff

float KpError = 0;
long Integral[4] = {0, 0, 0, 0};//Integral stuff
float Ki[4] = {0, 0.1, 0.05, 0.001};
float KiIntegral = 0;
int PrevError = 0;
float quadratic[4] = { 0, 25, 30, 45 };
float linear[4] = { 0, 25, 30, 45 };
float constant[4] = {0, 25, 30, 45};
float ErrorMargarine[4] = {0, 0.08, 0.04, 0.01};
//AutomaticSeymore
int BallThreshold = 3020; //Line
bool AutoGo = false;
bool Meter = true;
bool PossBall = false;
int BallCount = 0;
bool BallLoss = false;
int AutoToggle = 1;
bool SecondToggle = false;


int limit(int input, int min, int max) {
	if (input <= max && input >= min) {
		return input;
	}
	else {
		return (input > max ? max : min);
	}
}

void setFlywheelRange(int range) {
	int limitedRange = limit(range, 0, 3);

	n = limitedRange;
	TargetSpeed = TargetSpeeds[limitedRange];
}

void pre_auton()
{
	bStopTasksBetweenModes = true;
	SensorType[Yaw] = sensorNone;
	for(int i = 0; i<2000; i++)
	{
		cumBias += SensorValue[Yaw];
	}
	SensorType[Yaw] = sensorGyro;
	SensorBias[Yaw] = cumBias/2000;
}

void Motorspeeds()
{
	motor[LeftDrive1] = vexRT[Btn8L]==0?(LeftSpeed):0;//Single Variable Drive Control
	motor[LeftDrive2] = vexRT[Btn8L]==0?(LeftSpeed):0;
	motor[RightDrive1] = RightSpeed;
	motor[RightDrive2] = RightSpeed;
	motor[Fly1] = vexRT[Btn8L]==0?(Flyspeed):-vexRT[Ch3];//See Task PIDControl
	motor[Fly2] = vexRT[Btn8L]==0?(Flyspeed):-vexRT[Ch3];
	motor[Fly3] = vexRT[Btn8L]==0?(Flyspeed):-vexRT[Ch3];
	motor[Fly4] = vexRT[Btn8L]==0?(Flyspeed):-vexRT[Ch3];
	motor[Seymore] = SeymoreSpeed;//Function MechSeymore
	motor[FeedMe] = vexRT[Btn6U]*127 + vexRT[Btn6D]*-127;//AOI Logic
}

void MechSeymore()
{
	AutoGo = AutoToggle==1&&SensorValue[BallLaunch]>=BallThreshold&&SensorValue[BallFeed]<BallThreshold&&vexRT[Btn5D]==0?true:false;//Check for a ball
	Meter = SensorValue[BallLaunch]>=BallThreshold||(abs(Error)<=(ErrorMargarine[n]*TargetSpeed))?true:false;
	BallCount += PossBall==true&&AutoGo==false?1:0;
	PossBall = AutoGo==true?true:false;
	BallCount += BallLoss==true&&SensorValue[BallLaunch]>=BallThreshold?-1:0;
	BallLoss = SensorValue[BallLaunch]>=BallThreshold?false:true;
	AutoToggle = vexRT[Btn8L]==0&&SecondToggle==true?abs(AutoToggle-1):AutoToggle;
	SecondToggle = vexRT[Btn8L]==1?true:false;
	SeymoreSpeed = 100*AutoGo+127*vexRT[Btn5U]*Meter-127*vexRT[Btn5D];//AOI Combination
}

task PIDControl()
{
	while(true)
	{
		SensorValue[FlyWheel] = 0;
		wait1Msec(60);//TimeSample
		Error = TargetSpeed - SensorValue[FlyWheel]; //How much I am wrong
		Integral[n] += (Error + PrevError)/2; //How wrong I've been
		KiIntegral = Ki[n]*Integral[n];
		PrevError = Error;
		PIDPower = KiIntegral + quadratic[n]*exp(2*log(nImmediateBatteryLevel)) + linear[n]*nImmediateBatteryLevel + constant[n]; //PID Equation
	}
}

task Timer()
{
	//datalogClear();
 	while(true)
	{
		if(n!=0)
		{
			clearTimer(T1);
			waitUntil(abs(Error)<AccError[n]);
			//datalogAddValue(0, time1[T1]);
			waitUntil(abs(Error)>AccError[n]);
		}
	}
}

task usercontrol()
{
	startTask(PIDControl);
	startTask(Timer);
	while(true)
	{
		Motorspeeds();
		MechSeymore();
		LeftSpeed = abs(vexRT[Ch3])>10?vexRT[Ch3]:0;//Human Control
		RightSpeed = abs(vexRT[Ch2])>10?vexRT[Ch2]:0;
		for(int i = 0; i<4; i++)//Human Choice
		{
			if (vexRT[buttons[i]] == 1) setFlywheelRange(i);
		}
	}
}

//////////////////////////////////////
//           Tynan's region         //
//////////////////////////////////////

#define flywheelTimer T1
#define driveTimer T2
#define fireTimer T3
#define feedTimer T4

void setDrivePower(int left, int right) {
	motor[RightDrive1] = right;
	motor[RightDrive2] = right;
	motor[LeftDrive1] = left;
	motor[LeftDrive2] = left;
}

//turn
float degreesToTurn;
int maxTurnSpeed, waitAtEnd;

void turnEnd() {
	setDrivePower(sgn(degreesToTurn) * 10, -sgn(degreesToTurn) * 10);
	int brakeDelay = limit(250, 0, waitAtEnd);
	wait1Msec(brakeDelay);
	setDrivePower(0, 0);

	if (waitAtEnd > 250) wait1Msec(waitAtEnd - 250); //wait at end
}

task turnTask() {
	while (abs(SensorValue[Yaw]) < abs(degreesToTurn * 10)) { EndTimeSlice(); }
	turnEnd();
}

void turn(float _degreesToTurn_, int _maxTurnSpeed_=50, bool runAsTask=false, int _waitAtEnd_=250) {
	degreesToTurn = _degreesToTurn_;
	maxTurnSpeed = _maxTurnSpeed_;
	waitAtEnd = _waitAtEnd_;

	SensorValue[Yaw] = 0; //clear the gyro
	setDrivePower(-sgn(degreesToTurn) * maxTurnSpeed, sgn(degreesToTurn) * maxTurnSpeed); //begin turn

	if (runAsTask) {
		startTask(turnTask);
	}
	else {
		while (abs(SensorValue[Yaw]) < abs(degreesToTurn * 10)) { EndTimeSlice(); }
		turnEnd();
	}
}
//end turn

//driveStraight
float coeff = 300;
bool driveStraightRunning = false;
int clicks, direction, drivePower, delayAtEnd, driveTimeout, totalClicks, slavePower, driveStraightError;

void driveStraightRuntime() {
	setDrivePower(slavePower * direction, drivePower * direction);

	driveStraightError = SensorValue[Yaw];

	slavePower += driveStraightError * direction / coeff;

	totalClicks += (abs(SensorValue[leftE]) + abs(SensorValue[rightE])) / 2;
	SensorValue[leftE] = 0;
	SensorValue[rightE] = 0;
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
	slavePower = drivePower - 5;
	driveStraightError = 0;

	//initialize sensors
	SensorValue[leftE] = 0;
	SensorValue[rightE] = 0;
	SensorValue[Yaw] = 0;
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
bool firing = false; //also used in fire and autofeeding
int ballsInFeed; //also used in fire and photoresistor

task fireCounting() {
	while (firing) {
		while (SensorValue[BallLaunch] > BallThreshold || motor[Seymore] == 0) { EndTimeSlice(); }
		while (SensorValue[BallLaunch] < BallThreshold || motor[Seymore] == 0) { EndTimeSlice(); }
		ballsInFeed = limit(ballsInFeed-1, -100, 4);
		clearTimer(fireTimer);
		wait1Msec(250);
	}
}
//end ball counting

//photoresistor
int photoFeedPower = 0; //also used in feedControl

task autoFeeding() {
	motor[FeedMe] = 127;
	while (true) { motor[Seymore] = photoFeedPower;	}
}

task photoresistor() {
	startTask(fireCounting);
	ballsInFeed = 0;

	while (true) {
		while (SensorValue[BallFeed] > BallThreshold) { EndTimeSlice(); }
		while (SensorValue[BallFeed] < BallThreshold) {
			photoFeedPower = (SensorValue[BallLaunch] > BallThreshold) ? 127 : 0;
			EndTimeSlice();
		}

		ballsInFeed = limit(ballsInFeed+1, 1, 4);

		clearTimer(feedTimer);
		wait1Msec(50);
		photoFeedPower = 0;
	}
}
//end photoresistor

//fire
int fireTimeout, initialWait, ballsToFire;

task fireTask() {
	firing = true;
	stopTask(autoFeeding);
	clearTimer(fireTimer);
	int targetBalls = ballsInFeed - ballsToFire;

	while (ballsInFeed > targetBalls && time1(fireTimer) < fireTimeout) {
		motor[Seymore] = (abs(Error) < TargetSpeed * ErrorMargarine[n] || SensorValue[BallLaunch] > BallThreshold) ? 127 : 0;
		motor[FeedMe] = motor[Seymore];
		EndTimeSlice();
	}

	firing = false;
	ballsInFeed = 0;
	startTask(autoFeeding);
}

void fire(int _ballsToFire_=ballsInFeed, int _timeout_=2000) {
	fireTimeout = _timeout_;
	ballsToFire = _ballsToFire_;

	startTask(fireTask);
}

task simpleFireTask() {
	clearTimer(fireTimer);
	while (time1(fireTimer) < initialWait && SensorValue[BallLaunch] > BallThreshold) { //feed balls to top
		motor[Seymore] = 127;
		motor[FeedMe] = 127;
		EndTimeSlice();
	}

	clearTimer(fireTimer);
	while (time1(fireTimer) < fireTimeout) { //fire
		motor[Seymore] = (abs(Error) < TargetSpeed * ErrorMargarine[n] || SensorValue[BallLaunch] > BallThreshold) ? 127 : 0;
		motor[FeedMe] = motor[Seymore];
		if (SensorValue[BallLaunch] < BallThreshold) clearTimer(fireTimer);
		EndTimeSlice();
	}

	ballsInFeed = 0;
	firing = false;
	startTask(autoFeeding);
}

void simpleFire(int _initialWait_=3000, int _timeout_=1000) {
	fireTimeout = _timeout_;
	initialWait = _initialWait_;
	firing = true;
	stopTask(autoFeeding);
	clearTimer(fireTimer);

	startTask(simpleFireTask);
}
//end fire

bool right;

void initializeTasks(bool autonomous) {
	startTask(PIDControl);
	startTask(Timer);
	startTask(photoresistor);
	startTask(autoFeeding);
}

task stationaryAuto() {
	setFlywheelRange(3);
	simpleFire(); //startTask(skillzFiring);
	while (firing) { EndTimeSlice(); }
	n = 0;
	stopTask(autoFeeding);
	allMotorsOff();
}

int hoardingAutoConstants[14] = { 800, -31, 31, 16, -16, 600, -19, 19, -18, 18, 1100, 40, -40, 25 };

task hoardingAuto() {
	//Gabe's hoarding
	/*n = 3;
	driveStraight(440);
	turn(-36);
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }

	n = 2;
	turn(12);
	driveStraight(350);
	turn(-17);
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }
	driveStraight(-420);
	turn(90);*/

	//modified aggro
	setFlywheelRange(3);
	TargetSpeed = 375;
	driveStraight(hoardingAutoConstants[0]);
	turn(right ? hoardingAutoConstants[1] : hoardingAutoConstants[2]);
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }

	setFlywheelRange(2);
	turn(right ? hoardingAutoConstants[3] : hoardingAutoConstants[4]);
	driveStraight(hoardingAutoConstants[5]); //increase
	turn(right ? hoardingAutoConstants[6] : hoardingAutoConstants[7]); //increase
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }

	setFlywheelRange(1);
	turn(right ? hoardingAutoConstants[8] : hoardingAutoConstants[9]);
	driveStraight(hoardingAutoConstants[10]);
	turn(right ? hoardingAutoConstants[11] : hoardingAutoConstants[12]);
	driveStraight(hoardingAutoConstants[13]);
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }
}

int classicAutoConstants[15] = { 900, -23, 13, 900, 600, -750, -62, 300, 14, 375, 750, -300, 66, 96, 3250 }; //E team

task classicAuto() {
	setFlywheelRange(1);
	//pick up first stack
	driveStraight(classicAutoConstants[0]);

	turn(right ? classicAutoConstants[1] : classicAutoConstants[2]); //turn toward net
	driveStraight(classicAutoConstants[3]); //drive toward net
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }

	//pick up second stack
	driveStraight(classicAutoConstants[4]); //drive into net for realignment
	driveStraight(classicAutoConstants[5]); //move back
	//fire second stack
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }

	turn(classicAutoConstants[6]);
	driveStraight(classicAutoConstants[7]);
	turn(classicAutoConstants[8]); //turn toward third stack
	driveStraight(classicAutoConstants[9], classicAutoConstants[10]); //pick up third stack
	driveStraight(classicAutoConstants[11]); //drive backward
	turn(classicAutoConstants[12]); //aim at net
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }
	turn(classicAutoConstants[13]);
	driveStraight(classicAutoConstants[14]);
}

int pskillzConstants[21] = { -100, 1900, 26, 800, 59, 30, 800, -23, 18, 1150, 950, -750, -62, 700, 14, 375, 750, -300, 66, 96, 3250 };

task pskillz() {
	right = true;

	//start flywheel
	setFlywheelRange(2);

	wait1Msec(1000);
	simpleFire(13000, 2000); //startTask(skillzFiring);
	wait1Msec(50);
	//wait until first set of preloads are fired
	while (firing) { EndTimeSlice(); }

	turn(pskillzConstants[0]); //turn toward middle stack
	driveStraight(pskillzConstants[1]); //drive across field
	turn(pskillzConstants[2]); // turn toward starting tiles
	driveStraight(pskillzConstants[3]); //drive across field
	turn(pskillzConstants[4]); //turn toward net

	//fire remaining balls
	simpleFire(13000, 60000); //startTask(skillzFiring);
	while (firing) { EndTimeSlice(); }

	////////////////////////
	//END CLASSIC pSKILLZ//
	///////////////////////

	/*setFlywheelRange(1);
	turn(pskillzConstants[5]);
	//pick up first stack
	driveStraight(pskillzConstants[6]);

	turn(right ? pskillzConstants[7] : pskillzConstants[8]); //turn toward net
	driveStraight(pskillzConstants[9]); //drive toward net
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }

	//pick up second stack
	driveStraight(pskillzConstants[10]); //drive into net for realignment
	driveStraight(pskillzConstants[11]); //move back
	//fire second stack
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }

	turn(pskillzConstants[12]);
	driveStraight(pskillzConstants[13]);
	turn(pskillzConstants[14]); //turn toward third stack
	driveStraight(pskillzConstants[15], pskillzConstants[16]); //pick up third stack
	driveStraight(pskillzConstants[17]); //drive backward
	turn(pskillzConstants[18]); //aim at net
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }
	turn(pskillzConstants[19]);
	driveStraight(pskillzConstants[20]);*/
}

int aggroConstants[14] = { 800, -31, 31, 16, -16, 600, -19, 19, -18, 18, 1100, 40, -40, 25 };

task aggro() {
	setFlywheelRange(3);
	TargetSpeed = 375;
	driveStraight(aggroConstants[0]); //drive toward first stack
	turn(right ? aggroConstants[1] : aggroConstants[2]); //turn toward net
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }

	setFlywheelRange(2);
	turn(right ? aggroConstants[3] : aggroConstants[4]); //turn toward second stack
	driveStraight(aggroConstants[5]); //drive toward second stack -- TODO: increase
	turn(right ? aggroConstants[6] : aggroConstants[7]); //turn toward net
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }

	setFlywheelRange(1);
	turn(right ? aggroConstants[8] : aggroConstants[9]); //turn toward third stack -- TODO: change stack (probably reverse)
	driveStraight(aggroConstants[10]); //drive toward third stack
	turn(right ? aggroConstants[11] : aggroConstants[12]); //turn toward stack/net
	driveStraight(aggroConstants[13]); //drive toward net
	simpleFire(); //fire();
	while (firing) { EndTimeSlice(); }
}

task autonomous() {
	initializeTasks(true);

	right = SensorValue[sidePot] < 1000;

	if (SensorValue[sidePot] > 1000 && SensorValue[sidePot] < 2800) {
		startTask(pskillz);
	}
	else {
		if (SensorValue[modePot] < 545) {
			startTask(aggro);
		}
		else if (SensorValue[modePot] < 1885) {
			startTask(classicAuto);
		}
		else if (SensorValue[modePot] < 3415) {
			startTask(hoardingAuto);
		}
		else {
			startTask(stationaryAuto);
		}
	}

	while (true) {
		motor[Fly1] = Flyspeed; //See Task PIDControl
		motor[Fly2] = Flyspeed;
		motor[Fly3] = Flyspeed;
		motor[Fly4] = Flyspeed;
		EndTimeSlice();
	}
}
