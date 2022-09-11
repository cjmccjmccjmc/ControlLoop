
#ifndef _WATERBUCKETMODEL_H
#define _WATERBUCKETMODEL_H



/**
 * @brief A more complex model of a leaking bucket with water pipe with a slow changing valve filling it up.
 * 
 */


class WaterBucketSlowValveModel {

  public:

    WaterBucketSlowValveModel(double maxInputLPerMinute, double leakOutputLPerMinute, int secondsFromCloseToOpen) { 
        maxInputPerSecond = maxInputLPerMinute / 60.0;
        leakPerSecond = leakOutputLPerMinute / 60.0;
        currentWaterLevel = 0.0;
        targetValveState = 0.0;
        changePercentInSeconds = (double)secondsFromCloseToOpen / 100.0;
    };


    void advance() {

        unsigned long nowTime = millis();

        double deltaTimeSeconds = (nowTime - lastUpdateCurrentTime)/1000.0;
        double currValve = valveOpen;
        double valveDelta = targetValveState - valveOpen;                
        valveOpen = valveOpen + (valveDelta*deltaTimeSeconds*changePercentInSeconds);

        if ( valveOpen < 0.0 ) { 
            valveOpen = 0.0;
        } else if ( valveOpen > 1.0 ) { 
            valveOpen = 1.0;
        }


        double valveState = (valveOpen+currValve)/2;

        lastUpdateCurrentTime = nowTime;

        // Note, input and out are in Litres per minute, but time advances by second.
        double waterIn = valveState * maxInputPerSecond  * (double)deltaTimeSeconds;
        double waterOut = leakPerSecond * (double)deltaTimeSeconds;

        currentWaterLevel = currentWaterLevel + waterIn - waterOut;
        if ( currentWaterLevel < 0.0 ) {
            currentWaterLevel = 0.0;
        }

    }

    void changeValveOpening(double newOpening) {

        targetValveState = newOpening;

        // Bound parameter to be between 0.0 and 1.0 as t
        if ( newOpening < 0.0 ) {
            targetValveState = 0.0;
        } else if ( newOpening > 1.0 ) {
            targetValveState = 1.0;
        } else {
            targetValveState = newOpening; 
        }
    }


    double getCurrentWaterLevel() { return currentWaterLevel; }

    double getValveOpen() { return valveOpen; }

  private:

    // Current time, in seconds
    unsigned long lastUpdateCurrentTime = millis();
;

    // In Litres
    double currentWaterLevel = 0.0;

    // Following in Litres per second
    double maxInputPerSecond;
    double leakPerSecond;

    // How open input valve is open, in percentage.
    // Must bound to 0.0 to 1.0
    double valveOpen = 0.0;

    // Target state for slow valve
    double targetValveState = 0.0;
    double changePercentInSeconds;


};

#endif

