
#ifndef _CONTROLLOOP_H
#define _CONTROLLOOP_H

#include <Arduino.h>
#include <PID_v1.h>

class RelayUpdate {
  public:
    virtual void on() = 0;
    virtual void off() = 0;
    virtual void update(double) = 0;
};

class DataSource {
  public:
    virtual double get() = 0;
};



class ControlLoop {

  public:
    ControlLoop(DataSource*, DataSource*, RelayUpdate*, double);
    ControlLoop(DataSource* data, RelayUpdate* update, double setpoint) : ControlLoop(NULL, data, update, setpoint) {;};


    static const bool PID_DEBUG = true;

    static const int INNER  = 0;
    static const int OUTER = 1;

    static const int ONOFF = 10;
    static const int STD = 11;
    static const int CASCADE = 12;

    bool Compute();

    void setPoint(double);
    double getSetPoint() { return _setpoint; }
    double getInnerSetPoint();



    void enableBangBang() { _bangBangOn = true; }
    void disableBangBang() { _bangBangOn = false; }
    bool isBangBangOn() { return _bangBangOn;  }
    void setBangBangRange(double x) { setBangBangRange(-x, x); }
    void setBangBangRange(double, double);
    double getBangBangLower() { return _bangBangLower; }
    double getBangBangUpper() { return _bangBangUpper;}

    void setControlType(int);
    bool isControlOnOff() { return this->_ControlState == ControlLoop::ONOFF; }
    bool isControlStandardPID() { return this->_ControlState == ControlLoop::STD; }
    bool isControlCascadePID() { return this->_ControlState == ControlLoop::CASCADE; }
    int getControlType(){ return this->_ControlState; }


    void setSampleTime(int);
    void setOuterSampleFactor(int);

    bool isOn() { return _isOn; }
    void setOn() { setOnOff(true); }
    void setOff() { setOnOff(false); }

    void setOutputLimits(int, double, double);
    void setDirectionIncrease(int, bool);
    bool getDirectionIncrease(int);

    void setTunings(double p, double i, double d) { setTunings(ControlLoop::INNER, p, i, d);}
    void setTunings(int, double, double, double);
    double getKp(int);
    double getKi(int);
    double getKd(int);


  protected:

    void updateInputs();

    void setOnOff(bool);

  private:

    double outerIn = 0.0;
    double outerOutInnerSet = 0.0;
    double outerSet = 0.0;
    // Setpoint is shared with inner Out

    double innerIn = 0.0;
    double innerOut = 0.0;

    double _outMin = 0.0;
    double _outMax = 1.0;


    PID outer;
    PID inner;

    PID *getController(int c);

    long _sampleTimeMS = 2500;
    long _sampleFactor = 4; // Recommended to be 3-5 times

    int _ControlState = STD;

    double _setpoint;
    double _setpointLower;
    double _setpointUpper;

    bool _isOn = false;
    bool _bangBangOn = false;
    double _bangBangLower;
    double _bangBangUpper;


    DataSource *_innerDataSource = NULL;
    DataSource *_outerDataSource = NULL;
    RelayUpdate *_relay = NULL;

};

#endif

