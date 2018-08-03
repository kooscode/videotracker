/*
 * Arduino Code for controlling motor speed via Serial Messages
 * 
 * Copyright (C) 2017 Koos du Preez
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * CREATED BY: Koos du Preez - kdupreez@hotmail.com
 * 
*/


#define _debug false
                                                      
//Serial Speed & Port
#define serial_port Serial
#define baud_speed 115200

//Motor one Pin defines
#define m1_speed_pin 5
#define m1_rev_pin 6
#define m1_fwd_pin 7
#define m1_Hall1_pin 2

//Motor two Pin defines
#define m2_rev_pin 8
#define m2_fwd_pin 9
#define m2_speed_pin 10 
#define m2_Hall1_pin 3

//PWM (Voltages) sent to motors
#define MAX_PWM 255
#define MIN_PWM 0

#define MIN_SPEED 10
#define MAX_SPEED 255

#define STOP_TIMEOUT_MS 3000
long autostop_timeout = 0;

int _speed_m1 = 0;
int _speed_m2 = 0;

enum TRAVEL_DIRECTION
{
    M_FORWARD,
    M_REVERSE,
    M_LEFT_TURN,
    M_RIGHT_TURN,
    M_STOP
};

TRAVEL_DIRECTION _direction = M_FORWARD;

void setup() 
{

    //MOTOR1 CONTROL SETUP
    pinMode(m1_speed_pin, OUTPUT);
    pinMode(m1_fwd_pin, OUTPUT);
    pinMode(m1_rev_pin, OUTPUT);
    
    //MOTOR2 CONTROL SETUP
    pinMode(m2_speed_pin, OUTPUT);
    pinMode(m2_fwd_pin, OUTPUT);
    pinMode(m2_rev_pin, OUTPUT);

    serial_port.begin(baud_speed);
    
    delay(150);
      
    serial_port.println("[SRT]");
}

//ALLOW SERIAL COMMANDS...
String serialString;
String  cmdString = "";
boolean cmdReady = false;
String  nocmndString = "";
String  cmdMode = "NA";

void doSerialCommand()
{
    while (serial_port.available())
    {
        // Add Serial byte to string until newline is received.
        char inChar = (char)serial_port.read();
        if ((inChar == '\r') || (inChar == '\n'))
        {
          //only set command ready if not empty command string..
          if (cmdString.length() > 0)
            cmdReady = true;
        }
        else
        {
          cmdString += inChar;
        }
    }

  //Command String is ready for processing..
  if (cmdReady)
  {
    //reset aut stop on serial command activity...
    autostop_timeout = millis() + STOP_TIMEOUT_MS;

    //Force Forward
    _direction = M_FORWARD;

    serialString = cmdString;

    //Make it NOT case sensitive..
    cmdString.toUpperCase();

      if (cmdString.charAt(0) == 'S') // SPEED
      {
          
        //get speed
        int speed_m = cmdString.substring(3).toInt();
  
        if (cmdString.charAt(1) == 'L') // LEFT
        {

            //motor 2 speed
            _speed_m2 = speed_m;
            
            if (cmdString.charAt(2) == 'F') // FORWARD 
            {
              //motor 2 forward
              digitalWrite(m2_fwd_pin, HIGH);
              digitalWrite(m2_rev_pin, LOW);
            }
            else // REVERSE
            {
              //motor 2 reverse
              digitalWrite(m2_fwd_pin, LOW);
              digitalWrite(m2_rev_pin, HIGH);
            }
        }
        else // RIGHT
        {
            //motor 1 speed
            _speed_m1 = speed_m;
    
            if (cmdString.charAt(2) == 'F') // FORWARD 
            {
              //motor 1 forward
              digitalWrite(m1_fwd_pin, HIGH);
              digitalWrite(m1_rev_pin, LOW);
            }
            else // REVERSE
            {
              //motor 1 reverse
              digitalWrite(m1_fwd_pin, LOW);
              digitalWrite(m1_rev_pin, HIGH);
            }
        }

        if (_debug)
          serial_port.println(cmdString);

      }


      //ready for next command..
      cmdString = "";
      cmdReady = false;
  }
}

void loop() 
{
  
    //force stop at timeout intervals..
    if (millis() > autostop_timeout)
      _direction = M_STOP;

    doSerialCommand();

    //force speed to zero if in stop mode..
    int tmpspeed_m1 = (_direction == M_STOP) ? 0 : _speed_m1;
    int tmpspeed_m2 = (_direction == M_STOP) ? 0 : _speed_m2;
    
    analogWrite(m1_speed_pin, tmpspeed_m1);
    analogWrite(m2_speed_pin, tmpspeed_m2);     

}
