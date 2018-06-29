
//Interrupt lines for the RC servo channels
#define CH_1_INT 0
#define CH_2_INT 1

#define M1_RPWM 5
#define M1_LPWM 6

#define M2_RPWM 10
#define M2_LPWM 11

bool estop = false;

//Values are updated by the servo service routines
volatile double l_r_val, fwd_rev_val = 1500;
volatile double Chan1_val_last, Chan2_val_last = 1500;

//Store the times that the pulses started
unsigned long Chan1_startPulse, Chan2_startPulse;

//For determining if it's time to update the motor speeds
unsigned long StartMillis = 0;

//Called on the rising edge interrupt.
//Log the start time, and attach the interrupt to wait for the falling edge
void Chan1_begin() {
  Chan1_startPulse = micros();
  detachInterrupt(CH_1_INT);
  attachInterrupt(CH_1_INT, Chan1_end, FALLING);
}

//Called on falling interrupt, gets the time difference between the start and end
//Also attaches the interrupt to wait for the rising edge of the pulse
void Chan1_end() {
  l_r_val = micros() - Chan1_startPulse;
  detachInterrupt(CH_1_INT);
  attachInterrupt(CH_1_INT, Chan1_begin, RISING);
  if (l_r_val < 1000 || l_r_val > 2000) {
    l_r_val = Chan1_val_last;
  } else {
    Chan1_val_last = l_r_val;
  }
}

void Chan2_begin() {
  Chan2_startPulse = micros();
  detachInterrupt(CH_2_INT);
  attachInterrupt(CH_2_INT, Chan2_end, FALLING);
}

void Chan2_end() {
  fwd_rev_val = micros() - Chan2_startPulse;
  detachInterrupt(CH_2_INT);
  attachInterrupt(CH_2_INT, Chan2_begin, RISING);
  if (fwd_rev_val < 1000 || fwd_rev_val > 2000) {
    fwd_rev_val = Chan2_val_last;
  } else {
    Chan2_val_last = fwd_rev_val;
  }
}

//The setup function is called once at startup of the sketch
void setup()
{
  //Start up serial to get debug data
  Serial.begin(9600);

  //Stop both motors
  pinMode(M1_RPWM, OUTPUT);
  pinMode(M1_LPWM, OUTPUT);
  pinMode(M2_RPWM, OUTPUT);
  pinMode(M2_LPWM, OUTPUT);
  analogWrite(M1_RPWM, 0);
  analogWrite(M1_LPWM, 0);
  analogWrite(M2_RPWM, 0);
  analogWrite(M2_LPWM, 0);

  //Attach interrupts for the inputs from the RC receiver
  attachInterrupt(CH_1_INT, Chan1_begin, RISING);
  attachInterrupt(CH_2_INT, Chan2_begin, RISING);

  //Set up the timer
  StartMillis = millis();

  //clear the e-stop
  estop = false;

}

void loop()
{
  if ((millis() - StartMillis) > 100)
  {
    //Center at zero
    int rotation = l_r_val - 1500;
    int velocity = fwd_rev_val - 1500;

    //Debug printing for scaling
    //    Serial.print(rotation);
    //    Serial.print("\t");
    //    Serial.println(velocity);

    //Scale left/right to same range as fwd/rev
    rotation = map(rotation, -400, 400, -100, 100);
    velocity = map(velocity, -400, 400, -100, 100);

    //Convert for two motors
    int lMotor = -(velocity - rotation);
    int rMotor = velocity + rotation;

    //Positive is forward, negative is backward
    int lMotDir = lMotor > 0 ? 1 : 2; //1 is fwd, 2 is reverse
    int rMotDir = rMotor > 0 ? 1 : 2; //1 is fwd, 2 is reverse

    lMotor = constrain(lMotor, -100, 100);
    rMotor = constrain(rMotor, -100, 100);

    //Constrain to legit maximum analog output
    lMotor = map(abs(lMotor), 0, 100, 0, 255);
    rMotor = map(abs(rMotor), 0, 100, 0, 255);

    int deadband = 30;
    if (lMotor < deadband) {
      lMotor = 0;
    }
    if (rMotor < deadband) {
      rMotor = 0;
    }

    //heartbeat check
    if ((estop == false) && ((micros() - Chan1_startPulse) > 60000) || ((micros() - Chan2_startPulse) > 60000)) {
      //Too long since last pulse arrived
      //The only way to reset the estop is to run setup() again.
      estop = true;
    }

    if (estop == true)
    {
      lMotor = rMotor = 0;
      lMotDir = rMotDir = 0;
    }

    Serial.print(lMotor);
    Serial.print("\t");
    Serial.print(lMotDir);
    Serial.print("\t");
    Serial.print(rMotor);
    Serial.print("\t");
    Serial.println(rMotDir);

    //Set the motor speeds
    if (lMotDir == 1)
    {
      analogWrite(M1_LPWM, lMotor);
      analogWrite(M1_RPWM, 0);
    }
    else if (lMotDir == 2)
    {
      analogWrite(M1_LPWM, 0);
      analogWrite(M1_RPWM, lMotor);
    }
    else
    {
      analogWrite(M1_LPWM, 0);
      analogWrite(M1_RPWM, 0);
    }

    if (rMotDir == 1)
    {
      analogWrite(M2_LPWM, rMotor);
      analogWrite(M2_RPWM, 0);
    }
    else if (rMotDir == 2)
    {
      analogWrite(M2_LPWM, 0);
      analogWrite(M2_RPWM, rMotor);
    }
    else
    {
      analogWrite(M2_LPWM, 0);
      analogWrite(M2_RPWM, 0);
    }

    //Reset our timer
    StartMillis = millis();
  }
}
