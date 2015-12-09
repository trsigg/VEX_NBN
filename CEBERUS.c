#pragma platform(VEX)

/*
Right buttons should control feed
LU should control catapults while they are firing and should be continuous fire when flywheel is
LD should be fire once when flywheel is firing
*/

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

TButtonMasks btnToMonitor; //taskControl
int flywheelTargetSpeed; //flywheel and fire

const int minDrivePower = 20; //TO TEST
const int maxDrivePower = 127;
const int flywheelGearRatio = 100; //TO SET
const int kp = 1; //TO TUNE
const int ki = 1; //TO TUNE
const int kd = 1; //TO TUNE
const float firingErrorMargin = .05; //TO TUNE
const float bangBangErrorMargin = .1; //TO TUNE
const float integralMargin = .75; //TO TUNE
const int debounceTime = 750;

const TButtonMasks chooBtn = Btn6U; //TO SET
const TButtonMasks switchLauncherModesBtn = Btn8D; //TO SET
const TButtonMasks emergencyStopBtn = Btn7L; //TO SET
const TButtonMasks continuousFireBtn = Btn5U; //TO SET
const TButtonMasks fireOnceBtn = Btn5D; //TO SET
const TButtonMasks stopFireBtn = Btn6D; //TO SET

int limit(int input, int max, int min)
{
	if (abs(input) <= max && abs(input) >= min)
	{
		return input;
	}
	else
	{
		return ((abs(input) > max) ? (max * sgn(input)) : (min * sgn(input)));
	}
}

//set functions region
void setFeedPower(int power)
{
	motor[feedMe] = power;
	motor[seymore] = power;
}

void setLauncherPower(int power)
{
	motor[ce] = limit(power, 0, 127);
	motor[be] = limit(power, 0, 127);
	motor[ru] = limit(power, 0, 127);
	motor[s] = limit(power, 0, 127);
}

void setDrivePower(int right, int left)
{
	motor[right1] = right;
	motor[right2] = right;
	motor[left1] = left;
	motor[left2] = left;
}
//end set functions region

task cataChooChoo()
{
	while (true)
	{
		while (vexRT[chooBtn] == 0) { EndTimeSlice(); }
		setLauncherPower(127);
		while (vexRT[chooBtn] == 1) { EndTimeSlice(); }
		setLauncherPower(0);
	}
}

task fire()
{
	do
	{
		while (abs(flywheelTargetSpeed - /*velocity of flywheel motor*/ * gearRatio) < firingErrorMargin * flywheelTargetSpeed) { EndTimeSlice(); } //waits for flywheel to be within an acceptable range of the target speed --- TODO: find velocity function

		setFeedSpeed(127);

	} while (continuousFire);
}

task taskControl()
{
	while (true)
	{
		while (vexRT[continuousFireBtn] == 0 && vexRT[fireOnceBtn] == 0 && vexRT[stopFireBtn] == 0 && vexRT[switchLauncherModesBtn] == 0) { EndTimeSlice(); }

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
		else if (vexRT[stopFireBtn] == 1)
		{
			stopTask(fire);
			buttonToMonitor = stopFireBtn;
		}
		else //switchLauncherModesBtn is pressed
		{
			setLauncherPower(0);
			bMotorReflected[ce] = !bMotorReflected[ce];
			bMotorReflected[be] = !bMotorReflected[be];
			bMotorReflected[ru] = !bMotorReflected[ru];
			bMotorReflected[s] = !bMotorReflected[s];
		}

		wait1Msec(debounceTime);
	}
}

void pre_auton() { bStopTasksBetweenModes = true; }

void emergencyStop()
{
	stopTask(cataChooChoo);
	stopTask(taskControl);
	stopTask(waitForRelease);
	released = true;

	startTask(userControl);
}

task autonomous()
{

}

task usercontrol()
{
	startTask(cataChooChoo);
	startTask(taskControl);

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
