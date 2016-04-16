#pragma config(Sensor, dgtl1,  Flywheel,       sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  LeftDrive,      sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  RightDrive,     sensorQuadEncoder)
#pragma config(Sensor, dgtl7,  IntakeThree,    sensorDigitalIn)
#pragma config(Motor,  port1,           Conveyor,      tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           FLDrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           BLDrive,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           TFly,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           TMFly,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           BFly,          tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           BMFly,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           FRDrive,       tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           BRDrive,       tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          Intake,        tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

/////////////////////////////////////////////////////////////////////////////////////////
//
//                          Pre-Autonomous Functions
//
// You may want to perform some actions before the competition starts. Do them in the
// following function.
//
/////////////////////////////////////////////////////////////////////////////////////////

void FlyPower(int Power)
{
	motor[TFly]  = Power;
	motor[TMFly] = Power;
	motor[BMFly] = Power;
	motor[BFly]  = Power;
}

float Velocity;
float MainBattery;
task BackgroundInfo()  // Calculates the speed of flywheel by dividing change in encoder value by change in time
// Calculates Battery voltage from the main battery
{
	clearTimer(T1);
	SensorValue[Flywheel] = 0;
	int FlyInitial;
	int FlyFinal;
	int TimeInitial;
	int TimeFinal;
	wait1Msec(1);
	while(true)
	{
		FlyInitial  = abs(SensorValue[Flywheel]);
		TimeInitial = time1(T1)/60;
		wait1Msec(60);
		FlyFinal  = abs(SensorValue[Flywheel]);
		TimeFinal = time1(T1)/60;
		Velocity  = (FlyFinal - FlyInitial)/(TimeFinal - TimeInitial);

		MainBattery = nAvgBatteryLevel;
	}
}

int Launch;
int ShortShot;
void FlyControl(int target, int hold) // Controls the speed of flywheel, goes either full speed or holding speed
{
	if(Velocity < target)
	{
		FlyPower(127);
	}
	else
	{
		if (Velocity < (target-10))
		{
			FlyPower(hold + 10);
		}
		else
		{
			FlyPower(hold);
		}
	}
	if(Velocity < .98*target && ShortShot == 1)
	{
		Launch = 0;
	}
	else if(Velocity < .98*target)
	{
		Launch = 0;
	}
	else
	{
		Launch = 1;
	}
}


void pre_auton()
{
	bStopTasksBetweenModes = true;
}

task autonomous()
{
	startTask(BackgroundInfo);
}

float Bks = .0000035;
float Bkm = .0000035;
float Bkl = .0000018;

task usercontrol()
{
	startTask(BackgroundInfo);
	float hold     = 10;
	int distance   = -10;
	ShortShot   = 0;
	Launch = 1;
	int Move = 0;
	while (true)
	{
		FlyControl(distance, hold);
		if (vexRT[Btn7R] == 1)
		{
			distance = -10;
			hold     = 10;
			//FlyPower(0);
		}
		else if (vexRT[Btn7D] == 1)
		{
			distance  = 135;
			hold      = 1/(Bks*MainBattery);
			ShortShot = 1;
			//FlyPower(50);
		}
		else if (vexRT[Btn7L] == 1)
		{
			distance = 270;
			hold     = 1/(Bkm*MainBattery);
			ShortShot = 0;
			//FlyPower(55);
		}
		else if (vexRT[Btn7U] == 1)
		{
			distance = 348;
			hold     = 1/(Bkl*MainBattery);
			ShortShot = 0;
			//FlyPower(60);
		}

		// Feed Control //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if (vexRT[Btn6U] == 1)
		{
			motor[Intake] = 127;
		}
		else if (vexRT[Btn6D] == 1)
		{
			motor[Intake] = -127;
		}
		else
		{
			motor[Intake] = 0;
		}

		if (vexRT[Btn5D] == 0) // Picking-up mode
		{
			if (/*SensorValue[IntakeOne] == 0 &&*/ SensorValue[IntakeThree] == 1)
			{
				Move = 1;
			}
			else if (/*SensorValue[IntakeTwo] == 0 && SensorValue[IntakeOne] == 1 ||*/ SensorValue[IntakeThree] == 0)
			{
				Move = 0;
			}

			if(Move == 1)
			{
				motor[Conveyor] = 127;
			}
			else
			{
				motor[Conveyor] = -10;
			}

		}
		else // Firing mode
		{
			if (Launch == 1)         // Fire
			{
				motor[Conveyor] = 127;
			}
			else                     // Hold
			{
				motor[Conveyor] = 0;
			}
		}
		// Drive Control ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		motor[BLDrive] = vexRT[Ch3];
		motor[FLDrive] = vexRT[Ch3];
		motor[BRDrive] = vexRT[Ch2];
		motor[FRDrive] = vexRT[Ch2];
	}
}
