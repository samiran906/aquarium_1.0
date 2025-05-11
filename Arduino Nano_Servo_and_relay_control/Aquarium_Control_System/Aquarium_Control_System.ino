#define filter_bus      2
#define light_bus       3
#define autoFeeder_bus  6

#include <Servo.h>

Servo myservo;

bool light_flag = 0;
bool filter_flag = 0;

int value = 0;
int final_value = 0;

void setup()
{
  Serial.begin(9600);
  delay(5000);
  myservo.attach(autoFeeder_bus);
  myservo.write(0);   
  delay(1500);
  myservo.detach();

  pinMode(filter_bus, OUTPUT);
  pinMode(light_bus, OUTPUT);

  digitalWrite(filter_bus, HIGH);
  digitalWrite(light_bus, HIGH);
}

void loop() 
{
  for(int i=0; i<100; i++)
  {
    value = analogRead(A6);
    if(value > 200)
    {
      value = 1;
    }
    else
    {
      value = 0;
    }
    final_value += value;
  }
  
  if(final_value > 90)
  {
    delay(1000);
    if(analogRead(A6) > 200)
    {
      if(filter_flag == 0)
      {
        digitalWrite(filter_bus, HIGH);
        filter_flag = 1;
      }
    }
  }
  else
  {
    delay(1000);
    if(analogRead(A6) <= 200)
    {
      if(filter_flag == 1)
      {
        digitalWrite(filter_bus, LOW);
        filter_flag = 0;
      }
    }
  }
  final_value = 0;

//***********************************************
for(int i=0; i<100; i++)
  {
    value = analogRead(A5);
    if(value > 200)
    {
      value = 0;
    }
    else
    {
      value = 1;
    }
    final_value += value;
  }
  
  if(final_value > 60)
  {
    delay(1000);
    if(analogRead(A5) <= 200)
    {
      if(light_flag == 0)
      {
        digitalWrite(light_bus, LOW);
        light_flag = 1;
      }
    }
  }
  else
  {
    delay(1000);
    if(analogRead(A5) > 200)
    {
      if(light_flag == 1)
      {
        digitalWrite(light_bus, HIGH);
        light_flag = 0;
      }
    }
  }
  final_value = 0;

//***********************************************
for(int i=0; i<100; i++)
  {
    value = analogRead(A7);
    if(value > 200)
    {
      value = 1;
    }
    else
    {
      value = 0;
    }
    final_value += value;
  }
  
  if(final_value > 90)
  {
    delay(5000);
    if(analogRead(A7) > 200)
    {
      myservo.attach(autoFeeder_bus);
      myservo.write(167);   
      delay(500);
      myservo.write(0); 
      delay(1000);
      myservo.detach();
      delay(30000);
    }
  }
  final_value = 0;
}
