#pragma config(Sensor, dgtl3,  chooSwitch,     sensorDigitalIn)
#pragma config(Sensor, dgtl4,  wingusSwitch,   sensorDigitalIn)
#pragma config(Motor,  port1,           choo1,         tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port3,           dingus,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           right1,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           left1,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           left2,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port7,           choo2,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           wingus,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           feedMe,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          right2,        tmotorVex393_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

enum catapultState { REST, COCKING, READY, FIRING, MANUAL_OVERRIDE };
catapultState chooState = REST;

int chooSpeed;

void setFeedSpeed(int speed)
{
	motor[wingus] = speed;
	motor[dingus] = speed;
	motor[feedMe] = speed;
}

void setChooSpeed(int speed)
{
	motor[choo1] = speed;
	motor[choo2] = speed;
}

void setDriveSpeed(int right, int left)
{
	motor[right1] = right;
	motor[right2] = right;
	motor[left1] = left;
	motor[left2] = left;
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
	if (chooState == FIRING && SensorValue[chooSwitch] == 0 && time1[T2] > 750)
	{
		chooState = REST;
	}
	else if (chooState == REST && vexRT[Btn5U] == 1)
	{
		chooState = COCKING;
	}
	else if (chooState == COCKING && SensorValue[chooSwitch] == 0)
	{
		chooState = READY;
	}
	else if (chooState == READY && vexRT[Btn5U] == 1)
	{
		chooState = FIRING;
		clearTimer(T2);
	}
	else if (chooState == MANUAL_OVERRIDE && vexRT[Btn5D] == 0)
	{
		chooState = REST;
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
	case COCKING:
		chooSpeed = 127;
		break;
	case READY:
		chooSpeed = 20;
		break;
	case FIRING:
		chooSpeed = 127;
		break;
	case MANUAL_OVERRIDE:
		chooSpeed = 127;
		break;
	default:
		chooSpeed = 0;
	}

	setChooSpeed(chooSpeed);
}

void autonomous()
{
	for (int i = 0; i < 4; i++)
	{
		//cock and bring ball to top
		chooState = COCKING;
		while (!chooState == READY && SensorValue[wingusSwitch] == 1)
		{
			if (chooState != READY)
			{
				cataChooChoo();
			}
			if (SensorValue[wingusSwitch] == 1)
			{
				setFeedSpeed(127);
			}
			else
			{
				setFeedSpeed(0);
			}
		}

		//load ball
		while (SensorValue[wingusSwitch] == 0)
		{
			setFeedSpeed(127);
		}

		//fire
		chooState = FIRING;
		while (chooState != REST)
		{
			cataChooChoo();
		}
	}
}

task main()
{
	autonomous();

	while (true)
	{
		cataChooChoo();

		feedControl();

		setDriveSpeed(vexRT[Ch2], vexRT[Ch3]);
		}
	}
}
