;; Auto-generated. Do not edit!


(when (boundp 'sensor_driver_msgs::SlopeOGM)
  (if (not (find-package "SENSOR_DRIVER_MSGS"))
    (make-package "SENSOR_DRIVER_MSGS"))
  (shadow 'SlopeOGM (find-package "SENSOR_DRIVER_MSGS")))
(unless (find-package "SENSOR_DRIVER_MSGS::SLOPEOGM")
  (make-package "SENSOR_DRIVER_MSGS::SLOPEOGM"))

(in-package "ROS")
;;//! \htmlinclude SlopeOGM.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass sensor_driver_msgs::SlopeOGM
  :super ros::object
  :slots (_header _ogmwidth _ogmheight _ogmresolution _vehicle_x _vehicle_y _slopeval ))

(defmethod sensor_driver_msgs::SlopeOGM
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:ogmwidth __ogmwidth) 0)
    ((:ogmheight __ogmheight) 0)
    ((:ogmresolution __ogmresolution) 0.0)
    ((:vehicle_x __vehicle_x) 0)
    ((:vehicle_y __vehicle_y) 0)
    ((:slopeval __slopeval) (make-array 0 :initial-element 0.0 :element-type :float))
    )
   (send-super :init)
   (setq _header __header)
   (setq _ogmwidth (round __ogmwidth))
   (setq _ogmheight (round __ogmheight))
   (setq _ogmresolution (float __ogmresolution))
   (setq _vehicle_x (round __vehicle_x))
   (setq _vehicle_y (round __vehicle_y))
   (setq _slopeval __slopeval)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:ogmwidth
   (&optional __ogmwidth)
   (if __ogmwidth (setq _ogmwidth __ogmwidth)) _ogmwidth)
  (:ogmheight
   (&optional __ogmheight)
   (if __ogmheight (setq _ogmheight __ogmheight)) _ogmheight)
  (:ogmresolution
   (&optional __ogmresolution)
   (if __ogmresolution (setq _ogmresolution __ogmresolution)) _ogmresolution)
  (:vehicle_x
   (&optional __vehicle_x)
   (if __vehicle_x (setq _vehicle_x __vehicle_x)) _vehicle_x)
  (:vehicle_y
   (&optional __vehicle_y)
   (if __vehicle_y (setq _vehicle_y __vehicle_y)) _vehicle_y)
  (:slopeval
   (&optional __slopeval)
   (if __slopeval (setq _slopeval __slopeval)) _slopeval)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; int16 _ogmwidth
    2
    ;; int16 _ogmheight
    2
    ;; float32 _ogmresolution
    4
    ;; int16 _vehicle_x
    2
    ;; int16 _vehicle_y
    2
    ;; float32[] _slopeval
    (* 4    (length _slopeval)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; int16 _ogmwidth
       (write-word _ogmwidth s)
     ;; int16 _ogmheight
       (write-word _ogmheight s)
     ;; float32 _ogmresolution
       (sys::poke _ogmresolution (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; int16 _vehicle_x
       (write-word _vehicle_x s)
     ;; int16 _vehicle_y
       (write-word _vehicle_y s)
     ;; float32[] _slopeval
     (write-long (length _slopeval) s)
     (dotimes (i (length _slopeval))
       (sys::poke (elt _slopeval i) (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; int16 _ogmwidth
     (setq _ogmwidth (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _ogmheight
     (setq _ogmheight (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; float32 _ogmresolution
     (setq _ogmresolution (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; int16 _vehicle_x
     (setq _vehicle_x (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _vehicle_y
     (setq _vehicle_y (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; float32[] _slopeval
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _slopeval (instantiate float-vector n))
     (dotimes (i n)
     (setf (elt _slopeval i) (sys::peek buf ptr- :float)) (incf ptr- 4)
     ))
   ;;
   self)
  )

(setf (get sensor_driver_msgs::SlopeOGM :md5sum-) "4b5a053c15a636703b341c1ab43de555")
(setf (get sensor_driver_msgs::SlopeOGM :datatype-) "sensor_driver_msgs/SlopeOGM")
(setf (get sensor_driver_msgs::SlopeOGM :definition-)
      "Header header
int16 ogmwidth
int16 ogmheight
float32 ogmresolution
int16 vehicle_x
int16 vehicle_y
float32[] slopeval

================================================================================
MSG: std_msgs/Header
# Standard metadata for higher-level stamped data types.
# This is generally used to communicate timestamped data 
# in a particular coordinate frame.
# 
# sequence ID: consecutively increasing ID 
uint32 seq
#Two-integer timestamp that is expressed as:
# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
# time-handling sugar is provided by the client library
time stamp
#Frame this data is associated with
# 0: no frame
# 1: global frame
string frame_id

")



(provide :sensor_driver_msgs/SlopeOGM "4b5a053c15a636703b341c1ab43de555")


