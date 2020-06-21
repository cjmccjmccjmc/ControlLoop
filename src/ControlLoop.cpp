#include "ControlLoop.h"

//#define _SHOW_COMPUTE_TRACE(...) Serial.printf(__VA_ARGS__ )
#define _SHOW_COMPUTE_TRACE(...) 



ControlLoop::ControlLoop(DataSource* inDS, DataSource* otDS, RelayUpdate* r, double sp) :
  outer(&outerIn, &outerOutInnerSet, &outerSet, 1.0, 0.0, 0.0, DIRECT),
  inner(&innerIn, &innerOut, &outerOutInnerSet, 1.0, 0.0, 0.0, DIRECT) {

  _innerDataSource = inDS;
  _outerDataSource = otDS;
  _relay = r;
  _setpoint = sp;

  _outMin = 0.0;
  _outMax = 1.0;

  inner.SetOutputLimits(_outMin, _outMax);
  outer.SetOutputLimits(0.0, 90.0);

  _isOn = false;
  inner.SetMode(MANUAL);
  outer.SetMode(MANUAL);

  inner.SetSampleTime(_sampleTimeMS);
  outer.SetSampleTime(_sampleTimeMS*_sampleFactor);

  setControlType(STD);
  updateInputs();
}


PID* ControlLoop::getController(int c) {
  if ( c == ControlLoop::OUTER ) {
    return &outer;
  }
  return &inner;
}

void ControlLoop::updateInputs() {
  switch (this->_ControlState) {
  case ControlLoop::CASCADE:
    this->outerIn = _outerDataSource->get();
    this->innerIn = _innerDataSource->get();
    break;
    
  case ControlLoop::STD:
  case ControlLoop::ONOFF:
    this->outerIn = _outerDataSource->get();
    this->innerIn = this->outerIn;
    break;

  default:
    Serial.print(F("ERROR: unknown control type "));
    Serial.println(this->_ControlState);    
  }
}

bool ControlLoop::Compute() {

  bool updated = false;

  updateInputs();

  _SHOW_COMPUTE_TRACE("Control: %i bb: %i sp: %f, %f %f ", this->_ControlState, _bangBangOn, _setpoint, _setpointLower, _setpointUpper);

  // Jump out as off.
  if (! _isOn) {
    _SHOW_COMPUTE_TRACE("not on, returning\n");
    return false;
  }


  if ( this->_ControlState == ControlLoop::ONOFF ) {
    _SHOW_COMPUTE_TRACE("onoff ");

    if ( _bangBangOn ) {
      _SHOW_COMPUTE_TRACE("bb ");
      updated = true;

      if ( innerIn < _setpointLower ) {
        _SHOW_COMPUTE_TRACE("max ");
        innerOut = _outMax;

      } else if ( innerIn > _setpointUpper ) {
        _SHOW_COMPUTE_TRACE("min ");
        innerOut = _outMin;

      } else {
        _SHOW_COMPUTE_TRACE("n/a ");

        // No else to handle bewteen setpts
        // will continue previous setting
        // e.g. rising will continue
        // failing will continue
        ;

      }

    } else {
      _SHOW_COMPUTE_TRACE("!bb ");

      updated = true;
      if ( innerIn < _setpoint) {
        _SHOW_COMPUTE_TRACE("max ");
        innerOut = _outMax;

      } else if ( innerIn > _setpoint ) {
        _SHOW_COMPUTE_TRACE("min ");
        innerOut = _outMin;

      } else {
        // State is equal, keep doing previous.
        _SHOW_COMPUTE_TRACE("n/a ");
        ;
      }

    }



  } else if ( this->_ControlState == ControlLoop::STD ) {
    _SHOW_COMPUTE_TRACE("std ");

    if ( _bangBangOn && innerIn < _setpointLower ) {
      _SHOW_COMPUTE_TRACE("bb/low ");
      updated = true;
      innerOut = _outMax;

    } else if ( _bangBangOn && innerIn > _setpointUpper ) {
      _SHOW_COMPUTE_TRACE("bb/high ");
      updated = true;
      innerOut = _outMin;

    } else {
      _SHOW_COMPUTE_TRACE("compute ");
      updated = inner.Compute();
    }

  } else if ( this->_ControlState == ControlLoop::CASCADE ) {
    _SHOW_COMPUTE_TRACE("cas ");
    if ( _bangBangOn && innerIn < _setpointLower ) {
      _SHOW_COMPUTE_TRACE("bb/low ");
      updated = true;
      innerOut = _outMax;

    } else if ( _bangBangOn && innerIn > _setpointUpper ) {
      _SHOW_COMPUTE_TRACE("bb/high ");
      updated = true;
      innerOut = _outMin;

    } else {
      _SHOW_COMPUTE_TRACE("compute ");
      bool o = outer.Compute();
      bool i = inner.Compute();
      updated = i || o;
    }

  } else {
    Serial.print(F("ERROR: unknown control type "));
    Serial.println(this->_ControlState);
    updated = false;

  }

  if ( updated ) {
    _relay->update(innerOut);
  }

  _SHOW_COMPUTE_TRACE("upd rt: %i\n", updated);
  return updated;
}

void ControlLoop::setControlType(int ct) {

  this->_ControlState = ct;

  if ( this->_ControlState == ControlLoop::ONOFF ) {
    inner.SetMode(MANUAL);
    outer.SetMode(MANUAL);

  } else if ( this->_ControlState == ControlLoop::STD ) {
    inner.SetMode(AUTOMATIC);
    outer.SetMode(MANUAL);
    outerOutInnerSet = _setpoint;

  } else if ( this->_ControlState == ControlLoop::CASCADE ) {
    inner.SetMode(AUTOMATIC);
    outer.SetMode(AUTOMATIC);
    outerSet = _setpoint;
    outerOutInnerSet = _setpoint;

  } else {
    Serial.print(F("ERROR: unknown control type "));
    Serial.println(this->_ControlState);
  }

}

void ControlLoop::setPoint(double sp) {

  _setpoint = sp;
  _setpointLower = sp - _bangBangLower;
  _setpointUpper = sp + _bangBangUpper;
  setControlType(_ControlState);
}


double ControlLoop::getInnerSetPoint() {
  if (this->_ControlState == CASCADE ) {
    return outerOutInnerSet;
  }
  return -1.0;
}

void ControlLoop::setBangBangRange(double lower, double upper) {
  if ( lower > 0 && upper > 0 ) {
    _bangBangLower = lower;
    _bangBangUpper = upper;
    setPoint(_setpoint);
  }
}



void ControlLoop::setSampleTime(int ms) {
  _sampleTimeMS = ms;
  inner.SetSampleTime(_sampleTimeMS);
  outer.SetSampleTime(_sampleTimeMS*_sampleFactor);
}

void ControlLoop::setOuterSampleFactor(int factor) {
  _sampleFactor = factor;
  outer.SetSampleTime(_sampleTimeMS*_sampleFactor);
}


void ControlLoop::setOnOff(bool turnOn) {

  inner.SetMode(MANUAL);
  outer.SetMode(MANUAL);
  _isOn = turnOn;

  if ( _isOn ) {
    _relay->on();
    if ( this->_ControlState == ControlLoop::ONOFF ) {
      ;
    } else if ( this->_ControlState == ControlLoop::STD ) {
      inner.SetMode(AUTOMATIC);

    } else if ( this->_ControlState == ControlLoop::CASCADE ) {
      inner.SetMode(AUTOMATIC);
      outer.SetMode(AUTOMATIC);

    } else {
      Serial.print(F("ERROR: unknown control type "));
      Serial.println(this->_ControlState);
    }
  } else {
    _relay->off();
  }


}


void ControlLoop::setOutputLimits(int  c, double _min, double _max) {
  PID* thePid = this->getController(c);
  thePid->SetOutputLimits(_min, _max);

  if ( c == ControlLoop::INNER ) {
    _outMin = _min;
    _outMax = _max;
  }

}


void ControlLoop::setDirectionIncrease(int  c, bool dir) {
  PID *thePid = this->getController(c);
  if ( dir ) {
    thePid->SetControllerDirection(DIRECT);
  } else {
    thePid->SetControllerDirection(REVERSE);
  }
}

bool ControlLoop::getDirectionIncrease(int  c) {
  PID *thePid = this->getController(c);
  return thePid->GetDirection() == DIRECT;
}

void ControlLoop::setTunings(int  c, double p, double i, double d) {
  PID *thePid = this->getController(c);
  thePid->SetTunings(p, i, d);
}

double ControlLoop::getKp(int  c) {
  PID *thePid = this->getController(c);
  return thePid->GetKp();
}

double ControlLoop::getKi(int  c) {
  PID *thePid = this->getController(c);
  return thePid->GetKi();
}

double ControlLoop::getKd(int  c) {
  PID *thePid = this->getController(c);
  return thePid->GetKd();
}

