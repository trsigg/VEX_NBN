#pragma config(Sensor, dgtl3,  chooSwitch,     sensorDigitalIn)
#pragma config(Sensor, dgtl4,  feedSwitch,   sensorDigitalIn)
#pragma config(Motor,  port1,           choo1,         tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port3,           feedMe,        tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           choo2,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port8,           seymore,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           choo3,         tmotorVex393_MC29, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

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

void fire() //fires from cocked position
{
	setChooSpeed(127);
	wait1Msec(750);
	while (SensorValue[chooSwitch] == 1) {}
	setChooSpeed(15);
}

task main()
{
	//fires initial preload
	setChooSpeed(127);
	while (SensorValue[chooSwitch] == 1) {}
	fire();
	
	//feeds and fires three remaining balls
	for (int i = 0; i < 3; i++)
	{
		setFeedSpeed(127);
		while (SensorValue[chooSwitch] == 1) {}
		while (SensorValue[chooSwitch] == 0) {}
		setFeedSpeed(0);
		fire();
	}
}
