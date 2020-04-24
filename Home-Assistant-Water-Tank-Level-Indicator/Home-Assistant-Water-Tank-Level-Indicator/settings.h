
/////////////////////////////////////////////////////////////////
//                           Settings                          //
/////////////////////////////////////////////////////////////////

int BUILD_NUMBER = 0;                         // Build number is something like change some variables in code and building binary file. 


/***************************************************
 *      Hardware Settings
 **************************************************/

#define ECHOPIN 14                            // Echo Pin D5
#define TRIGPIN 12                            // Trigger Pin D6
                         

/***************************************************
 *        Variable Settings
 **************************************************/

#define FILTERSAMPLES 21                      // filterSamples should  be an odd number, no smaller than 3

//Our rectangular tank capacity in liters; 39,600 Liters full tank capacity
//(length x width x height)/1000(to convert to ltrs) = (472*290*274)/1000=37505120/1000 = 37505.12 liters; I just used 37,000 Ltrs

#define FULLTANK 37000                        //in liters                         
#define MINIMUMRANGE 20                       // sensor needs minimum 20 cm distance to work


#define WATER_TANK_LENGTH_IN_CM 472           // Water tank lenght in cm
#define WATER_TANK_WIDTH_IN_CM 290            // water tank width in cm

//Tank original height(depth) is: 310cm. Sensor takes 2 inches (5.08 cm) of height and sensor need minimum 20cm distance to work.
// And I keep 10cm keep extra margin.
//So from our total tank height we need reduce from extra space left in tank that is, (sensor height + sensor minimum distance)
//Waste tank height = 5.08cm + 20cm + 10cm = 35.08cm.  New tank height = original tank height - waste tank height.
//New tank height = 310cm - 35.08cm = 274.92cm; I consider 274cm.

#define WATER_TANK_HEIGHT_IN_CM 274           // 274cm is after reducing 20cm + 5.08cm +10cm height from original tank height to work sensor

/***************************************************
 * 
 *        Server Settings
 **************************************************/
      
#define OTA_HOSTNAME "Water Level Indicator"
 
