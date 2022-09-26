# ControlLoop
Embedded control loop class with multiple control loop method including on/off (BangBang), PID, and Cascade PID.

Leverages [Arduino-PID-Library](https://github.com/br3ttb/Arduino-PID-Library) to implement underlying PID.


## Table of Contents

- [Quick Start](#QuickStart)
- [Concepts](#Concepts)
- [Initial Setup](#InitialSetup)
- [On/Off aka BangBang](#On/OffakaBangBang)
- [PID](#PID)
- [Cascade PID](#CascadePID)
- [Other](#other)
    - [Handling Binary Relay  (on/off)](#HandlingBinaryRelay)
    - [Future topics to cover in Other](#PIDtuning)
- [API Documentation](#APIDocumentation)



## Quick Start

To start immediately, there are a number of examples under the [examples directory](https://github.com/cjmccjmccjmc/ControlLoop/tree/master/examples).

You will need to include the PID library.


### Arduino IDE

Add the library to the local installation of Arduino, in the IDE: 
- Open Sketch menu
- Click Include Library / Manage Library
- Library Manager will open
- Search for ControlLoop in upper right search box
- Click Install

To add it to your project:
- Open Sketch menu
- Click Include Library / ControlLoop


### PlatformIO IDE

To add to a PlatformIO IDE, open platform.ini and add:

``PID@1.2.1
  https://github.com/cjmccjmccjmc/ControlLoop.git``

To the ``lib_deps = '' entry

## Concepts

The ControlLoop abstracts control into:
- Input represented as an instance of a DataSource class.
- Relay controller 
- Compute() function

In the following section, a simple water tank example is used with the setpoint set to retain the water level to a specific level.

### Data Source
For a controller there can be  1 or 2 Data Sources for the variables that is being measured.  The second Data Source is used for Cascade PID which is discussed in a later section.

Using a water tank as an example, the Data Source would measure the volume of the water in the tank.   

### Relay Update

To change the variable under control, ControlLoop uses a RelayUpdate class to
set the variable.

So that you can start-up or shutdown in a safe manner.  ControlLoop will call on() when the ControlLoop starts up and off() when it is finished.  If the off() method is not implemented, the controlled variable will stay set at the last value passed into the update() method.


### setControlType()

ControlLoop's main rationale is to provide the ability to dynamically set the control method.   By default, it is set to PID, to change the controlling method pass in the following constants:
* ControlLoop::ONOFF Turns off if above upper bound of setpoint and on if below the lower bound of setpoint.
* ControlLoop::PID Sets the controller to ues Proportional Integral Derivative (PID) 
* ControlLoop::CASCADE Uses two PIDs chained together.  


### Compute()

In the loop() function, you must call Compute() as this will get the latest values of the measured variables; performs the selected calculation for the control type, and then call Relay Update to set the value if it has changed.


### Two Data Sources

For the Cascade PID, the Inner PID controls controls the setpoint for the Outer PID so for this you need to pass in Two Data Sources.   

In this case you create two Data Source classes one for the Inner and the other of the Outer measured variables.  

Note that the Outer DataSource is only used when the control type is set to  CASCADE by calling setControlType().


## Initial Setup

#### 1. Include the include file:


    #include <ControlLoop.h>

#### 2. Setup the DataSource, using an anonymous class:


    class : public DataSource {
      public:
        virtual double get() {
          return someMeasurement.getMeasure();
        }
    } theDataSource;

#### 3. Setup the RelayUpdate, also using an anonymous class:


    class : public RelayUpdate {
      public:
    
        virtual void on() {
            thingToBeControlled.startup();
        }
    
        virtual void off(){
            thingToBeControlled.shutdown();
        }
    
        virtual void update(double res){
            thingToBeControlled.change(res);
        }
    } relay;

#### 4. Construct the ControlLoop Object


    ControlLoop theControlLoop(&theDataSource, &relay, <initial setpoint>);

The Data Source and Relay Update classes are passed in by reference and need to have the initial setpoint so the object is in a known state.

Also, note that the DataSource and RelayUpdate classes don't need to be anonymous objects, but can be classes that inherit from those classes.

#### 5. In the setup() method

Optionally, you can change the type of control algorithm, by calling setControlType(); it defaults to PID.


    theControlLoop.setControlType(ControlLoop::ONOFF);

Finally, you will need to tell the ControlLoop to turn itself on otherwise calls to Compute() will not send a turn on change the Relay Update and make subsequent calls to the update() method to change the controlled variable.  


    theControlLoop.setOn();

This method can also be called outside of setup() when used as part of a state machine to handle the end user turning it on and off.   Don't always call it at the start of the  loop() as Arduino calls the loop() multiple times.  This could cause the thing being controlled to start-up each time.  

This is also a ``setOff()`` to reverse the above.

#### 6. In the loop() method

To run the calculation of the selected control algorithm using the latest data from the Data Source, you need to call:


    theControlLoop.Compute();

This takes into account if it has been turned Off so it can be called without side effects.

# Control Algorithms Implemented

This class implements following algorithms:
* On/Off
* PID
* Cascade PID

and the following variation to them all:
* BangBang


## On/Off 
This is the simplest algorithm, if the measured value is below the setpoint, the controlled variable is to set to 100% and if its above it, it is set to 0%.

To use this, pass ``ControlLoop::ONOFF`` to ``setControlType``.

## PID
The default algorithm is the PID

To use this, pass ``ControlLoop::PID`` to ``setControlType``.

Change the PID parameters by calling ``setTunings(p, i, d)``

The PID parameters have a default value for each but it is recommended to set these values to align to the process under control. 

## Cascade PID

This is two PIDs chained together, with the inner PID measuring the item under control.  The inner PID's output is used to change the set point of the outer PID which can control the 2nd item under control.

Using a heat exchange as an example:
* Heated water is pumped through a heat exchanger pipes into the main water under control.
* The inner PID takes in the main water tank's temperature as the measured variable and outputs the temperature that second tank.
* The outer PID uses the second tank's temperature as as the measured variable.
* The outer PID's output will drive the heating control.

To use this, pass ``ControlLoop::CASCADE`` to ``setControlType``.

To change each PID's parameter use:
   	  setTunings(ControlLoop::INNER, p,i,d);
   	  setTunings(ControlLoop::OUTER, p,i,d);

## With BangBang

The above control types, can be further modified by enabling BangBang.  This changes the behavior of the algorithms above by only engaging them them when the measured variable is within an lower and upper value.

To use this call ``enableBangBang()``

To set the range of values, there are methods that can be called:

       setBangRange(x)	    // sets the range from setpoint - x to setpoint + x
       or:
       setBangRange(y,z)    // sets the range from setpoint - y to setpoint + z

The behavior for ONOFF control with BangBang prevents the algorithm turning on and off as soon as the measured variable crosses over the setpoint.

For PID and Cascade, the behavior disengages algorithm and uses ON/OFF algorithm until within the range set above.   This benefits the PID-based algorithms as the larger initial error of the measured and setpoint value is not included in the history therefore improving the output result.


## Other

### Handling Binary (on/off) 

To handle outputs that can only be on or off, such as a heater, you use Pulse Width Modulation (PWM) to turn on the output for a period of time.   To do this use the Relay library that is included as standard.  

An example of connecting the RelayUpdate to the Relay class is below.  The RELAY_PIN is the output that controls the on/off and the DEFAULT_WINDOW_SIZE_SECS is the cycle time for the PWM.   As an example,  if output is 0.5, the output will be high for 1.5 seconds and low for 1.5 seconds.


    #include "Relay.h"

    const int DEFAULT_WINDOW_SIZE_SECS = 3;
    const int RELAY_PIN = 3; // Set to the pin to turn on and off.

    Relay relay(RELAY_PIN, DEFAULT_WINDOW_SIZE_SECS);

    
    class : public RelayUpdate {
      public:
    
        virtual void on() {
          relay.setRelayMode(relayModeAutomatic);
        }
        
        virtual void off(){
          relay.setRelayMode(relayModeManual);
          relay.setDutyCyclePercent(0.0);
        }
    
        virtual void update(double res){
          relay.setDutyCyclePercent(res);
        }
    } rs;

    void setup() {
      // ... Other setup code

      loop.setOutputLimits(ControlLoop::INNER, 0.0, 1.0);

      // ... Other setup code
    }



### Future topics to cover in Other
* PID tuning
* Cascade tuning

## API Documentation

### Constructors
        ControlLoop(DataSource*, RelayUpdate*, double)
        ControlLoop(DataSource*, DataSource*, RelayUpdate*, double)

The top constructor is for ONOFF and PID, if the algorithm is set to CASCADE it uses the single data source for both inner and outer PIDs.  The second constructor has the first DataSource class for the Outer PID and the second one for the inner PID.

The RelayUpdate class is used for the controlled variable and the last parameter, the double, is the initial setpoint for the targeted variable.  

### Data Source and Relay Update classes

The DataSource base class has one variable which is the current value of the measured variable.   This can be any unit, as long as it is returned as a double.


    class DataSource {
      public:
        virtual double get() = 0;
    };


The RelayUpdate is used to set the controlled variable, it is the class used to send back the result of the algorithm.  The main method is ``update()`` that is called each time the selected algorithm changes its output value.   

The two other methods, ``on()`` and ``off()``  are called at the start and end of the ControlLoop when ``setOn()`` and ``setOff()`` are called.   The ``off()`` method's implementation needs to at a minimum set what is being driven to a safe state.  E.g. for a heater, it should be turned off. 


    class RelayUpdate {
      public:
        virtual void on() = 0;
        virtual void off() = 0;
        virtual void update(double) = 0;
    };


### Compute

To enable and disable the  ControlLoop, the three methods are used to turn the control on and off.  The ``setOn()`` must be called before ``Compute()`` is called. as if set to off, the method will not run.

        void setOn()
        void setOff()
        bool isOn()

This method is the main method that turns the selected algorithm.  It will return true when it has been updated the output and recalculated.  It should be called in the ``loop()`` method so it is called regularly.  ControllLoop class will handle the timing of the update so it won't recalculate if insufficient time has passed since the last update.   This time period can be changed by calling ``setSampleTime(int)``.

The method will return true if an updated value has been passed into the RelayUpdate class. 

        bool Compute()


### Setting target value

The target setpoint of the measured value can be changed after the ControlLoop is created by calling the following method. 

        void setPoint(double)

Determining the current setPoint can be obtained by the following method with the special case method of ``getInnerSetPoint()`` is only for when using the Cascade PID.  This additional method is for information as the the inner PID's output controls the setPoint for the outer PID.

        double getSetPoint()
        double getInnerSetPoint()

### Setting which Control Algorithm to use
        static const int ONOFF
        static const int STD
        static const int CASCADE
        void setControlType(int)

        int getControlType()
        bool isControlOnOff()
        bool isControlStandardPID()
        bool isControlCascadePID()


### Enabling BangBang
        void enableBangBang()
        void disableBangBang()
        bool isBangBangOn()

        void setBangBangRange(double x)
        void setBangBangRange(double, double)
        double getBangBangLower()
        double getBangBangUpper()



### Configuring PID and Cascade
        static const int INNER
        static const int OUTER

        void setTunings(double, double, double)
        void setTunings(int, double, double, double)
        double getKp(int)
        double getKi(int)
        double getKd(int)

        void setSampleTime(int)
        void setOuterSampleFactor(int)

        void setOutputLimits(int, double, double)
        void setDirectionIncrease(int, bool)
        bool getDirectionIncrease(int)


