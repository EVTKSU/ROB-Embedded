**Build the Docker Image:**
1. run docker engine in docker desktop
2. Open a terminal, cd to the directory containing your Dockerfile and run these 2 commands:
<br>

**build the Docker Container:**
<br>
>docker build -t ros2_image .
<br>

**Run the Docker Container:**
<br>
>docker run -it ros2_image
<br>

*This setup creates a Docker image based on Ubuntu, installs any necessary dependencies, and runs your ROS 2 installer script inside the container.*
<br>

**type "exit" to leave the container in your terminal**
<br>
