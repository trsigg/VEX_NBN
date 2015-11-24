#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    potLeft,        sensorPotentiometer)
#pragma config(Sensor, in2,    potRight,       sensorPotentiometer)
#pragma config(Sensor, in3,    autoPot2,       sensorPotentiometer)
#pragma config(Sensor, in4,    autoPot3,       sensorPotentiometer)
#pragma config(Sensor, in5,    autoPot,        sensorPotentiometer)
#pragma config(Sensor, dgtl1,  latch,          sensorDigitalOut)
#pragma config(Sensor, dgtl2,  Claw,           sensorDigitalOut)
#pragma config(Sensor, dgtl3,  brake,          sensorDigitalOut)
#pragma config(Sensor, dgtl4,  conveyer,       sensorDigitalOut)
#pragma config(Sensor, dgtl5,  rightQuad,      sensorQuadEncoder)
#pragma config(Sensor, dgtl7,  leftQuad,       sensorQuadEncoder)
#pragma config(Sensor, I2C_1,  leftLifttEncoder, sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Sensor, I2C_2,  rightLiftEncoder, sensorQuadEncoderOnI2CPort,    , AutoAssign)
#pragma config(Motor,  port1,           leftWheelFront, tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port2,           leftWheelBack, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           leftArmTop,    tmotorVex393_MC29, openLoop, reversed, encoderPort, I2C_1)
#pragma config(Motor,  port4,           leftFeed,      tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           rightArmBottom, tmotorVex393_MC29, openLoop, encoderPort, I2C_2)
#pragma config(Motor,  port6,           rightFeed,     tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port7,           leftArmBottom, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           rightArmTop,   tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           rightWheelBack, tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          rightWheelFront, tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//
#pragma DebuggerWindows("Motors")
#pragma DebuggerWindows("Sensors")
#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(25)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

/////////////////////////////////////////////////////////////////////////////////////////
//
//                          Pre-Autonomous Functions
//
// You may want to perform some actions before the competition starts. Do them in the
// following function.
//
/////////////////////////////////////////////////////////////////////////////////////////



//--------------------------------------autonomous functions----------------------------------------
const int stillDown = 15;
const int stillUp = 15;


void liftMotors (int motorSpeed)
{
	motor[leftArmTop]     = motorSpeed;
	motor[leftArmBottom]  = motorSpeed;
	motor[rightArmTop]    = motorSpeed;
	motor[rightArmBottom] = motorSpeed;
}
void leftDrive( int driveSpeed)
{
	motor[leftWheelFront] = driveSpeed;
	motor[leftWheelBack] = driveSpeed;
}

void rightDrive(int driveSpeed)
{
	motor[rightWheelFront] = driveSpeed;
	motor[rightWheelBack] = driveSpeed;
}

void driveMotors (int driveSpeed)
{
	leftDrive(driveSpeed);
	rightDrive(driveSpeed);
}

void armDown(int target, int speed)
{
	while(SensorValue[rightLiftEncoder] > target)
	{
		liftMotors (-speed);
	}
	liftMotors (stillDown);
}


void armUp(int target, int speed)
{
	while(SensorValue[rightLiftEncoder] < target)
	{
		liftMotors (speed);
	}
	liftMotors (stillUp);
}


void autoGrab(int val)
{
	SensorValue[Claw] = val;
}

void driveBack (int rSpeed, int lSpeed, int rot)
{
	SensorValue[leftQuad] = 0;
	SensorValue[rightQuad] = 0;
	while(SensorValue[leftQuad] > rot)
	{
		motor[leftWheelFront] = lSpeed;
		motor[leftWheelBack] = lSpeed;
		motor[rightWheelFront] = rSpeed;
		motor[rightWheelBack] = rSpeed;
	}
	motor[leftWheelFront] = lSpeed*-.08;
	motor[leftWheelBack] = lSpeed*-.08;
	motor[rightWheelFront] = rSpeed*-.08;
	motor[rightWheelBack] = rSpeed*-.08;

	wait1Msec(100);
	driveMotors(0);
}

void driveForward (int rSpeed, int lSpeed, int rot)
{
	SensorValue[leftQuad] = 0;
	SensorValue[rightQuad] = 0;
	while(abs(SensorValue [leftQuad]) < rot)
	{
		motor[leftWheelFront] = lSpeed;
		motor[leftWheelBack] = lSpeed;
		motor[rightWheelFront] = rSpeed;
		motor[rightWheelBack] = rSpeed;
	}
	motor[leftWheelFront] = lSpeed*-.08;
	motor[leftWheelBack] = lSpeed*-.08;
	motor[rightWheelFront] = rSpeed*-.08;
	motor[rightWheelBack] = rSpeed*-.08;

	wait1Msec(100);
	driveMotors(0);
}

void stopRobot(const int mTime)
{
	driveMotors(0);
	motor[leftFeed] = 0;
	motor[rightFeed] = 0;
	wait1Msec(mTime);
}

void Feed(int speed)
{
	motor[leftFeed] = speed;
	motor[rightFeed] = speed;
	}

void autoFeed(int mTime, int speed)
{
		Feed(speed);
		wait1Msec(mTime);
		Feed(0);
}


void DriveLoop(int rightDirection, int leftDirection, int Target, float kP, float kI, float kD, float max, float min)
{
	int rightlasterror, righterror, rightintegral, rightdrive;
	int leftlasterror, lefterror, leftintegral, leftdrive;
	int lastdifference, difference;
	int errorwait = 0;
	SensorValue(rightQuad) = 0;
	SensorValue(leftQuad)  = 0;

	while(errorwait == 0)
	{
		rightlasterror = righterror;
		righterror = (Target - abs(SensorValue[rightQuad]));
		rightintegral = rightintegral + righterror;
		leftlasterror = lefterror;
		lefterror = (Target - abs(SensorValue[leftQuad]));
		leftintegral = leftintegral + lefterror;
		lastdifference = difference;
		difference = (abs(SensorValue[leftQuad]) - abs(SensorValue[rightQuad]));

		rightdrive = (kP*righterror + kI*rightintegral + kD*rightlasterror);
		leftdrive = (kP*lefterror + kI*leftintegral + kD*leftlasterror);

		if(rightdrive > max)
		{
			rightdrive = max;
		}
		if(rightdrive < -max)
		{
			rightdrive = -max;
		}

		if(rightdrive > 0 && rightdrive < min)
		{
			rightdrive = min;
		}
		if(rightdrive < 0 && rightdrive > -min)
		{
			rightdrive = -min;
		}

		if(leftdrive > max)
		{
			leftdrive = max;
		}
		if(leftdrive < -max)
		{
			leftdrive = -max;
		}

		if(leftdrive > 0 && leftdrive < min)
		{
			leftdrive = min;
		}
		if(leftdrive < 0 && leftdrive > -min)
		{
			leftdrive = -min;
		}

		leftDrive(leftDirection*leftdrive);
		rightDrive(rightDirection*(rightdrive + 2.45*difference));

		if(abs(righterror) > 5 || abs(lefterror) > 5)
		{
			clearTimer(T1);
		}
		if(time1(T1) > 50)
		{
			errorwait = 1;
		}

	}
	driveMotors(0);
}


void pre_auton()
{
	// Set bStopTasksBetweenModes to false if you want to keep user created tasks running between
	// Autonomous and Tele-Op modes. You will need to manage all user created tasks if set to false.
	bStopTasksBetweenModes = true;



}


task autonomous()/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
	int programmingSkills=0;
	int match=0;
	int red=0;
	int blue=0;
	int cube=0;
	int skyrise=0;
	SensorValue[leftLifttEncoder] = 0;
	SensorValue[rightLiftEncoder] = 0;

	if(SensorValue[autoPot2] > 1000) // competition or match
		{
			programmingSkills=1;
		}
		else

			match=1;

	if(SensorValue[autoPot] < 2500) //red or blue
		{
			red=1;
		}
		else

			blue=1;

	if(SensorValue[autoPot3] > 2000) // skyrise or cubes
		{
			skyrise=1;
		}
		else

			cube=1;


		//		DriveLoop(-1, 800, 0.11, 0.000000, 0.001);
		//  	DriveLoop( 1, 800, 0.11, 0.000000, 0.001);

			if(match==1 && skyrise==1 && red==1)//------------------RED SKYRISE-------
			{
				SensorValue[latch] = 1;
				armDown(0, 90);
				wait1Msec(300);
				armUp(130, 90);
				autoFeed(300, -127);
				autoFeed(1,0);
				wait1Msec(500);
				armDown(105, 90);
				autoGrab(1);
				armUp(300, 90);
				liftMotors(-18);
				DriveLoop(-1,-1, 890, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(0, 90);
				autoGrab(0);			//drop one
				armUp(160, 90);
				liftMotors(-10);
				DriveLoop(1,1, 890, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(105, 90);
				autoGrab(1);
				armUp(200, 90);
				DriveLoop(-1,-1, 900, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(220, 90);
				autoGrab(0);			//drop two
				armUp(290, 90);
				liftMotors(-15);
				DriveLoop(1,1, 900, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(105, 90);
				autoGrab(1);
				armUp(390, 90);
				DriveLoop(-1,-1, 900, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(320, 90);
				autoGrab(0);			//drop three
				armUp(400, 90);
				Feed(-40);
				driveForward(127, -60, 160);
				Feed(-127);
				wait1Msec(1000);
				Feed(0);
				wait1Msec(5000000);
				}

	if(match==1 && skyrise==1 && blue==1)//---BLUE SKYRISE-----
			{
				SensorValue[latch] = 1;
				wait1Msec(300);
				armUp(160, 90);
				autoFeed(300, -127);
				autoFeed(1,0);
				wait1Msec(500);
				armDown(80, 90);
				autoGrab(1);
				armUp(185, 90);
				liftMotors(-20);
				DriveLoop(-1,-1, 900, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(0, 90);
				autoGrab(0);			//drop one
				armUp(180, 90);
				liftMotors(-10);
				DriveLoop(1,1, 890, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(80, 90);
				autoGrab(1);
				armUp(220, 90);
				DriveLoop(-1,-1, 925, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(180, 90);
				autoGrab(0);			//drop two
				armUp(250, 90);
				liftMotors(-15);
				DriveLoop(1,1, 925, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(80, 90);
				autoGrab(1);
				armUp(340, 90);
				DriveLoop(-1,-1, 920, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(320, 90);
				autoGrab(0);			//drop three
				armUp(400, 90);
				driveForward(-60, 120, 100);
				Feed(-127);
				driveForward(-60, 120, 170);
				wait1Msec(5000);
				Feed(0);
				wait1Msec(5000000);

			}
	if(programmingSkills==1) //--------------------------------SKILLS-------------
			{
			SensorValue[latch] = 1;
				armDown(0, 90);
				wait1Msec(300);
				armUp(130, 90);
				autoFeed(300, -127);
				autoFeed(1,0);
				wait1Msec(500);
				armDown(90, 90);
				autoGrab(1);
				armUp(300, 90);
				liftMotors(-15);
				DriveLoop(-1,-1, 890, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(0, 90);
				autoGrab(0);			//drop one
				armUp(160, 90);
				liftMotors(-10);
				DriveLoop(1,1, 890, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(90, 90);
				autoGrab(1);
				armUp(200, 90);
				DriveLoop(-1,-1, 900, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(220, 90);
				autoGrab(0);			//drop two
				armUp(290, 90);
				liftMotors(-10);
				DriveLoop(1,1, 900, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(90, 90);
				autoGrab(1);
				armUp(380, 90);
				DriveLoop(-1,-1, 900, 0.15, 0.00000001, 0.1, 120, 30);
				armDown(320, 90);
				autoGrab(0);			//drop three
				armUp(370, 90);
				liftMotors(-15);
				DriveLoop(1,1, 905, 0.12, 0.00000001, 0.1, 80, 30);
				armDown(90, 90);
				autoGrab(1);
				armUp(560, 90);
				DriveLoop(-1,-1, 915, 0.12, 0.00000001, 0.1, 80, 30);
				armDown(530, 90);
				autoGrab(0);			//drop four
				armUp(610, 90);
				liftMotors(-15);
				DriveLoop(1,1, 915, 0.12, 0.00000001, 0.1, 80, 30);
				armDown(90, 90);
				autoGrab(1);
				armUp(750, 90);
				DriveLoop(-1,-1, 925, 0.12, 0.00000001, 0.1, 80, 30);
				armDown(700, 90);
				autoGrab(0);			//drop five
				armUp(750, 90);
				liftMotors(-15);
				DriveLoop(1,1, 950, 0.12, 0.00000001, 0.1, 80, 30);
				armDown(90, 90);
				autoGrab(1);
				armUp(950, 90);
				DriveLoop(-1,-1, 950, 0.11, 0.00000001, 0.1, 80, 30);
				armDown(900, 90);
				autoGrab(0);			//drop six
				armUp(950, 90);
				liftMotors(-15);
				DriveLoop(1,1, 950, 0.11, 0.00000001, 0.1, 80, 30);
				armDown(90, 90);
				autoGrab(1);
				armUp(1220, 90);
				DriveLoop(-1,-1, 900, 0.12, 0.00000001, 0.1, 80, 30);
				armDown(1110, 90);
				autoGrab(0);			//drop seven
				armUp(1220, 90);
				/*liftMotors(100);
				wait1Msec(500);
				liftMotors(15);*/
				driveForward(70, -70, 225);
				driveForward(70, 70, 100);
				autoFeed(750, -127);
				SensorValue[conveyer] = 1;
				driveForward(70, -70, 825);
				armDown(20, 127);
				liftMotors(-100);
				wait1Msec(500);
				liftMotors(-15);
				driveForward(70, 70, 100);
				autoFeed(1000, 127);
				driveForward(70, -70, 225);
				driveForward(70, 70, 100);
				autoFeed(1000, 127);
				driveForward(70, -70, 1000);
				armUp(1220, 127);
				driveForward(70, 70, 225);
				autoFeed(750, -127);
				wait1Msec(500);
				driveForward(70, 70, 50);
				autoFeed(750, -127);
				driveForward(-70, -70, 200);
				/*DriveLoop(1,-1, 250, 0.17, 0.00000001, 0.05, 120, 43);
				DriveLoop(1,1, 140, 0.14, 0.00000001, 0.05, 80, 35);
				autoFeed(1500, -127);
				DriveLoop(-1,-1, 140, 0.14, 0.00000001, 0.05, 80, 35);
				liftMotors(-50);
				DriveLoop(-1,1, 350, 0.17, 0.00000001, 0.05, 120, 43);
				liftMotors(-15);
				DriveLoop(1,1, 825, 0.12, 0.00000001, 0.05, 80, 35);
				DriveLoop(-1,1, 410, 0.12, 0.00000001, 0.05, 120, 43);
				liftMotors(-127);
				wait1Msec(1250);
				liftMotors(-15);
				DriveLoop(1,1, 175, 0.12, 0.00000001, 0.05, 80, 35);
				autoFeed(750, 127);
				wait1Msec(5000000);*/
			}
if(match==1 && cube==1 && red==1) //-----------RED CUBES Match
			{
				DriveLoop(-1,1, 400, 0.12, 0.00000001, 0.05, 120, 50);
		  	DriveLoop( 1,-1, 400, 0.12, 0.00000001, 0.05, 120, 50);
		  	wait1Msec(50000000000);


			/*SensorValue[latch] = 1;
				driveStop(300);
				armUp(1550, 90);
				liftMotors(-10);
				autoFeed(600, -127);
				autoFeed(1,0);
				wait1Msec(50);
				armDown(1200, 120);
				liftMotors(-127);
				wait1Msec(250);
				liftMotors(-15);
				driveForward(100, 100, 425);
				autoFeed(900, 127); //feed in red1
				stopRobot(10);
				liftMotors(60);
				driveForward(100, 100, 350);
				armUp(2300, 90);
				driveForward(90, -60, 250);
				autoFeed(800, -127); //feed out red1
				autoFeed(1,0);
				driveForward(-90, 60, 420);
				armDown(1200, 110);
				driveForward(100, 100, 100);
				wait1Msec(50);
				driveForward(70, -70, 125);
				liftMotors(-127);
				wait1Msec(450);
				liftMotors(-15);
				driveForward(100, 100, 650);
				autoFeed(700, 127); //pick up blue
				autoFeed(1, 0);
				driveForward(-70, 70, 15);
				autoFeed(375, 127); //move blue
				autoFeed(1, 0);
				driveForward(100, 100, 530);
				autoFeed(600, 127); //pick up red2
				driveForward(-70, 70, 10);
				autoFeed(1, 0);
				armUp(1800, 90);
				driveForward(90, -60, 250);
				wait1Msec(75);
				autoFeed(550, -127); //drop red2
				autoFeed(1,0);
				driveBack(-90, -90, -100);
				driveForward(-70, 70, 420);
				armDown(1390, 90);
				liftMotors(-10);
				driveForward(90, 90, 400);*/

		}
		if(match==1 && cube==1 && blue==1) //-----------BLUE CUBES-------------
			{
				SensorValue[latch] = 1;
				wait1Msec(300);
				armUp(1550, 90);
				liftMotors(-10);
				autoFeed(600, -127);
				autoFeed(1,0);
				wait1Msec(50);
				armDown(1425, 120);
				liftMotors(-127);
				wait1Msec(100);
				liftMotors(-15);
				SensorValue[conveyer] = 1;
				driveForward(100, 100, 425);
				autoFeed(925, 127); //feed in blue1
				stopRobot(10);
				liftMotors(40);
				driveForward(100, 100, 415);
				armUp(2300, 90);
				driveForward(-90, 60, 265);
				//	driveForward(90, 90, 20);
				autoFeed(800, -127); //feed out blue1
				autoFeed(1,0);
				//	driveBack(-90, -90, -20);
				driveForward(90, -60, 430);
				liftMotors(-40);
				driveForward(100, 100, 200);
				driveForward(-70, 70, 90);
				armDown(1425, 120);
				liftMotors(-127);
				wait1Msec(150);
				liftMotors(-15);
				driveForward(100, 100, 470);
				autoFeed(850, 127); //pick up red
				autoFeed(1, 0);
				//		driveForward(-70, 70, 15);
				autoFeed(250, 127); //move red
				autoFeed(1, 0);
				driveForward(100, 100, 590);
				autoFeed(650, 127); //pick up blue2
				//		driveForward(-70, 70, 10);
				autoFeed(1, 0);
				armUp(1800, 90);
				driveForward(-90, 60, 290);
				driveForward(90, 90, 40);
				autoFeed(550, -127); //drop blue2
				autoFeed(1,0);
				driveBack(-90, -90, -100);
				driveForward(-70, 70, 420);
				armDown(1390, 90);
				liftMotors(-10);
				driveForward(90, 90, 400);
				SensorValue[latch] = 1;
				wait1Msec(300);
				armUp(1550, 90);
				liftMotors(-10);
				autoFeed(600, -127);
				autoFeed(1,0);
				wait1Msec(50);
				armDown(1200, 120);
				liftMotors(-127);
				wait1Msec(250);
				liftMotors(-15);
				driveForward(100, 100, 425);
				autoFeed(900, 127); //feed in red1
				stopRobot(10);
				liftMotors(60);
				driveForward(100, 100, 350);
				armUp(2300, 90);
				driveForward(-90, 60, 250);
				autoFeed(800, -127); //feed out red1
				autoFeed(1,0);
				driveForward(90, -60, 420);
				armDown(1450, 110);
				driveForward(100, 100, 100);
				wait1Msec(50);
				driveForward(-70, 70, 85);
				armDown(1200, 110);
				liftMotors(-127);
				wait1Msec(450);
				liftMotors(-15);
				driveForward(100, 100, 650);
				autoFeed(700, 127); //pick up blue
				autoFeed(1, 0);
				//		driveForward(-70, 70, 15);
				autoFeed(375, 127); //move blue
				autoFeed(1, 0);
				driveForward(100, 100, 530);
				autoFeed(600, 127); //pick up red2
				//		driveForward(-70, 70, 10);
				autoFeed(1, 0);
				armUp(1800, 90);
				driveForward(-90, 60, 260);
				wait1Msec(75);
				autoFeed(550, -127); //drop red2
				autoFeed(1,0);
				driveBack(-90, -90, -100);
					driveForward(-70, 70, 420);
				armDown(1390, 90);
				liftMotors(-10);
				driveForward(90, 90, 400);
			}
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//                                 User Control Task
//
// This task is used to control your robot during the user control phase of a VEX Competition.
// You must modify the code to add your own robot specific commands here.
//
/////////////////////////////////////////////////////////////////////////////////////////

task usercontrol()
{
	int LiftDownSelector=0;
	int stillspeed = 15;
	int stillspeedLeft = 15;

	while (1 == 1)
	{
		const int downSpeed = 15;
		const int feedstill = 2;

		rightDrive ( vexRT[Ch2]);
		leftDrive ( vexRT[Ch3]);

		if(vexRT[Btn7U] == 1)
		{
			SensorValue[latch] = 1;
		}

		if(vexRT[Btn7R] == 1)
		{
			SensorValue[latch] = 0;
		}
		if(vexRT[Btn7D] == 1)
		{
			SensorValue[conveyer] = 1;
		}

		if(vexRT[Btn7L] == 1)
		{
			SensorValue[conveyer] = 0;
		}

		if(vexRT[Btn8L] == 1)
		{
			stillspeed = -10;
			stillspeedLeft = 10;
		}
		if(vexRT[Btn8U] == 1)
		{
			stillspeed = 15;
			stillspeedLeft = 15;
		}
		if(vexRT[Btn8D] == 1)
		{
			SensorValue[Claw] = 1;
		}
		if(vexRT[Btn8R] == 1)
		{
			SensorValue[Claw] = 0;
		}
		if(vexRT[Btn5U] == 1)
		{
			liftMotors(127);
		}
		else if(vexRT[Btn5D] == 1)
		{
			liftMotors(-127);
		}
		else
		{
			if(SensorValue[potLeft] > 1700)
			{
				motor[leftArmTop]     = stillspeedLeft;
				motor[leftArmBottom]  = stillspeedLeft;
				motor[rightArmTop]  	= stillspeed;
				motor[rightArmBottom] = stillspeed;
			}
			else
			{
				liftMotors(-downSpeed);
			}
		}
		if(vexRT[Btn6U] == 1)
		{
			motor[rightFeed] = 127;
			motor[leftFeed] = 127;
		}
		else if(vexRT[Btn6D] == 1)
		{
			motor[rightFeed] = -127;
			motor[leftFeed] = -127;
		}
		else
		{
			motor[rightFeed] = feedstill;
			motor[leftFeed] = feedstill;
		}
		if (vexRT[Btn8L]==1)
		{
			LiftDownSelector=1;
		}

	//	wait1Msec(25); //don't hog the cpu
	}
}
