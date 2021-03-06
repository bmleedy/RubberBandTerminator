/*!
 * @file Rubber_Band_Shooter.cpp
 * 
 * This class handles operation of all servos for a pan/tilt/fire rubber band servo system.
 * 
 */
#include "Rubber_Band_Shooter.h"

/*!
 * Fire the rubber band
 */
void Rubber_Band_Shooter::fire() {
  hammer.write(map(FIRE_HAMMER_POSITION,0,180,MIN_PULSE_WIDTH,MAX_PULSE_WIDTH));  //fiiiirrrre!
  delay(1000);  //wait a second for the servo to get there
  hammer.write(map(ARMED_HAMMER_POSITION,0,180,MIN_PULSE_WIDTH,MAX_PULSE_WIDTH));  //ready to load again
}

/*!
 * increase the elevation by one increment
 */
void Rubber_Band_Shooter::turn_up() {
  //move in positive direction
  elevation_command_position = elevation_command_position + ELEVATION_POSITION_INCREMENT;
  if(elevation_command_position > (ELEVATION_CENTER_POSITION + ELEVATION_MOVEMENT_RANGE) ){
    elevation_command_position = (ELEVATION_CENTER_POSITION + ELEVATION_MOVEMENT_RANGE);
    Serial.print(F("|Fixing elevation out-of-range elevation input!"));Serial.println(elevation_command_position);
  }

  elevation.write(map(elevation_command_position,0,180,MIN_PULSE_WIDTH,MAX_PULSE_WIDTH));
}

/*!
 * decrease the elevation by one increment
 */
void Rubber_Band_Shooter::turn_down() {
  //move in positive direction
  elevation_command_position = elevation_command_position - ELEVATION_POSITION_INCREMENT;
  if(elevation_command_position < (ELEVATION_CENTER_POSITION - ELEVATION_MOVEMENT_RANGE) )
    elevation_command_position = (ELEVATION_CENTER_POSITION - ELEVATION_MOVEMENT_RANGE);

  elevation.write(map(elevation_command_position,0,180,MIN_PULSE_WIDTH,MAX_PULSE_WIDTH));
}

/*!
 * turn clockwise by one increment
 */
void Rubber_Band_Shooter::turn_right(){
  int step_distance = BASE_STEP_INCREMENT * BASE_STEPS_PER_DEGREE;
  small_stepper->setSpeed(BASE_MAX_SPEED);
  small_stepper->step(step_distance);// Rotate CW
  delay(MAX_BLOCKING_DELAY);
}

/*!
 * turn counterclockwise by one increment
 */
void Rubber_Band_Shooter::turn_left(){
  int step_distance = -1 * BASE_STEP_INCREMENT * BASE_STEPS_PER_DEGREE;
  small_stepper->setSpeed(BASE_MAX_SPEED); //Max is 500
  small_stepper->step(step_distance);// Rotate CCW
  delay(MAX_BLOCKING_DELAY);
}


/*!
 * Constructor
 */
Rubber_Band_Shooter::Rubber_Band_Shooter(){
  Rubber_Band_Shooter(HAMMER_PIN, ELEVATION_PIN);
}


/*!
 * Constructor with pin specifier
 * 
 * @param hammer_pin
 *        The pin on which the hammer servo is driven
 * @param elevation_pin
 *        The pin on which the elevation servo is driven
 */
Rubber_Band_Shooter::Rubber_Band_Shooter(unsigned char hammer_pin, unsigned char elevation_pin){

  elevation_command_position = ELEVATION_CENTER_POSITION;
  elevation.attach(elevation_pin);  // attaches the servo on pin 9 to the servo object
  hammer.write(map(elevation_command_position,0,180,MIN_PULSE_WIDTH,MAX_PULSE_WIDTH));
  
  hammer.attach(hammer_pin);
  hammer.write(map(ARMED_HAMMER_POSITION,0,180,MIN_PULSE_WIDTH,MAX_PULSE_WIDTH));

  small_stepper = new Stepper(STEPS_PER_REV, 2, 6, 10, 7);

  Serial.println(F("| Rubber_Band_Shooter Setup complete."));
}
