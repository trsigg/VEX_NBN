#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    chooResistor,   sensorReflection)
#pragma config(Sensor, in2,    cataResistor,   sensorReflection)
#pragma config(Sensor, dgtl3,  chooSwitch,     sensorDigitalIn)
#pragma config(Sensor, dgtl4,  feedSwitch,     sensorDigitalIn)
#pragma config(Sensor, I2C_1,  giraffeEncoder, sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_2,  leftEncoder,    sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_3,  rightEncoder,   sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Motor,  port1,           choo1,         tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           feedMe,        tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port3,           seymore,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           right1,        tmotorVex393_MC29, openLoop, driveRight, encoderPort, I2C_1)
#pragma config(Motor,  port5,           left1,         tmotorVex393_MC29, openLoop, reversed, driveLeft, encoderPort, I2C_2)
#pragma config(Motor,  port6,           left2,         tmotorVex393_MC29, openLoop, reversed, driveLeft)
#pragma config(Motor,  port7,           choo2,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           choo3,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           giraffe,       tmotorVex393_MC29, PIDControl, encoderPort, I2C_3)
#pragma config(Motor,  port10,          right2,        tmotorVex393_HBridge, openLoop, reversed, driveRight)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

enum catapultState { BLOCKING, STILL, MANUAL_OVERRIDE };
catapultState chooState = BLOCKING;

//load
bool startTasksAfterCompletion = true;
bool loadRunning = false;
//driveStraight
bool driveStraightRunning = false;
int clicks;
int drivePower;

bool feedToTopRunning = false; //feedToTop
bool cockCatapultRunning = false; //cockCatapult
bool continuousFire = false; //fire
bool firstStart = true; //userControl

//group 5
const TButtonMasks progressCataChooChooBtn = Btn5U;
const TButtonMasks manualOverrideBtn = Btn5D;
const TButtonMasks stopDCLfireBtn = Btn5U;
//group 6
const TButtonMasks feedUpBtn = Btn6U;
const TButtonMasks feedDownBtn = Btn6D;
//group 7
const TButtonMasks giraffeUpBtn = Btn7U;
const TButtonMasks giraffeDownBtn = Btn7D;
const TButtonMasks fullCourtBtn = Btn7L;
const TButtonMasks netBtn = Btn7R;
//group 8
const TButtonMasks continuousFireBtn = Btn8D;
const TButtonMasks fireOnceBtn = Btn8U;
const TButtonMasks loadBtn = Btn8L;
const TButtonMasks emergencyStopBtn = Btn8R;
const TButtonMasks startDCLfireBtnOne = Btn8R;
const TButtonMasks startDCLfireBtnTwo = Btn7L;

const int fireDuration = 750; //amount of time motors run during firing
const int stillSpeed = 15;
const int giraffeUpwardPower = 127;
const int giraffeDownwardPower = -80;
const int giraffeStillSpeed = 15;
const int startingSquare = 0;
const int fullCourt = 20;
const int net = -50;
const int resistorCutoff = 75;
const int feedBackwardTime = 250;
const int giraffeError = 2;

//set functions region
void setFeedPower(int power)
{
	motor[feedMe] = power;
	motor[seymore] = power;
}

void setChooPower(int power)
{
	motor[choo1] = power;
	motor[choo2] = power;
	motor[choo3] = power;
}

void setDrivePower(int right, int left)
{
	motor[right1] = right;
	motor[right2] = right;
	motor[left1] = left;
	motor[left2] = left;
}
//end set functions region

//robot tasks region
void fireFromCocking()
{
	setChooPower(127);
	while (SensorValue[chooSwitch] == 1) {}
	setChooPower(15);
}

void DCLfire() //sets both catapults to continuous fire mode
{
	setChooPower(127);
	while (vexRT[stopDCLfireBtn] == 0)
	{
		setDrivePower(vexRT[Ch2], vexRT[Ch3]);

		if (vexRT[giraffeUpBtn] == 1)
		{
			motor[giraffe] = giraffeUpwardPower;
		}
		else if (vexRT[giraffeDownBtn] == 1)
		{
			motor[giraffe] = giraffeDownwardPower;
		}
		else
		{
			motor[giraffe] = 0;
		}
	}
}

task driveStraight()
{
	driveStraightRunning = true;

	const int coeff = 5;
  int totalClicks = 0;
  int slavePower = drivePower - 5;
  int error = 0;

  SensorValue[leftEncoder] = 0;
  SensorValue[rightEncoder] = 0;

  while(abs(totalClicks) < clicks)
  {
    setDrivePower(drivePower, slavePower);

    error = SensorValue[leftEncoder] - SensorValue[rightEncoder];

    slavePower += error / coeff;

    totalClicks += SensorValue[leftEncoder];
    SensorValue[leftEncoder] = 0;
    SensorValue[rightEncoder] = 0;

    wait1Msec(100);
  }
  setDrivePower(0, 0);
  driveStraightRunning = false;
}
//end robot tasks region

//user control region
task giraffeControl()
{
	int giraffePower = giraffeStillSpeed;
	int giraffeTarget = startingSquare;

	while (true)
	{
		motor[giraffe] = giraffePower;

		while (SensorValue[giraffeEncoder] > (giraffeTarget - giraffeError) && SensorValue[giraffeEncoder] < (giraffeTarget + giraffeError) && vexRT[giraffeUpBtn] == 0 && vexRT[giraffeDownBtn] == 0 && vexRT[netBtn] == 0 && vexRT[fullCourtBtn] == 0) { EndTimeSlice(); }

		if (vexRT[giraffeUpBtn] == 1)
		{
			motor[giraffe] = giraffeUpwardPower;
			while (vexRT[giraffeUpBtn] == 1) { EndTimeSlice(); }
			giraffeTarget = SensorValue[giraffeEncoder];
		}
		else if (vexRT[giraffeDownBtn] == 1)
		{
			motor[giraffe] = giraffeDownwardPower;
			while (vexRT[giraffeDownBtn] == 1) { EndTimeSlice(); }
			giraffeTarget = SensorValue[giraffeEncoder];
		}
		else if (vexRT[fullCourtBtn] == 1 || vexRT[netBtn] == 1)
		{
			giraffeTarget = (vexRT[fullCourtBtn] == fullCourt) ? (fullCourt) : (net);
			while (vexRT[netBtn] == 0 && vexRT[fullCourtBtn] == 0) { EndTimeSlice(); }
		}
		else
		{
			if (SensorValue[giraffeEncoder] > giraffeTarget)
			{
				motor[giraffe] = -127;
				while (SensorValue[giraffeEncoder] > giraffeTarget && vexRT[giraffeUpBtn] == 0 && vexRT[giraffeDownBtn] == 0 && vexRT[netBtn] == 0 && vexRT[fullCourtBtn] == 0) { EndTimeSlice(); }
				motor[giraffe] = giraffeStillSpeed;
			}
			else
			{
				motor[giraffe] = 127;
				while (SensorValue[giraffeEncoder] < giraffeTarget && vexRT[giraffeUpBtn] == 0 && vexRT[giraffeDownBtn] == 0 && vexRT[netBtn] == 0 && vexRT[fullCourtBtn] == 0) { EndTimeSlice(); }
				motor[giraffe] = giraffeStillSpeed;
			}
		}
	}
}

task feedControl()
{
	while (true)
	{
		setFeedPower(0);
		while (vexRT[feedUpBtn] == 0 && vexRT[feedDownBtn] == 0) { EndTimeSlice(); }
		if (vexRT[feedUpBtn] == 1)
		{
			setFeedPower(127);
			while (vexRT[feedUpBtn] == 1) { EndTimeSlice(); }
		}
		else //feedDownBtn is pressed
		{
			setFeedPower(-127);
			while (vexRT[feedDownBtn] == 1) { EndTimeSlice(); }
		}
	}
}

task cataChooChoo()
{
	while (true)
	{
		switch (chooState)
		{
		case BLOCKING:
			setChooPower(0);

			while (vexRT[progressCataChooChooBtn] == 0 && vexRT[manualOverrideBtn] == 0) { EndTimeSlice(); }

			if (vexRT[progressCataChooChooBtn] == 1)
			{
				//cocks catapult
				setChooPower(127);
				while (SensorValue[chooSwitch] == 1) { EndTimeSlice(); }

				chooState = STILL;
			}
			else
			{
				chooState = MANUAL_OVERRIDE;
			}

		case STILL:
			setChooPower(stillSpeed);

			while (vexRT[progressCataChooChooBtn] == 0 && vexRT[manualOverrideBtn] == 0) { EndTimeSlice(); }

			if (vexRT[progressCataChooChooBtn] == 1)
			{
				//fires
				setChooPower(127);
				wait1Msec(fireDuration);
				chooState = BLOCKING;
			}
			else
			{
				chooState = MANUAL_OVERRIDE;
			}
			break;

		default: //chooState is MANUAL_OVERRIDE
			setChooPower(127);
			while (vexRT[manualOverrideBtn] == 1) { EndTimeSlice(); }
			chooState = STILL;
		}
	}
}
//end user control region

//autobehavior region
task cockCatapult()
{
	setChooPower((SensorValue[chooSwitch] == 1) ? (127) : (0));
	cockCatapultRunning = true;
	while (SensorValue[chooSwitch] == 1) { EndTimeSlice(); }
	setChooPower(stillSpeed);
	cockCatapultRunning = false;
}

task feedToTop()
{
	setFeedPower((SensorValue[feedSwitch] == 1) ? (127) : (0));
	feedToTopRunning = true;
	while (SensorValue[feedSwitch] == 1) { EndTimeSlice() ;}
	setFeedPower(0);
	feedToTopRunning = false;
}

task giraffeToTarget() //TODO: implement error correction using this function
{
	stopTask(giraffeControl);

	startTask(giraffeControl);
}

task load()
{
	loadRunning = true;

	stopTask(cataChooChoo);
	stopTask(feedControl);

	startTask(cockCatapult);
	startTask(feedToTop);
	while (cockCatapultRunning || feedToTopRunning) { EndTimeSlice(); }

	setFeedPower(127);

	while (SensorValue[chooResistor] > resistorCutoff) { EndTimeSlice(); }

	if (SensorValue[feedSwitch] == 0)
	{
		setFeedPower(-127);
		wait1Msec(feedBackwardTime);
		setFeedPower(0);
	}

	if (startTasksAfterCompletion)
	{
		startTask(cataChooChoo);
		startTask(feedControl);
	}
	else
	{
		startTasksAfterCompletion = true;
	}

	loadRunning = false;
}

task fire()
{
	do
	{
		startTasksAfterCompletion = false;
		startTask(load);

		while (loadRunning) { EndTimeSlice(); }

		setChooPower(127);
		wait1Msec(fireDuration);
		setChooPower(0);  //TODO: eliminate momentary stop?
	} while(continuousFire && vexRT[continuousFireBtn] == 0);

	startTask(cataChooChoo);
	startTask(feedControl);
	continuousFire = false;
}

task autoBehaviors()
{
	while (true)
	{
		while (vexRT[continuousFireBtn] == 0 && vexRT[fireOnceBtn] == 0 && vexRT[loadBtn] == 0 && (vexRT[startDCLfireBtnOne] == 0 || vexRT[startDCLfireBtnTwo] == 0)) { EndTimeSlice(); }

		if (vexRT[continuousFireBtn] == 1)
		{
			continuousFire = true;
			startTask(fire);
		}
		else if (vexRT[fireOnceBtn] == 1)
		{
			continuousFire = false;
			startTask(fire);
		}
		else if (vexRT[loadBtn] == 1)
		{
			startTasksAfterCompletion = true;
			startTask(load);
		}
		else if (vexRT[startDCLfireBtnOne] == 1 && vexRT[startDCLfireBtnTwo] == 1)
		{
			DCLfire();
		}
	}
}
//end autobehavior region

void emergencyStop()
{
	stopTask(giraffeControl);
	stopTask(feedControl);
	stopTask(cataChooChoo);
	stopTask(cockCatapult);
	stopTask(feedToTop);
	stopTask(giraffeToTarget);
	stopTask(fire);
	stopTask(autoBehaviors);

	startTask(usercontrol);
}

void pre_auton()
{
  bStopTasksBetweenModes = true;
  SensorValue[giraffeEncoder] = 0;
}

task autonomous()
{
	motor[giraffe] = giraffeStillSpeed;

  //cocks and fires initial preload
	setChooPower(127);
	while (SensorValue[chooSwitch] == 1) {}
	fireFromCocking();

	//feeds and fires three remaining balls
	for (int i = 0; i < 4; i++)
	{
		setFeedPower(127);

		clearTimer(T1);
		while (SensorValue[chooResistor] == 1 && time1[T1] < 3000) {}

		if (SensorValue[feedSwitch] == 0)
		{
			setFeedPower(-127);
			wait1Msec(250);
			setFeedPower(0);
		}

		fireFromCocking();
	}
}

task usercontrol()
{
	if (firstStart)
	{
		DCLfire();
	}
	firstStart = false;

	startTask(cataChooChoo);
	startTask(feedControl);
	startTask(autoBehaviors);
	startTask(giraffeControl);

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
