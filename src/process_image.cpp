#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
    if(client.call(srv))
    {
        ROS_INFO("Service completed with ");
        // ROS_INFO(srv.response.msg_feedback);
    }

    // TODO: Request a service and pass the velocities to it to drive the robot
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    double side_fraction = 0.3;
    int left_num = 0;
    int right_num = 0;
    int center_num = 0;
    int section_left = img.width*side_fraction;
    int section_right = img.width*(1-side_fraction);

    int i = 0;
    for(int i = 0; i < img.data.size(); i++)
    {
        if(img.data[i] == white_pixel)
        {
            double position = double(i%img.step)/img.step;
            if(position < section_left) left_num++;
            else if(position > section_right) right_num++;
            else center_num++;
        }
    }

    if(left_num == 0 && center_num == 0 && right_num == 0)
        drive_robot(0,0);

    else if(left_num > center_num && left_num > right_num)
        drive_robot(1,0.2);

    else if(right_num > center_num && right_num > left_num)
        drive_robot(1,-0.2);

    else 
        drive_robot(1,0);

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}