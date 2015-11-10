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
catapultState chooState = BLOCKING;

const int fireDuration = 750; //amount of time motors run during firing
const int stillSpeed = 15;

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
	while (SensorValue[chooSwitch] == 1) { EndTimeSlice(); }
	setChooSpeed(stillSpeed);
}

task feedToTop()
{
	setFeedSpeed((SensorValue[feedSwitch] == 1) ? (127) : (0));
	while (SensorValue[feedSwitch] == 1) { EndTimeSlice() ;}
	setFeedSpeed(0);
}

task feedControl()
{
	while (true)
	{
		setFeedSpeed(0);
		while (vexRT[Btn6U] == 0 && vexRT[Btn6D] == 0 && vexRT[Btn8R] == 0) { EndTimeSlice(); }
		if (vexRT[Btn6U] == 1)
		{
			setFeedSpeed(127);
			while (vexRT[Btn6U] == 1) { EndTimeSlice(); }
		}
		else if (vexRT[Btn6D] == 1)
		{
			setFeedSpeed(-127);
			while (vexRT[Btn6U] == 1) { EndTimeSlice(); }
		}
		else
		{
			setFeedSpeed(127);
			while (vexRT[Btn8R] == 1) { EndTimeSlice(); }
			while (vexRT[Btn8R] == 0) { EndTimeSlice(); }
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

			while (vexRT[Btn5U] == 0 && vexRT[Btn5D] == 0 && vexRT[Btn8R] == 0) { EndTimeSlice(); }

			if (vexRT[Btn5U] == 1)
			{
				//cocks catapult
				setChooSpeed(127);
				while (SensorValue[chooSwitch] == 1) { EndTimeSlice(); }

				chooState = STILL;
			}
			else if (vexRT[Btn8R] == 1)
			{
				chooState = MANUAL_OVERRIDE;
			}
			else
			{
				chooState = CONTINUOUS;
			}

		case STILL:
			setChooSpeed(stillSpeed);

			while (vexRT[Btn5U] == 0 && vexRT[Btn5D] == 0 && vexRT[Btn8R] == 0) { EndTimeSlice(); }

			if (vexRT[Btn5D] == 1)
			{
				//fires
				setChooSpeed(127);
				wait1Msec(fireDuration);
				chooState = BLOCKING;

			}
			else if (vexRT[Btn5D] == 0)
			{
				chooState = MANUAL_OVERRIDE;
			}
			else
			{
				chooState = CONTINUOUS;
			}
			break;

		case CONTINUOUS:
			setChooSpeed(127);
			while(vexRT[Btn8L] == 1) { EndTimeSlice(); } //waits for button to be released
			while(vexRT[Btn8L] == 0) { EndTimeSlice(); }
			chooState = BLOCKING;
			break;

		default:
			while (vexRT[Btn5D] == 1)
			{
				setChooSpeed((vexRT[Btn7D] == 0) ? (127) : (-127));
				initial7D = vexRT[Btn7D];
				while (vexRT[Btn5D] == 1 && vexRT[Btn7D] == initial7D) { EndTimeSlice(); }
			}
			chooState = STILL;
		}
	}
}

task fire()
{
	stopTask(cataChooChoo);
	stopTask(feedControl);

	startTask(cockCatapult);
	startTask(feedToTop);
	while (SensorValue[chooSwitch] == 1 || SensorValue[feedSwitch] == 1) { EndTimeSlice(); }

	setFeedSpeed(127);
	while (SensorValue[feedSwitch] == 0) { EndTimeSlice(); }
	wait1Msec(750);
	setFeedSpeed(-127);
	wait1Msec(250);
	setFeedSpeed(0);
	setChooSpeed(127);
	wait1Msec(fireDuration);
	setChooSpeed(0);

	startTask(cataChooChoo);
	startTask(feedControl);
}

task autoBehaviors()
{
	while (true)
	{
		while (vexRT[Btn8D] == 0 && vexRT[Btn8U] == 0) { EndTimeSlice(); }

		if (vexRT[Btn8D] == 1)
		{
			startTask(fire);
		}
		else
		{
			stopTask(cataChooChoo);
			stopTask(feedControl);

			startTask(feedToTop);

			startTask(cataChooChoo);
			startTask(feedControl);
		}
	}
}

void emergencyStop()
{
	stopTask(cataChooChoo);
	stopTask(feedControl);
	stopTask(fire);
	stopTask(feedToTop);
	stopTask(cockCatapult);

	startTask(cataChooChoo);
	startTask(feedControl);
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
	startTask(cataChooChoo);
	startTask(feedControl);
	startTask(autoBehaviors);

	while (true)
	{
		while (vexRT[Btn7U] == 0 || vexRT[Btn7L] == 0)
		{
			setDriveSpeed(vexRT[Ch2], vexRT[Ch3]);
		}

		emergencyStop();
	}
}
