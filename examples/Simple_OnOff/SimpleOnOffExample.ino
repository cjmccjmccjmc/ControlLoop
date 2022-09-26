#include <ControlLoop.h>
#include "WaterBucketModel.hpp"


/*
 The Simulation parameters
*/

const long SECONDS_TO_ADVANCE_MODEL = 5;

/*
    Controls the model.   
    If Leak > Max water in, then it will never fill-up
*/
const double TARGET_LEVEL_LITRES = 10.5;
const double MAX_WATER_IN_LITRES_MINUTE = 20;
const double LEAK_OUT_LITRES_PER_MINUTE = 10;



WaterBucketModel theBucket(MAX_WATER_IN_LITRES_MINUTE, LEAK_OUT_LITRES_PER_MINUTE);


/*
 * Setup callback classes for use by ControlLoop   
 * 
 */


/*
  DataSource class is used to provide the current state of the variable being measured.  In this example it is the current water level.
*/

class : public DataSource {
  public:
    virtual double get() {
      return theBucket.getCurrentWaterLevel();
    }
} waterLevelDS;


/*
 RelayUpdate class callback is used to control the input varable, in this example how far open the water tap is.

 The purpose of this class is to map to the varaible being controlled.

 */

class : public RelayUpdate {
  public:

    // This is called when the controlled varible moves from off to on
    // Use this method for any initalisation 
    virtual void on() {
        ;
    }

    // Called when the ControlLoop is forced to turn-off
    // This should be used for hooks that are need to move the controlling varable to be dorment such as turn-off power, etc.    
    // In this case, by setting the valve to 0% output will turn it off.
    virtual void off(){
        this->update(0.0);
    }

    // Called each time the ControlLoop wants to change the variable.
    // This needs to trigger the change of the input varaible, in this change how far the valve is open.
    virtual void update(double res){
        theBucket.changeValveOpening(res);
    }
} theValve;


ControlLoop theControlLoop(&waterLevelDS, &theValve, TARGET_LEVEL_LITRES);


//The setup function is called once at startup of the sketch
void setup() {

  Serial.begin(9600);
  delay(50);


  // Preable, output to let use know the current parameters.
  Serial.println("ControlLoop - On Off example (Leaky Bucket)");
  Serial.printf("Model will advance %ld seconds\n", SECONDS_TO_ADVANCE_MODEL );
  Serial.printf("Target water level is %4.2f L with maxiumm in flow of %2.2f L/min and a leak of %2.2f L/min\n", TARGET_LEVEL_LITRES, MAX_WATER_IN_LITRES_MINUTE, LEAK_OUT_LITRES_PER_MINUTE);

  // Set what sort of control loop to use.
  // Note, we have set the starting setPoint when the object was created.
  theControlLoop.setControlType(ControlLoop::ONOFF);
 
  // Turn on the ControlLoop
  theControlLoop.setOn();

  while( true ) {
    // Call compute, to do the control calculations
    theControlLoop.Compute();

    // Move the model forward in time and print out the results
    theBucket.advance();
    Serial.printf("Time %4ld Target %4.1f Actual %4.1f Error %4.1f    Valve open %3.0f%%\n", 
        millis()/1000, 
        TARGET_LEVEL_LITRES, 
        theBucket.getCurrentWaterLevel(), 
        theBucket.getCurrentWaterLevel() - TARGET_LEVEL_LITRES,
        theBucket.getValveOpen()*100  );
    delay(SECONDS_TO_ADVANCE_MODEL*1000);
  }

}


void loop() {
  ;
  
}

