// Copyright 2013, Ji Zhang, Carnegie Mellon University
// Further contributions copyright (c) 2016, Southwest Research Institute
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from this
//    software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is an implementation of the algorithm described in the following paper:
//   J. Zhang and S. Singh. LOAM: Lidar Odometry and Mapping in Real-time.
//     Robotics: Science and Systems Conference (RSS). Berkeley, CA, July 2014.

#include <cmath>
#include "transform/rigid_transform.h"
#include <common/common.h>
#include <nav_msgs/Odometry.h>
#include <opencv/cv.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/PointCloud2.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_broadcaster.h>
#include <ros/package.h>
#include <fstream>
#include <list>
#include <glog/logging.h>

#include "velodyne/HDL32Structure.h"
#include "util/boostudp/boostudp.h"
#include "common/blocking_queue.h"
#include "common/make_unique.h"
#include "sensor_driver_msgs/OdometrywithGps.h"
#include "sensor_driver_msgs/NegativeOGM.h"

#define LOCAL_IP "192.168.0.112"
//#define LOCAL_IP "127.0.0.1"
#define FROMLADAR_LOCAL_PORT 9906

class PostProcess
{
public:
	typedef std::pair<double,transform::Rigid3d> TimePosePair;
	PostProcess(ros::NodeHandle& nodehandle):nodehandle_(nodehandle)
	,processthread_(NULL)
	,negativethread_(NULL)
	,lidarprocess(NULL)
	,processthreadfinished_ (false)
	{
		init();
	}
	~PostProcess()
	{
	  lidarOdoms_.stopQueue();
	  processthreadfinished_ = true;
	  processthread_->join();
	  negativethread_->join();
	}

	void init()
	{
		replay = 1;
		display = false;
		velodyneheight = 2.2;
		rigid_heightdiffthreshold = 0.3;
		ros::param::get("~N_SCANS",N_SCANS_);
		if(N_SCANS_ == 64)
			hdlCalibration = ros::package::getPath("sensor_driver") + "/config/64S3db.xml";//这里我设置成了绝对
		else if (N_SCANS_ == 32)
			hdlCalibration = "";
//		ros::param::get("~send_flag",send_flag_);
//		ros::param::get("~borderdetection_flag",borderdetection_flag_);
		hdlgrabber=new pcl::VelodyneGrabber(hdlCalibration, pcapFile, N_SCANS_);
		lidarprocess=new LidarProcess(*hdlgrabber,replay,display,velodyneheight,rigid_heightdiffthreshold,xmlconfig,calibvalue);
		ros::param::get("~heightdiff_threshold",heightdiffthreshold_);
		pubNegativeOGM_ = nodehandle_.advertise<sensor_driver_msgs::NegativeOGM> ("negativeOgm", 5);

//		subLaserOdometry_ = nodehandle_.subscribe<sensor_driver_msgs::OdometrywithGps>
//										 ("lidar_odometry_to_init", 5, boost::bind(&PostProcess::laserOdometryHandler,this,_1));//需要雷达里程计信息时需要，否则可以注释掉

		subLaserCloudFullRes_ = nodehandle_.subscribe<sensor_msgs::PointCloud2>
										 ("lidar_cloud_calibrated", 1, boost::bind(&PostProcess::laserCloudHandler,this,_1));//经过筛选且转换之后的点云
//		subCheck = nodehandle_.subscribe<sensor_driver_msgs::NegativeOGM>("negativeOgm", 5 ,boost::bind(&PostProcess::ogmChecker,this,_1));

		processthread_ = new boost::thread(boost::bind(&PostProcess::process,this));
		negativethread_ = new boost::thread(boost::bind(&PostProcess::pointCloudDisplay,this));
		//file_.open("/home/jkj/catkin_ws/result.txt",std::ios::out);

	}

	void SendData(OGMData<unsigned char>& ogmdata)  //udp通信 发送端例程
	{
		BoostUdp sendogmdata(LOCAL_IP,FROMLADAR_LOCAL_PORT);
		sendogmdata.connectRemoteEndpoint("192.168.0.111",9905);
	//	sendogmdata.connectRemoteEndpoint("127.0.0.1",9905);
		int lenth=2000;
		int package = ogmdata.ogmcell_size/lenth;
		int package_total;

		char ogm_total_data[ogmdata.ogmcell_size];
		int headernum = 3;
		char header[headernum];
		//memcpy(ogm_total_data,ogmdata.ogm,1);
		if(ogmdata.ogmcell_size - lenth * package>0)
		{
			package_total = package + 1;
		}
		for(int i =0;i < package;i++)
		{
			int ogmstart = lenth*i;
			char senddata[lenth+headernum];
			header[0]='a';
			header[1]=i;
			header[2]=package_total;
			memcpy(senddata,header,headernum);
			memcpy(senddata+headernum,ogmdata.ogm+ogmstart,lenth);
			sendogmdata.send(senddata,sizeof(senddata));
		}
		if(ogmdata.ogmcell_size - lenth * package>0)
		{
				int lenth_new = ogmdata.ogmcell_size - lenth * package;
				char senddata[lenth_new];
				header[0] = 0x88;
				header[1] = package;
				header[2] = package_total;
				memcpy(senddata,header,headernum);
				memcpy(senddata+headernum,ogmdata.ogm+lenth*package,lenth_new);
				sendogmdata.send(senddata,sizeof(senddata));
		}
	}

	void laserOdometryHandler(const sensor_driver_msgs::OdometrywithGps::ConstPtr& laserOdometry)  //雷达里程计
	{
	  double timeOdometry = laserOdometry->odometry.header.stamp.toSec();
	  static double last_stamp = -1;
	//  static geometry_msgs::Quaternion last_geoQuat;
	  static transform::Rigid3d lasttransformodometry;
	//  static float last_trans[6];
	//  double roll, pitch, yaw;
	  geometry_msgs::Quaternion geoQuat = laserOdometry->odometry.pose.pose.orientation;

	  Eigen::Quaterniond roatation(geoQuat.w,geoQuat.x,geoQuat.y,geoQuat.z);
	  Eigen::Vector3d translation(laserOdometry->odometry.pose.pose.position.x,
			  laserOdometry->odometry.pose.pose.position.y,
			  laserOdometry->odometry.pose.pose.position.z);

	  transform::Rigid3d transformodometry(translation,roatation);

	  lidarOdoms_.Push(common::make_unique<TimePosePair>(timeOdometry,transformodometry));

	}

	void ogmChecker(const sensor_driver_msgs::NegativeOGM::ConstPtr& negativeogm)
	{
		OGMData<unsigned char> ogm_data_negative(80,40,0.2,20,20);
		ogm_data_negative.ogmheight = negativeogm->ogmheight;
		ogm_data_negative.ogmwidth = negativeogm->ogmwidth;
		ogm_data_negative.ogmresolution = negativeogm->ogmresolution;
		ogm_data_negative.vehicle_x = negativeogm->vehicle_x;
		ogm_data_negative.vehicle_y = negativeogm->vehicle_y;
		for(int i = 0;i<ogm_data_negative.ogmcell_size;i++)
		{
			ogm_data_negative.ogm[i] = negativeogm->data[i];
		}
		LidarProcess::showOGM("test",ogm_data_negative);
		cvWaitKey(1);
	}

	void pointCloudDisplay()
	{
			boost::shared_ptr<PCLVisualizer> cloud_viewer_negative(new PCLVisualizer ("Obstacle Cloud"));
			cloud_viewer_negative->addCoordinateSystem (3.0);
			cloud_viewer_negative->setBackgroundColor (0, 0, 0);
			cloud_viewer_negative->initCameraParameters ();
			cloud_viewer_negative->setCameraPosition (0.0, 0.0, 30.0, 0.0, 1.0, 0.0, 0);
			cloud_viewer_negative->setCameraClipDistances (0.0, 100.0);
			Cloud::Ptr passableCloud(new Cloud);//
			Cloud::Ptr negativeCloud(new Cloud);//
			passableCloud->clear();
			negativeCloud->clear();
			char linename[20];
			float x1,x2,y1,y2,z;
		    pcl::PointXYZI pt1, pt2, pt3, pt4;
			for(int i = 0 ; i < 10 ; i++)
			{
				x1 = -20 ;
				x2 = 20 ;
				y1 = (i - 4) * 5 ;
				y2 = (i - 4) * 5;
				z = 0;
				pt1.x = min(x1 , x2) ;
				pt1.y = min(y1 , y2) ;
				pt1.z = z;
				pt2.x = max(x1 , x2) ;
				pt2.y = max(y1 , y2) ;
				pt2.z = z;
				memset(linename, 0 , 20);
				sprintf(linename , "lat%02d" , i);
				cloud_viewer_negative->addLine(pt1, pt2, linename);
			}

			for(int i = 0 ; i < 5 ; i++)
			{
				x1 = i * 10 - 20;
				x2 = i * 10 - 20;
				y1 = -20 ;
				y2 = 70 ;
				z = 0;
			    pt1.x = min(x1 , x2) ;
			    pt1.y = min(y1 , y2) ;
			    pt1.z = z;
			    pt2.x = max(x1 , x2) ;
			    pt2.y = max(y1 , y2) ;
			    pt2.z = z;
			    memset(linename, 0 , 20);
			    sprintf(linename , "lng%02d" , i);
			    cloud_viewer_negative->addLine(pt1, pt2, linename);
			}

			while(!cloud_viewer_negative->wasStopped())
		{
			cloud_mutex_.lock();
			for(int i = 0;i < totalCloud.points.size();i++)
			{
				if(totalCloud.points[i].passibility < 0.1 || totalCloud.points[i].passibility >3.1)
					negativeCloud->points.push_back(totalCloud.points[i]);
				else
					passableCloud->points.push_back(totalCloud.points[i]);
			}
			cloud_mutex_.unlock();
				cloud_viewer_negative->removeAllPointClouds();

				if(passableCloud->points.size()>0)
				{
					pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZI> passableCloudHandler (passableCloud, 0, 255, 0);
					if (!cloud_viewer_negative->updatePointCloud (passableCloud, passableCloudHandler, "passableCloud"))
					{
						cloud_viewer_negative->addPointCloud (passableCloud, passableCloudHandler, "passableCloud");
					}
				}
				if(negativeCloud->points.size()>0)
				{
					pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZI> negativeCloudHandler (negativeCloud, 255, 0, 255);
					if (!cloud_viewer_negative->updatePointCloud (negativeCloud, negativeCloudHandler, "negativeCloud"))
					{
						cloud_viewer_negative->addPointCloud (negativeCloud, negativeCloudHandler, "negativeCloud");
					}
				}

				cloud_viewer_negative->spinOnce();
				boost::this_thread::sleep (boost::posix_time::microseconds(100));
				passableCloud->clear();
				negativeCloud->clear();
			}
			cloud_viewer_negative->close();
	}

	void laserCloudHandler(const sensor_msgs::PointCloud2ConstPtr& laserCloudFullRes2) //点云数据
	{
		double timeLaserCloudFullRes = laserCloudFullRes2->header.stamp.toSec();
		LOG(INFO)<<std::fixed<<std::setprecision(3)<<"cloudtime:"<<timeLaserCloudFullRes;
		LOG(INFO)<<"starttime"<<ros::Time::now().toSec() - timeLaserCloudFullRes;
	  lidarCloudMsgs_.Push(laserCloudFullRes2);
	  if(lidarCloudMsgs_.Size()>1)
		  lidarCloudMsgs_.Pop();

	}

	void analysisCloud(pcl::PointCloud<pcl::PointXYZI>::Ptr inputcloud,
			std::vector<pcl::PointCloud<pcl::PointXYZI>::Ptr>& outputclouds,std::vector<pcl::PointXYZI>& lidarpropertys)
	{

		  //////////////////////////总的点云中可能包含了几组独立的点云数据，对发过来的点云进行处理，将每一组点云都提取出来////////////////////////////////////////
		  int cloudnum = inputcloud->size() % 16;//包含的点云包数目
		  vector<int> startnum;

		  for(int i =0;i<cloudnum;i++)
		  {
			  pcl::PointXYZI originpoint;
			  int flag = (*inputcloud)[inputcloud->size()-cloudnum+i].range;//每一包点云的第一个点的位置
			  (*inputcloud)[inputcloud->size()-cloudnum+i].range = -0.5;
			  originpoint.x = (*inputcloud)[inputcloud->size()-cloudnum+i].x;//每一包点云中对应的雷达在车体坐标系的x
			  originpoint.y = (*inputcloud)[inputcloud->size()-cloudnum+i].y;////每一包点云中对应的雷达在车体坐标系的y
			  originpoint.z = (*inputcloud)[inputcloud->size()-cloudnum+i].z;////每一包点云中对应的雷达在车体坐标系的z
			  originpoint.intensity = (*inputcloud)[inputcloud->size()-cloudnum+i].azimuth;//每一包点云中对应的雷达线束
			  startnum.push_back(flag);
			  lidarpropertys.push_back(originpoint);
		  }
		  for(int i = 0;i < startnum.size();i++)
		  {
			int length;
			pcl::PointCloud<pcl::PointXYZI>::Ptr lasercloudptr(new pcl::PointCloud<pcl::PointXYZI>);//每一包点云

			if(i == startnum.size()-1)
			{
				length = inputcloud->size() - cloudnum - startnum.at(i);
			}
			else
			{
				length = startnum.at(i+1) - startnum.at(i);
			}

			lasercloudptr->insert(lasercloudptr->begin(),inputcloud->begin()+startnum.at(i),inputcloud->begin()+startnum.at(i)+length);
			outputclouds.push_back(lasercloudptr);

		  }

	}

	void process()
	{

		while(!processthreadfinished_)
		{
			const sensor_msgs::PointCloud2ConstPtr cloudmsg = lidarCloudMsgs_.PopWithTimeout(common::FromSeconds(0.1));
			if(cloudmsg == nullptr)
				continue;
			pcl::PointCloud<pcl::PointXYZI> negativeCloud;
			pcl::PointCloud<pcl::PointXYZI> heightDiffCloud_total;
			pcl::PointCloud<pcl::PointXYZI> heightDiffCloud_left;
			pcl::PointCloud<pcl::PointXYZI> heightDiffCloud_right;
			pcl::PointCloud<pcl::PointXYZI> borderCloud_left;
			pcl::PointCloud<pcl::PointXYZI> borderCloud_right;
			pcl::PointCloud<pcl::PointXYZI> heightDiffCloud_l;
			pcl::PointCloud<pcl::PointXYZI> heightDiffCloud_r;

			OGMData<unsigned char> ogm_data_total(70,40,0.2,20,20);
			pcl::PointCloud<pcl::PointXYZI>::Ptr tempcloud(new pcl::PointCloud<pcl::PointXYZI>);//当前帧点云（雷达里程计坐标系）
			pcl::fromROSMsg(*cloudmsg, *tempcloud);//获取当前帧点云数据
			std::vector<pcl::PointCloud<pcl::PointXYZI>::Ptr> outputclouds;
			OGMData<unsigned char> ogm_data_negative(70,40,0.2,20,20);
			OGMData<unsigned char> ogm_data_border_left(70,40,0.2,20,20);
			OGMData<unsigned char> ogm_data_border_right(70,40,0.2,20,20);
			OGMData<unsigned char> ogm_data_border_total(70,40,0.2,20,20);
			OGMData<unsigned char> ogm_data_height_left(70,40,0.2,20,20);
			OGMData<unsigned char> ogm_data_height_right(70,40,0.2,20,20);
			OGMData<unsigned char> ogm_data_height_total(70,40,0.2,20,20);
			OGMData<unsigned char> ogm_data_single_temp_close(70,40,0.2,20,20);
			std::vector<pcl::PointXYZI> lidarpropertys;
			pcl::PointXYZI point;
			analysisCloud(tempcloud,outputclouds,lidarpropertys);
			point = lidarpropertys.at(0);
			negativeCloud = *outputclouds[0];
			borderCloud_left = *outputclouds[0];
			heightDiffCloud_left = *outputclouds[0];
			heightDiffCloud_right = *outputclouds[1];
			borderCloud_right = *outputclouds[1];

			lidarprocess->negativeDetection(negativeCloud,N_SCANS_);
			pcl::PointCloud<pcl::PointXYZI> borderdetection_left;
			pcl::PointCloud<pcl::PointXYZI> borderdetection_right;
			LidarProcess::border_detection(borderCloud_left,borderdetection_left,ogm_data_border_left,N_SCANS_,(double)point.x,(double)point.y);
			LidarProcess::border_detection(borderCloud_right,borderdetection_right,ogm_data_border_right,N_SCANS_,(double)point.x,(double)point.y);
			lidarprocess->heightdiffOgmDetection(heightDiffCloud_left,heightDiffCloud_l,ogm_data_height_left,0.2,heightdiffthreshold_,3);
			lidarprocess->heightdiffOgmDetection(heightDiffCloud_right,heightDiffCloud_r,ogm_data_height_right,0.2,heightdiffthreshold_,3);
			totalCloud = heightDiffCloud_left;
			totalCloud += heightDiffCloud_right;
			totalCloud += borderdetection_left;
			totalCloud += borderdetection_right;

			negative_mutex.lock();
			negativeDetectionCloud = negativeCloud;
			totalCloud += negativeCloud;
			negative_mutex.unlock();
			LidarProcess::cloud2OGM(negativeCloud,ogm_data_negative,3);
			LidarProcess::cloud2OGM(heightDiffCloud_l,ogm_data_height_left,3);
			LidarProcess::cloud2OGM(heightDiffCloud_r,ogm_data_height_right,3);
			for(int i = 0; i < ogm_data_height_total.ogmcell_size; i ++)
			{
				if(ogm_data_height_left.ogm[i] == RIGIDNOPASSABLE || ogm_data_height_right.ogm[i] == RIGIDNOPASSABLE)
					ogm_data_height_total.ogm[i] = RIGIDNOPASSABLE;
				else if(ogm_data_height_left.ogm[i] == PASSABLE || ogm_data_height_right.ogm[i] == PASSABLE)
				{
					ogm_data_height_total.ogm[i] = PASSABLE;
				}

			}
			for(int i = 0; i < ogm_data_total.ogmcell_size; i ++)
			{
				if(ogm_data_border_right.ogm[i] == NEGATIVENOPASSABLE || ogm_data_border_left.ogm[i] == NEGATIVENOPASSABLE)
					ogm_data_border_total.ogm[i] = NEGATIVENOPASSABLE;
				if(ogm_data_border_total.ogm[i] == NEGATIVENOPASSABLE || ogm_data_negative.ogm[i] == NEGATIVENOPASSABLE)
					ogm_data_negative.ogm[i] = NEGATIVENOPASSABLE;
			}
			for(int i = 0; i < ogm_data_total.ogmcell_size;i++)
			{
				if(ogm_data_negative.ogm[i] == NEGATIVENOPASSABLE && ogm_data_height_total.ogm[i] != RIGIDNOPASSABLE)
				{
					ogm_data_total.ogm[i] = NEGATIVENOPASSABLE;
				}
				else if(ogm_data_negative.ogm[i] != NEGATIVENOPASSABLE && ogm_data_height_total.ogm[i] == RIGIDNOPASSABLE)
				{
					ogm_data_total.ogm[i] = RIGIDNOPASSABLE;
				}
				else if(ogm_data_negative.ogm[i] == NEGATIVENOPASSABLE && ogm_data_height_total.ogm[i] == RIGIDNOPASSABLE)
				{
					ogm_data_total.ogm[i] = RIGIDNOPASSABLE;
				}
				else
				{
					ogm_data_total.ogm[i] = UNKNOWN;
				}
			}

			LidarProcess::showOGM("total_obstacle_frame",ogm_data_total);
			LidarProcess::showOGM("border_detection",ogm_data_border_total);
			negativeOGM.data.clear();
			negativeOGM.header.stamp = ros::Time(cloudmsg->header.stamp);
			negativeOGM.ogmheight = ogm_data_total.ogmheight_cell;
			negativeOGM.ogmwidth = ogm_data_total.ogmwidth_cell;
			negativeOGM.ogmresolution = ogm_data_total.ogmresolution;
			negativeOGM.vehicle_x = ogm_data_total.vehicle_x;
			negativeOGM.vehicle_y = ogm_data_total.vehicle_y;
			std::cout<<"ddddddddd="<<ogm_data_total.vehicle_x<<std::endl;
			for(int i = 0 ;i < ogm_data_total.ogmcell_size;i++)
			{
				negativeOGM.data.push_back(ogm_data_total.ogm[i]);
			}
			pubNegativeOGM_.publish(negativeOGM);
			cvWaitKey(2);
			LOG(INFO)<<"cloud num:"<<lidarpropertys.size();
			for(int i=0;i<lidarpropertys.size();i++)
			{
				LOG(INFO)<<i<<"\tpoint num:"<<outputclouds[i]->size();
				LOG(INFO)<<i<<"\tlidar pos:"<<lidarpropertys[i].x<<" "<<lidarpropertys[i].y<<" "<<lidarpropertys[i].z<<" ";
				LOG(INFO)<<i<<"\tlidar layernum:"<<int(lidarpropertys[i].intensity);
			}
//			lidarOdoms_.Pop(); //需要雷达里程计信息时需要，否则可以注释掉

		}

	}


	ros::Subscriber subLaserOdometry_ ;
	ros::Publisher pubNegativeOGM_;
	sensor_driver_msgs::NegativeOGM negativeOGM;

	ros::Subscriber subLaserCloudFullRes_ ;//经过筛选且转换之后的点云
	ros::Subscriber subCheck;
	double heightdiffthreshold_;
	common::BlockingQueue<std::unique_ptr<TimePosePair>> lidarOdoms_;
	common::BlockingQueue<sensor_msgs::PointCloud2ConstPtr> lidarCloudMsgs_;
//	std::fstream file_;
	boost::mutex cloud_mutex_;
	boost::mutex negative_mutex;
	pcl::PointCloud<pcl::PointXYZI> negativeDetectionCloud;
	boost::thread* processthread_;
	boost::thread* negativethread_;
	ros::NodeHandle& nodehandle_;
	bool processthreadfinished_;
	//negative detection
	LidarProcess* lidarprocess = NULL;
	XmlConf xmlconfig;
	pcl::CalibrationValue calibvalue;
	pcl::VelodyneGrabber* hdlgrabber;
	bool replay;
	bool display;
	float velodyneheight;
	int N_SCANS_;
	std::string hdlCalibration, pcapFile, srcip_H;
	float rigid_heightdiffthreshold;
	boost::thread* pointclouddisplay;
	pcl::PointCloud<pcl::PointXYZI> totalCloud;
};


int main(int argc, char** argv)
{
  ros::init(argc, argv, "negativenode");
  ros::NodeHandle nh;

  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  FLAGS_colorlogtostderr = true; //设置输出到屏幕的日志显示相应颜色
  PostProcess postprocess(nh);
  ros::spin();


  return 0;
}
