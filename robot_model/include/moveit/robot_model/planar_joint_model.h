/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2012, Willow Garage, Inc.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of Willow Garage, Inc. nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Ioan Sucan */

#ifndef MOVEIT_ROBOT_MODEL_PLANAR_JOINT_MODEL_
#define MOVEIT_ROBOT_MODEL_PLANAR_JOINT_MODEL_

#include <moveit/robot_model/joint_model.h>

namespace robot_model
{

/** \brief A planar joint */
class PlanarJointModel : public JointModel
{
  friend class RobotModel;
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  PlanarJointModel(const std::string& name);

  virtual void getVariableDefaultValues(std::vector<double> &values, const Bounds &other_bounds) const;
  virtual void getVariableRandomValues(random_numbers::RandomNumberGenerator &rng, std::vector<double> &values, const Bounds &other_bounds) const;
  virtual void getVariableRandomValuesNearBy(random_numbers::RandomNumberGenerator &rng, std::vector<double> &values, const Bounds &other_bounds,
                                             const std::vector<double> &near, const double distance) const;
  virtual void enforceBounds(std::vector<double> &values, const Bounds &other_bounds) const;
  virtual bool satisfiesBounds(const std::vector<double> &values, const Bounds &other_bounds, double margin) const;

  virtual unsigned int getStateSpaceDimension() const;
  virtual double getMaximumExtent(const Bounds &other_bounds) const;
  virtual double distance(const std::vector<double> &values1, const std::vector<double> &values2) const;
  virtual void interpolate(const std::vector<double> &from, const std::vector<double> &to, const double t, std::vector<double> &state) const;
  virtual std::vector<moveit_msgs::JointLimits> getVariableLimits() const;

  virtual void computeTransform(const std::vector<double>& joint_values, Eigen::Affine3d &transf) const;
  virtual void computeJointStateValues(const Eigen::Affine3d& transf, std::vector<double>& joint_values) const;
  virtual void updateTransform(const std::vector<double>& joint_values, Eigen::Affine3d &transf) const;

  double getAngularDistanceWeight() const
  {
    return angular_distance_weight_;
  }

  void setAngularDistanceWeight(double weight)
  {
    angular_distance_weight_ = weight;
  }

  /// Make the yaw component of a state's value vector be in the range [-Pi, Pi]. enforceBounds() also calls this function;
  /// Return true if a change is actually made
  bool normalizeRotation(std::vector<double> &values) const;

private:

  double angular_distance_weight_;
};

}
#endif
