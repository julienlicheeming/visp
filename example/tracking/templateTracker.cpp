/****************************************************************************
 *
 * $Id$
 *
 * This file is part of the ViSP software.
 * Copyright (C) 2005 - 2013 by INRIA. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact INRIA about acquiring a ViSP Professional
 * Edition License.
 *
 * See http://www.irisa.fr/lagadic/visp/visp.html for more information.
 *
 * This software was developed at:
 * INRIA Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 * http://www.irisa.fr/lagadic
 *
 * If you have questions regarding the use of this file, please contact
 * INRIA at visp@inria.fr
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Description:
 * Example of template tracking.
 *
 * Authors:
 * Amaury Dame
 * Aurelien Yol
 * Fabien Spindler
 *
 *****************************************************************************/

/*!
  \example templateTracker.cpp

  \brief Example of template tracking.
*/

#include <visp/vpConfig.h>
#include <visp/vpDebug.h>
#include <visp/vpDisplayD3D.h>
#include <visp/vpDisplayGTK.h>
#include <visp/vpDisplayGDI.h>
#include <visp/vpDisplayOpenCV.h>
#include <visp/vpDisplayX.h>
#include <visp/vpHomogeneousMatrix.h>
#include <visp/vpImageIo.h>
#include <visp/vpIoTools.h>
#include <visp/vpMath.h>
#include <visp/vpVideoReader.h>
#include <visp/vpParseArgv.h>

#include <visp/vpTemplateTrackerSSD.h>
#include <visp/vpTemplateTrackerSSDForwardAdditional.h>
#include <visp/vpTemplateTrackerSSDForwardCompositional.h>
#include <visp/vpTemplateTrackerSSDInverseCompositional.h>
#include <visp/vpTemplateTrackerSSDESM.h>
#include <visp/vpTemplateTrackerZNCCForwardAdditional.h>
#include <visp/vpTemplateTrackerZNCCInverseCompositional.h>

#include <visp/vpTemplateTrackerWarpAffine.h>
#include <visp/vpTemplateTrackerWarpHomography.h>
#include <visp/vpTemplateTrackerWarpHomographySL3.h>
#include <visp/vpTemplateTrackerWarpSRT.h>
#include <visp/vpTemplateTrackerWarpTranslation.h>

#if defined (VISP_HAVE_DISPLAY)


#define GETOPTARGS  "cdhi:l:pt:w:"

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef enum {
  WARP_AFFINE,
  WARP_HOMOGRAPHY,
  WARP_HOMOGRAPHY_SL3,
  WARP_SRT,
  WARP_TRANSLATION
} WarpType;

typedef enum {
  TRACKER_SSD_ESM,
  TRACKER_SSD_FORWARD_ADDITIONAL,
  TRACKER_SSD_FORWARD_COMPOSITIONAL,
  TRACKER_SSD_INVERSE_COMPOSITIONAL, // The most efficient
  TRACKER_ZNCC_FORWARD_ADDITIONEL,
  TRACKER_ZNCC_INVERSE_COMPOSITIONAL
} TrackerType;

#endif


void usage(const char *name, const char *badparam, const WarpType &warp_type,
           TrackerType &tracker_type, const long &last_frame)
{
  fprintf(stdout, "\n\
Example of template tracking.\n\
\n\
SYNOPSIS\n\
  %s [-i <test image path>] [-c] [-d] [-p] \n\
     [-w <warp type>] [-t <tracker type>] \n\
     [-l <last frame number>] [-h]\n", name );

  fprintf(stdout, "\n\
OPTIONS:                                                            Default\n\
  -i <input image path>                                \n\
     Set image input path.\n\
     From this path read images \n\
     \"ViSP-images/cube/image%%04d.pgm\". These \n\
     images come from ViSP-images-x.y.z.tar.gz available \n\
     on the ViSP website.\n\
     Setting the VISP_INPUT_IMAGE_PATH environment\n\
     variable produces the same behaviour than using\n\
     this option.\n\
          \n\
  -l <last frame number>                                              %ld\n\
     Last frame number to consider.\n\
          \n\
  -d \n\
     Turn off the display.\n\
          \n\
  -c\n\
     Disable the mouse click. Useful to automaze the \n\
     execution of this program without humain intervention.\n\
          \n\
  -w <warp type=[0,1,2,3,4]>                                          %d\n\
     Set the model used to warp the template. \n\
     Authorized values are:\n\
     %d : Affine\n\
     %d : Homography\n\
     %d : Homography in SL3\n\
     %d : SRT (scale, rotation, translation)\n\
     %d : Translation\n\
                  \n\
  -t <tracker type=[0,1,2,3,4,5]>                                     %d\n\
     Set the tracker used to track the template. \n\
     Authorized values are:\n\
     %d : SSD ESM\n\
     %d : SSD forward additional\n\
     %d : SSD forward compositional\n\
     %d : SSD inverse compositional\n\
     %d : ZNCC forward additional\n\
     %d : ZNCC inverse compositional\n\
                  \n\
  -p\n\
     Enable pyramidal tracking.\n\
                  \n\
  -h \n\
     Print the help.\n\n",
          last_frame, (int)warp_type,
          (int)WARP_AFFINE, (int)WARP_HOMOGRAPHY, (int)WARP_HOMOGRAPHY_SL3, (int)WARP_SRT, (int)WARP_TRANSLATION,
          (int)tracker_type,
          (int)TRACKER_SSD_ESM, (int)TRACKER_SSD_FORWARD_ADDITIONAL, (int)TRACKER_SSD_FORWARD_COMPOSITIONAL,
          (int)TRACKER_SSD_INVERSE_COMPOSITIONAL, (int)TRACKER_ZNCC_FORWARD_ADDITIONEL,
          (int)TRACKER_ZNCC_INVERSE_COMPOSITIONAL);
  if (badparam)
    fprintf(stdout, "\nERROR: Bad parameter [%s]\n", badparam);
}


bool getOptions(int argc, const char **argv, std::string &ipath, bool &click_allowed,
                bool &display, bool &pyramidal, WarpType &warp_type, TrackerType &tracker_type,
                long &last_frame)
{
  const char *optarg;
  int   c;
  while ((c = vpParseArgv::parse(argc, argv, GETOPTARGS, &optarg)) > 1) {

    switch (c) {
    case 'c': click_allowed = false; break;
    case 'd': display = false; break;
    case 'h': usage(argv[0], NULL, warp_type, tracker_type, last_frame); return false; break;
    case 'i': ipath = optarg; break;
    case 'l': last_frame = (long)atoi(optarg); break;
    case 'p': pyramidal = true; break;
    case 't': tracker_type = (TrackerType)atoi(optarg); break;
    case 'w': warp_type = (WarpType)atoi(optarg); break;

    default:
      usage(argv[0], optarg, warp_type, tracker_type, last_frame);
      return false; break;
    }
  }

  if (warp_type > WARP_TRANSLATION) {
    usage(argv[0], NULL, warp_type, tracker_type, last_frame);
    std::cerr << "ERROR: " << std::endl;
    std::cerr << "  Bad argument -w <warp type> with \"warp type\"=" << warp_type << std::endl << std::endl;
    return false;
  }
  if (tracker_type > TRACKER_ZNCC_INVERSE_COMPOSITIONAL) {
    usage(argv[0], NULL, warp_type, tracker_type, last_frame);
    std::cerr << "ERROR: " << std::endl;
    std::cerr << "  Bad argument -t <tracker type> with \"tracker type\"=" << tracker_type << std::endl << std::endl;
    return false;
  }
  if ((c == 1) || (c == -1)) {
    // standalone param or error
    usage(argv[0], NULL, warp_type, tracker_type, last_frame);
    std::cerr << "ERROR: " << std::endl;
    std::cerr << "  Bad argument " << optarg << std::endl << std::endl;
    return false;
  }

  return true;
}

int
main(int argc, const char ** argv)
{
  try {
    std::string env_ipath;
    std::string opt_ipath;
    std::string ipath;
    bool opt_click_allowed = true;
    bool opt_display = true;
    bool opt_pyramidal = false;
    TrackerType opt_tracker_type = TRACKER_SSD_INVERSE_COMPOSITIONAL;
    WarpType opt_warp_type = WARP_AFFINE;
    long opt_last_frame = 30;

    // Get the VISP_IMAGE_PATH environment variable value
    char *ptenv = getenv("VISP_INPUT_IMAGE_PATH");
    if (ptenv != NULL)
      env_ipath = ptenv;

    // Set the default input path
    if (! env_ipath.empty())
      ipath = env_ipath;

    // Read the command line options
    if (!getOptions(argc, argv, opt_ipath, opt_click_allowed, opt_display, opt_pyramidal,
                    opt_warp_type, opt_tracker_type, opt_last_frame)) {
      return (-1);
    }

    // Test if an input path is set
    if (opt_ipath.empty() && env_ipath.empty() ){
      usage(argv[0], NULL, opt_warp_type, opt_tracker_type, opt_last_frame);
      std::cerr << std::endl
                << "ERROR:" << std::endl;
      std::cerr << "  Use -i <visp image path> option or set VISP_INPUT_IMAGE_PATH "
                << std::endl
                << "  environment variable to specify the location of the " << std::endl
                << "  image path where test images are located." << std::endl
                << std::endl;

      return (-1);
    }

    // Get the option values
    if (!opt_ipath.empty())
      ipath = opt_ipath + vpIoTools::path("/ViSP-images/mire-2/image.%04d.pgm");
    else
      ipath = env_ipath + vpIoTools::path("/ViSP-images/mire-2/image.%04d.pgm");

    vpImage<unsigned char> I;
    vpVideoReader reader;

    reader.setFileName(ipath.c_str());
    reader.setFirstFrameIndex(1);
    reader.setLastFrameIndex(opt_last_frame);
    try{
      reader.open(I);
    }catch(...){
      std::cout << "Cannot open sequence: " << ipath << std::endl;
      return -1;
    }
    reader.acquire(I);

    vpDisplay *display = NULL;
    if (opt_display)
    {
      // initialise a  display
#if defined VISP_HAVE_X11
      display = new vpDisplayX;
#elif defined VISP_HAVE_GDI
      display = new vpDisplayGDI;
#elif defined VISP_HAVE_OPENCV
      display = new vpDisplayOpenCV;
#elif defined VISP_HAVE_D3D9
      display = new vpDisplayD3D;
#elif defined VISP_HAVE_GTK
      display = new vpDisplayGTK;
#else
      opt_display = false;
#endif
#if (defined VISP_HAVE_DISPLAY)
      display->init(I, 100, 100, "Test tracking") ;
#endif
      vpDisplay::display(I) ;
      vpDisplay::flush(I);
    }

    vpTemplateTrackerWarp *warp = NULL;
    switch(opt_warp_type) {
    case WARP_AFFINE:         warp = new vpTemplateTrackerWarpAffine; break;
    case WARP_HOMOGRAPHY:     warp = new vpTemplateTrackerWarpHomography; break;
    case WARP_HOMOGRAPHY_SL3: warp = new vpTemplateTrackerWarpHomographySL3; break;
    case WARP_SRT:            warp = new vpTemplateTrackerWarpSRT; break;
    case WARP_TRANSLATION:    warp = new vpTemplateTrackerWarpTranslation;  break;
    }

    vpTemplateTracker *tracker = NULL;
    switch(opt_tracker_type) {
    case TRACKER_SSD_ESM:                    tracker = new vpTemplateTrackerSSDESM(warp); break;
    case TRACKER_SSD_FORWARD_ADDITIONAL:     tracker = new vpTemplateTrackerSSDForwardAdditional(warp); break;
    case TRACKER_SSD_FORWARD_COMPOSITIONAL:  tracker = new vpTemplateTrackerSSDForwardCompositional(warp); break;
    case TRACKER_SSD_INVERSE_COMPOSITIONAL:  tracker = new vpTemplateTrackerSSDInverseCompositional(warp); break;
    case TRACKER_ZNCC_FORWARD_ADDITIONEL:    tracker = new vpTemplateTrackerZNCCForwardAdditional(warp); break;
    case TRACKER_ZNCC_INVERSE_COMPOSITIONAL: tracker = new vpTemplateTrackerZNCCInverseCompositional(warp); break;
    }

    tracker->setSampling(2,2);
    tracker->setLambda(0.001);
    tracker->setThresholdGradient(60.);
    tracker->setIterationMax(800);
    if (opt_pyramidal) {
      tracker->setPyramidal(2, 1);
    }
    bool delaunay = false;
    if (opt_display && opt_click_allowed)
      tracker->initClick(I, delaunay);
    else {
      std::vector<vpImagePoint> v_ip;
      vpImagePoint ip;
      ip.set_ij(166, 54);  v_ip.push_back(ip);
      ip.set_ij(284, 55);  v_ip.push_back(ip);
      ip.set_ij(259, 284); v_ip.push_back(ip); // ends the first triangle
      ip.set_ij(259, 284); v_ip.push_back(ip); // start the second triangle
      ip.set_ij(149, 240); v_ip.push_back(ip);
      ip.set_ij(167, 58);  v_ip.push_back(ip);

      tracker->initFromPoints(I, v_ip, false);
    }

    while (! reader.end())
    {
      std::cout << "Process image number " << reader.getFrameIndex() << std::endl;
      // Acquire a new image
      reader.acquire(I);
      // Display the image
      vpDisplay::display(I);
      // Track the template
      tracker->track(I);

      // Simulate a re-init
      if (reader.getFrameIndex() == 10){
        std::cout << "re-init simulation" << std::endl;
        if (opt_click_allowed)
          vpDisplay::getClick(I);

        tracker->resetTracker();

        if (opt_display && opt_click_allowed) {
          vpDisplay::displayCharString(I, 10, 10, "Re-init simulation", vpColor::red);
          vpDisplay::flush(I);
          tracker->initClick(I, delaunay);
        }
        else {
          std::vector<vpImagePoint> v_ip;
          vpImagePoint ip;
          ip.set_ij(146, 60);  v_ip.push_back(ip);
          ip.set_ij(254, 74);  v_ip.push_back(ip);
          ip.set_ij(228, 288); v_ip.push_back(ip); // ends the first triangle
          ip.set_ij(228, 288); v_ip.push_back(ip); // start the second triangle
          ip.set_ij(126, 242); v_ip.push_back(ip);
          ip.set_ij(146, 60);  v_ip.push_back(ip);

          tracker->initFromPoints(I, v_ip, false);
        }
      }

      // Display the template
#if 1
      tracker->display(I, vpColor::red, 3);
#else
      vpTemplateTrackerZone zoneWarp_ = tracker->getZoneWarp();
      vpTemplateTrackerWarp *warp_ = tracker->getWarp();
      vpColVector p_= tracker->getp();
      warp_->warpZone(zoneWarp_, p_);
      zoneWarp_.display(I, vpColor::red, 3);
#endif

      vpDisplay::flush(I) ;
    }
    if (opt_click_allowed) {
      vpDisplay::displayCharString(I, 10,10, "A click to exit...", vpColor::red);
      vpDisplay::flush(I) ;
      vpDisplay::getClick(I) ;
    }
    reader.close();
    if (display)
      delete display;

    delete warp;
    delete tracker;

    return 0;
  }
  catch(vpException e)
  {
    std::cout << "Catch an exception: " << e << std::endl;
    return -1;
  }
}

#else

int main()
{
  std::cout << "No display is available." << std::endl;
  return 0;
}

#endif