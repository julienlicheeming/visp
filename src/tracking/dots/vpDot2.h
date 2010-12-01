 /****************************************************************************
  *
  * $Id: vpDot2.h 2135 2009-04-29 13:51:31Z fspindle $
  *
  * This file is part of the ViSP software.
  * Copyright (C) 2005 - 2010 by INRIA. All rights reserved.
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
  * Track a white dot.
  *
  * Authors:
  * Fabien Spindler
  *
  *****************************************************************************/


/*
  \file vpDot2.h
  \brief This tracker is meant to track some zones on a vpImage.
*/

#ifndef vpDot2_hh
#define vpDot2_hh


#include <visp/vpConfig.h>
#include <visp/vpList.h>
#include <visp/vpImage.h>
#include <visp/vpRect.h>
#include <visp/vpTracker.h>
#include <visp/vpColor.h>
#include <visp/vpImagePoint.h>


/*!
  \class vpDot2

  \ingroup TrackingImageBasic

  \brief This tracker is meant to track a dot (connex pixels with same
  gray level) on a vpImage.

  The underground algorithm is based on a binarisation of the image
  and then a contour detection using the Freeman chain coding to
  determine the dot characteristics (location, moments, size...).

  The center of gravity of a vpDot2 zone has to be of the right color
  level. You can specify these color levels by setGrayLevelMin() and
  setGrayLevelMax(). This allows to track white objects on a black background
  and vice versa.

  The geometry of a vpDot2 zone is by default ellipsoid. If you want to track a
  non ellipsoid shape, you have to call setEllipsoidShapePrecision(0).

  track() and searchDotsInArea() are the most important features
  of this class.

  - track() estimate the current position of the dot using its previous
    position, then try to compute the new parameters of the dot. If everything
    went ok, tracking succeeds, otherwise we search this dot in a window
    around the last position of the dot.

  - searchDotsInArea() enable to find dots similar to this dot in a window. It
    is used when there was a problem performing basic tracking of the dot, but
    can also be used to find a certain type of dots in the full image.

  \sa vpDot
*/
class VISP_EXPORT vpDot2 : public vpTracker
{
public:
  vpDot2();
  vpDot2(const vpImagePoint &ip) ;
  vpDot2(const vpDot2& twinDot );
  virtual ~vpDot2();
  void operator=(const vpDot2& twinDot );

  /*!
    Initialize the dot coordinates with \e cog. 
  */
  inline void setCog(const vpImagePoint &cog) {
    this->cog = cog; 
  }
  /*!
    Activates the display of the border of the dot during the tracking.

    \warning To effectively display the dot graphics a call to
    vpDisplay::flush() is needed.

    \param activate If true, the border of the dot will be painted. false to
    turn off border painting.

  */
  void setGraphics(const bool activate) { graphics = activate ; }
  /*!

    Activates the dot's moments computation.

    \param activate true, if you want to compute the moments. If false, moments
    are not computed.

    Computed moment are vpDot::m00, vpDot::m10, vpDot::m01, vpDot::m11,
    vpDot::m20, vpDot::m02.

    The coordinates of the region's centroid (u, v) can be computed from the
    moments by \f$u=\frac{m10}{m00}\f$ and  \f$v=\frac{m01}{m00}\f$.

  */
  void setComputeMoments(const bool activate) { compute_moment = activate; }
  void setWidth( const double & width );
  void setHeight( const double & height );
  void setSurface( const double & surface );
  /*!

  Set the color level of the dot to search a dot in an area. This level will be
  used to know if a pixel in the image belongs to the dot or not. Only pixels
  with higher level can belong to the dot.  If the level is lower than the
  minimum level for a dot, set the level to MIN_IN_LEVEL.

  \param min : Color level of a dot to search in an area.

  \sa setGrayLevelMax(), setGrayLevelPrecision()

  */
  inline void setGrayLevelMin( const unsigned int & min ) {
    if (min > 255)
      this->gray_level_min = 255;
    else
      this->gray_level_min = min;
  };

  /*!

  Set the color level of pixels surrounding the dot. This is meant to be used
  to search a dot in an area.

  \param max : Intensity level of a dot to search in an area.

  \sa  setGrayLevelMin(), setGrayLevelPrecision()
  */
  inline void setGrayLevelMax( const unsigned int & max ) {
    if (max > 255)
      this->gray_level_max = 255;
    else
      this->gray_level_max = max;
  };
  void setGrayLevelPrecision( const double & grayLevelPrecision );
  void setSizePrecision( const double & sizePrecision );
  void setEllipsoidShapePrecision(const double & ellipsoidShapePrecision);
  void setMaxSizeSearchDistancePrecision(const double & maxSizeSearchDistancePrecision);

  void initTracking(vpImage<unsigned char>& I, unsigned int size = 0);
  void initTracking(vpImage<unsigned char>& I, const vpImagePoint &ip,
		    unsigned int size = 0);
  void initTracking(vpImage<unsigned char>& I, const vpImagePoint &ip,
		    unsigned int gray_level_min, unsigned int gray_level_max,
		    unsigned int size = 0 );

  void track(vpImage<unsigned char> &I);
  void track(vpImage<unsigned char> &I, vpImagePoint &cog);

  /*!
    Return the location of the dot center of gravity.

    \return The coordinates of the center of gravity.
  */
  inline vpImagePoint getCog() const {
    return cog;
  }

  /*!

    Return the list of all the image points on the dot
    border.

    \param ip_edges_list : The list of all the images points on the dot
    border. This list is update after a call to track().

  */
  void getEdges(vpList<vpImagePoint> &ip_edges_list) { 
    ip_edges_list = this->ip_edges_list;
  };
  void getFreemanChain(vpList<int> &freeman_chain) ;

  double getWidth() const;
  double getHeight() const;
  double getSurface() const;
  /*!

    Return the dot bounding box.

    \sa getWidth(), getHeight()

  */
  inline vpRect getBBox() {
    vpRect bbox;

    bbox.setRect(this->bbox_u_min,
		 this->bbox_v_min,
		 this->bbox_u_max - this->bbox_u_min + 1,
		 this->bbox_v_max - this->bbox_v_min + 1);

    return (bbox);
  };
  /*!
    Return the color level of pixels inside the dot.

    \sa getGrayLevelMax()
  */
  inline unsigned int getGrayLevelMin() const {
    return gray_level_min;
  };
  /*!
    Return the color level of pixels inside the dot.

    \sa getGrayLevelMin()
  */
  inline unsigned int getGrayLevelMax() const {
    return gray_level_max;
  };
  /*!

  \return The mean gray level value of the dot.

  */
  double getMeanGrayLevel() {
    return (this->mean_gray_level);
  };
  double getGrayLevelPrecision() const;
  double getSizePrecision() const;
  double getEllipsoidShapePrecision() const;
  double getMaxSizeSearchDistancePrecision() const;
  inline double getGamma() {return this->gamma;};

  void display(vpImage<unsigned char>& I, vpColor color = vpColor::red,
		     unsigned int thickness=1);

  double getDistance( const vpDot2& distantDot ) const;

  vpList<vpDot2>* searchDotsInArea( vpImage<unsigned char>& I,
            int area_u, int area_v,
            int area_w, int area_h );

  vpList<vpDot2>* searchDotsInArea( vpImage<unsigned char>& I );

  static vpMatrix defineDots(vpDot2 dot[], const unsigned int &n, const std::string &dotFile, vpImage<unsigned char>* I = NULL, vpColor col = vpColor::blue, bool trackDot = true);

private :

  virtual bool isValid(vpImage<unsigned char>& I, const vpDot2& wantedDot);

  virtual bool hasGoodLevel(const vpImage<unsigned char>& I,
			    const unsigned int &u,
			    const unsigned int &v) const;
  virtual bool hasReverseLevel(vpImage<unsigned char>& I,
			       const unsigned int &u,
			       const unsigned int &v) const;

  virtual vpDot2* getInstance();



public:
  /*!
    Writes the dot center of gravity coordinates in the frame (i,j) (For more details 
    about the orientation of the frame see the vpImagePoint documentation) to the stream \e os,
    and returns a reference to the stream. 
  */
  friend std::ostream& operator<< (std::ostream& os, vpDot2& d) {
    return (os << "(" << d.getCog() << ")" ) ;
  } ;

  void print(std::ostream& os) { os << *this << std::endl ; }


  double m00; /*!< Considering the general distribution moments for \f$ N \f$
		points defined by the relation \f$ m_{ij} = \sum_{h=0}^{N}
		u_h^i v_h^j \f$, \f$ m_{00} \f$ is a zero order moment obtained
		with \f$i = j = 0 \f$. This moment corresponds to the dot
		surface.

		\sa setComputeMoments()
	      */
  double m10; /*!< Considering the general distribution moments for \f$ N \f$
		points defined by the relation \f$ m_{ij} = \sum_{h=0}^{N}
		u_h^i v_h^j \f$, \f$ m_{10} \f$ is a first order moment
		obtained with \f$i = 1 \f$ and \f$ j = 0 \f$. \f$ m_{10} \f$
		corresponds to the inertia first order moment along the v axis.

		\sa setComputeMoments()
	      */
  double m01; /*!< Considering the general distribution moments for \f$ N \f$
		points defined by the relation \f$ m_{ij} = \sum_{h=0}^{N}
		u_h^i v_h^j \f$, \f$ m_{01} \f$ is a first order moment
		obtained with \f$i = 0 \f$ and \f$ j = 1 \f$. \f$ m_{01} \f$
		corresponds to the inertia first order moment along the u axis.

		\sa setComputeMoments()
	      */
  double m11; /*!< Considering the general distribution moments for \f$ N \f$
		points defined by the relation \f$ m_{ij} = \sum_{h=0}^{N}
		u_h^i v_h^j \f$, \f$ m_{11} \f$ is a first order moment
		obtained with \f$i = 1 \f$ and \f$ j = 1 \f$.

		\sa setComputeMoments()
	      */
  double m20; /*!< Considering the general distribution moments for \f$ N \f$
		points defined by the relation \f$ m_{ij} = \sum_{h=0}^{N}
		u_h^i v_h^j \f$, \f$ m_{20} \f$ is a second order moment
		obtained with \f$i = 2 \f$ and \f$ j = 0 \f$. \f$ m_{20} \f$
		corresponds to the inertia second order moment along the v
		axis.

		\sa setComputeMoments()
	      */
  double m02; /*!< Considering the general distribution moments for \f$ N \f$
		points defined by the relation \f$ m_{ij} = \sum_{h=0}^{N}
		u_h^i v_h^j \f$, \f$ m_{02} \f$ is a second order moment
		obtained with \f$i = 0 \f$ and \f$ j = 2 \f$. \f$ m_{02} \f$
		corresponds to the inertia second order moment along the u
		axis.

		\sa setComputeMoments()
	      */
  double mu11;/*!< \f$ \mu_{11} \f$ is a second order central moments defined by:
		\f$ \mu_{11} = m_{11} - \frac{m_{10}}{m_{00}}m_{01} \f$
		
		\sa setComputeMoments()
	      */
  double mu20;/*!< \f$ \mu_{20} \f$ is a second order central moments defined by:
		\f$ \mu_{20} = m_{20} - \frac{m_{10}}{m_{00}}m_{10} \f$
		
		\sa setComputeMoments()
	      */
  double mu02;/*!< \f$ \mu_{02} \f$ is a second order central moments defined by:
		\f$ \mu_{02} = m_{02} - \frac{m_{01}}{m_{00}}m_{01} \f$
		
		\sa setComputeMoments()
	      */

private:
  void init();

  bool computeParameters(const vpImage<unsigned char> &I,
			 const double &u = -1.0,
			 const double &v = -1.0);



  bool findFirstBorder(const vpImage<unsigned char> &I, const unsigned int &u,
                        const unsigned int &v, unsigned int &border_u,
                        unsigned int &border_v);
  void computeMeanGrayLevel(const vpImage<unsigned char>& I);

  /*!

  Get the starting point on a dot border. The dot border is
  computed from this point.

  \sa getFirstBorder_v()

  */
  int getFirstBorder_u() const {
    return this->firstBorder_u;
  }
  /*!

  Get the starting point on a dot border. The dot border is
  computed from this point.

  \sa getFirstBorder_u()

  */
  int getFirstBorder_v() const {
    return this->firstBorder_v;
  }

  bool computeFreemanChainElement(const vpImage<unsigned char> &I,
				  const unsigned int &u,
				  const unsigned int &v,
				  unsigned int &element);
  void computeFreemanParameters(const int &u_p,
				const int &v_p, unsigned int &element,
				int &du, int &dv, float &dS,
				float &dMu, float &dMv,
				float &dMuv,
				float &dMu2, float &dMv2);
  void updateFreemanPosition( unsigned int& u, unsigned int& v,
			      const unsigned int &dir );


  bool isInImage( vpImage<unsigned char> &I ) const;
  bool isInImage( vpImage<unsigned char> &I, const vpImagePoint &ip) const;

  bool isInArea(const unsigned int &u, const unsigned int &v) const;

  void getGridSize( unsigned int &gridWidth, unsigned int &gridHeight );
  void setArea(vpImage<unsigned char> &I,
	       int u, int v, int w, int h);
  void setArea(vpImage<unsigned char> &I);
  void setArea(const vpRect & a);

  unsigned char getMeanGrayLevel(vpImage<unsigned char>& I) const;
  //! coordinates (float) of the point center of gravity
  vpImagePoint cog;

  double width;
  double height;
  double surface;
  unsigned int gray_level_min;  // minumum gray level for the dot.
				// pixel with lower level don't belong
				// to this dot.

  unsigned int gray_level_max;  // maximum gray level for the dot.
				// pixel with higher level don't belong
				// to this dot.
  double mean_gray_level; // Mean gray level of the dot
  double grayLevelPrecision ;
  double gamma ;
  double sizePrecision ;
  double ellipsoidShapePrecision;
  double maxSizeSearchDistancePrecision;
  // Area where the dot is to search
  vpRect area;

  // other
  vpList<int> direction_list;
  vpList<vpImagePoint> ip_edges_list;

  // flag
  bool compute_moment ; // true moment are computed
  bool graphics ; // true for graphic overlay display

  // Bounding box
  int bbox_u_min, bbox_u_max, bbox_v_min, bbox_v_max;

  // The first point coodinate on the dot border
  unsigned int firstBorder_u;
  unsigned int firstBorder_v;

};

#endif

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */

