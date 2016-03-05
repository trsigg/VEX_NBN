#pragma config(Sensor, in1,    pot,            sensorPotentiometer)
#pragma config(Sensor, in2,    gyro,           sensorGyro)
#pragma config(Sensor, dgtl1,  flywheelEncoder, sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  leftEncoder,    sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  rightEncoder,   sensorQuadEncoder)
#pragma config(Sensor, dgtl9,  flywheelSwitch, sensorDigitalIn)
#pragma config(Sensor, dgtl10, feedSwitch,     sensorDigitalIn)
#pragma config(Motor,  port1,           ce,            tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           rb,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           er,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           us,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           rfdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           rbdrive,       tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port7,           lfdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           lbdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           feedMe,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          seymore,       tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#define sampleTime 25 //number of milliseconds between sampling the flywheel velocity and control adjustments in flywheel task
#define notFiringCutoff 15 //maximum error value considere not firing
//PID constants
#define Kp 4.0
#define Ki 0.001
#define Kd 1.7

#define fireBtn Btn5U
#define seymoreOutBtn Btn5D
#define feedInBtn Btn6U
#define feedOutBtn Btn6D
#define emergencyStopBtn Btn8R

#define driveTimer T1
#define firingTimer T2

//waitUntilNotFiring
int initialWait, timeWithoutFiring;
bool firing = false;
//driveStraight
bool driveStraightRunning = false;
int clicks, rightDirection, leftDirection, drivePower, delayAtEnd, timeout;
//turn
float degreesToTurn;
int maxTurnSpeed, waitAtEnd;
//flywheel variables
int flywheelVelocity = 0;
int targetVelocity = 0;
int flywheelPower = 0;
int defaultPower = 0;
float Integral  = 0;
float Error = 0;

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

void setLauncherPower(int power) {
	flywheelPower = limit(power, 0, 127);
	motor[ce] = flywheelPower;
	motor[rb] = flywheelPower;
	motor[er] = flywheelPower;
	motor[us] = flywheelPower;
}

void setFlywheelRange(int range) {
	int velocities[5] = {0, 215, 300, 183, 191};
	int defaultPowers[5] = {0, 60, 96, 65, 68};

	Integral = 0;
	targetVelocity = velocities[limit(range, 0, 4)];
	defaultPower = defaultPowers[limit(range, 0, 4)];
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

void turn(float _degreesToTurn_, int _maxTurnSpeed_=40, bool runAsTask=false, int _waitAtEnd_=250) {
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

	int coeff = 5;
	int totalClicks = 0;
	int slavePower = drivePower;
	int error = 0;

	SensorValue[leftEncoder] = 0;
	SensorValue[rightEncoder] = 0;
	SensorValue[gyro] = 0;
	clearTimer(driveTimer);

	while (abs(totalClicks) < clicks  && time1(driveTimer) < timeout)
	{
		setDrivePower(drivePower * leftDirection, slavePower * rightDirection);

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

int totalClicks;

void driveStraight(int _clicks_, int _leftDirection_, int _rightDirection_, int _drivePower_, bool startAsTask=false, int _delayAtEnd_=250, int _timeout_=15000) {
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
		totalClicks = 0;
		int slavePower = drivePower;
		int error = 0;

		SensorValue[leftEncoder] = 0;
		SensorValue[rightEncoder] = 0;
		SensorValue[gyro] = 0;
		clearTimer(driveTimer);

		while (abs(totalClicks) < clicks  && time1(driveTimer) < timeout)
		{
			setDrivePower(drivePower * leftDirection, slavePower * rightDirection);

			error = SensorValue[gyro];

			slavePower += error / coeff;

			totalClicks += abs(SensorValue[leftEncoder];
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
	wait1Msec(initialWait);
	clearTimer(firingTimer);

	while (time1(firingTimer) < timeWithoutFiring) {
		if (Error > notFiringCutoff) clearTimer(firingTimer);
	}
	firing = false;
}

void waitUntilNotFiring(int _initialWait_, int _timeWithoutFiring_=1000) {
	initialWait = _initialWait_;
	timeWithoutFiring = _timeWithoutFiring_;
	startTask(waitUntilNotFiringTask);
}
//end autonomous region

//begin user input region
task flywheel() {
	TVexJoysticks buttons[5] = { Btn8D, Btn7U, Btn7R, Btn7D, Btn7L }; //creating a pseudo-hash associating buttons with velocities and default motor powers

	while (true)
	{
		for (int i = 0; i < 5; i++)
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
void initializeTasks() {
	startTask(flywheel);
	startTask(flywheelStabilization);
}

void emergencyStop() {
	stopTask(flywheel);
	stopTask(flywheelStabilization);

	initializeTasks();
}
//end task control region

//task autonomous() {
//	//start flywheel
//	initializeTasks();
//	setFlywheelRange(0);

//	wait1Msec(1000);
//	motor[seymore] = 127; //start feed
//	//wait until first set of preloads are fired
//	waitUntilNotFiring(2000);
//	while (firing) { EndTimeSlice(); }
//	motor[seymore] = 0;

//	turn(93); //turn toward other starting tile
//	driveStraight(8000, 1, 1, 60); //drive across field
//	autonProgress = 1;
//	turn(-95); // turn toward net
//	autonProgress = 2;

//	//fire remaining balls
//	motor[seymore] = 127;
//	while (true) { EndTimeSlice(); }
//}

task usercontrol() {
	initializeTasks();
	setFlywheelRange(0);

	while (true)
	{
		while (vexRT[emergencyStopBtn] == 0)
		{
			setDrivePower(sgn(vexRT[Ch2]) * vexRT[Ch2] * vexRT[Ch2] / 127, sgn(vexRT[Ch3]) * vexRT[Ch3] * vexRT[Ch3] / 127); //drive
			motor[seymore] = 127*vexRT[fireBtn] - 127*vexRT[seymoreOutBtn];
			motor[feedMe] = 127*vexRT[feedInBtn] - 127*vexRT[feedOutBtn];
			EndTimeSlice();
		}

		emergencyStop(); //reassign emstop button
	}
}

int autonProgress;

task main () {
	//start flywheel
	initializeTasks();
	setFlywheelRange(0);

	wait1Msec(1000);
	motor[seymore] = 127; //start feed
	//wait until first set of preloads are fired
	waitUntilNotFiring(2000);
	while (firing) { EndTimeSlice(); }
	motor[seymore] = 0;

	turn(93); //turn toward other starting tile
	driveStraight(4000, 1, 1, 60); //drive across field
	autonProgress = 1;
	turn(-95); // turn toward net
	autonProgress = 2;

	//fire remaining balls
	motor[seymore] = 127;
	while (true) { EndTimeSlice(); }
}
