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

enum catapultState { REST, COCKING, STILL, FIRING, MANUAL_OVERRIDE };
catapultState chooState = REST;

enum autoBehavior { NONE, FEED, FIRE };
autoBehavior robotBehavior = NONE;

int fireDelay = 500; //amount of time between ball leaving feed and being fired during auto firing
int fireDuration = 750; //amount of time motors run during firing
int stillSpeed = 15;

bool feeding = false;
bool top = false;

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
	setChooSpeed(stillSpeed);
	setFeedSpeed(0);
	robotBehavior = NONE;
	chooState = STILL;
}

void feedControl()
{
	if (!(feeding))
	{
		if (vexRT[Btn6U] == 1)
		{
			setFeedSpeed(127);
		}
		else if (vexRT[Btn6D] == 1)
		{
			setFeedSpeed(-127);
		}
		else
		{
			setFeedSpeed(0);

			if (vexRT[Btn8L] == 1)
			{
				feeding = true;
				setFeedSpeed(127);
			}
		}
	}
	else
	{
		if (vexRT[Btn8R] == 1 || vexRT[Btn6U] == 1 || vexRT[Btn6D] == 1)
		{
			feeding = false;
		}
	}
}

void cataChooChoo()
{
	if (chooState == REST && vexRT[Btn5U] == 1) //TODO: incorporate into switch statement?
	{
		chooState = COCKING;
		setChooSpeed(127);
	}
	else if (chooState == COCKING && SensorValue[chooSwitch] == 0)
	{
		chooState = STILL;
		setChooSpeed(stillSpeed);
	}
	else if (chooState == STILL && vexRT[Btn5U] == 1)
	{
		chooState = FIRING;
		clearTimer(T2);
		setChooSpeed(127);
	}
	else if (chooState == FIRING && time1[T2] > fireDuration)
	{
		chooState = REST;
		setChooSpeed(0);
	}
	else if (chooState == MANUAL_OVERRIDE && vexRT[Btn5D] == 0)
	{
		chooState = STILL;
		setChooSpeed(stillSpeed);
	}

	if (vexRT[Btn5D] == 1)
	{
		chooState = MANUAL_OVERRIDE;
		setChooSpeed((vexRT[Btn7D] == 0) ? (127) : (-127));
	}
}

void fire()
{
	//controls feed
	if ( !(SensorValue[feedSwitch] == 0 && SensorValue[chooSwitch] == 1)) //not at top and uncocked
	{
		setFeedSpeed(127);
	}
	else
	{
		setFeedSpeed(0);
	}

	if (SensorValue[feedSwitch] == 0)
	{
		clearTimer(T3);
		top = true;
	}

	if (SensorValue[chooSwitch] == 1) //cocks catapult
	{
		setChooSpeed(127);
	}
	else if (SensorValue[feedSwitch] == 1 && time1[T3] > fireDelay && top) //fires
	{
		setFeedSpeed(0);
		setChooSpeed(127);
		chooState = FIRING;
		robotBehavior = NONE;
		top = false;
		clearTimer(T2);
	}
	else
	{
		setChooSpeed(stillSpeed);
	}
}

void feedToTop()
{
	if (SensorValue[feedSwitch] == 1)
	{
		setFeedSpeed(127);
	}
	else
	{
		setFeedSpeed(0);
		robotBehavior = NONE;
	}
}

void updateAutoBehavior()
{
	if (vexRT[Btn8D] == 1)
	{
		robotBehavior = FIRE;
	}
	else if (vexRT[Btn8U] == 1)
	{
		robotBehavior = FEED;
	}
}

void fireFromCocking()
{
	setChooSpeed(127);
	wait1Msec(750);
	while (SensorValue[chooSwitch] == 1) {}
	setChooSpeed(15);
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
		wait1Msec(750);
		setFeedSpeed(-127);
		wait1Msec(250);
		setFeedSpeed(0);
		fireFromCocking();
	}
}

task usercontrol()
{
	while (true)
	{
	 if (robotBehavior == NONE)
		{
			cataChooChoo();

			feedControl();

			updateAutoBehavior();
		}
		else
		{
			if (robotBehavior == FIRE)
			{
				fire();
			}
			else
			{
				feedToTop();
			}
		}

		if (vexRT[Btn7U] == 1 && vexRT[Btn7L] == 1)
		{
			emergencyStop();
		}

		setDriveSpeed(vexRT[Ch2], vexRT[Ch3]);
	}
}
