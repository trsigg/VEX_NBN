#pragma config(Sensor, I2C_3,  giraffeEncoder, sensorNone)
#pragma config(Motor,  port9,           giraffe,       tmotorVex393_MC29, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

task main()
{
	while (true)
	{
		if (vexRT[btn6U] == 1)
		{
			motor[giraffe] = 80;
		}
		else if (vexRT[btn6D] == 1)
		{
			motor[giraffe] = -60;
		}
		else if (vexRT[btn5U] == 1)
		{
			motor[giraffe] = 127;
		}
		else if (vexRT[btn5D] == 1)
		{
			motor[giraffe] = -127;
		}
		else
		{
			motor[giraffe] = 15;
		}
	}

	SensorValue[giraffeEncoder] = 0;
}
