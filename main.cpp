/*
 * App for detecting objects and steering vehicle towards Object
 * 
 * Copyright (C) 2017 Koos du Preez
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * CREATED BY: Koos du Preez - kdupreez@hotmail.com
 * 
*/

#include <cstdlib>
#include <iostream>

//OPENCV
 #include <opencv2/core/utility.hpp>
 #include <opencv2/tracking.hpp>
 #include <opencv2/videoio.hpp>
 #include <opencv2/highgui.hpp>

//YOLO V3 INCLUDES - https://github.com/AlexeyAB/darknet
//compiled for SO/Lib and OPENCV using GPU(Cuda/CuDNN) and GPU Tracking.
#define OPENCV
#define TRACK_OPTFLOW
#define GPU
#include <darknet/src/yolo_v2_class.hpp>

//Requires TerraClear Lib - static includes
//get it from https://github.com/TerraClear/libterraclear
#include "../libterraclear/src/error_base.hpp"
#include "../libterraclear/src/appsettings.hpp"
#include "../libterraclear/src/camera_usb.hpp"
#include "../libterraclear/src/basicserial.hpp"
#include "../libterraclear/src/stopwatch.hpp"
#include "basicpid.hpp"

using namespace std;

#define MAX_SPEED 255
#define MIN_SPEED 80

struct wheelspeed
{
    int left_speed = 0;
    int right_speed = 0;
};

int boxoverlapcenter(bbox_t srcbox, vector<bbox_t> boxes_all)
{
    int overlap_index = -1;

    for (uint32_t i = 0; i < boxes_all.size(); i++)
    {
        cv::Rect2d tmprect(srcbox.x, srcbox.y, srcbox.w, srcbox.h);
        
        unsigned int centerx = boxes_all[i].x + boxes_all[i].w / 2 ;
        unsigned int centery = boxes_all[i].y + boxes_all[i].h / 2 ;

        if (tmprect.contains(cv::Point(centerx, centery)))
        {
            overlap_index = i;
            break;
        }

    }
    
    return overlap_index;
}

// *** BASIC PID
terraclear::stopwatch sw;

double lastTime;
double errSum = 0, lastErr = 0;
double kp, ki, kd;

double PID_Compute(double Input, double Setpoint)
{
    double retval = 0;

    ///How long since we last calculated
   double now = (double) sw.get_elapsed_s();
  std::cout << "now:" << now << std::endl;
   double timeChange = now - lastTime;

   //P
   double error = Setpoint - Input;
  std::cout << "pErr:" << error << std::endl;
   
//TODO - Fix integral and derivative to calculate amount 
//of integrations based on how many  counts of intervals has passed... 
   //I
   errSum += (error * timeChange);
  std::cout << "iErr:" << errSum << std::endl;
   
   //D
   double dErr = (error - lastErr) * timeChange * 10;
   std::cout << "dErr:" << dErr << std::endl;
//   dErr = max<double>(dErr, 0.00f);
   
   //CAlc PID Output
   retval = kp * error + ki * errSum + kd * dErr;
  
   //Remember values
   lastErr = error;
   lastTime = now;
   
   return retval;
}
  
void PID_Tune(double Kp, double Ki, double Kd)
{
   kp = Kp;
   ki = Ki;
   kd = Kd;
}


//  ** END BASIC PID

int main(int argc, char** argv) 
{
    terraclear::basicpid pid_fwd(0,0,0,0);
    
    //start stopwatch
    sw.reset();
    bool _started = false;
    
    //SETUP SERIAL COMMS
#ifdef __linux__
    string serial_path = "/dev/ttyUSB0";
#else
    string serial_path = "/dev/tty.usbserial-A5047JL0";
#endif

    //setup serial port..
    terraclear::basicserial serial_port;
        
    bool usbok = false;
    try
    {
        //try to open serial port.
        serial_port.open(serial_path, terraclear::Baud::BAUD_115200);
        usbok = true;
        
    } 
    catch (terraclear::error_base err)
    {
        std::cout << "Error Opening Serial Port.. " << endl;
    }

    
    //init settings.
    terraclear::appsettings _settings("videotracker.json");
    
    //Set Desired Image Resolution/. 
    //1080
//    uint32_t ref_w = 1920;
//    uint32_t ref_h = 1080;

//720
    uint32_t ref_w = 1280;
    uint32_t ref_h = 960;
    
    uint32_t cam_w = ref_w;
    uint32_t cam_h = ref_h;

    //set image size..
    cv::Size ref_size = cv::Size(ref_w, ref_h);
    

    // show help
    if(argc<2){
      cout<<
        " Usage: videotracker <camera index | video_name>\n"
        " examples:\n"
        " videotracker videofile.mp4\n"
        " videotracker 0\n"
        << endl;
      return 0;
    }
    

    //Neural Network Model and Weights
    string net_config = _settings.getvalue_string("network-config");
    string net_weights = _settings.getvalue_string("network-weights");
    float net_threshold  =  _settings.getvalue_float("network-threshold");
    float net_confidence =  _settings.getvalue_float("network-confidence");
    uint32_t net_framerate = _settings.getvalue_float("network-framerate");
    float speed_gain = _settings.getvalue_float("speed-gain");
    
    //apply visual tracking
    bool do_track = _settings.getvalue_float("do-track",false);
    bool pid_enabled = false;
    int missed_frames = 0;

                        
    //Create new YoloV2 detector
    Detector* yolov2 = new Detector(net_config, net_weights);
     
    //** OPEN CV CUDA TRACKING Objects.
    Tracker_optflow tracker_engine;
    vector <bbox_t> tracker_objects;
    
    //Tracked Object BBox Color
    cv::Scalar color_tracked(0xff, 0x10, 0x10 );

    //Track Line Color..
    cv::Scalar color_track_line( 0x00, 0xff, 0x00 );
    
    //Create OpenCV Window
    char window_name[] = "Tracker";
    cv::namedWindow(window_name, cv::WINDOW_NORMAL | cv::WINDOW_FREERATIO);// | WINDOW_AUTOSIZE);
    
    //open video camera
    terraclear::camera_usb usbcam(stoi(argv[1]));
    
    //frame capture first Image
    usbcam.update_frames();
    cv::Mat frame = usbcam.getRGBFrame();
    
    //calculate target area square
    uint32_t target_area = _settings.getvalue_uint("target-area");
    cv::Point target_tl(ref_w / 2 - target_area / 2,  (ref_h - target_area) - 5);
    cv::Point target_br(target_tl.x + target_area, target_tl.y + target_area);
    cv::Point target_center(target_tl.x + target_area / 2, target_tl.y + target_area / 2);

    uint32_t framecnt = 0;

    //motor wheel speed..
    wheelspeed ws;

    //Detection & tracking Loop..
    for ( ;; )
    {
        
        // stop the program if no more images
        if((frame.rows > 0) && (frame.cols > 0))
        {
            //Scale image..
            cv::resize(frame, frame, ref_size);

            //track objects.
            tracker_objects = tracker_engine.tracking_flow(frame);

            //Only do detection every X frames..
            if (framecnt > 0)
            {
                framecnt--;
            }
            else
            {
                framecnt = net_framerate;
                
                //detect objects & update tracking data..
                vector<bbox_t> bboxes_detected_yolo = yolov2->detect(frame, net_threshold);  
                for (auto bbox : bboxes_detected_yolo)
                {
                    if (bbox.prob >= net_confidence)
                    {

                            //Add & update tracked objects
                            int overlap_index = boxoverlapcenter(bbox, tracker_objects);
                            if (overlap_index < 0) 
                            {                    
                               tracker_objects.push_back(bbox);
                            }
                            else
                            {
                               tracker_objects.at(overlap_index) = bbox;
                            }
                    }
                }

                //update tracker objects post detection..
                tracker_engine.update_cur_bbox_vec(tracker_objects);

            }


            //draw target area..
            cv::rectangle(frame, target_tl, target_br, cv::Scalar(0xff, 0x00, 0x00), 4, 8, 0 );            

            if (tracker_objects.size() > 0)
            {
                //ensure stopwatch is running..
                sw.start();

                bbox_t bbox = tracker_objects.at(0);

                cv::Point obj_box_tl = cv::Point(bbox.x, bbox.y);
                cv::Point obj_box_br = cv::Point(bbox.x + bbox.w, bbox.y + bbox.h);
                cv::rectangle(frame, obj_box_tl, obj_box_br, cv::Scalar(0x00, 0x00, 0xff), 4, 8, 0 );      

                //draw Confidence string
                std::stringstream strstrm;
                strstrm << "[" <<  std::fixed << std::setprecision(0) << bbox.prob * 100 << "%]";
                cv::putText(frame, strstrm.str(), obj_box_tl, cv::FONT_HERSHEY_PLAIN, 2,  cv::Scalar(0x00, 0xff, 0x00), 2);   

                //Line
                cv::Point track_end(bbox.x + bbox.w / 2, bbox.y + bbox.h / 2);
                cv::line(frame, target_center, track_end, color_track_line, 4, 8, 0 );       

                //correct speed & steering until target aquired
                cv::Rect2d target_rect(target_tl.x, target_tl.y, target_area, target_area);


                int err_val = target_center.y - track_end.y;

                //make sure PID is enabled AND error is larger than error margin pixels..
                if ((pid_enabled) && (abs(err_val) > 40))
                {
//PID TUNE
                    //apply PID to forward motion first
                    PID_Tune(0.2, 0.1, 0.75);

                   std::cout << "TARGET: " << track_end.y << std::endl;
                   std::cout << "END: " << target_center.y << std::endl;
                   std::cout << "ERR: " << err_val << std::endl;

                    if (!_started)
                    {
                        sw.reset();
                        _started = true;
                    }

                    double pid_value = PID_Compute(track_end.y, target_center.y);

                   std::cout << "pid_value: " << pid_value << std::endl;

                    int tmp_speed = pid_fwd.domain_transform(abs(pid_value), 0.00f, ref_size.height, MIN_SPEED, MAX_SPEED);

                    ws.left_speed = ws.right_speed = (pid_value > 0) ? tmp_speed: -tmp_speed;

                    std::cout << "speed: " <<  ws.left_speed << std::endl;
                    std::cout << "******\n";

                }
                else 
                {
                    if (abs(err_val) > 40)
                    {
                        sw.stop();
                        sw.reset();
                        _started = false; 
                        
                        errSum = lastErr = lastTime = 0;
                    }
                    
                    ws.left_speed = ws.right_speed = 0;
                }

                missed_frames = 0;

            }
            else
            {
                //time is not running while nothing detected.. for more than 10 frames
                if (missed_frames > 10)
                {
                    sw.stop();
                    sw.reset();
                    _started = false;
                    ws.left_speed = ws.right_speed = 0;
                    errSum = lastErr = lastTime = 0;

                }

                missed_frames++;
            }
 

            // show image with the tracked object
            cv::imshow(window_name,frame);
            
            //get next frame and resize
            usbcam.update_frames();
            frame = usbcam.getRGBFrame();
            
            //Continues Serial Port Motor Control System
            if (usbok && serial_port.isopen)
            {
                stringstream serial_strstrm;
                
                if (ws.left_speed > 0)
                    serial_strstrm << "SLF" << abs(ws.left_speed) << std::endl;
                else
                    serial_strstrm << "SLR" << abs(ws.left_speed) <<  std::endl;
                    
                if (ws.right_speed > 0)
                    serial_strstrm << "SRF" << abs(ws.right_speed) << std::endl;
                else
                    serial_strstrm << "SRR" << abs(ws.right_speed) << std::endl;
                       
                serial_port.writeString(serial_strstrm.str(), 250);
            }
            
        }
        else
        {
            //No more images from vide feed - Exit
            break;
        }
        
        //wait for Key and quit on ESC button
        int x = cv::waitKey(33);
        if(x == 27) //ESC = 27
        {
            break;       
        }
        else if (x == 112) //p = PID toggle;
        {
            pid_enabled = !pid_enabled;
            std::cout << "PID: " << pid_enabled << std::endl;
            
            sw.stop();
            sw.reset();
            _started = false;
            ws.left_speed = ws.right_speed = 0;
             errSum = lastErr = lastTime = 0;

            
        }
        else if (x > 0)
        {
            std::cout << "key = " << x << std::endl;
        }
        
    }
    
    serial_port.close();
    
    return 0;
}

