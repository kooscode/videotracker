
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
#include "../libterraclear/src/error_base.hpp"
#include "../libterraclear/src/appsettings.hpp"
#include "../libterraclear/src/camera_usb.hpp"
#include "../libterraclear/src/basicserial.hpp"

using namespace std;

#define MAX_SPEED 255
#define MIN_SPEED 125

struct wheelspeed
{
    int left_speed = 0;
    int right_speed = 0;
};

int transform_value(int value, int min_value, int max_value, int min_transform,  int max_transform)
{
    int value_scale = max_value - min_value;
    int transform_scale = max_transform - min_transform;
    
    int retval = (((float)(value - min_value) / value_scale) * transform_scale) + min_transform;
    
    return retval;
}

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


int main(int argc, char** argv) 
{
    
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

    
    //straight line tracking
    bbox_t bbox_start1;
    bbox_start1.x = 100;//ref_w / 2 - 50;
    bbox_start1.y = 50;//ref_h / 2 - 50;
    bbox_start1.w = 100;
    bbox_start1.h = 100;
    bbox_start1.track_id = 1;
    cv::Point track_start1(bbox_start1.x + bbox_start1.w / 2, bbox_start1.y + bbox_start1.h / 2);

    uint32_t framecnt = 0;
    //Detection & tracking Loop..
    for ( ;; )
    {
        
        // stop the program if no more images
        if((frame.rows > 0) && (frame.cols > 0))
        {
            //motor wheel speed..
            wheelspeed ws;

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

            //
            if (tracker_objects.size() > 0)
            {
                for (vector<bbox_t> ::iterator it = tracker_objects.begin(); it != tracker_objects.end(); it++)
                {        
                    bbox_t bbox = *it;

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
                    
                    if (!target_rect.contains(track_end))
                    {
                        
                       // set wheel_speed
                        ws.left_speed = ws.right_speed = transform_value(ref_size.height - track_end.y, 0, ref_size.height, MIN_SPEED, MAX_SPEED);

                        //correct for cross track error.
                        int turn_correct = target_center.x - track_end.x;
                        if (turn_correct < 0)
                        {
                            turn_correct = transform_value(abs(turn_correct), 0, ref_size.width, MIN_SPEED, MAX_SPEED) - MIN_SPEED;
                            ws.left_speed += turn_correct;
                            ws.right_speed -=  turn_correct;
                            ws.right_speed = (ws.right_speed < MIN_SPEED) ? -MIN_SPEED : ws.right_speed;
                        }
                        else if (turn_correct > 0)
                        {
                            turn_correct = transform_value(abs(turn_correct), 0, ref_size.width, MIN_SPEED, MAX_SPEED) - MIN_SPEED;
                            ws.right_speed += turn_correct;
                            ws.left_speed -= turn_correct;
                            ws.left_speed = (ws.left_speed < MIN_SPEED) ? -MIN_SPEED : ws.left_speed;
                        }

                        //correct for speed between 0 and MIN_SPEED
                        ws.left_speed = ((ws.left_speed > 0) && (ws.left_speed < MIN_SPEED)) ? MIN_SPEED : ((ws.left_speed < 0) && (ws.left_speed > -MIN_SPEED)) ? -MIN_SPEED : ws.left_speed;
                        ws.right_speed =  ((ws.right_speed > 0) && (ws.right_speed < MIN_SPEED)) ? MIN_SPEED : ((ws.right_speed < 0) && (ws.right_speed > -MIN_SPEED)) ? -MIN_SPEED : ws.right_speed;

                        //constrain within bounds.
                        ws.left_speed = (ws.left_speed < -MAX_SPEED) ? -MAX_SPEED : (ws.left_speed > MAX_SPEED) ? MAX_SPEED : ws.left_speed;
                        ws.right_speed = (ws.right_speed < -MAX_SPEED) ? -MAX_SPEED : (ws.right_speed > MAX_SPEED) ? MAX_SPEED : ws.right_speed;

                        //apply gains
                        ws.left_speed *= speed_gain;
                        ws.right_speed *= speed_gain;

                        
                    }

                }             
            }
            else
            {
                tracker_objects.push_back(bbox_start1);            
            }

            // show image with the tracked object
            cv::imshow(window_name,frame);
            
            //get next frame and resize
            usbcam.update_frames();
            frame = usbcam.getRGBFrame();
            
            //MOTOR SPEED CONTROL..
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
                cout << serial_strstrm.str() << std::endl;
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
        
    }
    
    serial_port.close();
    
    return 0;
}

