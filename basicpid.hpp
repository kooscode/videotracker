#ifndef BASICPID_HPP
#define BASICPID_HPP

#include <cstdlib>

#include "../libterraclear/src/stopwatch.hpp"

namespace terraclear
{
    class basicpid 
    {
        public:
            basicpid(double Kp, double Ki, double Kd, uint32_t interval_ms);
            virtual ~basicpid();
            
            //tranfsorm between domains i.e. from pixel space to rpm space, etc.
            double domain_transform(double from_value, double from_min, double from_max, double to_min,  double to_max);
             
            void start();
            void stop();
            double compute(double Input, double Setpoint);

        private:
            stopwatch sw;

            double lastTime;
            double errSum = 0, lastErr = 0;

            double kp, ki, kd;  

    };
}

#endif /* BASICPID_HPP */

