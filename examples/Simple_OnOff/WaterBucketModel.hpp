
#ifndef _WATERBUCKETMODEL_H
#define _WATERBUCKETMODEL_H



/**
 * @brief A simple model of a leaking bucket with water pipe with a valve filling it up.
 * 
 */


class WaterBucketModel {

  public:

    WaterBucketModel(double maxInputLPerMinute, double leakOutputLPerMinute): maxInput(maxInputLPerMinute), leak(leakOutputLPerMinute) { ; };


    void advance(long timeInSeconds) {

        // Note, input and out are in Litres per minute, but time advances by second.
        double waterIn = valveOpen * maxInput / 60.0 * timeInSeconds;
        double waterOut = leak / 60 * timeInSeconds;

        currentWaterLevel = currentWaterLevel + waterIn - waterOut;
        if ( currentWaterLevel < 0.0 ) {
            currentWaterLevel = 0.0;
        }
        theCurrentTime = theCurrentTime + timeInSeconds;

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

    long getCurrentTime() { return theCurrentTime; }

    double getValveOpen() { return valveOpen; }

  private:

    // Current time, in seconds
    long theCurrentTime = 0;

    // In Litres
    double currentWaterLevel = 0.0;

    // Following in Litres per minute
    double maxInput;
    double leak;

    // How open input valve is open, in percentage.
    // Must bound to 0.0 to 1.0
    double valveOpen = 0.0;


};

#endif

