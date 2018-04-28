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
#include <sensor_msgs/PointCloud2.h>//每个消息类型都有一个对应的c++头文件
#include <tf/transform_datatypes.h>
#include <tf/transform_broadcaster.h>
#include <fstream>
#include <list>
#include <glog/logging.h>

#include "velodyne/HDLslopeStructure.h"
#include "util/boostudp/boostudp.h"
#include "common/blocking_queue.h"
#include "common/make_unique.h"
#include "sensor_driver_msgs/OdometrywithGps.h"
#include "sensor_driver_msgs/SlopeOGM.h"

#define LOCAL_IP "192.168.0.112"
//#define LOCAL_IP "127.0.0.1"
#define FROMLADAR_LOCAL_PORT 9906

class PostProcess
{
public:
	typedef std::pair<double,transform::Rigid3d> TimePosePair;
	PostProcess(ros::NodeHandle& nodehandle):nodehandle_(nodehandle)
	,processthread_(NULL)
	,slopethread_(NULL)//mdj
	,lidarprocess(NULL)//mdj
	,processthreadfinished_ (false)
	{
		init();
	}
	~PostProcess()
	{
	  lidarOdoms_.stopQueue();
	  processthreadfinished_ = true;
	  processthread_->join();
	  slopethread_->join();
	}

	void init()
	{
		//mdj_begin
		replay = 1;
		display = false;
		velodyneheight = 2.2;
		N_SCANS=32;
		rigid_heightdiffthreshold = 0.3;
		hdlCalibration = "";
		hdlgrabber=new pcl::VelodyneGrabber(hdlCalibration, pcapFile, N_SCANS);
		lidarprocess=new LidarProcess(*hdlgrabber,replay,display,velodyneheight,rigid_heightdiffthreshold,xmlconfig,calibvalue);
		pubSlopeOGM_ = nodehandle_.advertise<sensor_driver_msgs::SlopeOGM> ("slope6yOgm", 5);
		/*while(ros::ok())
		{
			sensor_driver_msgs::SlopeOGM cloudmsg;
		}*/
		//mdj_end

//		subLaserOdometry_ = nodehandle_.subscribe<sensor_driver_msgs::OdometrywithGps>
//										 ("lidar_odometry_to_init", 5, boost::bind(&PostProcess::laserOdometryHandler,this,_1));//需要雷达里程计信息时需要，否则可以注释掉

		subLaserCloudFullRes_ = nodehandle_.subscribe<sensor_msgs::PointCloud2>
										 ("lidar_cloud_calibrated", 1, boost::bind(&PostProcess::laserCloudHandler,this,_1));//经过筛选且转换之后的点云

		processthread_ = new boost::thread(boost::bind(&PostProcess::process,this));
		slopethread_ = new boost::thread(boost::bind(&PostProcess::pointCloudDisplay,this));//mdj 新开一个线程
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

	void pointCloudDisplay()
		{
				boost::shared_ptr<PCLVisualizer> cloud_viewer_slope(new PCLVisualizer ("HDL Cloud"));
				cloud_viewer_slope->addCoordinateSystem (3.0);
				cloud_viewer_slope->setBackgroundColor (0, 0, 0);
				cloud_viewer_slope->initCameraParameters ();
				cloud_viewer_slope->setCameraPosition (0.0, 0.0, 30.0, 0.0, 1.0, 0.0, 0);
				cloud_viewer_slope->setCameraClipDistances (0.0, 100.0);
				Cloud::Ptr passableCloud(new Cloud);
				Cloud::Ptr slopeCloud(new Cloud);
				passableCloud->clear();
				slopeCloud->clear();
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
					cloud_viewer_slope->addLine(pt1, pt2, linename);
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
				    cloud_viewer_slope->addLine(pt1, pt2, linename);
				}

				while(!cloud_viewer_slope->wasStopped())
			{
				cloud_mutex_.lock();
				for(int i = 0;i < slopeDetectionCloud.points.size();i++)
				{
					if(slopeDetectionCloud.points[i].passibility < 0.1)
						slopeCloud->points.push_back(slopeDetectionCloud.points[i]);
					else
						passableCloud->points.push_back(slopeDetectionCloud.points[i]);
				}
				cloud_mutex_.unlock();
					cloud_viewer_slope->removeAllPointClouds();
					if(slopeDetectionCloud.points.size()>0)
					{
						if(passableCloud->points.size()>0)
						{
							pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZI> passableCloudHandler (passableCloud, 0, 255, 0);
							if (!cloud_viewer_slope->updatePointCloud (passableCloud, passableCloudHandler, "passableCloud"))
							{
								cloud_viewer_slope->addPointCloud (passableCloud, passableCloudHandler, "passableCloud");
							}
						}
						if(slopeCloud->points.size()>0)
						{
							pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZI> slopeCloudHandler (slopeCloud, 255, 0, 255);
							if (!cloud_viewer_slope->updatePointCloud (slopeCloud, slopeCloudHandler, "slopeCloud"))
							{
								cloud_viewer_slope->addPointCloud (slopeCloud, slopeCloudHandler, "slopeCloud");
							}
						}
					}
					cloud_viewer_slope->spinOnce();
					boost::this_thread::sleep (boost::posix_time::microseconds(100));
					passableCloud->clear();
					slopeCloud->clear();
				}
				cloud_viewer_slope->close();
		}
//mdj_end
	void laserCloudHandler(const sensor_msgs::PointCloud2ConstPtr& laserCloudFullRes2) //点云数据
	{
	    timeLaserCloudFullRes = laserCloudFullRes2->header.stamp;
		LOG(INFO)<<std::fixed<<std::setprecision(3)<<"cloudtime:"<<timeLaserCloudFullRes;
		LOG(INFO)<<"starttime"<<ros::Time::now() - timeLaserCloudFullRes;
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
			pcl::PointCloud<pcl::PointXYZI> slopeCloud;
			pcl::PointCloud<pcl::PointXYZI>::Ptr tempcloud(new pcl::PointCloud<pcl::PointXYZI>);//当前帧点云（雷达里程计坐标系）
			pcl::fromROSMsg(*cloudmsg, *tempcloud);//获取当前帧点云数据
			std::vector<pcl::PointCloud<pcl::PointXYZI>::Ptr> outputclouds;

			std::vector<pcl::PointXYZI> lidarpropertys;
			analysisCloud(tempcloud,outputclouds,lidarpropertys);
			//这里outputclouds[0]对应config/extrinsicparameter/toyota32/0.lua
			//当然outputclouds[1]对应config/extrinsicparameter/toyota32/1.lua
//			memset(&lidarprocess->slopemyogm,0,lidarprocess->slopemyogm.ogmcell_size);
			const OGMData<float>* slopeogm = lidarprocess->slopeprocess(outputclouds);
//			slopeogm = lidarprocess->slopeprocess(outputclouds[0]);
			slope_mutex.lock();

			slopeDetectionCloud = *tempcloud;
			slope_mutex.unlock();
			SlopeOGM.ogmheight = slopeogm->ogmheight_cell;
			SlopeOGM.ogmwidth = slopeogm->ogmwidth_cell;
			SlopeOGM.ogmresolution = slopeogm->ogmresolution;
			SlopeOGM.vehicle_x = 100;
			SlopeOGM.vehicle_y = 100;
			SlopeOGM.slopeval.resize(slopeogm->ogmcell_size);
			SlopeOGM.header.stamp = timeLaserCloudFullRes;
						for(int i = 0 ;i < slopeogm->ogmcell_size;i++)
						{
							SlopeOGM.slopeval[i]=slopeogm->ogm[i];
						}
						pubSlopeOGM_.publish(SlopeOGM);
			LOG(INFO)<<"cloud num:"<<lidarpropertys.size();
			for(int i=0;i<lidarpropertys.size();i++)
			{
				LOG(INFO)<<i<<"point num:"<<outputclouds[i]->size();
				LOG(INFO)<<i<<"lidar pos:"<<lidarpropertys[i].x<<""<<lidarpropertys[i].y<<""<<lidarpropertys[i].z;
				LOG(INFO)<<i<<"lidar layernum:"<<int(lidarpropertys[i].intensity);
			}
//			lidarOdoms_.Pop(); //需要雷达里程计信息时需要，否则可以注释掉

		}

	}


	ros::Subscriber subLaserOdometry_ ;


	ros::Subscriber subLaserCloudFullRes_ ;//经过筛选且转换之后的点云

	common::BlockingQueue<std::unique_ptr<TimePosePair>> lidarOdoms_;
	common::BlockingQueue<sensor_msgs::PointCloud2ConstPtr> lidarCloudMsgs_;
	boost::mutex cloud_mutex_;
	boost::mutex slope_mutex;
	pcl::PointCloud<pcl::PointXYZI> slopeDetectionCloud;//mdj
	boost::thread* processthread_;
	boost::thread* slopethread_;//mdj
	ros::NodeHandle& nodehandle_;
	bool processthreadfinished_;
	//mdj_begin
	LidarProcess* lidarprocess = NULL;
	bool replay;
	bool display;
	float velodyneheight;
	int N_SCANS;
	float rigid_heightdiffthreshold;
	std::string hdlCalibration, pcapFile, srcip_H;
	pcl::VelodyneGrabber* hdlgrabber;
	XmlConf xmlconfig;
	pcl::CalibrationValue calibvalue;
	ros::Publisher pubSlopeOGM_;
	sensor_driver_msgs::SlopeOGM SlopeOGM;
	ros::Time timeLaserCloudFullRes;
	//mdj_end
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "slopenode");//函数最后的参数是一个包含节点默认名的字符串
  ros::NodeHandle nh;//节点句柄 将该程序注册为ros节点管理器的一个节点  是该程序与ros系统交互的主要机制

  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  FLAGS_colorlogtostderr = true; //设置输出到屏幕的日志显示相应颜色
  PostProcess postprocess(nh);
  ros::spin();
  return 0;
}
