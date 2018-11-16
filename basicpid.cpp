#include "basicpid.hpp"

namespace terraclear
{
    basicpid::basicpid(double Kp, double Ki, double Kd, uint32_t interval_ms)
    {

    }

    basicpid::~basicpid() 
    {
    
    }
    
    double basicpid::domain_transform(double from_value, double from_min, double from_max, double to_min,  double to_max)
    {
        //clip input value to max/min
        double clip_value = std::min(std::max(from_min, from_value), from_max);;

        //normalize input and output values.
        double from_scale = from_max - from_min;
        double to_scale = to_max - to_min;
        double from_rebased =  (clip_value <= from_min) ? clip_value : clip_value - from_min;

        double retval = ((from_rebased / from_scale) * to_scale) + to_min;

        return retval;
    }

    void basicpid::start()
    {
        
    }
    
    void basicpid::stop()
    {
        
    }
    
    double basicpid::compute(double Input, double Setpoint)
    {
        return 0;
    }
}

