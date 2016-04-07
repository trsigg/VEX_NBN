#define sampleTime 25 //number of milliseconds between sampling the flywheel velocity and control adjustments in flywheel task

int errortest = 0;
//waitUntilNotFiring
int initialWait, timeWithoutFiring;
bool firing = false;
//flywheel variables
int flywheelVelocity=0, flywheelPower=0, defaultPower=0;

float P, I, D; //debugging

task flywheel() {
	TVexJoysticks buttons[4] = { Btn8D, Btn7U, Btn7R, Btn7D }; //creating a pseudo-hash associating buttons with velocities and default motor powers

	while (true)
	{
		for (int i = 0; i < 4; i++)
		{
			if (vexRT[buttons[i]] == 1)
			{
				setFlywheelRange(i);
			}
			EndTimeSlice();
		}
	}
}

task flywheelStabilization() { //modulates motor powers to maintain constant flywheel velocity
	float PrevError = 0;
	float DeltaE = 0;

	while(true)	{
		SensorValue[flywheelEncoder] = 0;
		PrevError = Error;
		wait1Msec(sampleTime);

		flywheelVelocity = abs(SensorValue[flywheelEncoder]);
		Error = targetVelocity - flywheelVelocity;
		errortest = Error;
		DeltaE = Error - PrevError;
		Integral += (Error + PrevError)/2;
		setLauncherPower(defaultPower + Kp*Error + Ki*Integral + Kd*DeltaE);

		//debug
		P = Kp*Error;
		I = Ki*Integral;
		D = Kd*DeltaE;
	}
}
//end user input region

void initializeTasks() {
	startTask(flywheel);
	startTask(flywheelStabilization);
	startTask(seymoreControl);
	startTask(liftSwitcher);
}

void emergencyStop() {
	stopTask(flywheel);
	stopTask(flywheelStabilization);
	stopTask(seymoreControl);
	stopTask(liftSwitcher);

	initializeTasks();
}
//end task control region

task usercontrol() {
	initializeTasks();
	setFlywheelRange(0);

	while (true)
	{
		while (vexRT[emergencyStopBtn] == 0)
		{
			setDrivePower(sgn(vexRT[Ch2]) * vexRT[Ch2] * vexRT[Ch2] / 127, sgn(vexRT[Ch3]) * vexRT[Ch3] * vexRT[Ch3] / 127); //drive
			EndTimeSlice();
		}

		emergencyStop(); //reassign emstop button
	}
}

//autonomii
void pre_auton() { bStopTasksBetweenModes = true; }

task skillPointAuto() { //fire into opposing net
	setFlywheelRange(2);
	wait1Msec(2000);
	motor[feedMe] = 127;
	startTask(fire);
	wait1Msec(7000);
	stopTask(fire);
}

task stationaryAuto() { //fire into our net
	setFlywheelRange(3);
	wait1Msec(2000);
	motor[feedMe] = 127;
	startTask(fire);
	wait1Msec(7000);
	stopTask(fire);
}

task hoardingAuto() { //push balls into our corner
	driveStraight(2000); //drive forward
	turn(-15); //turn
	driveStraight(-1000, 80); //back up to push first stack into start zone
	turn(18); //turn toward second stack
	setFlywheelRange(1);
	motor[feedMe] = 127; //start feed
	driveStraight(2300, 750); //pick up second stack
	//fire second stack
	startTask(fire);
	waitUntilNotFiring();
	while (firing) { EndTimeSlice(); }
	stopTask(fire);
	startTask(feedToTop);

	turn(-65); //turn toward third stack
	//pick up third stack
	driveStraight(1100);
}

task classicAuto() {
	setFlywheelRange(3);
	motor[feedMe] = 127;
	//fire four initial preloads
	startTask(fire);
	waitUntilNotFiring();
	while (firing) { EndTimeSlice(); }
	stopTask(fire);
	startTask(feedToTop);

	setFlywheelRange(2);
	turn(21); //turn toward first stack
	//pick up first stack
	startTask(feedToTop);
	driveStraight(900);

	turn(-18); //turn toward net
	driveStraight(1150); //drive toward net
	stopTask(feedToTop);
	startTask(fire);
	waitUntilNotFiring(3000);
	while (firing) { EndTimeSlice(); }
	stopTask(fire);

	//pick up second stack
	startTask(feedToTop);
	driveStraight(950); //drive into net for realignment
	driveStraight(-750); //move back
	//fire second stack
	stopTask(feedToTop);
	startTask(fire);
	waitUntilNotFiring();
	while (firing) { EndTimeSlice(); }
	stopTask(fire);

	turn(-65); //turn toward third stack
	//pick up third stack
	driveStraight(1100);
}

task skillz() {
	//start flywheel
	setFlywheelRange(2);

	wait1Msec(1000);
	startTask(fire);
	//wait until first set of preloads are fired
	waitUntilNotFiring(12000);
	while (firing) { EndTimeSlice(); }
	stopTask(fire);
	startTask(feedToTop);

	turn(108); //turn toward middle stack
	motor[feedMe] = 127; //startTask(feedToTop); //start feeding
	driveStraight(2300); //drive across field
	turn(-15); // turn toward starting tiles
	driveStraight(1200); //drive across field
	turn(-60); //turn toward net

	//fire remaining balls
	stopTask(feedToTop);
	startTask(fire);
	while (true) { EndTimeSlice(); }
}

task autonomous() {
	initializeTasks();
	stopTask(seymoreControl);

	//startTask(skillPointAuto);
	//startTask(stationaryAuto);
	//startTask(hoardingAuto);
	//startTask(classicAuto);
	//startTask(skillz);

	if (SensorValue[ternaryPot] < 1187) {
		if (SensorValue[binaryPot] < 1217) {
			startTask(stationaryAuto);
		}
		else {
			startTask(skillPointAuto);
		}
	}
	else if (SensorValue[ternaryPot] < 2577) {
		if (SensorValue[binaryPot] < 1217) {
			startTask(classicAuto);
		}
		else {
			startTask(hoardingAuto);
		}
	}
	else {
		startTask(skillz);
	}

	while(true) { EndTimeSlice(); }
}
