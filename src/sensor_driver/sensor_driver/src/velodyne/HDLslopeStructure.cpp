#include "HDLslopeStructure.h"
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include "util/xmlconf/xmlconf.h"
#include "mytime.h"
#include "data_types.hpp"
#define USE_OMP
#ifdef USE_OMP
#include <omp.h>
static omp_lock_t omplock;
#endif

using namespace std;
using namespace pcl;
using namespace pcl::console;
using namespace pcl::visualization;

using namespace boost;

boost::mutex displaymutex;

#define SHOW_FPS 0
#if SHOW_FPS
#define FPS_CALC(_WHAT_) \
    do \
{ \
    static unsigned count = 0;\
    static double last = getTime ();\
    double now = getTime (); \
    ++count; \
    if (now - last >= 1.0) \
    { \
        std::cout << "Average framerate("<< _WHAT_ << "): " << double(count)/double(now - last) << " Hz" <<  std::endl; \
        count = 0; \
        last = now; \
    } \
}while(false)
#else
#define FPS_CALC(_WHAT_) \
    do \
{ \
}while(false)
#endif

#define doNothing()
#define doSomething()


//#define GRASS_DETECT
#define OPEN_POLAR_DETECT       0
#define OPEN_VACANT_DETECT      0
#define OPEN_WATER_DETECT      0
//

#define HDL_MIN(a,b)  (fabs(a)<fabs(b)?a:b)
#define HDL_MAX(a,b)  (fabs(a)>fabs(b)?a:b)

LidarProcess::~LidarProcess()
{
    if(isinited)
    {
        flag_close=true;
#ifdef USE_OMP
        omp_destroy_lock(&omplock);
#endif

        fs_boundary.close();
        delete [] laneminintensity_ogm;
        delete [] lanemaxintensity_ogm;

        delete [] maxintensity_ogm;
        delete [] minintensity_ogm;
        cvReleaseImage(&intensityogm_img);
        if(thread_displayPointCloud)
        {
            thread_displayPointCloud->join();  //timed_join(boost::posix_time::seconds(1));
            delete thread_displayPointCloud;
            thread_displayPointCloud = NULL;
        }
        cloud_connection_H.disconnect ();
        std::cout<<"~hdlviewer"<<std::endl;
    }
}

void LidarProcess::cloud_callback_H (const CloudConstPtr& cloud)
{
    FPS_CALC ("cloud callback_H");
    boost::mutex::scoped_lock lock (cloud_mutex_H_);
    cloud_H_ = cloud;
}

void LidarProcess::cloud_callback (const CloudConstPtr& cloud, float startAngle, float endAngle)
{
    FPS_CALC ("cloud callback_H");

    boost::mutex::scoped_lock lock (cloud_mutex_H_);
    cloud_H_ = cloud;
}

void LidarProcess::keyboard_callback (const KeyboardEvent& event, void* cookie)
{
    if (event.keyUp ())
    {
        if(event.getKeyCode() ==  '1')
            freeze_ = 1;
        else if(event.getKeyCode() ==  '2')
            freeze_ = 0;
        return;
    }
}

void LidarProcess::mouse_callback (const MouseEvent& mouse_event, void* cookie)
{
    if (mouse_event.getType () == MouseEvent::MouseButtonPress &&
            mouse_event.getButton () == MouseEvent::LeftButton)
    {
        cout << mouse_event.getX () << " , " << mouse_event.getY () << endl;
    }
}


void LidarProcess::coordinate_from_vehicle_to_velodyne(float x, float y , float z, float& newx, float& newy, float& newz)
{
    newz = z;
    newx = x;
    newy = y;
}

void LidarProcess::setpointcloud (const CloudConstPtr& cloud)
{
    boost::mutex::scoped_lock lock (cloud_mutex_H_);
    cloud_H_ = cloud;
}

const Cloud& LidarProcess::getpointcloudaftercompute ()
{
  return *velodyne_pointcloud;
}

void LidarProcess::initHDL()
{
    if(isinited)
        return;
    isinited=true;

    boost::function<void (const pcl::PointCloud<pcl::PointXYZI>::ConstPtr&)> cloud_cb_H;
    if(fpointcloud_cb_!=NULL)
      cloud_cb_H=*fpointcloud_cb_;
    else
     cloud_cb_H = boost::bind(&LidarProcess::cloud_callback_H, this, _1);

    cloudsections.resize(LASER_LAYER);
    //坐标转换矩阵

    //使用角度来制作旋转平移矩阵

    alfa_h = calibvalue_.alfa*M_PI/180;
    beta_h = calibvalue_.beta*M_PI/180;
    gama_h = calibvalue_.gama*M_PI/180;//-178

    x_offset_h = calibvalue_.x_offset;
    y_offset_h = calibvalue_.y_offset ;//0.8;
    z_offset_h = calibvalue_.z_offset;
    transform_matrix_calibration_H2car_angle = Eigen::Matrix4f::Identity();

    transform_matrix_calibration_H2car_angle(0,0) = cos(beta_h)*cos(gama_h);
    transform_matrix_calibration_H2car_angle(0,1) = sin(alfa_h)*sin(beta_h)*cos(gama_h) - cos(alfa_h)*sin(gama_h);
    transform_matrix_calibration_H2car_angle(0,2) = cos(alfa_h)*sin(beta_h)*cos(gama_h) + sin(alfa_h)*sin(gama_h);
    transform_matrix_calibration_H2car_angle(0,3) = x_offset_h;
    transform_matrix_calibration_H2car_angle(1,0) = cos(beta_h)*sin(gama_h);
    transform_matrix_calibration_H2car_angle(1,1) = sin(alfa_h)*sin(beta_h)*sin(gama_h) + cos(alfa_h)*cos(gama_h);
    transform_matrix_calibration_H2car_angle(1,2) = cos(alfa_h)*sin(beta_h)*sin(gama_h) - sin(alfa_h)*cos(gama_h);
    transform_matrix_calibration_H2car_angle(1,3) = y_offset_h;
    transform_matrix_calibration_H2car_angle(2,0) = -sin(beta_h);
    transform_matrix_calibration_H2car_angle(2,1) = sin(alfa_h)*cos(beta_h);
    transform_matrix_calibration_H2car_angle(2,2) = cos(alfa_h)*cos(beta_h);
    transform_matrix_calibration_H2car_angle(2,3) = z_offset_h;
    transform_matrix_calibration_H2car_angle(3,0) = 0;
    transform_matrix_calibration_H2car_angle(3,1) = 0;
    transform_matrix_calibration_H2car_angle(3,2) = 0;
    transform_matrix_calibration_H2car_angle(3,3) = 1.0;

    laneminintensity_ogm = new float[laneogm.ogmcell_size];
    lanemaxintensity_ogm = new float[laneogm.ogmcell_size];

    maxintensity_ogm = new unsigned char[intensityogm.ogmcell_size];
    minintensity_ogm = new unsigned char[intensityogm.ogmcell_size];
    intensityogm_img = cvCreateImage(cvSize(intensityogm.ogmwidth_cell,intensityogm.ogmheight_cell),8,1);

    //任务1：确定车行人等障碍物
    //任务2：确定马路牙子等道路边界

    ogm_y_offset = 20.0f;//向y轴负方向移动了20
    farogm_y_offset = 20.0f - refineogm.ogmheight;//向y轴负方向移动了20 - ogmheightm
    laneogm_y_offset = 10.0f;//向y轴负方向移动了10m

    velodyne_pointcloud = Cloud::Ptr(new pcl::PointCloud<pcl::PointXYZI>);
    passablecloud = Cloud::Ptr(new pcl::PointCloud<pcl::PointXYZI>);//
    rigid_nopassablecloud = Cloud::Ptr(new pcl::PointCloud<pcl::PointXYZI>);//
    positiveslopecloud = Cloud::Ptr(new pcl::PointCloud<pcl::PointXYZI>);//
    negativeslopecloud = Cloud::Ptr(new pcl::PointCloud<pcl::PointXYZI>);//
    boundarycloud = Cloud::Ptr(new pcl::PointCloud<pcl::PointXYZI>);//
    roadorientationcloud = Cloud::Ptr(new pcl::PointCloud<pcl::PointXYZI>);//

    if(display_)
    {
        boost::function0<void> fdisplay = boost::bind(&LidarProcess::displayPointCloud,this);
        thread_displayPointCloud=new boost::thread(fdisplay);
    }
#ifdef USE_OMP
    omp_init_lock(&omplock);
#endif

    flag_ogmdetection=false;
    flag_diffdetection=false;
    flag_circletangential=false;
    flag_circleradius=false;

    cout<<"LASER_LAYER="<<LASER_LAYER<<endl;
    for(int i=0;i<LASER_LAYER;i++)
    {
        theorydis[i]=-z_offset_h/std::tan(grabber_H_.indexmaptable[i].angle*M_PI/180);
        if(theorydis[i]>0&&theorydis[i]<60)
            anglerange[i]=2.5/theorydis[i]*180/M_PI;
        cout<<"theorydis["<<i<<"]="<< theorydis[i]<<endl;
    }
}

void LidarProcess::displayPointCloud()
{
    boost::shared_ptr<PCLVisualizer> cloud_viewer_(new PCLVisualizer ("HDL Cloud"));

    cloud_viewer_->addCoordinateSystem (3.0);
    cloud_viewer_->setBackgroundColor (0, 0, 0);
    cloud_viewer_->initCameraParameters ();
    cloud_viewer_->setCameraPosition (0.0, 0.0, 30.0, 0.0, 1.0, 0.0, 0);
    cloud_viewer_->setCameraClipDistances (0.0, 100.0);
    cloud_viewer_->registerKeyboardCallback (&LidarProcess::keyboard_callback, *this);
    {
        //画车身
        float x1 = -1 , x2 = 1 , y1 = -1 , y2 = 3, z = 0;
        float newx1, newx2, newy1, newy2, newz;
        coordinate_from_vehicle_to_velodyne(x1,y1,z,newx1,newy1,newz);
        coordinate_from_vehicle_to_velodyne(x2,y2,z,newx2,newy2,newz);
        pcl::PointXYZI pt1, pt2, pt3, pt4;
        pt1.x = newx1 ;
        pt1.y = newy1 ;
        pt1.z = newz;
        pt2.x = newx1 ;
        pt2.y = newy2 ;
        pt2.z = newz;
        cloud_viewer_->addLine(pt1, pt2, "body1");

        pt1.x = newx2 ;
        pt1.y = newy2 ;
        pt1.z = newz;
        cloud_viewer_->addLine(pt1, pt2, "body2");

        pt2.x = newx2 ;
        pt2.y = newy1 ;
        pt2.z = newz;
        cloud_viewer_->addLine(pt1, pt2, "body3");

        pt1.x = newx1 ;
        pt1.y = newy1 ;
        pt1.z = newz;
        cloud_viewer_->addLine(pt1, pt2, "body4");

        //画上范围
        if(0)
        {
            float x1 = -20 , x2 = 20 , y1 = -1 , y2 = 40, z = Z_MAX;
            float newx1, newx2, newy1, newy2, newz;
            coordinate_from_vehicle_to_velodyne(x1,y1,z,newx1,newy1,newz);
            coordinate_from_vehicle_to_velodyne(x2,y2,z,newx2,newy2,newz);
            pcl::PointXYZI pt1, pt2, pt3, pt4;
            pt1.x = newx1 ;
            pt1.y = newy1 ;
            pt1.z = newz;
            pt2.x = newx1 ;
            pt2.y = newy2 ;
            pt2.z = newz;
            cloud_viewer_->addLine(pt1, pt2, "upper1");

            pt1.x = newx2 ;
            pt1.y = newy2 ;
            pt1.z = newz;
            cloud_viewer_->addLine(pt1, pt2, "upper2");

            pt2.x = newx2 ;
            pt2.y = newy1 ;
            pt2.z = newz;
            cloud_viewer_->addLine(pt1, pt2, "upper3");

            pt1.x = newx1 ;
            pt1.y = newy1 ;
            pt1.z = newz;
            cloud_viewer_->addLine(pt1, pt2, "upper4");
        }

        //画网格线
        char linename[20];
        for(int i = 0 ; i < 10 ; i++)
        {
            x1 = -20 ;
            x2 = 20 ;
            y1 = (i - 2) * 10 ;
            y2 = (i - 2) * 10;
            z = 0;
            coordinate_from_vehicle_to_velodyne(x1,y1,z,newx1,newy1,newz);
            coordinate_from_vehicle_to_velodyne(x2,y2,z,newx2,newy2,newz);
            pt1.x = min(newx1 , newx2) ;
            pt1.y = min(newy1 , newy2) ;
            pt1.z = newz;
            pt2.x = max(newx1 , newx2) ;
            pt2.y = max(newy1 , newy2) ;
            pt2.z = newz;
            memset(linename, 0 , 20);
            sprintf(linename , "lat%02d" , i);
            cloud_viewer_->addLine(pt1, pt2, linename);
        }

        for(int i = 0 ; i < 5 ; i++)
        {
            x1 = i * 10 - 20;
            x2 = i * 10 - 20;
            y1 = -20 ;
            y2 = 70 ;
            z = 0;
            coordinate_from_vehicle_to_velodyne(x1,y1,z,newx1,newy1,newz);
            coordinate_from_vehicle_to_velodyne(x2,y2,z,newx2,newy2,newz);
            pt1.x = min(newx1 , newx2) ;
            pt1.y = min(newy1 , newy2) ;
            pt1.z = newz;
            pt2.x = max(newx1 , newx2) ;
            pt2.y = max(newy1 , newy2) ;
            pt2.z = newz;
            memset(linename, 0 , 20);
            sprintf(linename , "lng%02d" , i);
            cloud_viewer_->addLine(pt1, pt2, linename);
        }
    }
}

void LidarProcess::ogmDetection()
{
	int LASER_LAYER = 32;
	float velodyne_z = 2.2;
	float y_offset = 1.99;
	float x_offset = 0.6;
    memset(hdl_ogm_data.ogm , 0 , hdl_ogm_data.ogmcell_size);
    memset(refineogm.ogm , UNKNOWN , refineogm.ogmcell_size);
    for(int i = 0 ; i < minzrefineogm.ogmcell_size ; i++)
        minzrefineogm.ogm[i] = 1000.0f;
    for(int i = 0 ; i < mindtafineogm.ogmcell_size ; i++)
        mindtafineogm.ogm[i] = 1000.0f;
    for(int i = 0 ; i < mindydfineogm.ogmcell_size ; i++)
        mindydfineogm.ogm[i] = 1000.0f;
    for(int i = 0 ; i < mindxdfineogm.ogmcell_size ; i++)
            mindxdfineogm.ogm[i] = 1000.0f;
    for(int i = 0 ; i < minztrefineogm.ogmcell_size ; i++)
        minztrefineogm.ogm[i] = 1000.0f;
    memset(farrefinedogm.ogm , UNKNOWN , farrefinedogm.ogmcell_size);
    for(int i = 0 ; i < farminzogm.ogmcell_size ; i++)
        farminzogm.ogm[i] = 1000.0f;
    memset(laneogm.ogm , UNKNOWN , laneogm.ogmcell_size);
    memset(intensityogm.ogm, UNKNOWN, intensityogm.ogmcell_size);
    Cloud::Ptr pointcloud(new pcl::PointCloud<pcl::PointXYZI>);
    for (int i = 0;i < velodyne_pointcloud->size();i++)
    {
	    if(velodyne_pointcloud->points[i].range >= 0.5  )
	    {
		    PointXYZI newpoint = velodyne_pointcloud->points[i];
		    pointcloud->points.push_back(newpoint);
	    }
    }
    if(1){
        for (int i = 0; i < velodyne_pointcloud->points.size(); i++){
            float x = velodyne_pointcloud->points[i].x,
                  y = velodyne_pointcloud->points[i].y,
                  z = velodyne_pointcloud->points[i].z;
            int intensity  = velodyne_pointcloud->points[i].intensity;
            if(velodyne_pointcloud->points[i].intensity < 15.0f)//排除噪声干扰，只考虑反射率较高的点
                continue;
            float newy = y + minzrefineogm.vehicle_y;
            float newx = x + minzrefineogm.vehicle_x;
            if((((6 <= newx && newx <= 18.5) || (21.5 <= newx && newx<= 34))) && (newy >=21 && newy < 40) && (z >= -3 && z <= 0.3))
           {
                int col = boost::math::round(newx / mindtafineogm.ogmresolution) ;
                int row = boost::math::round(newy / minzrefineogm.ogmresolution) ;

                if((row >=0 && row < minzrefineogm.ogmheight_cell)
                        && (col >=0 && col < minzrefineogm.ogmwidth_cell)){
                    int index = row * minzrefineogm.ogmwidth_cell + col;
                    if( minzrefineogm.ogm[index] > z)
                        minzrefineogm.ogm[index] = z;
                }
            }
        }

        int col_count = velodyne_pointcloud->size() / LASER_LAYER;
        int cloud_window_halflength = 13;
        for(int laser_i = 5 ; laser_i < LASER_LAYER *25/32; laser_i++)//编号15的俯仰角为0
        	{
        	int i = grabber_H_.indexmaptable[laser_i].number;
        	double angle = grabber_H_.indexmaptable[laser_i].angle;
            for(int j = cloud_window_halflength ; j < col_count - cloud_window_halflength; j++)
            {
            	int index = j * LASER_LAYER + i;//velodyne_pointcloud上点的索引
                float x = velodyne_pointcloud->points[index].x,
                      y = velodyne_pointcloud->points[index].y,
                      z = velodyne_pointcloud->points[index].z;
                int intensity = velodyne_pointcloud->points[index].intensity;
                float newy = y + mindtafineogm.vehicle_y;
                float newx = x + mindtafineogm.vehicle_x;
                if(velodyne_pointcloud->points[index].intensity < 15.0f)
                     continue;
                if(x>DEADZONE_LEFT&&x<DEADZONE_RIGHT&&y>DEADZONE_BACK&&y<DEADZONE_FRONT)
                	 continue;
                 if(((17 <= newx  && newx <= 23) ) && (newy >=21.5 && newy < 40) && (z >= 0 && z <= 3))
                 {
                	 int col = boost::math::round(newx / mindtafineogm.ogmresolution) ;
                     int row = boost::math::round(newy / mindtafineogm.ogmresolution) ;
                     if(( row >=0 && row < mindtafineogm.ogmheight_cell) && (col >=0 && col < mindtafineogm.ogmwidth_cell))
                     {
                    	 int index_s = row * mindtafineogm.ogmwidth_cell + col;//删格图上点的索引
                         if(velodyne_pointcloud->points[index].azimuth> 2 && velodyne_pointcloud->points[index].azimuth < 178)
                         {
                        	 int index1 = (j + cloud_window_halflength) * LASER_LAYER + i;//求切点的索引
                             int index2 = (j - cloud_window_halflength) * LASER_LAYER + i;
                             if(velodyne_pointcloud->points[index1].range < 1.6)
                            	 continue;
                             if(velodyne_pointcloud->points[index2].range < 1.6)
                                 continue;
                             float expected_tangential_angle = velodyne_pointcloud->points[index].azimuth - 90;
                             float actual_tangential_angle = atan2(velodyne_pointcloud->points[index1].y - velodyne_pointcloud->points[index2].y,
                                    		velodyne_pointcloud->points[index1].x - velodyne_pointcloud->points[index2].x) * 180 / M_PI;
                             float delta_angle = abs(actual_tangential_angle - expected_tangential_angle);
                             if( mindtafineogm.ogm[index_s] > delta_angle)
                            	 mindtafineogm.ogm[index_s] = delta_angle;
                         }
                     }
                 }

                 newy = y + minztrefineogm.vehicle_y;
                 newx = x + minztrefineogm.vehicle_x;
                 if((17 <= newx  && newx <= 23) && (newy >=21.5 && newy < 40) && (z >= 0 && z <= 3) )
                 {
                	 int col = boost::math::round(newx / minztrefineogm.ogmresolution);
                     int row = boost::math::round(newy / minztrefineogm.ogmresolution);
                     if((row >=0 && row < minztrefineogm.ogmheight_cell) && (col >= 0 && col < minztrefineogm.ogmwidth_cell))
                     {
                    	 int index_z = row * minztrefineogm.ogmwidth_cell + col;
                         if( minztrefineogm.ogm[index_z] > z)
                        	 minztrefineogm.ogm[index_z] = z;
                     }
                 }

                 newy = y + mindydfineogm.vehicle_y;
                 newx = x + mindydfineogm.vehicle_x;
                 if((17 <= newx && newx <= 23) && (newy >=21.5 && newy < 40) && (z >=0 && z <= 3))
                 {
                	 int col = boost::math::round(newx / mindydfineogm.ogmresolution);
                	 int row = boost::math::round(newy / mindydfineogm.ogmresolution);
                	 if((row >=0 && row < mindydfineogm.ogmheight_cell) && (col < mindydfineogm.ogmwidth_cell))
                	 {
                		 int index_y = row * mindydfineogm.ogmwidth_cell + col;
                		 if(velodyne_pointcloud->points[index].azimuth> 2 && velodyne_pointcloud->points[index].azimuth < 178)
                		 {
                			 float expected_process_y = velodyne_z * tan(angle);
                			 float expected_y = expected_process_y * cos(abs(velodyne_pointcloud->points[index].azimuth -90)) + y_offset;
                			 float delta_y = expected_y - y;
                			 if(mindydfineogm.ogm[index_y] > delta_y)
                				 mindydfineogm.ogm[index_y] = delta_y;
                		 }
                	 }
                 }

                 newy = y + mindxdfineogm.vehicle_y;
                 newx = x + mindxdfineogm.vehicle_x;
                 if((17 <= newx && newx <= 23) && (newy >=21.5 && newy < 40) && (z >=0 && z <= 3))
                 {
                	 int col = boost::math::round(newx / mindxdfineogm.ogmresolution);
                     int row = boost::math::round(newy / mindxdfineogm.ogmresolution);
                     if((row >=0 && row < mindxdfineogm.ogmheight_cell) && (col >= 0 && col < mindxdfineogm.ogmwidth_cell))
                     {
                    	 int index_x = row * mindxdfineogm.ogmwidth_cell + col;
                         if(velodyne_pointcloud->points[index].azimuth> 2 && velodyne_pointcloud->points[index].azimuth < 178)
                         {
                        	 float expected_process_y = velodyne_z * tan(angle);
                             float expected_x = expected_process_y * sin(abs(velodyne_pointcloud->points[index].azimuth -90)) + x_offset;
                             float delta_x = expected_x - abs(x);
                             if(mindxdfineogm.ogm[index_x] > delta_x)
                            	 mindxdfineogm.ogm[index_x] = delta_x;
                         }
                     }
                 }
            }
        	}
    }
}



void LidarProcess::haarGradient(const OGMData<float>& minzogm,OGMData<float>& slopexogm, OGMData<float>& slopeyogm , int window_halfnum, float slopeminthreshold , float slopemaxthreshold)
{
if(0)
{
    {
        for(int j=window_halfnum;j<minzogm.ogmheight_cell-window_halfnum;j++)
        {
            for(int i=window_halfnum;i<minzogm.ogmwidth_cell-window_halfnum;i++)
            {
                int upcounter=0;
                int downcounter=0;
                double upzsum = 0;
                double downzsum = 0;
                float upz = 0;
                float downz = 0;
                float heightdiff = 0;
                float slopeval = 0;
                float slopedegree = 0;
                for(int window_i=i-window_halfnum ; window_i<=i+window_halfnum ; window_i++)
                {
                    for(int window_j=j-window_halfnum ; window_j<=j ; window_j++)
                    {
                        float val = minzogm.ogm[window_i + window_j*minzogm.ogmwidth_cell];
                        unsigned char val1 = hdl_ogm_data.ogm[window_i + window_j*hdl_ogm_data.ogmwidth_cell];
                        if(val<500 && val1 !=RIGIDNOPASSABLE)
                        {
                            downzsum +=val;
                            downcounter+=1;

                        }
                    }
                }
                if(downcounter<1)
                    continue;
                for(int window_i=i-window_halfnum ; window_i<=i+window_halfnum ; window_i++)
                {
                    for(int window_j=j; window_j<=j + window_halfnum ; window_j++)
                    {
                        float val = minzogm.ogm[window_i + window_j*minzogm.ogmwidth_cell];
                        unsigned char val1 = hdl_ogm_data.ogm[window_i + window_j*hdl_ogm_data.ogmwidth_cell];
                        if(val<500 && val1 !=RIGIDNOPASSABLE)
                        {
                            upzsum +=val;
                            upcounter+=1;
                        }
                    }
                }
                if(upcounter<1)
                    continue;
                upz = upzsum/upcounter;
                downz = downzsum/downcounter;

                heightdiff = downz - upz  ;
                slopeval = heightdiff/(window_halfnum*minzogm.ogmresolution);
                if(fabs(slopeval) > slopeminthreshold)
                {
                    slopeyogm.ogm[i + j * slopeyogm.ogmwidth_cell] = slopeval;
                }
            }
        }
    }
}

//slopex_ogm
    if(1)
    {
        for(int j=window_halfnum;j<minzogm.ogmheight_cell-window_halfnum;j++)
        {
            for(int i=window_halfnum;i<minzogm.ogmwidth_cell-window_halfnum;i++)
            {
                int leftcounter=0;
                int rightcounter=0;
                double leftzsum = 0;
                double rightzsum = 0;
                float leftz = 0;
                float rightz = 0;
                float heightdiff = 0;
                float slopeval = 0;
                float slopedegree = 0;
                for(int window_j=j-window_halfnum ; window_j<=j+window_halfnum ; window_j++)
                {
                    for(int window_i=i-window_halfnum ; window_i<i ; window_i++)
                    {
                        float val = minzogm.ogm[window_i + window_j*minzogm.ogmwidth_cell];
                        if(val<500)
                        {
                            leftzsum += val;
                            leftcounter += 1;
                        }
                    }
                }
                if(leftcounter<8)
                    continue;
                for(int window_j=j-window_halfnum ; window_j<=j+window_halfnum ; window_j++)
                {
                    for(int window_i=i+1; window_i<=i + window_halfnum ; window_i++)
                    {
                        float val = minzogm.ogm[window_i + window_j*minzogm.ogmwidth_cell];
                        if(val<500 )
                        {
                            rightzsum +=val;
                            rightcounter+=1;
                        }
                    }
                }
                if(rightcounter<8)
                    continue;
                leftz = leftzsum/leftcounter;
                rightz = rightzsum/rightcounter;

                heightdiff = rightz - leftz;
                if(i > minzogm.ogmwidth_cell/2)
                    heightdiff = -heightdiff;
                slopeval = heightdiff/(window_halfnum*minzogm.ogmresolution);
                if(slopeval > slopeminthreshold && slopeval < slopemaxthreshold)   //slopeval仅由heightdiff一个变量决定，所以就用其中一个做阈值判断即可
                {
                    slopexogm.ogm[i + j * slopexogm.ogmwidth_cell] = slopeval;
                    slopedegree = atan(slopeval)*180/M_PI;
                    std::cout << slopedegree << std::endl;
                }
            }
        }
    }
}

    template<class T>
void LidarProcess::showHaarOGM(const char* windowname ,const OGMData<T>& ogmdata)
{
    cvNamedWindow(windowname,0);
    CvMat *slopemat = cvCreateMat(ogmdata.ogmheight_cell,ogmdata.ogmwidth_cell,CV_8UC3);
    cvZero(slopemat);
    int heightnum = ogmdata.ogmheight_cell/(10/ogmdata.ogmresolution);
            int widthnum = ogmdata.ogmwidth_cell/(10/ogmdata.ogmresolution);
            for(int i = 0; i<heightnum ;i++)
                      {
                        cvLine(slopemat,cvPoint(0,slopemat->height*i/heightnum),
                                cvPoint(slopemat->width-1,slopemat->height*i/heightnum),cvScalar(255,0,0));
                      }
                    for(int i=1;i<widthnum;i++)
                    {
                        cvLine(slopemat,cvPoint(slopemat->width*i/widthnum,0),
                                cvPoint(slopemat->width*i/widthnum,slopemat->height-1),cvScalar(255,0,0));
                    }
                    float ogmresolution = ogmdata.ogmresolution;
                        cvRectangle(slopemat,cvPoint(slopemat->width/2-1/ogmresolution,
                    			slopemat->height-(20+2)/ogmresolution),
                                cvPoint(slopemat->width/2+1/ogmresolution,slopemat->height-(20-2)/ogmresolution),
                                cvScalar(0,255,0));
    for(int j=0;j<ogmdata.ogmheight_cell;j++)
    {
        unsigned char* pdata = (unsigned char*)(slopemat->data.ptr + (ogmdata.ogmheight_cell - 1 - j)* slopemat->step);
        for(int i=0 ;i < ogmdata.ogmwidth_cell ; i++)
        {
            float val = ogmdata.ogm[i + j*ogmdata.ogmwidth_cell];
            if(val<500)
            {
                if(val<=-1)
                    pdata[3*i+2] = 0;
                else if(val >= 1)
                    pdata[3*i+2] = 250;
                else
                    pdata[3*i+2] =static_cast<unsigned char>((val - (-1))*250/(1 - (-1)));
            }
            else
            {
                pdata[3*i+1] = 125;
            }
        }
    }
    cvShowImage(windowname,slopemat);
    cvWaitKey(1);
    cvReleaseMat(&slopemat);
}

        template<class T>
    void LidarProcess::showdtaOGM(const char* windowname ,const OGMData<T>& ogmdata)
    {
        cvNamedWindow(windowname,0);
        CvMat *slopemat = cvCreateMat(ogmdata.ogmheight_cell,ogmdata.ogmwidth_cell,CV_8UC3);
        cvZero(slopemat);
        int heightnum = ogmdata.ogmheight_cell/(10/ogmdata.ogmresolution);
        int widthnum = ogmdata.ogmwidth_cell/(10/ogmdata.ogmresolution);
        for(int i = 0; i<heightnum ;i++)
          {
            cvLine(slopemat,cvPoint(0,slopemat->height*i/heightnum),
                    cvPoint(slopemat->width-1,slopemat->height*i/heightnum),cvScalar(255,0,0));
          }
        for(int i=1;i<widthnum;i++)
        {
            cvLine(slopemat,cvPoint(slopemat->width*i/widthnum,0),
                    cvPoint(slopemat->width*i/widthnum,slopemat->height-1),cvScalar(255,0,0));
        }
        float ogmresolution = ogmdata.ogmresolution;
            cvRectangle(slopemat,cvPoint(slopemat->width/2-1/ogmresolution,
        			slopemat->height-(20+2)/ogmresolution),
                    cvPoint(slopemat->width/2+1/ogmresolution,slopemat->height-(20-2)/ogmresolution),
                    cvScalar(0,255,0));
        for(int j=0;j<ogmdata.ogmheight_cell;j++)
        {
            unsigned char* pdata = (unsigned char*)(slopemat->data.ptr + (ogmdata.ogmheight_cell - 1 - j)* slopemat->step);
            for(int i=0 ;i < ogmdata.ogmwidth_cell ; i++)
            {
                float val = ogmdata.ogm[i + j*ogmdata.ogmwidth_cell];
                if(val < 500)
                {
                	if(val<= 0)
                		pdata[3*i+1] = 0;
                    else if(val >= 1)
                    	pdata[3*i+1] = 250;
                    else
                    	pdata[3*i+1] =static_cast<unsigned char>((val + 1)*125);
                }
                else
                {
                	pdata[3*i+2] = 125;
                }
            }
        }
        cvShowImage(windowname,slopemat);
        cvWaitKey(1);
        cvReleaseMat(&slopemat);
    }

void LidarProcess::haarProcess()
{
        int window_halfnum=5;   //遍历窗口放小可以减少对凹障碍的误检
        float slopeminthreshold = 0.1405;   //8度
        float slopemaxthreshold = 0.4363;   //25度
        haarGradient(minzrefineogm,slopemyogm,slope5yogm,window_halfnum,slopeminthreshold , slopemaxthreshold);
}



const OGMData<float>* LidarProcess::slopeprocess(std::vector<pcl::PointCloud<pcl::PointXYZI>::Ptr> pointcloud)
{
	{
		for(int i=0;i<slopemyogm.ogmcell_size;i++){
			slopemyogm.ogm[i]=1000.0f;
		}
	}
	for(int i=0;i<pointcloud.size();i++){
	velodyne_pointcloud = pointcloud[i];
    ogmDetection();
    haarProcess();
}
    showHaarOGM("slope5xogm",slopemyogm);
    return &slopemyogm;
}
