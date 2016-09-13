/********
Stereo raw images and calibration to ROS server publisher

Jay Patravali
Carnegie Mellon University
July 2016

*********/

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include<sstream>

#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <iterator>
#include <image_transport/image_transport.h>
#include <camera_info_manager/camera_info_manager.h>
#include <cv_bridge/cv_bridge.h>

using namespace cv;
using namespace std; 
using namespace ros;
using namespace camera_info_manager;



int main(int argc, char **argv)
{       
	vector<String> filenames_left; 
	vector<String> filenames_right; 
	String folder = "/home/jay/image_stitching/opencv_tools/images_cam0_png";
	String folder1 = "/home/jay/image_stitching/opencv_tools/images_cam0_png";

	glob(folder, filenames_left); 
	glob(folder1, filenames_right);
	char image_dir[100],  image_dir1[100];

	ros::init(argc, argv, "image_publisher");
	ros::NodeHandle nh("~");

	ros::NodeHandle	nhleft("/pubimage/left");
	ros::NodeHandle nhright("/pubimage/right");

	image_transport::ImageTransport it(nh);
	image_transport::CameraPublisher pub_left = it.advertiseCamera("/cam0/image_raw", 1, false);
	image_transport::CameraPublisher pub_right = it.advertiseCamera("/cam1/image_raw", 1, false);

	ros::Rate loop_rate(0.5);

	int width, height;
	
	camera_info_manager::CameraInfoManager *LInfo;
	camera_info_manager::CameraInfoManager *RInfo;
	

	string left_cam_name =  "cam0";
	string right_cam_name = "cam1";


	LInfo =	new camera_info_manager::CameraInfoManager (nhleft, "pgr_camera_13306261", "file:///home/jay/catkin_ws/src/publish_images/calibration/cam0_params.yaml");
	RInfo =	new camera_info_manager::CameraInfoManager (nhright, "pgr_camera_13330289", "file:///home/jay/catkin_ws/src/publish_images/calibration/cam1_params.yaml");

	cv_bridge::CvImage left_img_bridge;
	sensor_msgs::Image left_img_msg; 


	cv_bridge::CvImage right_img_bridge;
	sensor_msgs::Image right_img_msg; 

	int i=0;
	while(ros::ok() && i <150)
	{
		Mat left_img = imread(filenames_left[i],0);//CV_LOAD_IMAGE_COLOR);
		Mat right_img = imread(filenames_right[i],0);//CV_LOAD_IMAGE_COLOR);

		//namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
   		//imshow( "Display window", left_img  );
		//cout<<filenames_left[i]<<endl;
		//cout<<filenames_right[i]<<endl;	

               // waitKey(0);                       // Wait for a keystroke in the window  
		if(!left_img.data)
		    cerr << "Problem loading image!!!" << endl;	

		if(!right_img.data)
		    cerr << "Problem loading image!!!" << endl;	


		Size s = left_img.size();
		height = s.height;
		width = s.width;

		ros::Time stamp = ros::Time::now();

		std_msgs::Header header_left; // empty header
		header_left.frame_id = left_cam_name; 
		header_left.stamp = stamp; // time
		left_img_bridge = cv_bridge::CvImage(header_left, sensor_msgs::image_encodings::MONO8, left_img);
		left_img_bridge.toImageMsg(left_img_msg); // from cv_bridge to sensor_msgs::Image


		std_msgs::Header header_right; // empty header
		header_right.frame_id = right_cam_name;
		header_right.stamp = stamp; // time
		right_img_bridge = cv_bridge::CvImage(header_right, sensor_msgs::image_encodings::MONO8, right_img);
		right_img_bridge.toImageMsg(right_img_msg); // from cv_bridge to sensor_msgs::Image


		sensor_msgs::Image left_data;
		left_data.header.frame_id= left_cam_name;
		left_data.height= height;
		left_data.width= width;
		left_data.encoding= "mono8";
		left_data.is_bigendian= false;
		left_data.step= width;

		sensor_msgs::Image right_data;
		right_data.header.frame_id= right_cam_name;
		right_data.height= height;
		right_data.width= width;
		right_data.encoding= "mono8";
		right_data.is_bigendian= false;
		right_data.step= width;


		ROS_INFO("Processing: Frame %0d", i );

		left_data.data = left_img_msg.data;
		right_data.data = right_img_msg.data;

		sensor_msgs::CameraInfo left_info;
		left_info = LInfo->getCameraInfo();
		left_info.header = left_data.header;

		sensor_msgs::CameraInfo right_info;
		right_info = RInfo->getCameraInfo();
		right_info.header = right_data.header;

		pub_left.publish(left_data, left_info,stamp);
		pub_right.publish(right_data, right_info, stamp);

		i++;
		ros::spinOnce();
		loop_rate.sleep();
		}	
		delete LInfo;
		delete RInfo;
			
	cout<< " Complete!  exiting..."<<endl;
	return 0;
}


