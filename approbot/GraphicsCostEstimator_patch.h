#pragma once

// 1) Pretend we already included the real header:
#define OSG_GRAPHICSCOSTESTIMATOR

// 2) Pull in only the minimal bits we need:
#include <osg/Referenced>
#include <osg/ref_ptr>
#include <utility>

namespace osg
{
  class Geometry;
  class Texture;
  class Program;
  class Node;
  class RenderInfo;

  typedef std::pair<double,double> CostPair;

  class OSG_EXPORT GraphicsCostEstimator : public osg::Referenced
  {
  public:
    GraphicsCostEstimator();
    void setDefaults();
    void calibrate(osg::RenderInfo& renderInfo);

    // only declarations, no inline bodies:
    CostPair estimateCompileCost(const osg::Geometry*) const;
    CostPair estimateDrawCost   (const osg::Geometry*) const;

    CostPair estimateCompileCost(const osg::Texture*) const;
    CostPair estimateDrawCost   (const osg::Texture*) const;

    CostPair estimateCompileCost(const osg::Program*) const;
    CostPair estimateDrawCost   (const osg::Program*) const;

    CostPair estimateCompileCost(const osg::Node*) const;
    CostPair estimateDrawCost   (const osg::Node*) const;

  protected:
    virtual ~GraphicsCostEstimator();

    osg::ref_ptr<class GeometryCostEstimator> _geometryEstimator;
    osg::ref_ptr<class TextureCostEstimator>  _textureEstimator;
    osg::ref_ptr<class ProgramCostEstimator>  _programEstimator;
  };
}
