#include <Arduino.h>
#include <MillisTimer.h>

//boolean triggerSet = false;
boolean timerReset = true;
const int inputPin = 2;
int buttonState = 0;
int pirState = LOW;

const int outputPin = 9;
const int maxRepeats = 7;

int brightnessArrayMax = 0;
float brightnessArray[256];

// Create a timer that fires every 1000 milliseconds.
MillisTimer timer1 = MillisTimer(1000);

// This is the function that is called when the timer expires.
void myTimerFunction(MillisTimer &mt)
{
  int repeat = mt.getRemainingRepeats();
  Serial.print(Constant("Repeat: "));
  Serial.println(repeat);

  if (repeat > 0 && repeat < maxRepeats){
    analogWrite(outputPin,  255);
  }
  if (repeat == 0){
    // ramp down
    for (size_t i = brightnessArrayMax/2; i < brightnessArrayMax; i++) {
      analogWrite(outputPin,  brightnessArray[i]);
      delay(10);
    }
    analogWrite(outputPin,  0);
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(inputPin, INPUT);
  pinMode(outputPin, OUTPUT);
  InitBrightness(brightnessArray, &brightnessArrayMax);
}

void loop()
{
    //triggerSet = istriggerSet();
    pirState = isMotionDetected(inputPin);

    if (!timer1.isRunning() && pirState==HIGH)
    {
      Serial.println("Start Timer");
      timer1.setInterval(1000*5);
      timer1.expiredHandler(myTimerFunction);
      timer1.setRepeats(maxRepeats);
      timer1.start();
      timerReset=false;

      // ramp up
      for (size_t i = 0; i < brightnessArrayMax/2; i++) {
        analogWrite(outputPin,  brightnessArray[i]);
        delay(10);
      }
    }
    if (timer1.isRunning())timer1.run();

    if (!timer1.isRunning()&& !timerReset)
    {
      Serial.println("Timer finished!");

      if (!timerReset)
      {
        timer1.reset();
        timerReset=true;
        Serial.print("reset true.");
      }
    }
}

void InitBrightness(float *arr, int *max)
{
  const int pwmIntervals = 100;
  // Calculate the R variable (only needs to be done once at setup)
  float R = (pwmIntervals * log10(2))/(log10(255));

  // ramp up
  int i=0;
  for (int interval = 0; interval <= pwmIntervals; interval++)
  {
      // Calculate the required PWM value for this interval step
      int brightness = pow (2, (interval / R)) - 1;
      if (brightness < 2) continue;
      // Set the LED output to the calculated brightness
      //analogWrite(outputPin, brightness);
      arr[i++] = brightness;
  }
  for (int j = i, k=0; j<2*i;j++,k++) arr[j] =  arr[i-k];

  *max = i*2;
}

int isMotionDetected(int ledPin)
{
  int val = digitalRead(inputPin);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    digitalWrite(ledPin, HIGH);  // turn LED ON
    if (pirState == LOW) {
      // we have just turned on
      return HIGH;
    }
  } else {
    digitalWrite(ledPin, LOW); // turn LED OFF
    if (pirState == HIGH){
      // we have just turned of
      return LOW;
    }
  }
}

boolean istriggerSet()
{
  buttonState = digitalRead(inputPin);
  if (buttonState == HIGH) {
    // turn LED on:
    return true;
  } else {
    // turn LED off:
    return false;
  }
}
