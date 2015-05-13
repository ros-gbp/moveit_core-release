Name:           ros-jade-moveit-core
Version:        0.6.15
Release:        0%{?dist}
Summary:        ROS moveit_core package

Group:          Development/Libraries
License:        BSD
URL:            http://moveit.ros.org
Source0:        %{name}-%{version}.tar.gz

Requires:       assimp
Requires:       boost-devel
Requires:       console-bridge-devel
Requires:       eigen3-devel
Requires:       ros-jade-actionlib-msgs
Requires:       ros-jade-eigen-conversions
Requires:       ros-jade-eigen-stl-containers
Requires:       ros-jade-fcl
Requires:       ros-jade-geometric-shapes >= 0.3.4
Requires:       ros-jade-geometry-msgs
Requires:       ros-jade-kdl-parser
Requires:       ros-jade-moveit-msgs
Requires:       ros-jade-octomap
Requires:       ros-jade-octomap-msgs
Requires:       ros-jade-random-numbers
Requires:       ros-jade-rostime
Requires:       ros-jade-sensor-msgs
Requires:       ros-jade-shape-msgs
Requires:       ros-jade-shape-tools
Requires:       ros-jade-srdfdom
Requires:       ros-jade-std-msgs
Requires:       ros-jade-trajectory-msgs
Requires:       ros-jade-visualization-msgs
Requires:       urdfdom-devel
Requires:       urdfdom-headers-devel
BuildRequires:  assimp
BuildRequires:  boost-devel
BuildRequires:  console-bridge-devel
BuildRequires:  eigen3-devel
BuildRequires:  pkgconfig
BuildRequires:  ros-jade-actionlib-msgs
BuildRequires:  ros-jade-angles
BuildRequires:  ros-jade-catkin
BuildRequires:  ros-jade-cmake-modules
BuildRequires:  ros-jade-eigen-conversions
BuildRequires:  ros-jade-eigen-stl-containers
BuildRequires:  ros-jade-fcl
BuildRequires:  ros-jade-geometric-shapes >= 0.3.4
BuildRequires:  ros-jade-geometry-msgs
BuildRequires:  ros-jade-kdl-parser
BuildRequires:  ros-jade-moveit-msgs
BuildRequires:  ros-jade-moveit-resources
BuildRequires:  ros-jade-octomap
BuildRequires:  ros-jade-octomap-msgs
BuildRequires:  ros-jade-orocos-kdl
BuildRequires:  ros-jade-random-numbers
BuildRequires:  ros-jade-roslib
BuildRequires:  ros-jade-rostime
BuildRequires:  ros-jade-sensor-msgs
BuildRequires:  ros-jade-shape-msgs
BuildRequires:  ros-jade-shape-tools
BuildRequires:  ros-jade-srdfdom
BuildRequires:  ros-jade-std-msgs
BuildRequires:  ros-jade-tf-conversions
BuildRequires:  ros-jade-trajectory-msgs
BuildRequires:  ros-jade-visualization-msgs
BuildRequires:  urdfdom-devel
BuildRequires:  urdfdom-headers-devel

%description
Core libraries used by MoveIt!

%prep
%setup -q

%build
# In case we're installing to a non-standard location, look for a setup.sh
# in the install tree that was dropped by catkin, and source it.  It will
# set things like CMAKE_PREFIX_PATH, PKG_CONFIG_PATH, and PYTHONPATH.
if [ -f "/opt/ros/jade/setup.sh" ]; then . "/opt/ros/jade/setup.sh"; fi
mkdir -p obj-%{_target_platform} && cd obj-%{_target_platform}
%cmake .. \
        -UINCLUDE_INSTALL_DIR \
        -ULIB_INSTALL_DIR \
        -USYSCONF_INSTALL_DIR \
        -USHARE_INSTALL_PREFIX \
        -ULIB_SUFFIX \
        -DCMAKE_INSTALL_PREFIX="/opt/ros/jade" \
        -DCMAKE_PREFIX_PATH="/opt/ros/jade" \
        -DSETUPTOOLS_DEB_LAYOUT=OFF \
        -DCATKIN_BUILD_BINARY_PACKAGE="1" \

make %{?_smp_mflags}

%install
# In case we're installing to a non-standard location, look for a setup.sh
# in the install tree that was dropped by catkin, and source it.  It will
# set things like CMAKE_PREFIX_PATH, PKG_CONFIG_PATH, and PYTHONPATH.
if [ -f "/opt/ros/jade/setup.sh" ]; then . "/opt/ros/jade/setup.sh"; fi
cd obj-%{_target_platform}
make %{?_smp_mflags} install DESTDIR=%{buildroot}

%files
/opt/ros/jade

%changelog
* Tue May 12 2015 Sachin Chitta <robot.moveit@gmail.com> - 0.6.15-0
- Autogenerated by Bloom

