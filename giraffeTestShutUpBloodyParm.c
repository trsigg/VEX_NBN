#pragma config(I2C_Usage, I2C1, i2cSensors)
#pragma config(Sensor, in1,    chooResistor,   sensorReflection)
#pragma config(Sensor, in2,    modePoten,      sensorPotentiometer)
#pragma config(Sensor, in3,    ballsToLaunchPoten, sensorPotentiometer)
#pragma config(Sensor, in4,    sidePoten,      sensorPotentiometer)
#pragma config(Sensor, in5,    giraffeSetPoten, sensorPotentiometer)
#pragma config(Sensor, dgtl1,  chooSwitch,     sensorDigitalIn)
#pragma config(Sensor, dgtl2,  feedSwitch,     sensorDigitalIn)
#pragma config(Sensor, I2C_1,  giraffeEncoder, sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_2,  rightEncoder,   sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Sensor, I2C_3,  leftEncoder,    sensorQuadEncoderOnI2CPort,    , AutoAssign )
#pragma config(Motor,  port1,           feedMe,        tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           seymore,       tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port3,           left1,         tmotorVex393_MC29, openLoop, encoderPort, I2C_3)
#pragma config(Motor,  port4,           left2,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           choo1,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           choo3,         tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           right1,        tmotorVex393_MC29, openLoop, reversed, encoderPort, I2C_2)
#pragma config(Motor,  port8,           right2,        tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port9,           choo2,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          giraffe,       tmotorVex393_HBridge, openLoop, encoderPort, I2C_1)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

int giraffeTarget = 0;
int giraffePower;
int debug;

//group 5
#define progressCataChooChooBtn Btn5U
#define manualOverrideBtn Btn5D
//group 6
#define feedUpBtn Btn6U
#define feedDownBtn Btn6D
//group 7
#define fireOnceBtn Btn7U
#define continuousFireBtn Btn7D
#define emergencyStopBtn Btn7L
#define loadBtn Btn7R
//group 8
#define fullCourtBtn Btn8U
#define netBtn Btn8D
#define giraffeDownBtn Btn8L
#define giraffeUpBtn Btn8R

const int giraffeMinVelocity = 5; //not actually velocity, but I know what I mean
const int giraffeUpwardPower = 100;
const int giraffeDownwardPower = -80;
const int giraffeStillSpeed = 20;
const int giraffeError = 10;
const int giraffeSampleTime = 100;
const int netPos = -900;

task main()
{
	int oldPosition;

	while (true)
	{
		motor[giraffe] = giraffeStillSpeed;

		while (abs(SensorValue[giraffeEncoder] - giraffeTarget) < giraffeError && vexRT[giraffeUpBtn] == 0 && vexRT[giraffeDownBtn] == 0 && vexRT[fullCourtBtn] == 0 && (vexRT[netBtn] == 0 || giraffeTarget != netPos)) { giraffePower = motor[giraffe]; }

		if (vexRT[giraffeUpBtn] == 1)
		{
			motor[giraffe] = giraffeUpwardPower;
			while (vexRT[giraffeUpBtn] == 1) { giraffePower = motor[giraffe]; }
 			giraffeTarget = SensorValue[giraffeEncoder];
		}
		else if (vexRT[giraffeDownBtn] == 1)
 		{
 			motor[giraffe] = giraffeDownwardPower;
 			while (vexRT[giraffeDownBtn] == 1) { giraffePower = motor[giraffe]; }
 			giraffeTarget = SensorValue[giraffeEncoder];
 		}
 		else if (vexRT[fullCourtBtn] == 1)
 		{
 			motor[giraffe] = 127;

 			do
 			{
 				oldPosition = SensorValue[giraffeEncoder];
 				clearTimer(T1);
 				while (time1(T1) < giraffeSampleTime) { giraffePower = motor[giraffe]; }
 				debug = SensorValue[giraffeEncoder] - oldPosition;
 			} while (SensorValue[giraffeEncoder] - oldPosition > giraffeMinVelocity && vexRT[giraffeUpBtn] == 0 && vexRT[giraffeDownBtn] == 0);

 			motor[giraffe] = giraffeStillSpeed;

 			if (vexRT[giraffeUpBtn] == 0 && vexRT[giraffeDownBtn] == 0)
 			{
 				SensorValue[giraffeEncoder] = 0;
				giraffeTarget = 0;
 			}
 		}
 		else if(vexRT[netBtn] == 1)
 		{
 			giraffeTarget = netPos;
 			setMotorTarget(giraffeTarget);
 		}
		else
 		{
 			setMotorTarget(giraffe, giraffeTarget, 127, 0);
 			while (!getMotorTargetCompleted(giraffe) && vexRT[giraffeUpBtn] == 0 && vexRT[giraffeDownBtn] == 0) { giraffePower = motor[giraffe]; }
 			if (vexRT[giraffeUpBtn] == 0 && vexRT[giraffeDownBtn] == 0)
 			{
 				motor[giraffe] = giraffeStillSpeed;
 			}
 		}
	}
}