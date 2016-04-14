Name:           ros-indigo-moveit-core
Version:        0.7.1
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
Requires:       ros-indigo-actionlib-msgs
Requires:       ros-indigo-eigen-conversions
Requires:       ros-indigo-eigen-stl-containers
Requires:       ros-indigo-fcl
Requires:       ros-indigo-geometric-shapes >= 0.3.4
Requires:       ros-indigo-geometry-msgs
Requires:       ros-indigo-kdl-parser
Requires:       ros-indigo-moveit-msgs
Requires:       ros-indigo-octomap
Requires:       ros-indigo-octomap-msgs
Requires:       ros-indigo-random-numbers
Requires:       ros-indigo-rostime
Requires:       ros-indigo-sensor-msgs
Requires:       ros-indigo-shape-msgs
Requires:       ros-indigo-srdfdom
Requires:       ros-indigo-std-msgs
Requires:       ros-indigo-trajectory-msgs
Requires:       ros-indigo-visualization-msgs
Requires:       urdfdom-devel
Requires:       urdfdom-headers-devel
BuildRequires:  assimp
BuildRequires:  boost-devel
BuildRequires:  console-bridge-devel
BuildRequires:  eigen3-devel
BuildRequires:  pkgconfig
BuildRequires:  ros-indigo-actionlib-msgs
BuildRequires:  ros-indigo-angles
BuildRequires:  ros-indigo-catkin
BuildRequires:  ros-indigo-cmake-modules
BuildRequires:  ros-indigo-eigen-conversions
BuildRequires:  ros-indigo-eigen-stl-containers
BuildRequires:  ros-indigo-fcl
BuildRequires:  ros-indigo-geometric-shapes >= 0.3.4
BuildRequires:  ros-indigo-geometry-msgs
BuildRequires:  ros-indigo-kdl-parser
BuildRequires:  ros-indigo-moveit-msgs
BuildRequires:  ros-indigo-moveit-resources
BuildRequires:  ros-indigo-octomap
BuildRequires:  ros-indigo-octomap-msgs
BuildRequires:  ros-indigo-orocos-kdl
BuildRequires:  ros-indigo-random-numbers
BuildRequires:  ros-indigo-roslib
BuildRequires:  ros-indigo-rostime
BuildRequires:  ros-indigo-sensor-msgs
BuildRequires:  ros-indigo-shape-msgs
BuildRequires:  ros-indigo-srdfdom
BuildRequires:  ros-indigo-std-msgs
BuildRequires:  ros-indigo-tf-conversions
BuildRequires:  ros-indigo-trajectory-msgs
BuildRequires:  ros-indigo-visualization-msgs
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
if [ -f "/opt/ros/indigo/setup.sh" ]; then . "/opt/ros/indigo/setup.sh"; fi
mkdir -p obj-%{_target_platform} && cd obj-%{_target_platform}
%cmake .. \
        -UINCLUDE_INSTALL_DIR \
        -ULIB_INSTALL_DIR \
        -USYSCONF_INSTALL_DIR \
        -USHARE_INSTALL_PREFIX \
        -ULIB_SUFFIX \
        -DCMAKE_INSTALL_LIBDIR="lib" \
        -DCMAKE_INSTALL_PREFIX="/opt/ros/indigo" \
        -DCMAKE_PREFIX_PATH="/opt/ros/indigo" \
        -DSETUPTOOLS_DEB_LAYOUT=OFF \
        -DCATKIN_BUILD_BINARY_PACKAGE="1" \

make %{?_smp_mflags}

%install
# In case we're installing to a non-standard location, look for a setup.sh
# in the install tree that was dropped by catkin, and source it.  It will
# set things like CMAKE_PREFIX_PATH, PKG_CONFIG_PATH, and PYTHONPATH.
if [ -f "/opt/ros/indigo/setup.sh" ]; then . "/opt/ros/indigo/setup.sh"; fi
cd obj-%{_target_platform}
make %{?_smp_mflags} install DESTDIR=%{buildroot}

%files
/opt/ros/indigo

%changelog
* Thu Apr 14 2016 Sachin Chitta <robot.moveit@gmail.com> - 0.7.1-0
- Autogenerated by Bloom

* Thu Apr 14 2016 Sachin Chitta <robot.moveit@gmail.com> - 0.7.0-1
- Autogenerated by Bloom

* Sat Jan 30 2016 Sachin Chitta <robot.moveit@gmail.com> - 0.7.0-0
- Autogenerated by Bloom

* Tue Jan 20 2015 Sachin Chitta <robot.moveit@gmail.com> - 0.6.15-0
- Autogenerated by Bloom

* Thu Jan 15 2015 Sachin Chitta <robot.moveit@gmail.com> - 0.6.14-0
- Autogenerated by Bloom

* Sat Dec 20 2014 Sachin Chitta <robot.moveit@gmail.com> - 0.6.13-0
- Autogenerated by Bloom

* Wed Dec 03 2014 Sachin Chitta <robot.moveit@gmail.com> - 0.6.12-0
- Autogenerated by Bloom

* Mon Nov 03 2014 Sachin Chitta <robot.moveit@gmail.com> - 0.6.11-0
- Autogenerated by Bloom

* Mon Oct 27 2014 Sachin Chitta <robot.moveit@gmail.com> - 0.6.10-0
- Autogenerated by Bloom

