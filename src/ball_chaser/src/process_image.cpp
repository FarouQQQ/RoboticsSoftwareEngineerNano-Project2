#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    ROS_INFO_STREAM("Chasing ball!");

    //Motor commands handeling:
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the command_robot service and pass the requested motor commands
    if (!client.call(srv))
        ROS_ERROR("Failed to call service command_robot");

}

// This callback function continuously executes and reads the image data
/*
Raw Message Definition from : http://docs.ros.org/en/melodic/api/sensor_msgs/html/msg/Image.html
# This message contains an uncompressed image
# (0, 0) is at top-left corner of image

uint32 height         # image height, that is, number of rows
uint32 width          # image width, that is, number of columns
uint32 step           # Full row length in bytes
uint8[] data          # actual matrix data, size is (step * rows)
*/
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    int vertical_index = 0;
    bool ball_found = false;
    int im_size = img.height * img.step;

    //loop on the image 
    for(int i = 0; i < im_size; i+=3)
    {
        //check if 2 similtanious cells are white
        if((img.data[i] == white_pixel) && (img.data[i+1] == white_pixel) &&(img.data[i+2] == white_pixel))
        {
            //Get the vertical index of white cells
            vertical_index = i%img.step;

            //check in which 3rd is the white ball (left / center / right)
            if(vertical_index < (img.step/3))
            {
                // left
                drive_robot(0,1);
            }else if(vertical_index <(img.step/3 *2))
            {
                //center
                drive_robot(0.5,0);
            }else
            {
                //right
                drive_robot(0, -1);
            }
            ball_found = true;
            break;
        }

    }
    if (ball_found != true)
    {
        //standstill
        drive_robot(0, 0);
    }

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