/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2013, Ioan A. Sucan
*  Copyright (c) 2011-2013, Willow Garage, Inc.
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
*   * Neither the name of the Willow Garage nor the names of its
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

#include <moveit/robot_state/conversions.h>
#include <geometric_shapes/shape_operations.h>
#include <eigen_conversions/eigen_msg.h>

namespace moveit
{
namespace core
{

// ********************************************
// * Internal (hidden) functions
// ********************************************

namespace
{

static bool _jointStateToRobotState(const sensor_msgs::JointState &joint_state, RobotState& state, std::set<std::string> *missing)
{
  if (joint_state.name.size() != joint_state.position.size())
  {
    logError("Different number of names and positions in JointState message: %u, %u",
             (unsigned int)joint_state.name.size(), (unsigned int)joint_state.position.size());
    return false;
  }
  
  state.setVariableValues(joint_state);
  if (missing)
  {
    std::vector<std::string> missing_variables;
    state.getRobotModel()->getMissingVariableNames(joint_state.name, missing_variables);
    missing->clear();
    for (std::size_t i = 0; i < missing_variables.size(); ++i)
      missing->insert(missing_variables[i]);
  }
  
  return true;
}

static bool _multiDOFJointsToRobotState(const sensor_msgs::MultiDOFJointState &mjs, RobotState& state, const Transforms *tf)
{
  std::size_t nj = mjs.joint_names.size();
  if (nj != mjs.transforms.size())
  {
    logError("Different number of names, values or frames in MultiDOFJointState message.");
    return false;
  }
  
  bool error = false;
  Eigen::Affine3d inv_t;
  bool use_inv_t = false;
  
  if (nj > 0 && !Transforms::sameFrame(mjs.header.frame_id, state.getRobotModel()->getModelFrame()))
  {
    if (tf)
      try
      {
        // find the transform that takes the given frame_id to the desired fixed frame
        const Eigen::Affine3d &t2fixed_frame = tf->getTransform(mjs.header.frame_id);
        // we update the value of the transform so that it transforms from the known fixed frame to the desired child link
        inv_t = t2fixed_frame.inverse();
        use_inv_t = true;
      }
      catch (std::runtime_error&)
      {
        error = true;
      }
    else
      error = true;
    if (error)
      logWarn("The transform for multi-dof joints was specified in frame '%s' but it was not possible to transform that to frame '%s'",
              mjs.header.frame_id.c_str(), state.getRobotModel()->getModelFrame().c_str());
  }

  for (std::size_t i = 0 ; i < nj ; ++i)
  {
    const std::string &joint_name = mjs.joint_names[i];
    if (!state.getRobotModel()->hasJointModel(joint_name))
    {
      logWarn("No joint matching multi-dof joint '%s'", joint_name.c_str());
      error = true;
      continue;
    }
    Eigen::Affine3d transf;
    tf::transformMsgToEigen(mjs.transforms[i], transf);
    // if frames do not mach, attempt to transform
    if (use_inv_t)
      transf = transf * inv_t;
    
    state.setJointPositions(joint_name, transf);
  }

  return !error;
}

static inline void _robotStateToMultiDOFJointState(const RobotState& state, sensor_msgs::MultiDOFJointState &mjs)
{
  const std::vector<const JointModel*> &js = state.getRobotModel()->getMultiDOFJointModels();
  mjs.joint_names.clear();
  mjs.transforms.clear();
  for (std::size_t i = 0 ; i < js.size() ; ++i)
  {
    geometry_msgs::Transform p;
    if (state.dirtyJointTransform(js[i]))
    {
      Eigen::Affine3d t;
      t.setIdentity();
      js[i]->computeTransform(state.getJointPositions(js[i]), t);
      tf::transformEigenToMsg(t, p);
    }
    else
      tf::transformEigenToMsg(state.getJointTransform(js[i]), p);
    mjs.joint_names.push_back(js[i]->getName());
    mjs.transforms.push_back(p);
  }
  mjs.header.frame_id = state.getRobotModel()->getModelFrame();
}

class ShapeVisitorAddToCollisionObject : public boost::static_visitor<void>
{
public:

  ShapeVisitorAddToCollisionObject(moveit_msgs::CollisionObject *obj) :
    boost::static_visitor<void>(), obj_(obj)
  {
  }

  void addToObject(const shapes::ShapeMsg &sm, const geometry_msgs::Pose &pose)
  {
    pose_ = &pose;
    boost::apply_visitor(*this, sm);
  }

  void operator()(const shape_msgs::Plane &shape_msg) const
  {
    obj_->planes.push_back(shape_msg);
    obj_->plane_poses.push_back(*pose_);
  }

  void operator()(const shape_msgs::Mesh &shape_msg) const
  {
    obj_->meshes.push_back(shape_msg);
    obj_->mesh_poses.push_back(*pose_);
  }

  void operator()(const shape_msgs::SolidPrimitive &shape_msg) const
  {
    obj_->primitives.push_back(shape_msg);
    obj_->primitive_poses.push_back(*pose_);
  }

private:

  moveit_msgs::CollisionObject *obj_;
  const geometry_msgs::Pose *pose_;
};

static void _attachedBodyToMsg(const AttachedBody &attached_body, moveit_msgs::AttachedCollisionObject &aco)
{
  aco.link_name = attached_body.getAttachedLinkName();
  aco.detach_posture = attached_body.getDetachPosture();
  const std::set<std::string> &touch_links = attached_body.getTouchLinks();
  aco.touch_links.clear();
  for (std::set<std::string>::const_iterator it = touch_links.begin() ; it != touch_links.end() ; ++it)
    aco.touch_links.push_back(*it);
  aco.object.header.frame_id = aco.link_name;
  aco.object.id = attached_body.getName();

  aco.object.operation = moveit_msgs::CollisionObject::ADD;
  const std::vector<shapes::ShapeConstPtr>& ab_shapes = attached_body.getShapes();
  const EigenSTL::vector_Affine3d& ab_tf = attached_body.getFixedTransforms();
  ShapeVisitorAddToCollisionObject sv(&aco.object);
  aco.object.primitives.clear();
  aco.object.meshes.clear();
  aco.object.planes.clear();
  aco.object.primitive_poses.clear();
  aco.object.mesh_poses.clear();
  aco.object.plane_poses.clear();
  for (std::size_t j = 0 ; j < ab_shapes.size() ; ++j)
  {
    shapes::ShapeMsg sm;
    if (shapes::constructMsgFromShape(ab_shapes[j].get(), sm))
    {
      geometry_msgs::Pose p;
      tf::poseEigenToMsg(ab_tf[j], p);
      sv.addToObject(sm, p);
    }
  }
}

static void _msgToAttachedBody(const Transforms *tf, const moveit_msgs::AttachedCollisionObject &aco, RobotState& state)
{
  if (aco.object.operation == moveit_msgs::CollisionObject::ADD)
  {
    if (!aco.object.primitives.empty() || !aco.object.meshes.empty() || !aco.object.planes.empty())
    {
      if (aco.object.primitives.size() != aco.object.primitive_poses.size())
      {
        logError("Number of primitive shapes does not match number of poses in collision object message");
        return;
      }

      if (aco.object.meshes.size() != aco.object.mesh_poses.size())
      {
        logError("Number of meshes does not match number of poses in collision object message");
        return;
      }

      if (aco.object.planes.size() != aco.object.plane_poses.size())
      {
        logError("Number of planes does not match number of poses in collision object message");
        return;
      }

      const LinkModel *lm = state.getLinkModel(aco.link_name);
      if (lm)
      {
        std::vector<shapes::ShapeConstPtr> shapes;
        EigenSTL::vector_Affine3d poses;

        for (std::size_t i = 0 ; i < aco.object.primitives.size() ; ++i)
        {
          shapes::Shape *s = shapes::constructShapeFromMsg(aco.object.primitives[i]);
          if (s)
          {
            Eigen::Affine3d p;
            tf::poseMsgToEigen(aco.object.primitive_poses[i], p);
            shapes.push_back(shapes::ShapeConstPtr(s));
            poses.push_back(p);
          }
        }
        for (std::size_t i = 0 ; i < aco.object.meshes.size() ; ++i)
        {
          shapes::Shape *s = shapes::constructShapeFromMsg(aco.object.meshes[i]);
          if (s)
          {
            Eigen::Affine3d p;
            tf::poseMsgToEigen(aco.object.mesh_poses[i], p);
            shapes.push_back(shapes::ShapeConstPtr(s));
            poses.push_back(p);
          }
        }
        for (std::size_t i = 0 ; i < aco.object.planes.size() ; ++i)
        {
          shapes::Shape *s = shapes::constructShapeFromMsg(aco.object.planes[i]);
          if (s)
          {
            Eigen::Affine3d p;
            tf::poseMsgToEigen(aco.object.plane_poses[i], p);

            shapes.push_back(shapes::ShapeConstPtr(s));
            poses.push_back(p);
          }
        }

        // transform poses to link frame
        if (!Transforms::sameFrame(aco.object.header.frame_id, aco.link_name))
        {
          Eigen::Affine3d t0;
          if (state.knowsFrameTransform(aco.object.header.frame_id))
            t0 = state.getFrameTransform(aco.object.header.frame_id);
          else
            if (tf && tf->canTransform(aco.object.header.frame_id))
              t0 = tf->getTransform(aco.object.header.frame_id);
            else
            {
              t0.setIdentity();
              logError("Cannot properly transform from frame '%s'. The pose of the attached body may be incorrect", aco.object.header.frame_id.c_str());
            }
          Eigen::Affine3d t = state.getGlobalLinkTransform(lm).inverse() * t0;
          for (std::size_t i = 0 ; i < poses.size() ; ++i)
            poses[i] = t * poses[i];
        }

        if (shapes.empty())
          logError("There is no geometry to attach to link '%s' as part of attached body '%s'", aco.link_name.c_str(), aco.object.id.c_str());
        else
        {
          if (state.clearAttachedBody(aco.object.id))
            logDebug("The robot state already had an object named '%s' attached to link '%s'. The object was replaced.",
                      aco.object.id.c_str(), aco.link_name.c_str());
          state.attachBody(aco.object.id, shapes, poses, aco.touch_links, aco.link_name, aco.detach_posture);
          logDebug("Attached object '%s' to link '%s'", aco.object.id.c_str(), aco.link_name.c_str());
        }
      }
    }
    else
      logError("The attached body for link '%s' has no geometry", aco.link_name.c_str());
  }
  else
    if (aco.object.operation == moveit_msgs::CollisionObject::REMOVE)
    {
      state.clearAttachedBody(aco.object.id);
    }
    else
      logError("Unknown collision object operation: %d", aco.object.operation);
}

static bool _robotStateMsgToRobotStateHelper(const Transforms *tf, const moveit_msgs::RobotState &robot_state, RobotState& state, bool copy_attached_bodies)
{
  std::set<std::string> missing;
  bool result1 = _jointStateToRobotState(robot_state.joint_state, state, &missing);
  bool result2 = _multiDOFJointsToRobotState(robot_state.multi_dof_joint_state, state, tf);
  
  if (copy_attached_bodies)
  {
    if (!robot_state.is_diff)
      state.clearAttachedBodies();
    for (std::size_t i = 0 ; i < robot_state.attached_collision_objects.size() ; ++i)
      _msgToAttachedBody(tf, robot_state.attached_collision_objects[i], state);
  }
  
  if (result1 && result2)
  {
    if (!missing.empty())
      for (std::size_t i = 0 ; i < robot_state.multi_dof_joint_state.joint_names.size(); ++i)
      {
        const JointModel *jm = state.getJointModel(robot_state.multi_dof_joint_state.joint_names[i]);
        if (jm)
        {
          const std::vector<std::string> &vnames = jm->getVariableNames();
          for (std::size_t i = 0 ; i < vnames.size(); ++i)
            missing.erase(vnames[i]);
        }
      }
    
    return missing.empty();
  }
  else
    return false;
}

}
}
}


// ********************************************



// ********************************************
// * Exposed functions
// ********************************************

bool moveit::core::jointStateToRobotState(const sensor_msgs::JointState &joint_state, RobotState& state)
{
  bool result = _jointStateToRobotState(joint_state, state, NULL);
  state.update();
  return result;
}

bool moveit::core::robotStateMsgToRobotState(const moveit_msgs::RobotState &robot_state, RobotState& state, bool copy_attached_bodies)
{
  bool result = _robotStateMsgToRobotStateHelper(NULL, robot_state, state, copy_attached_bodies);
  state.update();
  return result;
}

bool moveit::core::robotStateMsgToRobotState(const Transforms &tf, const moveit_msgs::RobotState &robot_state, RobotState& state, bool copy_attached_bodies)
{
  bool result = _robotStateMsgToRobotStateHelper(&tf, robot_state, state, copy_attached_bodies);
  state.update();
  return result;
}

void moveit::core::robotStateToRobotStateMsg(const RobotState& state, moveit_msgs::RobotState &robot_state, bool copy_attached_bodies)
{
  robotStateToJointStateMsg(state, robot_state.joint_state);
  _robotStateToMultiDOFJointState(state, robot_state.multi_dof_joint_state);
  if (copy_attached_bodies)
  {
    std::vector<const AttachedBody*> attached_bodies;
    state.getAttachedBodies(attached_bodies);
    robot_state.attached_collision_objects.resize(attached_bodies.size());
    for (std::size_t i = 0 ; i < attached_bodies.size() ; ++i)
      _attachedBodyToMsg(*attached_bodies[i], robot_state.attached_collision_objects[i]);
  }
}

void moveit::core::robotStateToJointStateMsg(const RobotState& state, sensor_msgs::JointState &joint_state)
{
  const std::vector<const JointModel*> &js = state.getRobotModel()->getSingleDOFJointModels();
  joint_state = sensor_msgs::JointState();
  
  for (std::size_t i = 0 ; i < js.size() ; ++i)
  {
    joint_state.name.push_back(js[i]->getName());
    joint_state.position.push_back(state.getVariablePosition(js[i]->getFirstVariableIndex()));
    if (state.hasVelocities())
      joint_state.velocity.push_back(state.getVariableVelocity(js[i]->getFirstVariableIndex()));
  }
  
  // if inconsistent number of velocities are specified, discard them
  if (joint_state.velocity.size() != joint_state.position.size())
    joint_state.velocity.clear();

  joint_state.header.frame_id = state.getRobotModel()->getModelFrame();
}
