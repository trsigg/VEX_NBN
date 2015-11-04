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
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

enum catapultState { REST, MOVING, STILL, MANUAL_OVERRIDE };
catapultState chooState = REST;

enum autoBehavior { NONE, FEED, FIRE };
autoBehavior robotBehavior = NONE;

int chooSpeed;

int fireDelay = 1000; //TODO: adjust?

int chooVal = SensorValue[chooSwitch];
int feedVal = SensorValue[feedSwitch];
int millis = time1[T2];

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
	setChooSpeed(0);
	setFeedSpeed(0);
	robotBehavior = NONE;
}

void feedControl()
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
	}
}

void cataChooChoo()
{
	if (chooState == REST && vexRT[Btn5U] == 1) //TODO: incorporate into switch statement
	{
		chooState = MOVING;
	}
	else if (chooState == STILL && vexRT[Btn5U] == 1)
	{
		chooState = MOVING;
		clearTimer(T2);
	}
	else if (chooState == MOVING && SensorValue[chooSwitch] == 0 && time1[T2] > 750)
	{
		chooState = STILL;
	}
	else if (chooState == MANUAL_OVERRIDE && vexRT[Btn5D] == 0)
	{
		chooState = STILL;
	}

	if (vexRT[Btn5D] == 1)
	{
		chooState = MANUAL_OVERRIDE;
	}

	switch (chooState)
	{
	case REST:
		chooSpeed = 0;
		break;
	case MOVING:
		chooSpeed = 127;
		break;
	case STILL:
		chooSpeed = 15;
		break;
	case MANUAL_OVERRIDE:
		chooSpeed = 127;
		break;
	default:
		chooSpeed = 0;
	}

	setChooSpeed(chooSpeed);
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
	}

	if (SensorValue[chooSwitch] == 1) //cocks catapult
	{
		setChooSpeed(127);
	}
	else if (SensorValue[feedSwitch] == 1 && time1[T3] > fireDelay) //fires TODO: add a timer? and change delay
	{
		chooState = MOVING;
		robotBehavior = NONE;
		setFeedSpeed(0);
		setChooSpeed(127);
		clearTimer(T2);
	}
	else
	{
		setChooSpeed(0);
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
	}
}

void updateAutoBehavior()
{
	if (vexRT[Btn7D] == 1)
	{
		robotBehavior = NONE;
	}
	else if (vexRT[Btn8D] == 1)
	{
		robotBehavior = FIRE;
	}
	else if (vexRT[Btn7U] == 1)
	{
		robotBehavior = FEED;
	}
}

void executeAutoBehavior()
{
	switch (robotBehavior)
	{
	case FIRE:
		fire();
		break;
	case FEED:
		feedToTop();
	default:
		robotBehavior = NONE;
	}
}

task main()
{

	while (true)
	{
		chooVal = SensorValue[chooSwitch];
		feedVal = SensorValue[feedSwitch];
		millis = time1[T2];

		updateAutoBehavior();

		if (robotBehavior == NONE)
		{
			cataChooChoo();

			feedControl();
		}
		else
		{
			executeAutoBehavior();
		}

		if (vexRT[Btn7U] == 1 && vexRT[Btn7L] == 1)
		{
			emergencyStop();
		}

		setDriveSpeed(vexRT[Ch2], vexRT[Ch3]);
	}
}
