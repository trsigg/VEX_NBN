#pragma config(Sensor, dgtl1,  giraffeEncoder, sensorQuadEncoder)
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
#pragma config(Motor,  port9,           giraffe,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          right2,        tmotorVex393_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

enum catapultState { BLOCKING, STILL, MANUAL_OVERRIDE };
catapultState chooState = BLOCKING;

int giraffeTarget;

const TButtonMasks progressCataChooChooBtn = Btn6U;
const TButtonMasks manualOverrideBtn = Btn6D;
const TButtonMasks feedUpBtn = Btn6U;
const TButtonMasks feedDownBtn = Btn6D;
const TButtonMasks fireBtn = Btn8D;
const TButtonMasks feedToTopBtn = Btn8U;
const TButtonMasks continuousFeedBtn = Btn8L;
const TButtonMasks continuousCatapultBtn = Btn8R;
const TButtonMasks emergencyStopBtnOne = Btn7L;
const TButtonMasks emergencyStopBtnTwo = Btn7U;
const TButtonMasks reverseManualOverrideBtn = Btn7D;
const TButtonMasks giraffeUpBtn = Btn7U;
const TButtonMasks giraffeDownBtn = Btn7D;
const TButtonMasks fullCourtBtn = Btn7L;
const TButtonMasks netBtn = Btn7R;

const int fireDuration = 750; //amount of time motors run during firing
const int stillSpeed = 15;
const int buttonDelay = 750;
const int giraffeUpwardSpeed = 100;
const int giraffeDownwardSpeed = -80;
const int startingSquare = 70;
const int fullCourt = 80;
const int net = 40;

//set functions region
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
//end set functions region

void fireFromCocking()
{
	setChooSpeed(127);
	wait1Msec(fireDuration);
	while (SensorValue[chooSwitch] == 1) {}
	setChooSpeed(15);
}

//user control region
task giraffeControl()
{
	while(true)
	{
		while (true)
		{
			motor[giraffe] = 0;
			while (vexRT[giraffeUpBtn] == 0 && vexRT[giraffeDownBtn] == 0 && vexRT[netBtn] == 0 && vexRT[fullCourtBtn] == 0) { EndTimeSlice(); }
			if (vexRT[giraffeUpBtn] == 1)
			{
				motor[giraffe] = giraffeUpwardSpeed;
				while (vexRT[giraffeUpBtn] == 1) { EndTimeSlice(); }
			}
			else if (vexRT[giraffeDownBtn] == 1)
			{
				motor[giraffe] = giraffeDownwardSpeed;
				while (vexRT[giraffeDownBtn] == 1) { EndTimeSlice(); }
			}
			/*else //net or fullCourt Btns are pressed
			{
				giraffeTarget = (vexRT[fullCourtBtn] == fullCourt) ? (fullCourt) : (net);
				startTask(giraffeToTarget);
			}*/
		}
	}
}

task feedControl()
{
	while (true)
	{
		setFeedSpeed(0);
		while (vexRT[feedUpBtn] == 0 && vexRT[feedDownBtn] == 0) { EndTimeSlice(); }
		if (vexRT[feedUpBtn] == 1)
		{
			setFeedSpeed(127);
			while (vexRT[feedUpBtn] == 1) { EndTimeSlice(); }
		}
		else //feedDownBtn is pressed
		{
			setFeedSpeed(-127);
			while (vexRT[feedDownBtn] == 1) { EndTimeSlice(); }
		}
	}
}

task cataChooChoo()
{
	int initialReverse = vexRT[reverseManualOverrideBtn];

	while (true)
	{
		switch (chooState)
		{
		case BLOCKING:
			setChooSpeed(0);

			while (vexRT[progressCataChooChooBtn] == 0 && vexRT[manualOverrideBtn] == 0) { EndTimeSlice(); }

			if (vexRT[progressCataChooChooBtn] == 1)
			{
				//cocks catapult
				setChooSpeed(127);
				while (SensorValue[chooSwitch] == 1) { EndTimeSlice(); }

				chooState = STILL;
			}
			else
			{
				chooState = MANUAL_OVERRIDE;
			}

		case STILL:
			setChooSpeed(stillSpeed);

			while (vexRT[progressCataChooChooBtn] == 0 && vexRT[manualOverrideBtn] == 0) { EndTimeSlice(); }

			if (vexRT[manualOverrideBtn] == 1)
			{
				//fires
				setChooSpeed(127);
				wait1Msec(fireDuration);
				chooState = BLOCKING;
			}
			else
			{
				chooState = MANUAL_OVERRIDE;
			}
			break;

		default: //chooState is MANUAL_OVERRIDE
			while (vexRT[manualOverrideBtn] == 1)
			{
				setChooSpeed((vexRT[reverseManualOverrideBtn] == 0) ? (127) : (-127));
				initialReverse = vexRT[reverseManualOverrideBtn];
				while (vexRT[manualOverrideBtn] == 1 && vexRT[reverseManualOverrideBtn] == initialReverse) { EndTimeSlice(); }
			}
			chooState = STILL;
		}
	}
}
//end user control region

//autobehavior region
task continuousCatapult()
{
	stopTask(cataChooChoo);
	setChooSpeed(127);
	while(vexRT[continuousCatapultBtn] == 1) { EndTimeSlice(); } //waits for button to be released
	while(vexRT[continuousCatapultBtn] == 0) { EndTimeSlice(); }
	setChooSpeed(0);
	startTask(cataChooChoo);
}

task continuousFeed()
{
	stopTask(feedControl);
	setFeedSpeed(127);
	while(vexRT[continuousFeedBtn] == 1) { EndTimeSlice(); } //waits for button to be released
	while(vexRT[continuousFeedBtn] == 0) { EndTimeSlice(); }
	setChooSpeed(0);
	startTask(feedControl);
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

task giraffeToTarget() //TODO: implement error correction using this function
{
	stopTask(giraffeControl);
	while (SensorValue[giraffeEncoder] != giraffeTarget)
	{
		if (SensorValue[giraffeEncoder] > giraffeTarget)
		{
			motor[giraffe] = -127;
			while(SensorValue[giraffeEncoder] > giraffeTarget) { EndTimeSlice(); }
			motor[giraffe] = 0;
		}
		else
		{
			motor[giraffe] = 127;
			while(SensorValue[giraffeEncoder] < giraffeTarget) { EndTimeSlice(); }
			motor[giraffe] = 0;
		}
	}
	startTask(giraffeControl);
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
		while (vexRT[fireBtn] == 0 && vexRT[feedToTopBtn] == 0 && ((vexRT[continuousCatapultBtn] == 0 || vexRT[continuousFeedBtn] == 0) && time1) { EndTimeSlice(); }

		if (vexRT[fireBtn] == 1)
		{
			startTask(fire);
		}
		else if (vexRT[feedToTopBtn] == 1)
		{
			stopTask(cataChooChoo);
			stopTask(feedControl);

			startTask(feedToTop);

			startTask(cataChooChoo);
			startTask(feedControl);
		}
		else if (vexRT[continuousCatapultBtn] == 1)
		{
			startTask(continuousCatapult);
		}
		else //continuous feed button is pressed
		{
			startTask(continuousFeed);
		}
	}
}
//end autobehavior region

void emergencyStop()
{
	stopAllTasks();

	startTask(usercontrol);
	startTask(autoBehaviors);
	startTask(giraffeControl);
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
	startTask(giraffeControl);

	while (true)
	{
		while (vexRT[emergencyStopBtnOne] == 0 || vexRT[emergencyStopBtnTwo] == 0)
		{
			setDriveSpeed(vexRT[Ch2], vexRT[Ch3]);
			EndTimeSlice();
		}

		emergencyStop();
	}
}
