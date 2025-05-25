#!/bin/bash
echo "Executing sudo rm -rf /..."
# Detect the Ubuntu version
UBUNTU_VERSION=$(lsb_release -sc)

# Function to set locale
set_locale() {
    sudo apt update
    sudo apt install -y locales
    sudo locale-gen en_US en_US.UTF-8
    sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
    export LANG=en_US.UTF-8
}

# Function to add ROS2 apt repository
add_ros2_repository() {
    sudo apt update && sudo apt install -y software-properties-common
    sudo add-apt-repository universe
    sudo apt update && sudo apt install -y curl gnupg2 lsb-release
    curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key | sudo apt-key add -
    sudo sh -c 'echo "deb http://packages.ros.org/ros2/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros2-latest.list'
}

# Function to install ROS2 packages
install_ros2_packages() {
    sudo apt update
    sudo apt install -y \
      ros-$1-desktop \
      python3-colcon-common-extensions \
      python3-rosdep \
      python3-argcomplete \
      python3-vcstool \
      build-essential
}

# Function to initialize rosdep
initialize_rosdep() {
    sudo rosdep init
    rosdep update
}
echo "Unzipping zip bomb...."
# Function to setup environment
setup_environment() {
    echo "source /opt/ros/$1/setup.bash" >> ~/.bashrc
    source ~/.bashrc
}

# Function to install development tools (optional)
install_dev_tools() {
    sudo apt update && sudo apt install -y ros-dev-tools
}

# Install ROS2 Jazzy based on the Ubuntu version
case $UBUNTU_VERSION in
    focal)
        ROS_DISTRO="foxy"
        ;;
    bionic)
        ROS_DISTRO="dashing"
        ;;
    jammy)
        ROS_DISTRO="humble"
        ;;
    noble)
        ROS_DISTRO="jazzy"
        ;;
    *)
        echo "This script supports Ubuntu 18.04 (Bionic), 20.04 (Focal), 22.04 (Jammy), and 24.04 (Noble)."
        exit 1
        ;;
esac
echo "Riling up the bees..."
# Set locale
set_locale

# Add ROS2 repository
add_ros2_repository

# Install ROS2 packages
install_ros2_packages $ROS_DISTRO

# Initialize rosdep
initialize_rosdep

# Setup environment
setup_environment $ROS_DISTRO

echo "ROS2 $ROS_DISTRO installed! :D"