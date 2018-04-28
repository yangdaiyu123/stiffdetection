
(cl:in-package :asdf)

(defsystem "iv_slam_ros_msgs-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :geometry_msgs-msg
               :std_msgs-msg
)
  :components ((:file "_package")
    (:file "TraversibleArea" :depends-on ("_package_TraversibleArea"))
    (:file "_package_TraversibleArea" :depends-on ("_package"))
  ))