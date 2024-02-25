/*  ------------------------------------------------------------------------------------
 *  Collection of transformation functions for Temperatur sensors
 *  ------------------------------------------------------------------------------------
 */   


 
 /*  ------------------------------------------------------------------------------------  
 *   parameters an function to read a pt-1000 temperature sensor (NTC). 
 *   
 *   wiring: 
 *   
 *   
 *      vcc --+----------+--------- 
 *            |          |
 *            |         [R] 1k
 *           ---         |
 *           --- 100nF   +-------------------- ADC in
 *            |          |
 *            |        v[R] pt-1000
 *            |          |
 *      GND --+----------+---------
 *
 *  source: https://www.mikrocontroller.net/topic/353666
*/



// Parameter for PT1000 temp sensor -------------------------------------------
#define ADC_MAX           1023            // with a 10 Bit ADC
#define ERR_VALUE         999             // result if pt1000 is disconnected
#define R_PT0             1000.0          // at 0 degree centigree
#define R_REF             1000.0          // ext. resistor with accuracy 0.1%
#define R_ZULEITUNG       0               // resistance for long lines
#define PT_FAKTOR         3.85            // calibrated factor
#define LINEAR_OFFESET    -3              // drift


int read_pt1000(int adValue)           
// takes the read of an ad-port and calculates to degrees centigrade
{
    int temperatur;
    float resistance;
    
    if(adValue < ADC_MAX) {                            // only valid values
      resistance = R_REF * adValue / (ADC_MAX - adValue);   // calculate resistance
      resistance = resistance - R_PT0 - R_ZULEITUNG;   //  decrement by offset for 0 degree
      temperatur = resistance  / PT_FAKTOR;      // scale to celsius scale
      temperatur += LINEAR_OFFESET;              // linear drift 
    } 
    else 
       temperatur = ERR_VALUE;        // if PT1000 is disconnected
    return temperatur;
}


/*  --------------------------------------------------------------------
 *  Temperaturesensor from DELL battery packs (PTC)
 *   wiring: 
 *   
 *   
 *      vcc --+----------+--------- 
 *            |          |
 *            |        v[R] dell battery sensor
 *           ---         |
 *           --- 100nF   +-------------------- ADC in
 *            |          |
 *            |         [R] 10k
 *            |          |
 *      GND --+----------+---------
*/

int read_dellakku_tempsensor(int adValue) {
    int interpolated = int((0.115 * float(adValue)) - 34); 
    return interpolated;
}
