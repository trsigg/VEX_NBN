#pragma config(Sensor, dgtl1,  chooEncoder,    sensorQuadEncoder)
#pragma config(Motor,  port1,           choo1,         tmotorVex393_HBridge, openLoop, reversed)
#pragma config(Motor,  port7,           choo2,         tmotorVex393_MC29, openLoop, reversed)

int speed = 20;
int direction = 1;
int newPosition = direction * SensorValue[chooEncoder];

task main()
{
	int target = 135;
	int prevPosition = 0;
	int dlay = 500;
	int increment = 1;

	SensorValue[chooEncoder] = 0;

	while (newPosition < (.9 * target)){
		motor[choo1] = 127;
		motor[choo2] = 127;
		newPosition = direction * SensorValue[chooEncoder];
	}

	while (newPosition < target){
		motor[choo1] = 50;
		motor[choo2] = 50;
		prevPosition = newPosition;
		newPosition = direction * SensorValue[chooEncoder];
	}

	motor[choo1] = 20;
	motor[choo2] = 20;

	int distance = newPosition - prevPosition;

	while (true){

		if (newPosition > prevPosition){
			if (newPosition > target) {
				speed -= (int)(increment * distance);
			} else if (newPosition < target){
				speed = speed;
			}else{
				speed = (int)(speed * 3 / 4);
			}
		} else if (newPosition < prevPosition){
			if (newPosition > target){
				speed = speed;
			} else if (newPosition < target){
				speed -= (int)(increment * distance);
			}else{
				speed = (int)(speed * 5 / 4);
			}
		} else {
			if (newPosition > target) {
				speed += increment;
			} else if (newPosition < target){
				speed -= increment;
			}else{
				speed = speed;
			}
		}

		prevPosition = newPosition;
		newPosition = direction * SensorValue[chooEncoder];
		motor[choo1] = speed;
		motor[choo2] = speed;
		wait1Msec(dlay);
	}
}
