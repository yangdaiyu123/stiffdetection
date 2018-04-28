;; Auto-generated. Do not edit!


(when (boundp 'sensor_driver_msgs::stiffwater)
  (if (not (find-package "SENSOR_DRIVER_MSGS"))
    (make-package "SENSOR_DRIVER_MSGS"))
  (shadow 'stiffwater (find-package "SENSOR_DRIVER_MSGS")))
(unless (find-package "SENSOR_DRIVER_MSGS::STIFFWATER")
  (make-package "SENSOR_DRIVER_MSGS::STIFFWATER"))

(in-package "ROS")
;;//! \htmlinclude stiffwater.msg.html
(if (not (find-package "STD_MSGS"))
  (ros::roseus-add-msgs "std_msgs"))


(defclass sensor_driver_msgs::stiffwater
  :super ros::object
  :slots (_header _ogmheight _ogmwidth _vehicle_x _vehicle_y _resolution _data ))

(defmethod sensor_driver_msgs::stiffwater
  (:init
   (&key
    ((:header __header) (instance std_msgs::Header :init))
    ((:ogmheight __ogmheight) 0)
    ((:ogmwidth __ogmwidth) 0)
    ((:vehicle_x __vehicle_x) 0)
    ((:vehicle_y __vehicle_y) 0)
    ((:resolution __resolution) 0.0)
    ((:data __data) (make-array 0 :initial-element 0 :element-type :integer))
    )
   (send-super :init)
   (setq _header __header)
   (setq _ogmheight (round __ogmheight))
   (setq _ogmwidth (round __ogmwidth))
   (setq _vehicle_x (round __vehicle_x))
   (setq _vehicle_y (round __vehicle_y))
   (setq _resolution (float __resolution))
   (setq _data __data)
   self)
  (:header
   (&rest __header)
   (if (keywordp (car __header))
       (send* _header __header)
     (progn
       (if __header (setq _header (car __header)))
       _header)))
  (:ogmheight
   (&optional __ogmheight)
   (if __ogmheight (setq _ogmheight __ogmheight)) _ogmheight)
  (:ogmwidth
   (&optional __ogmwidth)
   (if __ogmwidth (setq _ogmwidth __ogmwidth)) _ogmwidth)
  (:vehicle_x
   (&optional __vehicle_x)
   (if __vehicle_x (setq _vehicle_x __vehicle_x)) _vehicle_x)
  (:vehicle_y
   (&optional __vehicle_y)
   (if __vehicle_y (setq _vehicle_y __vehicle_y)) _vehicle_y)
  (:resolution
   (&optional __resolution)
   (if __resolution (setq _resolution __resolution)) _resolution)
  (:data
   (&optional __data)
   (if __data (setq _data __data)) _data)
  (:serialization-length
   ()
   (+
    ;; std_msgs/Header _header
    (send _header :serialization-length)
    ;; int16 _ogmheight
    2
    ;; int16 _ogmwidth
    2
    ;; int16 _vehicle_x
    2
    ;; int16 _vehicle_y
    2
    ;; float32 _resolution
    4
    ;; int16[] _data
    (* 2    (length _data)) 4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; std_msgs/Header _header
       (send _header :serialize s)
     ;; int16 _ogmheight
       (write-word _ogmheight s)
     ;; int16 _ogmwidth
       (write-word _ogmwidth s)
     ;; int16 _vehicle_x
       (write-word _vehicle_x s)
     ;; int16 _vehicle_y
       (write-word _vehicle_y s)
     ;; float32 _resolution
       (sys::poke _resolution (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; int16[] _data
     (write-long (length _data) s)
     (dotimes (i (length _data))
       (write-word (elt _data i) s)
       )
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; std_msgs/Header _header
     (send _header :deserialize buf ptr-) (incf ptr- (send _header :serialization-length))
   ;; int16 _ogmheight
     (setq _ogmheight (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _ogmwidth
     (setq _ogmwidth (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _vehicle_x
     (setq _vehicle_x (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; int16 _vehicle_y
     (setq _vehicle_y (sys::peek buf ptr- :short)) (incf ptr- 2)
   ;; float32 _resolution
     (setq _resolution (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; int16[] _data
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _data (instantiate integer-vector n))
     (dotimes (i n)
     (setf (elt _data i) (sys::peek buf ptr- :short)) (incf ptr- 2)
     ))
   ;;
   self)
  )

(setf (get sensor_driver_msgs::stiffwater :md5sum-) "88737214cd1e847e12b0a969fb43ebbd")
(setf (get sensor_driver_msgs::stiffwater :datatype-) "sensor_driver_msgs/stiffwater")
(setf (get sensor_driver_msgs::stiffwater :definition-)
      "std_msgs/Header header
int16 ogmheight
int16 ogmwidth
int16 vehicle_x
int16 vehicle_y
float32 resolution
int16[] data

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



(provide :sensor_driver_msgs/stiffwater "88737214cd1e847e12b0a969fb43ebbd")


