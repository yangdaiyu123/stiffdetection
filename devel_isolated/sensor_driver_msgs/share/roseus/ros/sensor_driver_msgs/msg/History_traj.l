;; Auto-generated. Do not edit!


(when (boundp 'sensor_driver_msgs::History_traj)
  (if (not (find-package "SENSOR_DRIVER_MSGS"))
    (make-package "SENSOR_DRIVER_MSGS"))
  (shadow 'History_traj (find-package "SENSOR_DRIVER_MSGS")))
(unless (find-package "SENSOR_DRIVER_MSGS::HISTORY_TRAJ")
  (make-package "SENSOR_DRIVER_MSGS::HISTORY_TRAJ"))

(in-package "ROS")
;;//! \htmlinclude History_traj.msg.html


(defclass sensor_driver_msgs::History_traj
  :super ros::object
  :slots (_time_stamp _line_num _line_point _center_point ))

(defmethod sensor_driver_msgs::History_traj
  (:init
   (&key
    ((:time_stamp __time_stamp) 0.0)
    ((:line_num __line_num) 0)
    ((:line_point __line_point) (let (r) (dotimes (i 0) (push (instance sensor_driver_msgs::Points :init) r)) r))
    ((:center_point __center_point) (instance sensor_driver_msgs::Points :init))
    )
   (send-super :init)
   (setq _time_stamp (float __time_stamp))
   (setq _line_num (round __line_num))
   (setq _line_point __line_point)
   (setq _center_point __center_point)
   self)
  (:time_stamp
   (&optional __time_stamp)
   (if __time_stamp (setq _time_stamp __time_stamp)) _time_stamp)
  (:line_num
   (&optional __line_num)
   (if __line_num (setq _line_num __line_num)) _line_num)
  (:line_point
   (&rest __line_point)
   (if (keywordp (car __line_point))
       (send* _line_point __line_point)
     (progn
       (if __line_point (setq _line_point (car __line_point)))
       _line_point)))
  (:center_point
   (&rest __center_point)
   (if (keywordp (car __center_point))
       (send* _center_point __center_point)
     (progn
       (if __center_point (setq _center_point (car __center_point)))
       _center_point)))
  (:serialization-length
   ()
   (+
    ;; float64 _time_stamp
    8
    ;; int32 _line_num
    4
    ;; sensor_driver_msgs/Points[] _line_point
    (apply #'+ (send-all _line_point :serialization-length)) 4
    ;; sensor_driver_msgs/Points _center_point
    (send _center_point :serialization-length)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; float64 _time_stamp
       (sys::poke _time_stamp (send s :buffer) (send s :count) :double) (incf (stream-count s) 8)
     ;; int32 _line_num
       (write-long _line_num s)
     ;; sensor_driver_msgs/Points[] _line_point
     (write-long (length _line_point) s)
     (dolist (elem _line_point)
       (send elem :serialize s)
       )
     ;; sensor_driver_msgs/Points _center_point
       (send _center_point :serialize s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; float64 _time_stamp
     (setq _time_stamp (sys::peek buf ptr- :double)) (incf ptr- 8)
   ;; int32 _line_num
     (setq _line_num (sys::peek buf ptr- :integer)) (incf ptr- 4)
   ;; sensor_driver_msgs/Points[] _line_point
   (let (n)
     (setq n (sys::peek buf ptr- :integer)) (incf ptr- 4)
     (setq _line_point (let (r) (dotimes (i n) (push (instance sensor_driver_msgs::Points :init) r)) r))
     (dolist (elem- _line_point)
     (send elem- :deserialize buf ptr-) (incf ptr- (send elem- :serialization-length))
     ))
   ;; sensor_driver_msgs/Points _center_point
     (send _center_point :deserialize buf ptr-) (incf ptr- (send _center_point :serialization-length))
   ;;
   self)
  )

(setf (get sensor_driver_msgs::History_traj :md5sum-) "75726c9405694c920b00351cd5eee0e3")
(setf (get sensor_driver_msgs::History_traj :datatype-) "sensor_driver_msgs/History_traj")
(setf (get sensor_driver_msgs::History_traj :definition-)
      "    float64 time_stamp
    int32 line_num
    Points[] line_point
    Points center_point

================================================================================
MSG: sensor_driver_msgs/Points
float64 x
float64 y
float64 z

")



(provide :sensor_driver_msgs/History_traj "75726c9405694c920b00351cd5eee0e3")


