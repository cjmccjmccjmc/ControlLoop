
#ifndef _WATERBUCKETMODEL_H
#define _WATERBUCKETMODEL_H



/**
 * @brief A simple model of a leaking bucket with water pipe with a valve filling it up.
 * 
 */


class WaterBucketModel {

  public:

    WaterBucketModel(double maxInputLPerMinute, double leakOutputLPerMinute) { 
        maxInputPerSecond = maxInputLPerMinute / 60.0;
        leakPerSecond = leakOutputLPerMinute / 60.0;
        currentWaterLevel = 0.0;
    };


    void advance() {

        unsigned long nowTime = millis();

        unsigned long deltaTimeSeconds = (nowTime - lastUpdateCurrentTime)/1000;
        lastUpdateCurrentTime = nowTime;

        // Note, input and out are in Litres per minute, but time advances by second.
        double waterIn = valveOpen * maxInputPerSecond  * (double)deltaTimeSeconds;
        double waterOut = leakPerSecond * (double)deltaTimeSeconds;

        currentWaterLevel = currentWaterLevel + waterIn - waterOut;
        if ( currentWaterLevel < 0.0 ) {
            currentWaterLevel = 0.0;
        }

    }

    void changeValveOpening(double newOpening) {
        // Bound parameter to be between 0.0 and 1.0 as t
        if ( newOpening < 0.0 ) {
            valveOpen = 0.0;
        } else if ( newOpening > 1.0 ) {
            valveOpen = 1.0;
        } else {
            valveOpen = newOpening; 
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


};

#endif

