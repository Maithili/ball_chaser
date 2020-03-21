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
        ROS_INFO("Service completed.");
    }
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel_sum = 255*3;
    double side_fraction = 0.3;
    int left_num = 0;
    int right_num = 0;
    int center_num = 0;
    ROS_INFO("height [%i]", img.height);
    ROS_INFO("width [%i]", img.width);
    ROS_INFO("step [%i]", img.step);

    int i = 0;
    for(int i = 0; i < img.data.size(); i+=3)
    {
        int pixel_sum = img.data[i] + img.data[i+1] + img.data[i+2];
        if(pixel_sum == white_pixel_sum)
        {
            double position = double(i%img.step)/img.step;
            if(position < side_fraction) left_num++;
            else if(position > (1-side_fraction)) right_num++;
            else center_num++;
        }
    }

    ROS_INFO("Image ratios [%d], [%d], [%d]", left_num, center_num, right_num);

    if(left_num == 0 && center_num == 0 && right_num == 0)
        drive_robot(0,0);

    else if(left_num > center_num && left_num > right_num)
        drive_robot(1,0.3);

    else if(right_num > center_num && right_num > left_num)
        drive_robot(1,-0.3);

    else 
        drive_robot(1,0);
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