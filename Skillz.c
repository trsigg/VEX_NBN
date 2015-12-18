#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    chooResistor,   sensorReflection)
#pragma config(Sensor, in2,    two,            sensorPotentiometer)
#pragma config(Sensor, in3,    one,            sensorPotentiometer)
#pragma config(Sensor, in4,    sidePoten,      sensorPotentiometer)
#pragma config(Sensor, in5,    giraffeSetPoten, sensorPotentiometer)
#pragma config(Sensor, dgtl1,  chooSwitch,     sensorDigitalIn)
#pragma config(Sensor, dgtl2,  feedSwitch,     sensorDigitalIn)
#pragma config(Sensor, I2C_1,  giraffeEncoder, sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  rightEncoder,   sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_3,  leftEncoder,    sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           feedMe,        tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           seymore,       tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port3,           left1,         tmotorVex393_MC29, openLoop, encoderPort, I2C_3)
#pragma config(Motor,  port4,           left2,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           choo1,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           choo3,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           right1,        tmotorVex393_MC29, openLoop, reversed, encoderPort, I2C_2)
#pragma config(Motor,  port8,           right2,        tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           choo2,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          giraffe,       tmotorVex393_HBridge, openLoop, encoderPort, I2C_1)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

//cataChooChoo
enum catapultState { BLOCKING, STILL, MANUAL_OVERRIDE };
catapultState chooState = BLOCKING;
//load
bool startTasksAfterCompletion = true;
bool loadRunning = false;
//fire
bool continuousFire = false;
int shotsFired = 0;
int ballsToLoadAndFire;

bool feedToTopRunning = false; //feedToTop
bool cockCatapultRunning = false; //cockCatapult
bool continuousFeedRunning = false; //continuous feed
bool continuousCatapultRunning = false; //continuous catapult

//group 5
#define progressCataChooChooBtn Btn5U
#define manualOverrideBtn Btn5D
//group 6
#define feedUpBtn Btn6U
#define feedDownBtn Btn6D
//group 7
#define fireOnceBtn Btn7U
#define continuousFireBtn Btn7D
#define emergencyStopBtn Btn7L
#define loadBtn Btn7R
//group 8
#define giraffeUpBtn Btn8U
#define giraffeDownBtn Btn8D
#define continuousFeedBtn Btn8L
#define continuousCatapultBtn Btn8R

#define fireDuration 300 //amount of time motors run during firing
#define stillSpeed 15
#define giraffeUpwardPower 80
#define giraffeDownwardPower -60
#define giraffeStillSpeed 20
#define resistorCutoff 700
#define feedBackwardTime 250
#define coeff 5 //coefficient for driveStraight adjustments

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
void driveStraight(int clicks, int drivePower, int rightSign, int leftSign) //TODO: allow for turning
{
  int totalClicks = 0;
  int slavePower = drivePower - 5; //left is master, right is slave

  SensorValue[leftEncoder] = 0;
  SensorValue[rightEncoder] = 0;

  while(abs(totalClicks) < clicks)
  {
    setDrivePower(leftSign * drivePower, rightSign * slavePower);

    slavePower += (abs(SensorValue[leftEncoder]) - abs(SensorValue[rightEncoder])) / coeff;

    totalClicks += SensorValue[leftEncoder];
    SensorValue[leftEncoder] = 0;
    SensorValue[rightEncoder] = 0;

    wait1Msec(100);
  }
  setDrivePower(0, 0);
}
//end robot tasks region

//user control region
task giraffeControl()
{
	while (true)
	{
		motor[giraffe] = giraffeStillSpeed;
		while (vexRT[giraffeUpBtn] == 0 && vexRT[giraffeDownBtn] == 0) { EndTimeSlice(); }

		if (vexRT[giraffeUpBtn] == 1)
		{
			motor[giraffe] = giraffeUpwardPower;
			while (vexRT[giraffeUpBtn] == 1) {}
		}
		else //giraffeDownBtn is pressed
		{
			motor[giraffe] = giraffeDownwardPower;
			while (vexRT[giraffeDownBtn] == 1) {}
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

task load()
{
	loadRunning = true;

	stopTask(cataChooChoo);
	stopTask(feedControl);

	startTask(feedToTop);
	startTask(cockCatapult);
	while (cockCatapultRunning || feedToTopRunning) { EndTimeSlice(); }

	setFeedPower(127);

	while (SensorValue[chooResistor] < resistorCutoff) { EndTimeSlice(); }

	if (SensorValue[feedSwitch] == 0)
	{
		setFeedPower(-127);
		wait1Msec(feedBackwardTime);
	}
	setFeedPower(0);

	if (startTasksAfterCompletion)
	{
		startTask(cataChooChoo);
		startTask(feedControl);
	}
	else
	{
		startTasksAfterCompletion = true;
	}

	setChooPower(stillSpeed);
	chooState = STILL;
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
		shotsFired++;
	} while(continuousFire && vexRT[continuousFireBtn] == 0);

	setChooPower(0);

	startTask(cataChooChoo);
	startTask(feedControl);
	continuousFire = false;
}

task continuousFeed()
{
    stopTask(feedControl);
    continuousFeedRunning = true;
    setFeedPower(127);
    while(vexRT[continuousFeedBtn] == 1) { EndTimeSlice(); } //waits for button to be released
    while(vexRT[continuousFeedBtn] == 0) { EndTimeSlice(); }
    setFeedPower(0);
    startTask(feedControl);

    while(vexRT[continuousFeedBtn] == 1) { EndTimeSlice(); }
    continuousFeedRunning = false;
}

task continuousCatapult()
{
    stopTask(cataChooChoo);
    continuousCatapultRunning = true;
    setChooPower(127);
    while(vexRT[continuousCatapultBtn] == 1) { EndTimeSlice(); } //waits for button to be released
    while(vexRT[continuousCatapultBtn] == 0) { EndTimeSlice(); }
    setChooPower(0);
    startTask(cataChooChoo);

    while(vexRT[continuousCatapultBtn] == 1) { EndTimeSlice(); }
    continuousCatapultRunning = false;
}

task autoBehaviors()
{
	while (true)
	{
		while (vexRT[continuousFireBtn] == 0 && vexRT[fireOnceBtn] == 0 && vexRT[loadBtn] == 0 && vexRT[continuousFeedBtn] == 0 && vexRT[continuousCatapultBtn] == 0) { EndTimeSlice(); }

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
		else if (vexRT[continuousFeedBtn] == 1 && !continuousFeedRunning)
		{
			startTask(continuousFeed);
		}
		else if (vexRT[continuousCatapultBtn] == 1 && !continuousCatapultRunning)
		{
			startTask(continuousCatapult);
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
	stopTask(fire);
	stopTask(autoBehaviors);

	startTask(usercontrol);
}

void pre_auton()
{
  bStopTasksBetweenModes = true;
}

task autonomous()
{
	motor[giraffe] = giraffeStillSpeed;
	setChooPower(127);
	while (true) {}
}

task usercontrol()
{
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
