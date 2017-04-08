/*
 *
 * sickio.cc
 *
 *  Created on: Mar 2, 2014
 *      Author: bst
 */

/*!
 * \file sickio.cc
 * \brief A simple application using the Sick LMS 5xx driver.
 *
 * Code by Jason C. Derenick and Christopher R. Mansley.
 * Contact jasonder(at)seas(dot)upenn(dot)edu
 *
 * The Sick LIDAR Matlab/C++ Toolbox
 * Copyright (c) 2009, Jason C. Derenick and Christopher R. Mansley
 * All rights reserved.
 *
 * This software is released under a BSD Open-Source License.
 * See http://sicktoolbox.sourceforge.net
 */

#include <stdio.h>
#include <string>
#include <ostream>
#include <sicklms5xx/SickLMS5xx.hh>

using namespace SickToolbox;
using namespace std;

int main(int argc, char* argv[])
{

  /*
   * Instantiate an instance
   */
  SickLMS5xx sick_lms_5xx;

  /*
   * Initialize the Sick LMS 2xx
   */
  try {
    sick_lms_5xx.Initialize();
  }

  catch(...) {
    fprintf(stderr,"Initialize failed! Are you using the correct IP address?\n");
    return -1;
  }

  try {
    unsigned int status = 1;
    unsigned int num_measurements = 0;
    unsigned int range_1_vals[SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS];
    //unsigned int range_2_vals[SickLMS5xx::SICK_LMS_5XX_MAX_NUM_MEASUREMENTS];
    //sick_lms_5xx.SetSickScanFreqAndRes(SickLMS5xx::SICK_LMS_5XX_SCAN_FREQ_25,
    //SickLMS5xx::SICK_LMS_5XX_SCAN_RES_25);
    //sick_lms_5xx.SetSickScanDataFormat(SickLMS5xx::SICK_LMS_5XX_DIST_DOUBLE_PULSE,
    //				         SickLMS5xx::SICK_LMS_5XX_REFLECT_NONE);
    sick_lms_5xx.SetSickScanDataFormat(SickLMS5xx::SICK_LMS_5XX_SCAN_FORMAT_UNKNOWN);
    for (int i = 0; i < 1000; i++) {
      sick_lms_5xx.GetSickMeasurements(range_1_vals, NULL, NULL, NULL, NULL,
                                       range_1_vals, NULL, NULL, NULL, NULL,
                                       num_measurements,&status);
      printf("%d: %d %d\n",i,num_measurements,status);
    }
  }

  catch(const SickConfigException & sick_exception) {
    printf("%s\n",sick_exception.what());
  }

  catch(const SickIOException & sick_exception) {
    printf("%s\n",sick_exception.what());
  }

  catch(const SickTimeoutException & sick_exception) {
    printf("%s\n",sick_exception.what());
  }

  catch(...) {
    fprintf(stderr,"An Error Occurred!\n");
    return -1;
  }


  /*
   * Uninitialize the device
   */
  try {
    sick_lms_5xx.Uninitialize();
  }

  catch(...) {
    fprintf(stderr,"Uninitialize failed!\n");
    return -1;
  }

  /* Success! */
  return 0;

}



