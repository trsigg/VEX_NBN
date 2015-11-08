#pragma config(Sensor, dgtl3,  chooSwitch,     sensorDigitalIn)
#pragma config(Sensor, dgtl4,  feedSwitch,     sensorDigitalIn)
#pragma config(Motor,  port1,           choo1,         tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           feedMe,        tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port3,           seymore,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           right1,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           left1,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           left2,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port7,           choo2,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           choo3,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          right2,        tmotorVex393_HBridge, openLoop, reversed)

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

enum catapultState { BLOCKING, STILL, MANUAL_OVERRIDE, CONTINUOUS };
catapultState chooState = REST;

const int fireDelay = 500; //amount of time between ball leaving feed and being fired during auto firing
const int fireDuration = 750; //amount of time motors run during firing
const int stillSpeed = 15;

bool autoBehaviorRunning = false;

void setFeedSpeed(int speed)
{
	motor[feedMe] = speed;
	motor[seymore] = speed;
}

void setChooSpeed(int speed)
{
	motor[choo1] = speed;
	motor[choo2] = speed;
	motor[choo3] = speed;
}

void setDriveSpeed(int right, int left)
{
	motor[right1] = right;
	motor[right2] = right;
	motor[left1] = left;
	motor[left2] = left;
}

void emergencyStop()
{
	StopTask(cataChooChoo);
	StopTask(feedControl);
	StopTask(fire);
	StopTask(feedToTop);
	StopTask(cockCatapult);

	StartTask(cataChooChoo);
	StartTask(feedControl)
}

void fireFromCocking()
{
	setChooSpeed(127);
	wait1Msec(fireDuration);
	while (SensorValue[chooSwitch] == 1) {}
	setChooSpeed(15);
}

task cockCatapult()
{
	setChooSpeed((SensorValue[chooSwitch] == 1) ? (127) : (0));
	while (SensorValue[chooSwitch] == 1) {}
	setChooSpeed(stillSpeed);
}

task feedToTop()
{
	setFeedSpeed((SensorValue[feedSwitch] == 1) ? (127) : (0));
	while (SensorValue[feedSwitch] == 1) {}
	seFeedSpeed(0);
}

task fire()
{
	StopTask(cataChooChoo);
	StopTask(feedControl);

	StartTask(cockCatapult);
	StartTask(feedToTop);
	while (chooSwitch == 1 || feedSwitch == 1) {}

	setFeedSpeed(127);
	while (SensorValue[feedSwitch] == 0) {}
	wait1Msec()

	StartTask(cataChooChoo);
	StartTask(feedControl)
}

task autoBehaviors()
{
	while (true)
	{
		while (vexRT[Btn8D] == 0 && vexRT[Btn8U] == 0) {}

		if (vexRT[Btn8D] == 1)
		{
			StartTask(fire);
		}
		else
		{
			StopTask(cataChooChoo);
			StopTask(feedControl);

			StartTask(feedToTop);

			StartTask(cataChooChoo);
			StartTask(feedControl);
		}
	}
}

task feedControl()
{
	while (true)
	{
		setFeedSpeed(0);
		while (vexRT[Btn6U] == 0 && vexRT[Btn6D] == 0 && vexRT[Btn8R] == 0) {}
		if (vexRT[Btn6U] == 1)
		{
			setFeedSpeed(127);
			while (vexRT[Btn6U] == 1) {}
		}
		else if (vexRT[Btn6D] == 1)
		{
			setFeedSpeed(-127);
			while (vexRT[Btn6U] == 1) {}
		}
		else
		{
			setFeedSpeed(127);
			while (vexRT[Btn8R] == 1) {}
			while (vexRT[Btn8R] == 0) {}
		}
	}
}

task cataChooChoo()
{
	int initial7D = vexRT[Btn7D];

	while (true)
	{
		switch (chooState)
		{
		case BLOCKING:			
			setChooSpeed(0);

			while (vexRT[Btn5U] == 0 && vexRT[Btn5D] == 0 && vexRT[Btn8R] == 0) {}

			if ([Btn5U] == 1)
			{
				//cocks catapult
				setChooSpeed(127);
				while (SensorValue[chooSwitch] == 1) {}

				chooState = STILL;
			}
			else if (vexRT[Btn8R] == 1)
			{
				chooState = MANUAL_OVERRIDE
			}
			else
			{
				chooState = CONTINUOUS;
			}

		case STILL:
			setChooSpeed(stillSpeed);

			while (vexRT[Btn5U] == 0 && vexRT[Btn5D] == 0 && vexRT[Btn8R] == 0) {}

			if ([Btn5D] == 1)
			{
				//fires
				setChooSpeed(127);
				wait1Msec(fireDuration);
				chooState = BLOCKING;

			}
			else if (vexRT[Btn5D] == 0)
			{
				chooState = MANUAL_OVERRIDE
			}
			else
			{
				chooState = CONTINUOUS;
			}
			break;

		case CONTINUOUS:
			setChooSpeed(127);
			while(vexRT[Btn8L] == 1) {} //waits for button to be released
			while(vexRT[Btn8L] == 0) {}
			chooState = BLOCKING;
			break;

		default:
			while (vexRT[Btn5D] == 1)
			{
				setChooSpeed((vexRT[Btn7D] == 0) ? (127) : (-127));
				initial7D = vexRT[Btn7D];
				while (vexRT[Btn5D] == 1 && vexRT[Btn7D] == initial7D) {}
			}
			chooState = STILL;
		}
	}
}

void pre_auton()
{
  bStopTasksBetweenModes = true;
}

task autonomous()
{
  //cocks and fires initial preload
	setChooSpeed(127);
	while (SensorValue[chooSwitch] == 1) {}
	fireFromCocking();

	//feeds and fires three remaining balls
	for (int i = 0; i < 4; i++)
	{
		setFeedSpeed(127);
		clearTimer(T1);
		while (SensorValue[feedSwitch] == 1 && time1[T1] < 3000) {}
		while (SensorValue[feedSwitch] == 0 && time1[T1] < 1000) {}
		wait1Msec(fireDuration);
		setFeedSpeed(-127);
		wait1Msec(250);
		setFeedSpeed(0);
		fireFromCocking();
	}
}

task usercontrol()
{
	StartTask(cataChooChoo);
	StartTask(feedControl);
	StartTask(autoBehaviors);

	while (true)
	{
		while (vexRT[Btn7U] == 0 || vexRT[Btn7L] == 0)
		{
			setDriveSpeed(vexRT[Ch2], vexRT[Ch3]);
		}

		emergencyStop();
	}
}
