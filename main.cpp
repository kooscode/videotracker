
#include <cstdlib>

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

// END YOLO V3

using namespace std;

int main(int argc, char** argv) 
{
    //Set Desired Image Resolution

    //1080
//    uint32_t ref_w = 1920;
//    uint32_t ref_h = 1080;

    //720
    uint32_t ref_w = 1280;
    uint32_t ref_h = 720;
    
    uint32_t cam_w = ref_w;
    uint32_t cam_h = ref_h;
    

    // show help
    if(argc<2){
      cout<<
        " Usage: videotracker <camera index | video_name>\n"
        " examples:\n"
        " videotracker videofile.mp4\n"
        << endl;
      return 0;
    }
    
    //split screen for stereo cam
    bool stereocam = false;
    if (argc > 2)
    {
        std::string stereostr = argv[2];
        if (stereostr == "s")
        {
            stereocam = true;
            cam_w = ref_w * 2;
        }
    }
    
    
    
    //Neural Network Model and Weights
    string net_config = "/home/koos/ml/prod/cfg/net-yolov2-voc.cfg";
    string net_weights = "/home/koos/ml/prod/weights/net-yolov2-voc-train_promixal-testbed.weights"; 

    //Create new YoloV2 detector
    // Detector* yolov2 = new Detector(net_config, net_weights);
     
    //** OPEN CV CUDA TRACKING Objects.
    Tracker_optflow tracker_engine;
    vector <bbox_t> tracker_objects;
    
    //Tracked Object BBox Color
    cv::Scalar color_tracked(0xff, 0x10, 0x10 );

    //Track Line Color..
    cv::Scalar color_track_line( 0x00, 0xff, 0x00 );
    
    //Create OpenCV Window
    char window_name[] = "YoloBox";
    cv::namedWindow(window_name, cv::WINDOW_NORMAL | cv::WINDOW_FREERATIO);// | WINDOW_AUTOSIZE);
    
    //frame capture Image
    cv::Mat frame;
    
    //Set input video
    std::string video = argv[1];
    cv::VideoCapture cap;
    
    //set video size..
    cv::Size ref_size = cv::Size(ref_w, ref_h);


    //Open Camera if digits 0,1,2,3 else open file
    if (video == "0" || video == "1" || video == "2" || video == "3")
    {
        if(!cap.open(stoi(video)))
        {
            cout << "Could not open video feed." << endl;
           return 0;
        }
        
        //Set Camera Resolution to specific camera resolution
        cap.set(3,cam_w);
        cap.set(4,cam_h);
    }
    else
    {
      if(!cap.open(video))
      {
          cout << "Could not open video feed." << endl;
         return 0;
      }  
    }
    
    
    // Stream 1st frame from the video / Camera
    cap >> frame;
    
    //grab only left half of stero image..
    if (stereocam)
       frame = frame(cv::Rect(0, 0, frame.cols/2, frame.rows));
    
    //Output some info regarding input video 
    cout << "Input:" << frame.cols << "X" << frame.rows << std::endl;

    //target resize and output info on resized resolution..
    cv::resize(frame, frame, ref_size);
    cout << "Scaled:" << frame.cols << "X" << frame.rows << std::endl;
    
    
    // perform the tracking process
    printf("Start the tracking process, press ESC to quit.\n");

    //tracking box location..
    bbox_t bbox_start1;
    bbox_start1.x = 100;//ref_w / 2 - 50;
    bbox_start1.y = 50;//ref_h / 2 - 50;
    bbox_start1.w = 100;
    bbox_start1.h = 100;
    bbox_start1.track_id = 1;

    bbox_t bbox_start2;
    bbox_start2.x = ref_w - 250;
    bbox_start2.y = 50;//ref_h / 2 - 50;
    bbox_start2.w = 100;
    bbox_start2.h = 100;
    bbox_start2.track_id = 2;
    
    //original point1
    cv::Point track_start1(bbox_start1.x + bbox_start1.w / 2, bbox_start1.y + bbox_start1.h / 2);

    //original point2
    cv::Point track_start2(bbox_start2.x + bbox_start2.w / 2, bbox_start2.y + bbox_start2.h / 2);
    
    //add box to tracker..
    
    //Tracking Loop..
    for ( ;; )
    {
        //draw tracked boxes..
        
        if (tracker_objects.size() > 0)
        {
            for (vector<bbox_t> ::iterator it = tracker_objects.begin(); it != tracker_objects.end(); it++)
            {        
                bbox_t bbox = *it;

                //draw detected obj..                    
                cv::Point obj_box_tl = cv::Point(bbox.x, bbox.y);
                cv::Point obj_box_br = cv::Point(bbox.x + bbox.w, bbox.y + bbox.h);
                cv::rectangle(frame, obj_box_tl, obj_box_br, color_tracked, 4, 8, 0 );

                //Line
                cv::Point track_end(bbox.x + bbox.w / 2, bbox.y + bbox.h / 2);
                if (bbox.track_id == 1)
                {
                    cv::line(frame, track_start1, track_end, color_track_line, 4, 8, 0 );                
                }
                else
                {
                    cv::line(frame, track_start2, track_end, color_track_line, 4, 8, 0 );               
                }

            }             
        }
        else
        {
            tracker_objects.push_back(bbox_start1);            
            tracker_objects.push_back(bbox_start2);
        }
       

        // stop the program if no more images
        if((frame.rows > 0) && (frame.cols > 0))
        {
            
            // show image with the tracked object
            cv::imshow(window_name,frame);

            
            //get next frame and resize
            cap >> frame;

            if (stereocam)
                frame = frame(cv::Rect(0, 0, frame.cols/2, frame.rows));

            
             if((frame.rows > 0) && (frame.cols > 0))
             {
                 //resize
                cv::resize(frame, frame, ref_size);

                //update tracker objects..
                tracker_engine.update_cur_bbox_vec(tracker_objects);

                //update tracking position..
               tracker_objects = tracker_engine.tracking_flow(frame);
             }
            
        }
        else
        {
            //No more images from vide feed - Exit!
            break;
        }
        
        //wait for Key and quit on ESC button
        int x = cv::waitKey(10);
        if(x == 27) //ESC = 27
        {
            break;       
        }
        
    }
    
    return 0;
}

