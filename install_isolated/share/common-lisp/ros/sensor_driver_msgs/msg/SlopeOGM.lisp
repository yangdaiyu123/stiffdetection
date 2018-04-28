; Auto-generated. Do not edit!


(cl:in-package sensor_driver_msgs-msg)


;//! \htmlinclude SlopeOGM.msg.html

(cl:defclass <SlopeOGM> (roslisp-msg-protocol:ros-message)
  ((header
    :reader header
    :initarg :header
    :type std_msgs-msg:Header
    :initform (cl:make-instance 'std_msgs-msg:Header))
   (ogmwidth
    :reader ogmwidth
    :initarg :ogmwidth
    :type cl:fixnum
    :initform 0)
   (ogmheight
    :reader ogmheight
    :initarg :ogmheight
    :type cl:fixnum
    :initform 0)
   (ogmresolution
    :reader ogmresolution
    :initarg :ogmresolution
    :type cl:float
    :initform 0.0)
   (vehicle_x
    :reader vehicle_x
    :initarg :vehicle_x
    :type cl:fixnum
    :initform 0)
   (vehicle_y
    :reader vehicle_y
    :initarg :vehicle_y
    :type cl:fixnum
    :initform 0)
   (slopeval
    :reader slopeval
    :initarg :slopeval
    :type (cl:vector cl:float)
   :initform (cl:make-array 0 :element-type 'cl:float :initial-element 0.0)))
)

(cl:defclass SlopeOGM (<SlopeOGM>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <SlopeOGM>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'SlopeOGM)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name sensor_driver_msgs-msg:<SlopeOGM> is deprecated: use sensor_driver_msgs-msg:SlopeOGM instead.")))

(cl:ensure-generic-function 'header-val :lambda-list '(m))
(cl:defmethod header-val ((m <SlopeOGM>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader sensor_driver_msgs-msg:header-val is deprecated.  Use sensor_driver_msgs-msg:header instead.")
  (header m))

(cl:ensure-generic-function 'ogmwidth-val :lambda-list '(m))
(cl:defmethod ogmwidth-val ((m <SlopeOGM>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader sensor_driver_msgs-msg:ogmwidth-val is deprecated.  Use sensor_driver_msgs-msg:ogmwidth instead.")
  (ogmwidth m))

(cl:ensure-generic-function 'ogmheight-val :lambda-list '(m))
(cl:defmethod ogmheight-val ((m <SlopeOGM>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader sensor_driver_msgs-msg:ogmheight-val is deprecated.  Use sensor_driver_msgs-msg:ogmheight instead.")
  (ogmheight m))

(cl:ensure-generic-function 'ogmresolution-val :lambda-list '(m))
(cl:defmethod ogmresolution-val ((m <SlopeOGM>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader sensor_driver_msgs-msg:ogmresolution-val is deprecated.  Use sensor_driver_msgs-msg:ogmresolution instead.")
  (ogmresolution m))

(cl:ensure-generic-function 'vehicle_x-val :lambda-list '(m))
(cl:defmethod vehicle_x-val ((m <SlopeOGM>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader sensor_driver_msgs-msg:vehicle_x-val is deprecated.  Use sensor_driver_msgs-msg:vehicle_x instead.")
  (vehicle_x m))

(cl:ensure-generic-function 'vehicle_y-val :lambda-list '(m))
(cl:defmethod vehicle_y-val ((m <SlopeOGM>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader sensor_driver_msgs-msg:vehicle_y-val is deprecated.  Use sensor_driver_msgs-msg:vehicle_y instead.")
  (vehicle_y m))

(cl:ensure-generic-function 'slopeval-val :lambda-list '(m))
(cl:defmethod slopeval-val ((m <SlopeOGM>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader sensor_driver_msgs-msg:slopeval-val is deprecated.  Use sensor_driver_msgs-msg:slopeval instead.")
  (slopeval m))
(cl:defmethod roslisp-msg-protocol:serialize ((msg <SlopeOGM>) ostream)
  "Serializes a message object of type '<SlopeOGM>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'header) ostream)
  (cl:let* ((signed (cl:slot-value msg 'ogmwidth)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'ogmheight)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'ogmresolution))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let* ((signed (cl:slot-value msg 'vehicle_x)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let* ((signed (cl:slot-value msg 'vehicle_y)) (unsigned (cl:if (cl:< signed 0) (cl:+ signed 65536) signed)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) unsigned) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) unsigned) ostream)
    )
  (cl:let ((__ros_arr_len (cl:length (cl:slot-value msg 'slopeval))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) __ros_arr_len) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) __ros_arr_len) ostream))
  (cl:map cl:nil #'(cl:lambda (ele) (cl:let ((bits (roslisp-utils:encode-single-float-bits ele)))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream)))
   (cl:slot-value msg 'slopeval))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <SlopeOGM>) istream)
  "Deserializes a message object of type '<SlopeOGM>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'header) istream)
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'ogmwidth) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'ogmheight) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'ogmresolution) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'vehicle_x) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
    (cl:let ((unsigned 0))
      (cl:setf (cl:ldb (cl:byte 8 0) unsigned) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) unsigned) (cl:read-byte istream))
      (cl:setf (cl:slot-value msg 'vehicle_y) (cl:if (cl:< unsigned 32768) unsigned (cl:- unsigned 65536))))
  (cl:let ((__ros_arr_len 0))
    (cl:setf (cl:ldb (cl:byte 8 0) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 8) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 16) __ros_arr_len) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 24) __ros_arr_len) (cl:read-byte istream))
  (cl:setf (cl:slot-value msg 'slopeval) (cl:make-array __ros_arr_len))
  (cl:let ((vals (cl:slot-value msg 'slopeval)))
    (cl:dotimes (i __ros_arr_len)
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:aref vals i) (roslisp-utils:decode-single-float-bits bits))))))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<SlopeOGM>)))
  "Returns string type for a message object of type '<SlopeOGM>"
  "sensor_driver_msgs/SlopeOGM")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'SlopeOGM)))
  "Returns string type for a message object of type 'SlopeOGM"
  "sensor_driver_msgs/SlopeOGM")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<SlopeOGM>)))
  "Returns md5sum for a message object of type '<SlopeOGM>"
  "4b5a053c15a636703b341c1ab43de555")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'SlopeOGM)))
  "Returns md5sum for a message object of type 'SlopeOGM"
  "4b5a053c15a636703b341c1ab43de555")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<SlopeOGM>)))
  "Returns full string definition for message of type '<SlopeOGM>"
  (cl:format cl:nil "Header header~%int16 ogmwidth~%int16 ogmheight~%float32 ogmresolution~%int16 vehicle_x~%int16 vehicle_y~%float32[] slopeval~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%# 0: no frame~%# 1: global frame~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'SlopeOGM)))
  "Returns full string definition for message of type 'SlopeOGM"
  (cl:format cl:nil "Header header~%int16 ogmwidth~%int16 ogmheight~%float32 ogmresolution~%int16 vehicle_x~%int16 vehicle_y~%float32[] slopeval~%~%================================================================================~%MSG: std_msgs/Header~%# Standard metadata for higher-level stamped data types.~%# This is generally used to communicate timestamped data ~%# in a particular coordinate frame.~%# ~%# sequence ID: consecutively increasing ID ~%uint32 seq~%#Two-integer timestamp that is expressed as:~%# * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')~%# * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')~%# time-handling sugar is provided by the client library~%time stamp~%#Frame this data is associated with~%# 0: no frame~%# 1: global frame~%string frame_id~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <SlopeOGM>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'header))
     2
     2
     4
     2
     2
     4 (cl:reduce #'cl:+ (cl:slot-value msg 'slopeval) :key #'(cl:lambda (ele) (cl:declare (cl:ignorable ele)) (cl:+ 4)))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <SlopeOGM>))
  "Converts a ROS message object to a list"
  (cl:list 'SlopeOGM
    (cl:cons ':header (header msg))
    (cl:cons ':ogmwidth (ogmwidth msg))
    (cl:cons ':ogmheight (ogmheight msg))
    (cl:cons ':ogmresolution (ogmresolution msg))
    (cl:cons ':vehicle_x (vehicle_x msg))
    (cl:cons ':vehicle_y (vehicle_y msg))
    (cl:cons ':slopeval (slopeval msg))
))
