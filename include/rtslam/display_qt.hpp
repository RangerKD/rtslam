/**
 * \file display_qt.hpp
 * \author croussil@laas.fr
 * \date 25/03/2010
 * File defining a display architecture for qt with qdisplay module
 * \ingroup rtslam
 */

#ifndef DISPLAY_QT__HPP_
#define DISPLAY_QT__HPP_

#include "qdisplay/ImageView.hpp"
#include "qdisplay/Viewer.hpp"
#include "qdisplay/Shape.hpp"
#include "qdisplay/Ellipsoid.hpp"

#include "rtslam/display.hpp"
#include "rtslam/rawImage.hpp"

#include "jmath/misc.hpp"

#define DEFINE_USELESS_OBJECTS 1

namespace jafar {
namespace rtslam {
namespace display {

//******************************************************************************
// Objects

#if DEFINE_USELESS_OBJECTS
/** **************************************************************************

*/
class WorldQt : public WorldDisplay
{
	public:
		WorldQt(rtslam::WorldAbstract *_slamWor, rtslam::WorldAbstract *garbage):
			WorldDisplay(_slamWor, garbage) {}
		void bufferize() {}
		void render() {}
};

/** **************************************************************************

*/
class MapQt : public MapDisplay
{
	public:
		MapQt(rtslam::MapAbstract *_slamMap, WorldDisplay *_dispWorld):
			MapDisplay(_slamMap, _dispWorld) {}
		void bufferize() {}
		void render() {}
};

/** **************************************************************************

*/
class RobotQt : public RobotDisplay
{
	public:
		// buffered data
		//Gaussian pose_;
		//std::string model3d_;
		// graphical objects
	public:
		RobotQt(rtslam::RobotAbstract *_slamRob, MapDisplay *_dispMap):
			RobotDisplay(_slamRob, _dispMap) {}
		void bufferize() {}
		void render() {}
};
#endif

/** **************************************************************************

*/
class SensorQt : public SensorDisplay
{
	public:
		// buffered data
		// graphical objects
		qdisplay::Viewer *viewer_;
		qdisplay::ImageView* view_;
		image::Image image;
	public:
		SensorQt(rtslam::SensorAbstract *_slamSen, RobotDisplay *_dispRob): 
			SensorDisplay(_slamSen, _dispRob)
		{
			viewer_ = new qdisplay::Viewer();
			view_ = new qdisplay::ImageView();
			viewer_->setImageView(view_, 0, 0);
		}
		~SensorQt()
		{
			delete view_;
			delete viewer_;
		}
		void bufferize()
		{
			raw_ptr_t raw = slamSen_->getRaw();
			image = dynamic_cast<RawImage&>(*raw).img;
		}
		void render()
		{
			switch (type_)
			{
				case SensorDisplay::stCameraPinhole:
				case SensorDisplay::stCameraBarreto:
					view_->setImage(image);
					break;
				default:
					JFR_ERROR(RtslamException, RtslamException::UNKNOWN_SENSOR_TYPE, "Don't know how to display this type of sensor" << type_);
			}
		}
};

#if DEFINE_USELESS_OBJECTS
/** **************************************************************************

*/
class LandmarkQt : public LandmarkDisplay
{
	public:
		// buffered data
		//jmath::vec data_;
		// graphical objects
	public:
		LandmarkQt(rtslam::LandmarkAbstract *_slamLmk, MapDisplay *_dispMap):
			LandmarkDisplay(_slamLmk, _dispMap) {}
		void bufferize() {}
		void render() {}
};
#endif

/** **************************************************************************

*/
class ObservationQt : public ObservationDisplay
{
	public:
		// buffered data
		bool visible_;
		bool matched_;
		bool predicted_;
		jblas::vec predObs_;
		jblas::sym_mat predObsCov_;
		jblas::vec measObs_;
		unsigned int id_;
		// TODO grid
		// graphical objects
		qdisplay::ImageView* view_; // not owner
		std::list<QGraphicsItemGroup*> items_;
	public:
		ObservationQt(rtslam::ObservationAbstract *_slamObs, SensorQt *_dispSen):
			ObservationDisplay(_slamObs, _dispSen), view_(_dispSen->view_) 
		{
			switch (landmarkType_)
			{
				case LandmarkDisplay::ltPoint:
					predObs_.resize(2);
					predObsCov_.resize(2);
					measObs_.resize(2);
					break;
				default:
					JFR_ERROR(RtslamException, RtslamException::UNKNOWN_FEATURE_TYPE, "Don't know how to display this type of landmark" << landmarkType_);
					break;
			}
		}
		~ObservationQt()
		{
			for(std::list<QGraphicsItemGroup*>::iterator it = items_.begin(); it != items_.end(); ++it)
			{
				view_->removeFromGroup(*it);
				delete *it;
			}
		}
		
		void bufferize()
		{
			switch (landmarkType_)
			{
				case LandmarkDisplay::ltPoint:
					//LandmarkEuclidean *lmk = static_cast<LandmarkEuclidean*>(slamObs_->landmarkPtr->convertToStandardParametrization());
					predicted_ = slamObs_->events.predicted;
					matched_ = slamObs_->events.matched;
					visible_ = slamObs_->events.visible;
					id_ = slamObs_->landmarkPtr()->id();
					if (visible_)
					{
						if (predicted_)
						{
							predObs_ = slamObs_->expectation.x();
							predObsCov_ = slamObs_->expectation.P();
						}
						if (matched_)
						{
							measObs_ = slamObs_->measurement.x();
						}
					}
					break;
				default:
					JFR_ERROR(RtslamException, RtslamException::UNKNOWN_FEATURE_TYPE, "Don't know how to display this type of landmarks" << landmarkType_);
					break;
			}
		}
		void render()
		{
			switch (landmarkType_)
			{
				case LandmarkDisplay::ltPoint:
					if (items_.size() != 3)
					{
						// clear
						items_.clear();
						if (!(visible_ && predicted_))
						{
							predObs_.clear();
							predObsCov_ = identity_mat(2);
						}
						if (!(visible_ && matched_))
						{
							measObs_.clear();
						}
						
						// prediction point
						qdisplay::Shape *s = new qdisplay::Shape(qdisplay::Shape::ShapeCross, predObs_(0), predObs_(1), 3, 3);
						s->setVisible(visible_ && predicted_);
						s->setColor(0, 0, 255); // blue
						s->setLabel(jmath::toStr(id_).c_str());
						items_.push_back(s);
						view_->addShape(s);
						
						// prediction ellipse
						s = new qdisplay::Ellipsoid(predObs_, predObsCov_, 3.0);
						s->setVisible(visible_ && predicted_);
						s->setColor(255,255,0); // yellow
						items_.push_back(s);
						view_->addShape(s);
						
						// measure point
						s = new qdisplay::Shape(qdisplay::Shape::ShapeCross, measObs_(0), measObs_(1), 3, 3, 45);
						s->setVisible(visible_ && matched_);
						s->setColor(255, 0, 0); // red
						items_.push_back(s);
						view_->addShape(s);
						
					} else
					{
						// prediction point
						std::list<QGraphicsItemGroup*>::iterator it = items_.begin();
						(*it)->setVisible(visible_ && predicted_);
						if (visible_ && predicted_)
							(*it)->setPos(predObs_(0), predObs_(1));
						
						// prediction ellipse
						++it;
						(*it)->setVisible(visible_ && predicted_);
						if (visible_ && predicted_)
						{
							qdisplay::Ellipsoid *ell = dynamic_cast<qdisplay::Ellipsoid*>(*it);
							ell->set(predObs_, predObsCov_, 3.0);
						}
						
						// measure point
						++it;
						(*it)->setVisible(visible_ && matched_);
						if (visible_ && matched_)
							(*it)->setPos(measObs_(0), measObs_(1));
					}
					break;

				default:
					JFR_ERROR(RtslamException, RtslamException::UNKNOWN_FEATURE_TYPE, "Don't know how to display this type of landmark" << landmarkType_);
			}
		}
};



//******************************************************************************
// Viewer

#if DEFINE_USELESS_OBJECTS
typedef Viewer<WorldQt,MapQt,RobotQt,SensorQt,LandmarkQt,ObservationQt> ViewerQt;
#else
typedef Viewer<WorldDisplay,MapDisplay,RobotDisplay,SensorQt,LandmarkDisplay,ObservationQt> ViewerQt;
#endif



}}}

#endif
