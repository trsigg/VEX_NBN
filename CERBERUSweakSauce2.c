#pragma config(Sensor, dgtl1,  flywheelEncoder, sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  feedSwitch,     sensorDigitalIn)
#pragma config(Motor,  port1,           ce,            tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           rb,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           er,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           us,            tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           rfdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           rbdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           lfdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           lbdrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           feedMe,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          seymore,       tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)
//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)
#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

#define maxAcc 50 //the maximum amount a motor's power value can be safely changed in a quarter second
#define fireErrorMargin .05 //percent error allowable in flywheel velocity for firing
#define velocitySampleTime 50 //number of milliseconds between sampling the flywheel velocity
#define gearRatio 1 //gear ratio between flywheelEncoder and flywheel

#define fireBtn Btn5U
#define continuousFireBtn Btn5D
#define stopFireBtn Btn5U
#define punchBtn Btn5U
#define feedInBtn Btn6U
#define feedOutBtn Btn6D
#define switchLauncherModesBtn Btn8L
#define emergencyStopBtn Btn8R

bool flywheelRunning = false;
bool velocityUpdated = false;
bool continuousFire = false;
float flywheelVelocity = 0;
float targetVelocity = 0;
int flywheelPower = 0;
int targetPower = 0;

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

bool shouldFire()
{
	if
}

void setFeedPower(int power)
{
	motor[feedMe] = power;
	motor[seymore] = power;
}

void setDrivePower(int right, int left)
{
	motor[rfdrive] = right;
	motor[rbdrive] = right;
	motor[lfdrive] = left;
	motor[lbdrive] = left;
}

void setLauncherPower(int power)
{
	int adjustedPower = limit(power, 0, 127) * (flywheelRunning ? 1 : -1);
	motor[ce] = adjustedPower;
	motor[rb] = adjustedPower;
	motor[er] = adjustedPower;
	motor[us] = adjustedPower;
	flywheelPower = adjustedPower;
}

task calcVelocity()
{
	while (true)
	{
		SensorValue[flywheelEncoder] = 0;
		wait1Msec(velocitySampleTime);
		flywheelVelocity = SensorValue[flywheelEncoder] * gearRatio / velocitySampleTime;
		velocityUpdated = true;
	}
}

task feedToTop() //loads a ball
{
	stopTask(feedControl);
	while (
}

task fireControl() //waits for ideal launch conditions and then fire
{
	while (true)
	{
		while (vexRT[continuousFireBtn] == 0 && vexRT[fireBtn] == 0) { EndTimeSlice(); }
		continuousFire = vexRT[continuousFireBtn] == 1;
		startTask(feedToTop);
		while (vexRT[fireBtn] == 1 || continuousFire && vexRT[stopFireBtn] == 1)
		{
			while (SensorValue[feedSwitch] == 1 && (continuousFire && vexRT[stopFireBtn] == 0 || vexRT[fireBtn] == 1))
			if (abs(targetVelocity - flywheelVelocity) < targetVelocity * fireErrorMargin && (continuousFire && vexRT[stopFireBtn] == 0 || vexRT[fireBtn] == 1) /*move to shouldFire along with continuousFire updating*/)
			{

			}
		}
		stopTask(feedToTop);
		startTask(feedControl);
	}
}

task feedControl() //
{

}

task puncher() //
{

}

task flywheel() //modulates motor powers to maintain constant flywheel velocity
{

}

task spinUpControl()
{
	while (targetPower == flywheelPower) { EndTimeSlice(); }
	while (targetPower - flywheelPower > maxAcc)
	{
		setLauncherPower(flywheelPower + );
		wait1Msec(250);
	}
}

task taskControl()
{

}

void initializeTasks()
{
	if (flywheelRunning)
	{
		startTask(flywheel);
		startTask(spinUpControl);
		startTask(fireControl);
		startTask(calcVelocity);
	}
	else
	{
		startTask(puncher);
	}
	startTask(taskControl);
	startTask(feedControl);
}

void emergencyStop()
{
	stopTask(flywheel);
	stopTask(puncher);
	stopTask(feedControl);
	stopTask(spinUpControl);
	stopTask(fireControl);
	stopTask(calcVelocity);
	stopTask(taskControl);
	stopTask(feedToTop);

	initializeTasks();
}

void pre_auton() { bStopTasksBetweenModes = true; }

task autonomous()
{
	AutonomousCodePlaceholderForTesting();
}

task usercontrol()
{
	initializeTasks();

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
